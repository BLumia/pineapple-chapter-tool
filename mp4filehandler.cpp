#include "mp4filehandler.h"

#include <QDebug>

#ifndef NO_LIBMP4V2
#include <mp4v2/mp4v2.h>
#endif // NO_LIBMP4V2

Mp4FileHandler::Mp4FileHandler()
{

}

Mp4FileHandler::~Mp4FileHandler()
{

}

FileHandlerInterface::Status Mp4FileHandler::setFile(const QString filePath)
{
    m_file = filePath;

    return SUCCESS;
}

ChapterItem *Mp4FileHandler::createChapterTree() const
{
    if (m_file.isEmpty()) return nullptr;

#ifdef NO_LIBMP4V2
    return nullptr;
#else

    MP4FileHandle hM4a = MP4Read(m_file.toStdString().c_str());
    if (hM4a == MP4_INVALID_FILE_HANDLE ) {
        return nullptr;
    }

    MP4Chapter_t * chapters = 0;
    uint32_t chapterCount = 0;
    MP4ChapterType chapterType = MP4GetChapters(hM4a, &chapters, &chapterCount, MP4ChapterTypeAny);
    if (chapterCount == 0) {
        return nullptr;
    }

    qDebug() << chapterType;

    ChapterItem * tocItem = new ChapterItem("pseudoTOC");
    tocItem->setColumnCount(3);

    MP4Duration durationMs = 0; // yeah, it's in msec since m4a chapter marker tracks uses scale 1000.
    for (uint32_t i = 0; i < chapterCount; ++i) {
        ChapterItem * chapterItem = new ChapterItem(QString::number(durationMs));
        chapterItem->setColumnCount(3);
        chapterItem->setItemProperty(ChapterTitle, QString(chapters[i].title));
        chapterItem->setItemProperty(ChapterStartTimeMs, static_cast<int>(durationMs));
        tocItem->appendRow(chapterItem);
//        qDebug() << "start:" << durationMs << chapters[i].title;
        durationMs += chapters[i].duration;
    }

    MP4Free(chapters);
    MP4Close(hM4a);

    return tocItem;

#endif // NO_LIBMP4V2
}

FileHandlerInterface::ChapterFeatures Mp4FileHandler::chapterFeatures() const
{
    return ChapterFeatures(StartTimeMs | Title);
}

FileHandlerInterface::Status Mp4FileHandler::importFromFile()
{
    return SUCCESS;
}

#ifndef NO_LIBMP4V2
MP4TrackId getFirstAudioTrack(MP4FileHandle file, bool & out_isVideoTrack)
{
    uint32_t trackCnt = MP4GetNumberOfTracks(file);
    if (trackCnt == 0) return MP4_INVALID_TRACK_ID;

    out_isVideoTrack = false;

    MP4TrackId firstTrackId = MP4_INVALID_TRACK_ID;
    for (uint32_t i = 0; i < trackCnt; i++) {
        MP4TrackId id = MP4FindTrackId(file, i);
        const char * type = MP4GetTrackType(file, id);
        if (MP4_IS_VIDEO_TRACK_TYPE(type)) {
            firstTrackId = id;
            out_isVideoTrack = true;
            break;
        } else if (MP4_IS_AUDIO_TRACK_TYPE(type)) {
            firstTrackId = id;
            out_isVideoTrack = false;
            break;
        }
    }

    return firstTrackId;
}
#endif // NO_LIBMP4V2

FileHandlerInterface::Status Mp4FileHandler::writeToFile(ChapterItem *chapterRoot)
{
#ifdef NO_LIBMP4V2
    Q_UNUSED(chapterRoot);
    return EXPORT_NOT_SUPPORTED;
#else // NO_LIBMP4V2
    MP4FileHandle hM4a = MP4Modify(m_file.toStdString().c_str());
    if (hM4a == MP4_INVALID_FILE_HANDLE) {
        return FILE_STAT_ERROR;
    }

    std::vector<MP4Chapter_t> chapters;

    // before we start, let's remove all existed chapters...
    MP4DeleteChapters(hM4a, MP4ChapterTypeAny);

    ChapterItem * item = chapterRoot;
    if (item && item->hasChildren()) {
        // get ready to write our new chapter list.
        ChapterItem * currentItem = static_cast<ChapterItem *>(item->child(0));
        QStandardItemModel * currentModel = currentItem->model();
        while (true) {
            MP4Chapter_t chap;

            std::string chapterTitle(currentItem->data(ChapterTitle).toString().toStdString());
            size_t titleLen = qMin(chapterTitle.length(), (size_t)MP4V2_CHAPTER_TITLE_MAX);
            strncpy(chap.title, chapterTitle.c_str(), titleLen);
            chap.title[titleLen] = 0;
            chap.duration = currentItem->data(ChapterStartTimeMs).toUInt();

            chapters.push_back(chap);

            QModelIndex nextItemModel = currentModel->indexFromItem(currentItem).siblingAtRow(currentItem->row() + 1);
            if (nextItemModel.isValid()) {
                QStandardItem * nextItem = currentModel->itemFromIndex(nextItemModel);
                currentItem = static_cast<ChapterItem *>(nextItem);
            } else {
                break;
            }
        }
    }

    bool isVideoTrack = false;
    MP4TrackId firstTrackId = getFirstAudioTrack(hM4a, isVideoTrack);
    if (!MP4_IS_VALID_TRACK_ID(firstTrackId)) {
        return FILE_STAT_ERROR;
    }

    MP4Duration durationTicks = MP4GetTrackDuration(hM4a, firstTrackId);
    uint32_t tickPerSec = MP4GetTrackTimeScale(hM4a, firstTrackId);
    uint32_t durationMs = durationTicks * 1.0 / tickPerSec * 1000;

    // check out-of-duration chapter markers
    for(std::vector<MP4Chapter_t>::iterator it = chapters.begin(); it != chapters.end(); ) {
        if (durationMs <= it->duration) {
            it = chapters.erase(it);
        } else {
            it++;
        }
    }

    for(std::vector<MP4Chapter_t>::iterator it = chapters.begin(); it != chapters.end(); it++) {
        MP4Duration currDuration = (*it).duration;
        MP4Duration nextDuration =  chapters.end() == it + 1 ? durationMs : (*(it+1)).duration;

        (*it).duration = nextDuration - currDuration;
    }

    // finally, apply chapter markers
    MP4SetChapters(hM4a, &chapters[0], (uint32_t)chapters.size(), MP4ChapterTypeAny);
    MP4Close(hM4a);

    // This is optional.
    MP4Optimize(m_file.toStdString().c_str());

    return SUCCESS;
#endif // NO_LIBMP4V2
}
