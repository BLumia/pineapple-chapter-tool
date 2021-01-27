#pragma once

#include "chapteritem.h"

#include <QStandardItemModel>

namespace TagLib{
namespace Ogg{
class XiphComment;
}
}

class ChapterTreeModel : public QStandardItemModel
{
    Q_OBJECT
public:
    using QStandardItemModel::QStandardItemModel;

    bool loadFromFile(const QString & pathToFile);
    void loadFromMpegFile(const QString & pathToFile);
    void loadFromVorbisFile(const QString & pathToFile);
    void loadFromOpusFile(const QString & pathToFile);
    void loadFromM4aFile(const QString & pathToFile);

    bool saveToFile(const QString & pathToFile);
    bool saveToMpegFile(const QString & pathToFile);
    bool saveToVorbisFile(const QString & pathToFile);
    bool saveToOpusFile(const QString & pathToFile);
    bool saveToM4aFile(const QString & pathToFile);

    bool clearChapterTreeButKeepTOC();
    QModelIndex appendChapter(const QModelIndexList &selectedIndexes);
    QModelIndex appendChapter(int startTimeMs, const QString & title);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    QModelIndex appendChapter(QStandardItem * parentItem, const QString & title, int startTimeMs, int rowAt = -1);
};
