#include "chaptertreemodel.h"

#include "filehandlermanager.h"
#include "filehandlerinterface.h"
#include "taglibutils_p.h"

#include <QDebug>
#include <QItemSelectionModel>
#include <QMimeDatabase>
#include <QSaveFile>
#include <QTime>

bool ChapterTreeModel::loadFromFile(const QString &pathToFile)
{
    clear();

    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(pathToFile);

    FileHandlerInterface * handler = FileHandlerManager::instance()->createHandlerByMimeType(mimeType);
    if (handler) {
        handler->setFile(pathToFile);
        handler->importFromFile();
        ChapterItem * created = handler->createChapterTree();
        if (created) {
            appendRow(created);
        }
        return true;
    }

    qDebug() << mimeType;

    return false;
}

bool ChapterTreeModel::saveToFile(const QString &pathToFile)
{
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(pathToFile);

    FileHandlerInterface * handler = FileHandlerManager::instance()->createHandlerByMimeType(mimeType);
    if (handler) {
        QStandardItem * item = invisibleRootItem()->child(0);
        ChapterItem * chapterItem = static_cast<ChapterItem *>(item);

        handler->setFile(pathToFile);
        handler->writeToFile(chapterItem);

        return true;
    }

    return false;
}

bool ChapterTreeModel::exportToFile(const QString &pathToFile)
{
    QSaveFile sf(pathToFile);
    sf.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

    QStandardItem * item = invisibleRootItem()->child(0);
    ChapterItem * chapterItem = static_cast<ChapterItem *>(item);

    if (chapterItem) {
        int currentChapterNum = 1; // start from 1.
        ChapterItem::forEach(chapterItem, [&](const ChapterItem * currentItem) {
            if (currentItem->hasChildren()) return;

            // write time and title
            std::string timeStrLine = TagLibUtils::ogmChapterKey(currentChapterNum, TagLibUtils::OgmChapterTime);
            timeStrLine += "=";
            timeStrLine += TagLibUtils::ogmTimeStr(currentItem->data(ChapterStartTimeMs).toInt());
            QByteArray ba;
            ba.append(timeStrLine.data(), timeStrLine.length());
            ba.append('\n');

            std::string titleStrLine = TagLibUtils::ogmChapterKey(currentChapterNum, TagLibUtils::OgmChapterName);
            titleStrLine += "=";
            QByteArray ba2;
            ba2.append(titleStrLine.data(), titleStrLine.length());
            ba2.append(currentItem->data(ChapterTitle).toString().toUtf8());
            ba2.append('\n');

            sf.write(ba);
            sf.write(ba2);

            currentChapterNum++;
        });
    }

    return sf.commit();
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
QModelIndex ChapterTreeModel::appendChapter(QItemSelectionModel * selectionModel)
{
    QModelIndex parent;
    int row = -1;
    int startTimeMs = 0;
    QString chapterName(QStringLiteral("New Chapter"));
    ChapterItem * selectedChapter = nullptr;

    if (selectionModel && !selectionModel->selectedIndexes().isEmpty()) {
        QModelIndexList selectedIndexes(selectionModel->selectedIndexes());
        parent = selectedIndexes[0].parent();
        row = selectedIndexes[0].row();
        selectedChapter = static_cast<ChapterItem *>(itemFromIndex(selectedIndexes[0]));
    }

    QStandardItem * parentItem = parent.isValid() ? itemFromIndex(parent)
                                                  : invisibleRootItem()->child(0);

    if (selectedChapter) {
        startTimeMs = selectedChapter->data(ChapterStartTimeMs).toInt();
    }

    return appendChapter(parentItem, chapterName, startTimeMs, row);
}

QModelIndex ChapterTreeModel::appendChapter(int startTimeMs, const QString &title)
{
    QStandardItem * parentItem = invisibleRootItem()->child(0);
    if (!parentItem) {
        ChapterItem * tocItem = new ChapterItem("presudoTOC");
        tocItem->setColumnCount(3);
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
