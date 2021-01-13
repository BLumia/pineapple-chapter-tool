#ifndef CHAPTERTREEVIEW_H
#define CHAPTERTREEVIEW_H

#include <QModelIndexList>
#include <QTreeView>

class ChapterTreeView : public QTreeView
{
    Q_OBJECT
public:
    using QTreeView::QTreeView;

signals:
    void viewSelectionChanged();

protected:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
};

#endif // CHAPTERTREEVIEW_H
