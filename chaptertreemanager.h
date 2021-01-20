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
    ChapterPicture,
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
    void setAudioLengthMs(int len);
    int audioLengthMs() const;

private:
    QMap<QString, ChapterItem *> m_itemsMap; // <element id, item>
    int m_audioLengthMs = 0;
};
