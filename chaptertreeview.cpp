#include "chaptertreeview.h"

void ChapterTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    emit viewSelectionChanged();
    return QTreeView::selectionChanged(selected, deselected);
}
