#include "ytdlfilehandler.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

YtdlFileHandler::YtdlFileHandler()
{

}

YtdlFileHandler::~YtdlFileHandler()
{

}

FileHandlerInterface::Status YtdlFileHandler::setFile(const QString filePath)
{
    m_file = filePath;

    return SUCCESS;
}

ChapterItem *YtdlFileHandler::createChapterTree() const
{
    QFile jsonFile(m_file);
    jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);

    ChapterItem * tocItem = new ChapterItem("pseudoTOC");
    tocItem->setColumnCount(3);

    QJsonDocument && json = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject && jsonObj = json.object();
//    if (jsonObj.contains("duration")) {
//        m_duration = jsonObj.value("duration").toInt() * 1000;
//    }
    if (jsonObj.contains("chapters")) {
        QJsonArray && chapterArr = jsonObj.value("chapters").toArray();
        for (const QJsonValueRef & chapter : chapterArr) {
            QJsonObject && chapterObj = chapter.toObject();

            ChapterItem * chapterItem = new ChapterItem();
            chapterItem->setColumnCount(3);
            if (chapterObj.contains("title")) {
                chapterItem->setItemProperty(ChapterTitle, chapterObj.value("title").toString());
            }
            if (chapterObj.contains("start_time")) {
                chapterItem->setItemProperty(ChapterStartTimeMs, chapterObj.value("start_time").toInt() * 1000);
            }
            if (chapterObj.contains("end_time")) {
                chapterItem->setItemProperty(ChapterEndTimeMs, chapterObj.value("end_time").toInt() * 1000);
            }

            tocItem->appendRow(chapterItem);
        }
    }

    jsonFile.close();

    return tocItem;
}

FileHandlerInterface::ChapterFeatures YtdlFileHandler::chapterFeatures() const
{
    return ChapterFeatures(StartTimeMs | EndTimeMs | Title);
}

FileHandlerInterface::Status YtdlFileHandler::importFromFile()
{
    return SUCCESS;
}

FileHandlerInterface::Status YtdlFileHandler::writeToFile(ChapterItem *chapterRoot)
{
    QSaveFile sf(m_file);
    sf.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    sf.fileName();

    QJsonObject parentObj;
    QJsonArray chapterArr;
    ChapterItem::forEach(chapterRoot, [&](const ChapterItem * currentItem) {
        if (currentItem->hasChildren()) return;

        QJsonObject chapterObj;

        chapterObj.insert("title", QJsonValue::fromVariant(currentItem->data(ChapterTitle)));
        chapterObj.insert("start_time", QJsonValue(currentItem->data(ChapterStartTimeMs).toInt() / 1000));
        QVariant && endTimeMs = currentItem->data(ChapterEndTimeMs);
        if (endTimeMs.isValid()) {
            chapterObj.insert("end_time", QJsonValue(endTimeMs.toInt() / 1000));
        }

        chapterArr.append(QJsonValue(chapterObj));
    });
    parentObj.insert("chapters", QJsonValue(chapterArr));

    QJsonDocument json(parentObj);

    QTextStream out(&sf);
    out.setCodec("UTF-8");
    out << json.toJson();

    out.flush();
    sf.commit();

    return SUCCESS;
}

FileHandlerInterface::Status YtdlFileHandler::exportToFile(ChapterItem *chapterRoot)
{
    return writeToFile(chapterRoot);
}
