#include "chaptertreemodel.h"

#include "mpegfilehandler.h"

#include <fileref.h>

#include <vorbisfile.h>

#include <opusfile.h>

#ifndef NO_LIBMP4V2
#include <mp4v2/mp4v2.h>
#endif // NO_LIBMP4V2

#include <QDebug>
#include <QMimeDatabase>
#include <QTime>

bool ChapterTreeModel::loadFromFile(const QString &pathToFile)
{
    clear();

    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(pathToFile);
    if (mimeType.inherits("audio/mpeg")) {
        loadFromMpegFile(pathToFile);
        return true;
    } else if (mimeType.inherits("audio/x-vorbis+ogg")) {
        loadFromVorbisFile(pathToFile);
        return true;
    } else if (mimeType.inherits("audio/x-opus+ogg")) {
        loadFromOpusFile(pathToFile);
        return true;
    } else if (mimeType.inherits("audio/mp4")) {
#ifdef NO_LIBMP4V2
        return false;
#else
        loadFromM4aFile(pathToFile);
        return true;
#endif // NO_LIBMP4V2
    }

    qDebug() << mimeType;

    return false;
}

// spec: https://id3.org/id3v2-chapters-1.0
void ChapterTreeModel::loadFromMpegFile(const QString &pathToFile)
{
    MpegFileHandler mfh;
    mfh.setFile(pathToFile);
    mfh.importFromFile();
    ChapterItem * created = mfh.createChapterTree();

    if (created) {
        appendRow(created);
    }
}

