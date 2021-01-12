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

#include <QDebug>
#include <QMimeDatabase>
#include <QTime>

namespace ID3v2 = TagLib::ID3v2;

void ChapterTreeModel::loadFromFile(const QString &pathToFile)
{
    clear();

    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(pathToFile);
    if (mimeType.inherits("audio/mpeg")) {
        return loadFromMpegFile(pathToFile);
    } else if (mimeType.inherits("audio/x-vorbis+ogg")) {
        return loadFromVorbisFile(pathToFile);
    }

    qDebug() << mimeType;

    return;
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
                        chapterItem->setItemProperty(ChapterTitle, QString::fromStdString(chapterTitle->toString().to8Bit()));
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
//       https://wiki.xiph.org/Chapter_Extension
void ChapterTreeModel::loadFromVorbisFile(const QString &pathToFile)
{
    constexpr int substrPos = sizeof("CHAPTERXXX") - 1;

    ChapterItem * tocItem = m_manager.registerItem("pseudoTOC");
    appendRow(tocItem);

    TagLib::Ogg::Vorbis::File file(pathToFile.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();
    if (tags) {
        const TagLib::Ogg::FieldListMap & fieldMap = tags->fieldListMap();
        for (auto kv : fieldMap) {
            if (kv.first.startsWith("CHAPTER")) {
                TagLib::String chapterId(kv.first.substr(0, substrPos));
                TagLib::String subStr(kv.first.substr(substrPos));

                QString value(QString::fromStdString(kv.second.toString().to8Bit()));

                ChapterItem * chapterItem = m_manager.registerItem(QString::fromStdString(chapterId.to8Bit()));

                if (subStr.isEmpty()) {
                    // CHAPTER001=00:00:00.000
                    qDebug() << value << QTime::fromString(value, QStringLiteral("hh:mm:ss.zzz")).msec();
                    chapterItem->setItemProperty(ChapterStartTimeMs, QTime::fromString(value, QStringLiteral("hh:mm:ss.zzz")).msecsSinceStartOfDay());
                    tocItem->appendRow(chapterItem);
                } else if (subStr == "NAME") {
                    // CHAPTER001NAME=Chapter 1
                    chapterItem->setItemProperty(ChapterTitle, value);
                } else if (subStr == "URL") {
                    // CHAPTER001URL=http://...
                    chapterItem->setItemProperty(ChapterUrl, value);
                }
                std::cout << chapterId.to8Bit() << std::endl;
            }
            QString commentKey(QString::fromStdString(kv.first.to8Bit()));
            //std::cout << kv.first.toCString() << " : " << kv.second.toString().toCString() << std::endl;
        }
    }
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

    switch (index.column()) {
    case 1: {
        QTime time(QTime::fromMSecsSinceStartOfDay(itemFromIndex(index.siblingAtColumn(0))->data(ChapterStartTimeMs).toInt()));
        return time.toString();
    }
    case 2: {
        QTime time(QTime::fromMSecsSinceStartOfDay(itemFromIndex(index.siblingAtColumn(0))->data(ChapterEndTimeMs).toInt()));
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
