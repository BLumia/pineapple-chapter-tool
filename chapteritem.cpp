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
