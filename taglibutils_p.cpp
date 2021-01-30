#include "taglibutils_p.h"

#include <QString>
#include <QTime>

#include "chapteritem.h"
#include "xiphcomment.h"

// spec: https://wiki.xiph.org/Chapter_Extension
ChapterItem *TagLibUtils::loadFromXiphComment(TagLib::Ogg::XiphComment *tags)
{
    constexpr int substrPos = sizeof("CHAPTERXXX") - 1;

    ChapterItem * tocItem = new ChapterItem("pseudoTOC");
    tocItem->setColumnCount(3);

    if (tags) {
        QMap<QString, ChapterItem *> chapters;

        const TagLib::Ogg::FieldListMap & fieldMap = tags->fieldListMap();
        for (const auto & kv : fieldMap) {
            if (kv.first.startsWith("CHAPTER")) {
                TagLib::String chapterId(kv.first.substr(0, substrPos));
                TagLib::String subStr(kv.first.substr(substrPos));

                QString chapterIdStr(QString::fromStdString(chapterId.to8Bit()));
                QString value(QString::fromStdString(kv.second.toString().to8Bit()));

                ChapterItem * chapterItem = nullptr;
                if (chapters.contains(chapterIdStr)) {
                    chapterItem = chapters[chapterIdStr];
                } else {
                    chapterItem = new ChapterItem(QString::fromStdString(chapterId.to8Bit()));
                    chapterItem->setColumnCount(3);
                    chapters[chapterIdStr] = chapterItem;
                }

                if (subStr.isEmpty()) {
                    // CHAPTER001=00:00:00.000
                    chapterItem->setItemProperty(ChapterStartTimeMs, QTime::fromString(value, QStringLiteral("hh:mm:ss.zzz")).msecsSinceStartOfDay());
                    tocItem->appendRow(chapters[chapterIdStr]);
                } else if (subStr == "NAME") {
                    // CHAPTER001NAME=Chapter 1
                    chapterItem->setItemProperty(ChapterTitle, value);
                } else if (subStr == "URL") {
                    // CHAPTER001URL=http://...
                    chapterItem->setItemProperty(ChapterUrl, value);
                }
                //std::cout << chapterId.to8Bit() << std::endl;
            }
//            QString commentKey(QString::fromStdString(kv.first.to8Bit()));
            //std::cout << kv.first.toCString() << " : " << kv.second.toString().toCString() << std::endl;
        }
    }

    return tocItem;
}

bool TagLibUtils::saveToXiphComment(ChapterItem *rootItem, TagLib::Ogg::XiphComment *xiphComment)
{
    if (!xiphComment || !rootItem) return false;

    // before we start, let's remove all existed chapters...
    const TagLib::Ogg::FieldListMap & fieldMap = xiphComment->fieldListMap();
    TagLib::StringList needRemoved;
    for (const auto & kv : fieldMap) {
        if (kv.first.startsWith("CHAPTER")) {
            needRemoved.append(kv.first);
        }
    }
    for (const TagLib::String & fieldName : needRemoved) {
        xiphComment->removeFields(fieldName);
    }

    int currentChapterNumber = 1; // yeah it starts from 1.
    ChapterItem::forEach(rootItem, [&](const ChapterItem * currentItem) {
        // XiphComment doesn't care about TOC item.
        if (currentItem->hasChildren()) return;

        // CHAPTER001=00:00:00.000
        char chapterTime[11]; // CHAPTERXXX
        snprintf(chapterTime, 11, "CHAPTER%03d", currentChapterNumber);
        QTime startTime(QTime::fromMSecsSinceStartOfDay(currentItem->data(ChapterStartTimeMs).toInt()));
        TagLib::String timeStr(startTime.toString("hh:mm:ss.zzz").toStdString(), TagLib::String::UTF8);
        xiphComment->addField(chapterTime, timeStr);

        if (currentItem->data(ChapterTitle).isValid()) {
            char chapterName[15]; // CHAPTERXXXNAME
            snprintf(chapterName, 15, "CHAPTER%03dNAME", currentChapterNumber);
            TagLib::String titleStr(currentItem->data(ChapterTitle).toString().toStdString(), TagLib::String::UTF8);
            xiphComment->addField(chapterName, titleStr);
        }

        currentChapterNumber++;
    });

    return true;
}
