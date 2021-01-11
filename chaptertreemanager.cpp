#include "chaptertreemanager.h"

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
        break;
    }

    return QStandardItem::data(role);
}

ChapterTreeManager::ChapterTreeManager()
{

}

ChapterTreeManager::~ChapterTreeManager()
{

}

ChapterItem *ChapterTreeManager::registerItem(const QString &elementId)
{
    if (m_itemsMap.contains(elementId)) {
        return m_itemsMap.value(elementId);
    }

    ChapterItem * result = new ChapterItem(elementId);
    result->setColumnCount(3);
    m_itemsMap[elementId] = result;

    return result;
}
