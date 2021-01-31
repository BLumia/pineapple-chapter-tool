#pragma once

#include "chapteritem.h"

#include <QStandardItemModel>

namespace TagLib{
namespace Ogg{
class XiphComment;
}
}

QT_BEGIN_NAMESPACE
class QItemSelectionModel;
QT_END_NAMESPACE

class ChapterTreeModel : public QStandardItemModel
{
    Q_OBJECT
public:
    using QStandardItemModel::QStandardItemModel;

    bool loadFromFile(const QString & pathToFile);
    bool saveToFile(const QString & pathToFile);
    bool exportToFile(const QString & pathToFile);

    bool clearChapterTreeButKeepTOC();
    QModelIndex appendChapter(QItemSelectionModel * selectionModel);
    QModelIndex appendChapter(int startTimeMs, const QString & title);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    QModelIndex appendChapter(QStandardItem * parentItem, const QString & title, int startTimeMs, int rowAt = -1);
};
