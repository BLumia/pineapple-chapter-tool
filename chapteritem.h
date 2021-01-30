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

    static void forEach(const ChapterItem * root,
                        std::function<void(const ChapterItem *)> callback);
};
