#include "opusfilehandler.h"

#include "taglibutils_p.h"

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
