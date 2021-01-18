#include "chaptertreemodel.h"

#include <mpegfile.h>
#include <id3v2tag.h>
#include <chapterframe.h>
#include <tbytevectorlist.h> // since tableofcontentsframe is missing the declaration of ByteVectorList...
#include <tableofcontentsframe.h>
#include <textidentificationframe.h>
#include <urllinkframe.h>
#include <attachedpictureframe.h>

#include <vorbisfile.h>

#include <opusfile.h>

#ifndef NO_LIBMP4V2
#include <mp4v2/mp4v2.h>
#endif // NO_LIBMP4V2

#include <QDebug>
#include <QMimeDatabase>
#include <QTime>

namespace ID3v2 = TagLib::ID3v2;

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
    TagLib::MPEG::File file(pathToFile.toLocal8Bit().data());
    ID3v2::Tag * id3v2Tag = file.ID3v2Tag();
    if (id3v2Tag) {
        const ID3v2::FrameList & framelist = id3v2Tag->frameList();
        for (const ID3v2::Frame * frame : framelist) {
            if (frame->frameID() != "CHAP" && frame->frameID() != "CTOC") {
                continue;
            }

            if (frame->frameID() == "CTOC") {
                const ID3v2::TableOfContentsFrame * tocFrame = dynamic_cast<const ID3v2::TableOfContentsFrame *>(frame);
                QString elementId(QString::fromLatin1(tocFrame->elementID().data(), tocFrame->elementID().size()));

                ChapterItem * tocItem = m_manager.registerItem(elementId);
                tocItem->setItemProperty(FrameId, "CTOC");
                if (tocFrame->isTopLevel()) {
                    appendRow(tocItem);
                }

                QStringList subElementsIds;
                for (const TagLib::ByteVector & bv : tocFrame->childElements()) {
                    QString chapterElementId(QString::fromLatin1(bv.data(), bv.size()));
                    ChapterItem * chapterItem = m_manager.registerItem(chapterElementId);
                    chapterItem->setItemProperty(FrameId, "CHAP");

                    tocItem->appendRow(chapterItem);
                }
            } else if (frame->frameID() == "CHAP") {
                const ID3v2::ChapterFrame * chapterFrame = dynamic_cast<const ID3v2::ChapterFrame *>(frame);
                QString elementId(QString::fromLatin1(chapterFrame->elementID().data(), chapterFrame->elementID().size()));

                ChapterItem * chapterItem = m_manager.registerItem(elementId);
                chapterItem->setItemProperty(ChapterStartTimeMs, chapterFrame->startTime());
                chapterItem->setItemProperty(ChapterEndTimeMs, chapterFrame->endTime());

                const ID3v2::FrameList subFrames = chapterFrame->embeddedFrameList();
                for (const ID3v2::Frame * subFrame : subFrames) {
                    if (subFrame->frameID() == "TIT2") {
                        // TIT2: TextIdentificationFrame
                        const ID3v2::TextIdentificationFrame * chapterTitle = dynamic_cast<const ID3v2::TextIdentificationFrame *>(subFrame);
                        chapterItem->setItemProperty(ChapterTitle, QString::fromStdString(chapterTitle->toString().to8Bit(true)));
                    } else if (subFrame->frameID() == "WXXX") {
                        // WXXX: UserUrlLinkFrame
                        const TagLib::ID3v2::UserUrlLinkFrame * wwwLink = dynamic_cast<const ID3v2::UserUrlLinkFrame *>(subFrame);
                        chapterItem->setItemProperty(ChapterUrl, QString::fromStdString(wwwLink->toString().to8Bit()));
                    } else if (subFrame->frameID() == "APIC") {
                        // APIC: AttachedPictureFrame
                        const TagLib::ID3v2::AttachedPictureFrame * pic = dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame *>(subFrame);
                        std::cout << pic->description() << " " << pic->mimeType() << std::endl;
                    }
                }
            }
        }
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

    MP4Duration durationMs = 0;
    for (uint32_t i = 0; i < chapterCount; ++i) {
        ChapterItem * chapterItem = m_manager.registerItem(QString::number(durationMs));
        chapterItem->setItemProperty(ChapterTitle, QString(chapters[i].title));
        chapterItem->setItemProperty(ChapterStartTimeMs, static_cast<int>(durationMs));
        tocItem->appendRow(chapterItem);
//        qDebug() << "start:" << durationMs << chapters[i].title;
        durationMs += chapters[i].duration;
    }

    MP4Free(chapters);

#endif // NO_LIBMP4V2
}

