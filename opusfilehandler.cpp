#include "opusfilehandler.h"

#include "taglibutils_p.h"

#include <QSaveFile>
#include <opusfile.h>

OpusFileHandler::OpusFileHandler()
{

}

OpusFileHandler::~OpusFileHandler()
{

}

FileHandlerInterface::Status OpusFileHandler::setFile(const QString filePath)
{
    m_file = filePath;

    return SUCCESS;
}

// seems same as the vorbis one...
ChapterItem *OpusFileHandler::createChapterTree() const
{
    if (m_file.isEmpty()) return nullptr;

    TagLib::Ogg::Opus::File file(m_file.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();

    return TagLibUtils::loadFromXiphComment(tags);
}

FileHandlerInterface::ChapterFeatures OpusFileHandler::chapterFeatures() const
{
    return ChapterFeatures(StartTimeMs | Title);
}

FileHandlerInterface::Status OpusFileHandler::importFromFile()
{
    return SUCCESS;
}

FileHandlerInterface::Status OpusFileHandler::writeToFile(ChapterItem *chapterRoot)
{
    TagLib::Ogg::Opus::File file(m_file.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();

    Q_CHECK_PTR(tags);

    TagLibUtils::saveToXiphComment(chapterRoot, tags);

    return file.save() ? SUCCESS : FILE_STAT_ERROR;
}

FileHandlerInterface::Status OpusFileHandler::exportToFile(ChapterItem *chapterRoot)
{
    QSaveFile sf(m_file);
    sf.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

    QStandardItem * item = chapterRoot;
    ChapterItem * chapterItem = static_cast<ChapterItem *>(item);

    if (chapterItem) {
        int currentChapterNum = 1; // start from 1.
        ChapterItem::forEach(chapterItem, [&](const ChapterItem * currentItem) {
            if (currentItem->hasChildren()) return;

            // write time and title
            std::string timeStrLine = TagLibUtils::ogmChapterKey(currentChapterNum, TagLibUtils::OgmChapterTime);
            timeStrLine += "=";
            timeStrLine += TagLibUtils::ogmTimeStr(currentItem->data(ChapterStartTimeMs).toInt());
            QByteArray ba;
            ba.append(timeStrLine.data(), timeStrLine.length());
            ba.append('\n');

            std::string titleStrLine = TagLibUtils::ogmChapterKey(currentChapterNum, TagLibUtils::OgmChapterName);
            titleStrLine += "=";
            QByteArray ba2;
            ba2.append(titleStrLine.data(), titleStrLine.length());
            ba2.append(currentItem->data(ChapterTitle).toString().toUtf8());
            ba2.append('\n');

            sf.write(ba);
            sf.write(ba2);

            currentChapterNum++;
        });
    }

    return sf.commit() ? SUCCESS : FILE_STAT_ERROR;
}
