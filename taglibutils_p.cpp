#include "taglibutils_p.h"

#include <QString>
#include <QTime>

#include "chapteritem.h"
#include "xiphcomment.h"

// spec: https://wiki.xiph.org/Chapter_Extension
std::string TagLibUtils::ogmChapterKey(int chapterNumber, OgmKeyType type)
{
    char chapterName[15] = {'\0'}; // CHAPTERXXXNAME
    switch (type) {
    case OgmChapterTime: // CHAPTER001=00:00:00.000
        snprintf(chapterName, 11, "CHAPTER%03d", chapterNumber);
        break;
    case OgmChapterName: // CHAPTER001NAME=Chapter 1
        snprintf(chapterName, 15, "CHAPTER%03dNAME", chapterNumber);
        break;
    default:
        break;
    }

    return std::string(chapterName);
}

std::string TagLibUtils::ogmTimeStr(int ms)
{
    QTime startTime(QTime::fromMSecsSinceStartOfDay(ms));
    return startTime.toString("hh:mm:ss.zzz").toStdString();
}

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
        TagLib::String timeStr(ogmTimeStr(currentItem->data(ChapterStartTimeMs).toInt()), TagLib::String::UTF8);
        xiphComment->addField(ogmChapterKey(currentChapterNumber), timeStr);

        if (currentItem->data(ChapterTitle).isValid()) {
            TagLib::String titleStr(currentItem->data(ChapterTitle).toString().toStdString(), TagLib::String::UTF8);
            xiphComment->addField(ogmChapterKey(currentChapterNumber, OgmChapterName), titleStr);
        }

        currentChapterNumber++;
    });

    return true;
}
