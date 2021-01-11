#pragma once

#include <QString>
#include <QMap>
#include <QStandardItem>

enum ChapterProperties : int {
    FrameId = Qt::UserRole + 1,
    ChapterStartTimeMs,
    ChapterEndTimeMs,
    ChapterTitle,
    ChapterUrl,
};

class ChapterItem : public QStandardItem
{
public:
    using QStandardItem::QStandardItem;
    void setItemProperty(enum ChapterProperties prop, const QVariant &value);

    QVariant data(int role = Qt::UserRole + 1) const override;
};

class ChapterTreeManager
{
public:
    ChapterTreeManager();
    ~ChapterTreeManager();

    ChapterItem * registerItem(const QString & elementId);

private:
    QMap<QString, ChapterItem *> m_itemsMap; // <element id, item>
};
