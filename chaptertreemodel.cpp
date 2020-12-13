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

namespace ID3v2 = TagLib::ID3v2;

ChapterTreeModel::ChapterTreeModel()
{

}

void ChapterTreeModel::loadFromFile(const QString &pathToFile)
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

                if (tocFrame->isTopLevel()) {
                    m_manager.setTopLevelElementId(elementId);
                }

                TOCElement * el = m_manager.registerElement<TOCElement>(elementId);
                QStringList subElementsIds;
                for (const TagLib::ByteVector & bv : tocFrame->childElements()) {
                    QString chapterElementId(QString::fromLatin1(bv.data(), bv.size()));
                    ChapterElement * chapterEl = m_manager.registerElement<ChapterElement>(chapterElementId);
                    chapterEl->setParent(el);
                    subElementsIds.append(chapterElementId);
                }
                el->setSubElementsIds(subElementsIds);
            } else if (frame->frameID() == "CHAP") {
                const ID3v2::ChapterFrame * chapterFrame = dynamic_cast<const ID3v2::ChapterFrame *>(frame);
                QString elementId(QString::fromLatin1(chapterFrame->elementID().data(), chapterFrame->elementID().size()));

                ChapterElement * el = m_manager.registerElement<ChapterElement>(elementId);
                el->setProperty(P_START_TIME_MS, chapterFrame->startTime());
                el->setProperty(P_END_TIME_MS, chapterFrame->endTime());

                const ID3v2::FrameList subFrames = chapterFrame->embeddedFrameList();
                for (const ID3v2::Frame * subFrame : subFrames) {
                    if (subFrame->frameID() == "TIT2") {
                        const ID3v2::TextIdentificationFrame * chapterTitle = dynamic_cast<const ID3v2::TextIdentificationFrame *>(subFrame);
                        el->setProperty(P_CHAP_TITLE, QString::fromStdString(chapterTitle->toString().to8Bit()));
                    } else if (subFrame->frameID() == "WXXX") {
                        const TagLib::ID3v2::UserUrlLinkFrame * wwwLink = dynamic_cast<const ID3v2::UserUrlLinkFrame *>(subFrame);
                        el->setProperty(P_CHAP_URL, QString::fromStdString(wwwLink->url().to8Bit()));
                    }
                }
            }
        }
    }

    return;
}

TOCElement *ChapterTreeModel::rootElement() const
{
    return m_manager.topLevelTOCElement();
}


QModelIndex ChapterTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    Element *parentItem = element<Element>(parent);
    if (parentItem == nullptr) {
        parentItem = rootElement();
    }

    if (parentItem->isTOCElement()) {
        TOCElement * tocEl = dynamic_cast<TOCElement *>(parentItem);
        QString childElementId = tocEl->subElementAt(row);
        if (childElementId.isEmpty()) {
            return QModelIndex();
        } else {
            return createIndex(row, column, m_manager.element(childElementId));
        }
    } else {
        return QModelIndex();
    }
}

QModelIndex ChapterTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    Element *childItem = element<Element>(child);
    TOCElement *parentItem = childItem->parent();
    QString elementId = m_manager.elementId(childItem);

    if (parentItem == rootElement()) {
        return QModelIndex();
    }

    return createIndex(parentItem->subElementAt(elementId), 0, parentItem);
}

int ChapterTreeModel::rowCount(const QModelIndex &parent) const
{
    Element * el = element<Element>(parent);
    if (el == nullptr) {
        el = rootElement();
    }

    if (el->isTOCElement()) {
        TOCElement * tocEl = dynamic_cast<TOCElement *>(el);
        return tocEl->subElementsCount();
    } else {
        return 0;
    }
}

int ChapterTreeModel::columnCount(const QModelIndex &parent) const
{
    Element * el = element<Element>(parent);
    if (el == nullptr) {
        el = rootElement();
    }

    return std::max(el->propertyCount(), 3);
}

QVariant ChapterTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    Element * el = element<Element>(index);
    if (el->isTOCElement()) {
        return "Placeholder";
    } else {
        return el->property(el->propertyType(index.column()));
    }
}
