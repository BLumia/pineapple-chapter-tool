#include "vorbisfilehandler.h"

#include "taglibutils_p.h"

#include <vorbisfile.h>

VorbisFileHandler::VorbisFileHandler()
{

}

VorbisFileHandler::~VorbisFileHandler()
{

}

FileHandlerInterface::Status VorbisFileHandler::setFile(const QString filePath)
{
    m_file = filePath;

    return SUCCESS;
}

ChapterItem *VorbisFileHandler::createChapterTree() const
{
    if (m_file.isEmpty()) return nullptr;

    TagLib::Ogg::Vorbis::File file(m_file.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();

    return TagLibUtils::loadFromXiphComment(tags);
}

FileHandlerInterface::ChapterFeatures VorbisFileHandler::chapterFeatures() const
{
    return ChapterFeatures(StartTimeMs | Title);
}

FileHandlerInterface::Status VorbisFileHandler::importFromFile()
{
    return SUCCESS;
}

FileHandlerInterface::Status VorbisFileHandler::writeToFile(ChapterItem *chapterRoot)
{
    TagLib::Ogg::Vorbis::File file(m_file.toLocal8Bit().data());
    TagLib::Ogg::XiphComment * tags = file.tag();

    Q_CHECK_PTR(tags);

    TagLibUtils::saveToXiphComment(chapterRoot, tags);

    return file.save() ? SUCCESS : FILE_STAT_ERROR;
}
