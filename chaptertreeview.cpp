#include "chaptertreeview.h"

#include "chaptertreemodel.h"

ChapterTreeModel *ChapterTreeView::model() const
{
    return qobject_cast<ChapterTreeModel *>(QTreeView::model());
}

void ChapterTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    emit viewSelectionChanged();
    return QTreeView::selectionChanged(selected, deselected);
}
