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

template<
    class Trait,
    std::enable_if_t <
        std::is_same<ChapterItem *, Trait>::value
            || std::is_same<const ChapterItem *, Trait>::value,
        bool
    > = true
>
static void forEachTemplate(Trait root, std::function<void (Trait)> callback) {
    if (!root) return;

    callback(root);

    if (root->hasChildren()) {
        int childrenCount = root->rowCount();
        for (int i = 0; i < childrenCount; i++) {
            QStandardItem * nextItem = root->child(i);
            ChapterItem * currentItem = static_cast<ChapterItem *>(nextItem);
            forEachTemplate<Trait>(currentItem, callback);
        }
    }
}

// DFS iteration
void ChapterItem::forEach(ChapterItem *root, std::function<void (ChapterItem *)> callback)
{
    forEachTemplate<ChapterItem *>(root, callback);
}

// DFS iteration
void ChapterItem::forEach(const ChapterItem *root,
                          std::function<void (const ChapterItem *)> callback)
{
    forEachTemplate<const ChapterItem *>(root, callback);
}