bool ChapterTreeModel::saveToFile(const QString &pathToFile)
{
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(pathToFile);
    if (mimeType.inherits("audio/mpeg")) {
        return saveToMpegFile(pathToFile);
    }

    return false;
}

bool ChapterTreeModel::saveToMpegFile(const QString &pathToFile)
{
    TagLib::MPEG::File file(pathToFile.toLocal8Bit().data());
    ID3v2::Tag * id3v2Tag = file.ID3v2Tag(true);

    Q_ASSERT(id3v2Tag != nullptr);

    // remove all existed chapter frames.
    id3v2Tag->removeFrames(TagLib::ByteVector("CTOC", 4));
    id3v2Tag->removeFrames(TagLib::ByteVector("CHAP", 4));

    QStandardItem * item = invisibleRootItem()->child(0);
    if (item) {
        ChapterItem * currentItem = static_cast<ChapterItem *>(item);
        // if no item then no need to save chapters, if no children we are also no need
        // to save a empty CTOC frame with no sub-chapters inside it.
        if (currentItem && currentItem->hasChildren()) {
            int nextCTOC = 0;
            while (true) {
                if (currentItem->hasChildren()) {
                    // It's a TOC item, create a CTOC frame and iterator all its children
                    char tocElementId[24];
                    snprintf(tocElementId, 24, "toc%d", nextCTOC);
                    int childrenCount = currentItem->rowCount();
                    TagLib::ByteVectorList elementIdList;
                    for (int i = 0; i < childrenCount; i++) {
                        char chapterElementId[24];
                        snprintf(chapterElementId, 24, "chp%d_%d", nextCTOC, i);
                        elementIdList.append(chapterElementId);
                    }
                    ID3v2::TableOfContentsFrame * tocFrame = new ID3v2::TableOfContentsFrame(
                        TagLib::ByteVector(tocElementId, strlen(tocElementId)),
                        elementIdList
                    );
                    id3v2Tag->addFrame(tocFrame);

                    nextCTOC++;

                    QStandardItem * nextItem = currentItem->child(0);
                    currentItem = static_cast<ChapterItem *>(nextItem);
                } else {
                    // It's a chapter item, create a CHAP frame, and move to next item
                    ID3v2::FrameList subFrameList;
                    char chapterElementId[24];
                    snprintf(chapterElementId, 24, "chp%d_%d", nextCTOC - 1, currentItem->row());

                    QString chapterTitle(currentItem->data(ChapterTitle).toString());
                    if (!chapterTitle.isEmpty()) {
                        TagLib::String titleStr(chapterTitle.toStdString());
                        ID3v2::TextIdentificationFrame * chapterTitleFrane = new ID3v2::TextIdentificationFrame(
                            "TIT2", TagLib::String::Latin1
                        );
                        chapterTitleFrane->setText(titleStr);
                        subFrameList.append(chapterTitleFrane);
                    }

                    ID3v2::ChapterFrame * chapterFrame = new ID3v2::ChapterFrame(
                        TagLib::ByteVector(chapterElementId, strlen(chapterElementId)),
                        currentItem->data(ChapterStartTimeMs).toInt(),
                        currentItem->data(ChapterEndTimeMs).toInt(),
                        ~0u, ~0u,
                        subFrameList
                    );
                    id3v2Tag->addFrame(chapterFrame);

                    QModelIndex nextItemModel = indexFromItem(currentItem).siblingAtRow(currentItem->row() + 1);
                    if (nextItemModel.isValid()) {
                        QStandardItem * nextItem = itemFromIndex(nextItemModel);
                        currentItem = static_cast<ChapterItem *>(nextItem);
                    } else {
                        break;
                    }
                }
            }
        }
    }

    file.save();

    return true;
}

// actually only one item can be selected..
QModelIndex ChapterTreeModel::appendChapter(const QModelIndexList &selectedIndexes)
{
    QModelIndex parent;
    int row = -1;

    if (!selectedIndexes.isEmpty()) {
        parent = selectedIndexes[0].parent();
        row = selectedIndexes[0].row();
    }

    QStandardItem * parentItem = parent.isValid() ? itemFromIndex(parent)
                                                  : invisibleRootItem()->child(0);

    ChapterItem * newChapter = new ChapterItem();
    newChapter->setColumnCount(3);
    newChapter->setItemProperty(FrameId, "CHAP");
    newChapter->setItemProperty(ChapterTitle, "New Chapter");
    if (row != -1) {
        parentItem->insertRow(row + 1, newChapter);
    } else {
        parentItem->appendRow(newChapter);
    }

    return indexFromItem(newChapter);
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
