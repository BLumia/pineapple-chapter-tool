#include "chaptertreemodel.h"

#include <mpegfile.h>
#include <id3v2tag.h>
#include <chapterframe.h>
#include <tbytevectorlist.h> // since tableofcontentsframe is missing the declaration of ByteVectorList...
#include <tableofcontentsframe.h>
#include <textidentificationframe.h>
#include <urllinkframe.h>
#include <attachedpictureframe.h>

#include <QDebug>
#include <QTime>

namespace ID3v2 = TagLib::ID3v2;

void ChapterTreeModel::loadFromFile(const QString &pathToFile)
{
    clear();

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
                        const ID3v2::TextIdentificationFrame * chapterTitle = dynamic_cast<const ID3v2::TextIdentificationFrame *>(subFrame);
                        chapterItem->setItemProperty(ChapterTitle, QString::fromStdString(chapterTitle->toString().to8Bit()));
                    } else if (subFrame->frameID() == "WXXX") {
                        const TagLib::ID3v2::UserUrlLinkFrame * wwwLink = dynamic_cast<const ID3v2::UserUrlLinkFrame *>(subFrame);
                        chapterItem->setItemProperty(ChapterUrl, QString::fromStdString(wwwLink->toString().to8Bit()));
                    }
                }
            }
        }
    }

    return;
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
