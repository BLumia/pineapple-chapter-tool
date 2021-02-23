#pragma once

#include "filehandlerinterface.h"

class VorbisFileHandler : public FileHandlerInterface
{
public:
    VorbisFileHandler();
    ~VorbisFileHandler();

    // FileHandlerInterface interface
public:
    Status setFile(const QString filePath) override;
    ChapterItem *createChapterTree() const override;
    ChapterFeatures chapterFeatures() const override;
    Status importFromFile() override;
    Status writeToFile(ChapterItem *chapterRoot) override;
    Status exportToFile(ChapterItem *chapterRoot) override;

private:
    QString m_file;
};
