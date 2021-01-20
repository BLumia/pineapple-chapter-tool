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
        if (hasChildren()) {
            return QObject::tr("Table of Contents");
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

void ChapterTreeManager::setAudioLengthMs(int len)
{
    m_audioLengthMs = len;
}

int ChapterTreeManager::audioLengthMs() const
{
    return m_audioLengthMs;
}
