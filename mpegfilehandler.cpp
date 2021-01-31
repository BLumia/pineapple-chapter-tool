#include "mpegfilehandler.h"

#include <fileref.h>

#include <mpegfile.h>
#include <id3v2tag.h>
#include <chapterframe.h>
#include <tbytevectorlist.h> // since tableofcontentsframe is missing the declaration of ByteVectorList...
#include <tableofcontentsframe.h>
#include <textidentificationframe.h>
#include <urllinkframe.h>
#include <attachedpictureframe.h>

namespace ID3v2 = TagLib::ID3v2;

class ID3v2ChapterTreeManager
{
public:
    ID3v2ChapterTreeManager() {}
    ~ID3v2ChapterTreeManager() {}

    ChapterItem * registerItem(const QString & elementId);
    void setAudioLengthMs(int len);
    int audioLengthMs() const;

private:
    QMap<QString, ChapterItem *> m_itemsMap; // <element id, item>
    int m_audioLengthMs = 0;
};

ChapterItem *ID3v2ChapterTreeManager::registerItem(const QString &elementId)
{
    if (m_itemsMap.contains(elementId)) {
        return m_itemsMap.value(elementId);
    }

    ChapterItem * result = new ChapterItem(elementId);
    result->setColumnCount(3);
    m_itemsMap[elementId] = result;

    return result;
}

void ID3v2ChapterTreeManager::setAudioLengthMs(int len)
{
    m_audioLengthMs = len;
}

int ID3v2ChapterTreeManager::audioLengthMs() const
{
    return m_audioLengthMs;
}

// ------------------------- Item Get Border Line -------------------------

MpegFileHandler::MpegFileHandler()
{

}

MpegFileHandler::~MpegFileHandler()
{

}

FileHandlerInterface::Status MpegFileHandler::setFile(const QString filePath)
{
    m_file = filePath;

    return SUCCESS;
}

// spec: https://id3.org/id3v2-chapters-1.0
ChapterItem * MpegFileHandler::createChapterTree() const
{
    if (m_file.isEmpty()) return nullptr;

    ChapterItem * topLevelItem = nullptr;
    ID3v2ChapterTreeManager m_manager;

    TagLib::MPEG::File file(m_file.toLocal8Bit().data());
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
                    topLevelItem = tocItem;
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

    return topLevelItem;
}

FileHandlerInterface::ChapterFeatures MpegFileHandler::chapterFeatures() const
{
    return ChapterFeatures(StartTimeMs | EndTimeMs | Title | Url);
}

FileHandlerInterface::Status MpegFileHandler::importFromFile()
{
    return SUCCESS;
}

FileHandlerInterface::Status MpegFileHandler::writeToFile(ChapterItem *chapterRoot)
{
    fillAllEndTimeMs(chapterRoot);

    TagLib::MPEG::File file(m_file.toLocal8Bit().data());
    ID3v2::Tag * id3v2Tag = file.ID3v2Tag(true);

    Q_ASSERT(id3v2Tag != nullptr);

    // remove all existed chapter frames.
    id3v2Tag->removeFrames(TagLib::ByteVector("CTOC", 4));
    id3v2Tag->removeFrames(TagLib::ByteVector("CHAP", 4));

    if (id3v2Tag->title().isEmpty() && id3v2Tag->comment().isEmpty()) {
        // set a non-empty title or comment here. (haven't try other attr)
        // if we don't do this for mp3 file that didn't have ID3 tag before,
        // potplayer won't display ID3v2 chapter info at all.
        // it can be a taglib bug or a potplayer bug.
        id3v2Tag->setComment("Pineapple Chapter Tool :)");
    }

    int nextCTOC = 0;
    ChapterItem::forEach(chapterRoot, [&](const ChapterItem* currentItem) {
        if (currentItem->hasChildren()) {
            // It's a TOC item, create a CTOC frame
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
            tocFrame->setIsTopLevel(nextCTOC == 0);
            id3v2Tag->addFrame(tocFrame);

            nextCTOC++;
        } else {
            // It's a chapter item, create a CHAP frame
            ID3v2::FrameList subFrameList;
            char chapterElementId[24];
            snprintf(chapterElementId, 24, "chp%d_%d", nextCTOC - 1, currentItem->row());

            QString chapterTitle(currentItem->data(ChapterTitle).toString());
            if (!chapterTitle.isEmpty()) {
                TagLib::String titleStr(chapterTitle.toStdString(), TagLib::String::UTF8);
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
        }
    });

    file.save();

    return SUCCESS;
}

bool MpegFileHandler::loadDuration()
{
    TagLib::FileRef fileRef(m_file.toLocal8Bit().data());

    if (!fileRef.isNull() && fileRef.audioProperties()) {
        TagLib::AudioProperties *prop = fileRef.audioProperties();
        m_duration = prop->lengthInMilliseconds();
        return true;
    }

    return false;
}

void MpegFileHandler::fillAllEndTimeMs(ChapterItem * chapterRoot)
{
    if (!chapterRoot) return;

    ChapterItem * prevChapterItem = nullptr;
    ChapterItem::forEach(chapterRoot, [&](ChapterItem * currentItem) {
        if (currentItem->hasChildren()) {
            // TOC item, just skip to its child item, do nothing here.
            return;
        } else {
            // It's a chapter item
            // Check if we should fill the end time of previous chapter.
            if (prevChapterItem && prevChapterItem->data(ChapterEndTimeMs).isNull()) {
                prevChapterItem->setItemProperty(ChapterEndTimeMs, currentItem->data(ChapterStartTimeMs));
            }
            prevChapterItem = currentItem;
        }
    });

    // last one.
    if (prevChapterItem && prevChapterItem->data(ChapterEndTimeMs).isNull()) {
        if (m_duration == 0) loadDuration();
        prevChapterItem->setItemProperty(ChapterEndTimeMs, m_duration);
    }
}