// spec: https://wiki.xiph.org/VorbisComment#Chapter_Extension
void ChapterTreeModel::loadFromVorbisFile(const QString &pathToFile)
{
    TagLib::Ogg::Vorbis::File file(pathToFile.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();

    loadFromXiphComment(tags);
}

// seems same as the vorbis one...
void ChapterTreeModel::loadFromOpusFile(const QString &pathToFile)
{
    TagLib::Ogg::Opus::File file(pathToFile.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();

    loadFromXiphComment(tags);
}

void ChapterTreeModel::loadFromM4aFile(const QString &pathToFile)
{
#ifdef NO_LIBMP4V2
    Q_UNUSED(pathToFile);
#else // NO_LIBMP4V2
    MP4FileHandle hM4a = MP4Read(pathToFile.toStdString().c_str());
    if (hM4a == MP4_INVALID_FILE_HANDLE ) {
        return;
    }

    MP4Chapter_t * chapters = 0;
    uint32_t chapterCount = 0;
    MP4ChapterType chapterType = MP4GetChapters(hM4a, &chapters, &chapterCount, MP4ChapterTypeAny);
    if (chapterCount == 0) {
        return;
    }

    qDebug() << chapterType;

    ChapterItem * tocItem = m_manager.registerItem("pseudoTOC");
    appendRow(tocItem);

    MP4Duration durationMs = 0; // yeah, it's in msec since m4a chapter marker tracks uses scale 1000.
    for (uint32_t i = 0; i < chapterCount; ++i) {
        ChapterItem * chapterItem = m_manager.registerItem(QString::number(durationMs));
        chapterItem->setItemProperty(ChapterTitle, QString(chapters[i].title));
        chapterItem->setItemProperty(ChapterStartTimeMs, static_cast<int>(durationMs));
        tocItem->appendRow(chapterItem);
//        qDebug() << "start:" << durationMs << chapters[i].title;
        durationMs += chapters[i].duration;
    }

    MP4Free(chapters);
    MP4Close(hM4a);

#endif // NO_LIBMP4V2
}

bool ChapterTreeModel::saveToFile(const QString &pathToFile)
{
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(pathToFile);
    if (mimeType.inherits("audio/mpeg")) {
        return saveToMpegFile(pathToFile);
    } else if (mimeType.inherits("audio/x-vorbis+ogg")) {
        return saveToVorbisFile(pathToFile);
    } else if (mimeType.inherits("audio/x-opus+ogg")) {
        return saveToOpusFile(pathToFile);
    } else if (mimeType.inherits("audio/mp4")) {
#ifdef NO_LIBMP4V2
        return false;
#else
        return saveToM4aFile(pathToFile);
#endif // NO_LIBMP4V2
    }

    return false;
}

bool ChapterTreeModel::saveToMpegFile(const QString &pathToFile)
{
    QStandardItem * item = invisibleRootItem()->child(0);
    ChapterItem * chapterItem = static_cast<ChapterItem *>(item);

    MpegFileHandler mfh;
    mfh.setFile(pathToFile);
    mfh.writeToFile(chapterItem);

    return true;
}

bool ChapterTreeModel::saveToVorbisFile(const QString &pathToFile)
{
    TagLib::Ogg::Vorbis::File file(pathToFile.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();

    Q_CHECK_PTR(tags);

    saveToXiphComment(tags);

    return file.save();
}

bool ChapterTreeModel::saveToOpusFile(const QString &pathToFile)
{
    TagLib::Ogg::Opus::File file(pathToFile.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();

    Q_CHECK_PTR(tags);

    saveToXiphComment(tags);

    return file.save();
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

bool ChapterTreeModel::saveToM4aFile(const QString &pathToFile)
{
#ifdef NO_LIBMP4V2
    Q_UNUSED(pathToFile);
    return false;
#else // NO_LIBMP4V2
    MP4FileHandle hM4a = MP4Modify(pathToFile.toStdString().c_str());
    if (hM4a == MP4_INVALID_FILE_HANDLE) {
        return false;
    }

    std::vector<MP4Chapter_t> chapters;

    // before we start, let's remove all existed chapters...
    MP4DeleteChapters(hM4a, MP4ChapterTypeAny);

    QStandardItem * item = invisibleRootItem()->child(0);
    if (item && item->hasChildren()) {
        // get ready to write our new chapter list.
        ChapterItem * currentItem = static_cast<ChapterItem *>(item->child(0));
        while (true) {
            MP4Chapter_t chap;

            std::string chapterTitle(currentItem->data(ChapterTitle).toString().toStdString());
            size_t titleLen = qMin(chapterTitle.length(), (size_t)MP4V2_CHAPTER_TITLE_MAX);
            strncpy(chap.title, chapterTitle.c_str(), titleLen);
            chap.title[titleLen] = 0;
            chap.duration = currentItem->data(ChapterStartTimeMs).toUInt();

            chapters.push_back(chap);

            QModelIndex nextItemModel = indexFromItem(currentItem).siblingAtRow(currentItem->row() + 1);
            if (nextItemModel.isValid()) {
                QStandardItem * nextItem = itemFromIndex(nextItemModel);
                currentItem = static_cast<ChapterItem *>(nextItem);
            } else {
                break;
            }
        }
    }

    bool isVideoTrack = false;
    MP4TrackId firstTrackId = getFirstAudioTrack(hM4a, isVideoTrack);
    if (!MP4_IS_VALID_TRACK_ID(firstTrackId)) {
        return false;
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
    MP4Optimize(pathToFile.toStdString().c_str());

    return true;
#endif // NO_LIBMP4V2
}

bool ChapterTreeModel::clearChapterTreeButKeepTOC()
{
    QStandardItem * parentItem = invisibleRootItem()->child(0);
    if (parentItem) {
        QModelIndex tocIndex(indexFromItem(parentItem));
        removeRows(0, parentItem->rowCount(), tocIndex);
        return true;
    }

    return false;
}

// actually only one item can be selected..
QModelIndex ChapterTreeModel::appendChapter(const QModelIndexList &selectedIndexes)
{
    QModelIndex parent;
    int row = -1;
    int startTimeMs = 0;
    QString chapterName(QStringLiteral("New Chapter"));
    bool hasSelected = false;

    if (!selectedIndexes.isEmpty()) {
        parent = selectedIndexes[0].parent();
        row = selectedIndexes[0].row();
        hasSelected = true;
    }

    QStandardItem * parentItem = parent.isValid() ? itemFromIndex(parent)
                                                  : invisibleRootItem()->child(0);

    ChapterItem * selectedChapter = hasSelected ? static_cast<ChapterItem *>(itemFromIndex(selectedIndexes[0]))
                                                : nullptr;
    if (selectedChapter) {
        startTimeMs = selectedChapter->data(ChapterStartTimeMs).toInt();
    }

    return appendChapter(parentItem, chapterName, startTimeMs, row);
}

QModelIndex ChapterTreeModel::appendChapter(int startTimeMs, const QString &title)
{
    QStandardItem * parentItem = invisibleRootItem()->child(0);
    if (!parentItem) {
        ChapterItem * tocItem = m_manager.registerItem("presudoTOC");
        tocItem->setItemProperty(FrameId, "CTOC");
        appendRow(tocItem);
        parentItem = tocItem;
    }
    return appendChapter(parentItem, title, startTimeMs);
}

QVariant ChapterTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Start Time");
        case 2:
            return tr("End Time");
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QVariant ChapterTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QStandardItemModel::data(index, role);
    if (role != Qt::DisplayRole) return QStandardItemModel::data(index, role);

    // only affect text display
    switch (index.column()) {
    case 1: {
        QTime time(QTime::fromMSecsSinceStartOfDay(itemFromIndex(index.siblingAtColumn(0))->data(ChapterStartTimeMs).toInt()));
        return time.toString();
    }
    case 2: {
        QVariant data(itemFromIndex(index.siblingAtColumn(0))->data(ChapterEndTimeMs));
        if (data.isNull()) {
            return QStringLiteral("N/A");
        }
        QTime time(QTime::fromMSecsSinceStartOfDay(data.toInt()));
        return time.toString();
    }
    default:
        break;
    }

    return QStandardItemModel::data(index, role);
}

int ChapterTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

// spec: https://wiki.xiph.org/Chapter_Extension
void ChapterTreeModel::loadFromXiphComment(TagLib::Ogg::XiphComment *tags)
{
    constexpr int substrPos = sizeof("CHAPTERXXX") - 1;

    ChapterItem * tocItem = m_manager.registerItem("pseudoTOC");
    appendRow(tocItem);

    if (tags) {
        const TagLib::Ogg::FieldListMap & fieldMap = tags->fieldListMap();
        for (const auto & kv : fieldMap) {
            if (kv.first.startsWith("CHAPTER")) {
                TagLib::String chapterId(kv.first.substr(0, substrPos));
                TagLib::String subStr(kv.first.substr(substrPos));

                QString value(QString::fromStdString(kv.second.toString().to8Bit()));

                ChapterItem * chapterItem = m_manager.registerItem(QString::fromStdString(chapterId.to8Bit()));

                if (subStr.isEmpty()) {
                    // CHAPTER001=00:00:00.000
                    chapterItem->setItemProperty(ChapterStartTimeMs, QTime::fromString(value, QStringLiteral("hh:mm:ss.zzz")).msecsSinceStartOfDay());
                    tocItem->appendRow(chapterItem);
                } else if (subStr == "NAME") {
                    // CHAPTER001NAME=Chapter 1
                    chapterItem->setItemProperty(ChapterTitle, value);
                } else if (subStr == "URL") {
                    // CHAPTER001URL=http://...
                    chapterItem->setItemProperty(ChapterUrl, value);
                }
                //std::cout << chapterId.to8Bit() << std::endl;
            }
            QString commentKey(QString::fromStdString(kv.first.to8Bit()));
            //std::cout << kv.first.toCString() << " : " << kv.second.toString().toCString() << std::endl;
        }
    }
}

bool ChapterTreeModel::saveToXiphComment(TagLib::Ogg::XiphComment *xiphComment)
{
    if (!xiphComment) return false;

    // before we start, let's remove all existed chapters...
    const TagLib::Ogg::FieldListMap & fieldMap = xiphComment->fieldListMap();
    TagLib::StringList needRemoved;
    for (const auto & kv : fieldMap) {
        if (kv.first.startsWith("CHAPTER")) {
            needRemoved.append(kv.first);
        }
    }
    for (const TagLib::String & fieldName : needRemoved) {
        xiphComment->removeFields(fieldName);
    }

    QStandardItem * item = invisibleRootItem()->child(0);
    if (item && item->hasChildren()) {
        // get ready to write our new chapter list.
        ChapterItem * currentItem = static_cast<ChapterItem *>(item->child(0));
        int currentChapterNumber = 1; // yeah it starts from 1.
        while (true) {
            // CHAPTER001=00:00:00.000
            char chapterTime[11]; // CHAPTERXXX
            snprintf(chapterTime, 11, "CHAPTER%03d", currentChapterNumber);
            QTime startTime(QTime::fromMSecsSinceStartOfDay(currentItem->data(ChapterStartTimeMs).toInt()));
            TagLib::String timeStr(startTime.toString("hh:mm:ss.zzz").toStdString(), TagLib::String::UTF8);
            xiphComment->addField(chapterTime, timeStr);

            if (currentItem->data(ChapterTitle).isValid()) {
                char chapterName[15]; // CHAPTERXXXNAME
                snprintf(chapterName, 15, "CHAPTER%03dNAME", currentChapterNumber);
                TagLib::String titleStr(currentItem->data(ChapterTitle).toString().toStdString(), TagLib::String::UTF8);
                xiphComment->addField(chapterName, titleStr);
            }

            currentChapterNumber++;
            QModelIndex nextItemModel = indexFromItem(currentItem).siblingAtRow(currentItem->row() + 1);
            if (nextItemModel.isValid()) {
                QStandardItem * nextItem = itemFromIndex(nextItemModel);
                currentItem = static_cast<ChapterItem *>(nextItem);
            } else {
                break;
            }
        }
    }

    return true;
}

QModelIndex ChapterTreeModel::appendChapter(QStandardItem *parentItem, const QString &title, int startTimeMs, int rowAt)
{
    ChapterItem * newChapter = new ChapterItem();
    newChapter->setColumnCount(3);
    newChapter->setItemProperty(FrameId, "CHAP");
    newChapter->setItemProperty(ChapterTitle, title);
    newChapter->setItemProperty(ChapterStartTimeMs, startTimeMs);
    if (rowAt != -1) {
        parentItem->insertRow(rowAt + 1, newChapter);
    } else {
        parentItem->appendRow(newChapter);
    }

    return indexFromItem(newChapter);
}
