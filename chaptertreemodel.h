#pragma once

#include "chaptertreemanager.h"

#include <QAbstractItemModel>

class ChapterTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ChapterTreeModel();

    void loadFromFile(const QString & pathToFile);
    TOCElement * rootElement() const;

    template<typename Trait>
    Trait * element(const QModelIndex & index) const {
        if (index.isValid()) {
            Trait *item = static_cast<Trait*>(index.internalPointer());
            if (item) {
                return item;
            }
        }
        return nullptr;
    }

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    ChapterTreeManager m_manager;
};
