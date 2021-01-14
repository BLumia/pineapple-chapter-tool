#pragma once

#include "chaptertreemanager.h"

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

    QModelIndex appendChapter(const QModelIndexList &selectedIndexes);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void loadFromXiphComment(TagLib::Ogg::XiphComment * tags);

private:
    ChapterTreeManager m_manager;
};
