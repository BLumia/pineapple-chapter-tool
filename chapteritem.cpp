#include "chapteritem.h"

void ChapterItem::setItemProperty(ChapterProperties prop, const QVariant & value)
{
    setData(value, prop);
}

QVariant ChapterItem::data(int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        if (data(ChapterTitle).isValid()) {
            return data(ChapterTitle);
        }
        if (hasChildren()) {
            return QObject::tr("Table of Contents");
        }
        break;
    }

    return QStandardItem::data(role);
}

// DFS iteration
void ChapterItem::forEach(const ChapterItem *root,
                          std::function<void (const ChapterItem *)> callback)
{
    if (!root) return;

    callback(root);

    if (root->hasChildren()) {
        int childrenCount = root->rowCount();
        for (int i = 0; i < childrenCount; i++) {
            QStandardItem * nextItem = root->child(i);
            ChapterItem * currentItem = static_cast<ChapterItem *>(nextItem);
            ChapterItem::forEach(currentItem, callback);
        }
    }
}
