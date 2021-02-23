#pragma once

#include "filehandlerinterface.h"

class YtdlFileHandler : public FileHandlerInterface
{
public:
    YtdlFileHandler();
    ~YtdlFileHandler();

    // FileHandlerInterface interface
public:
    Status setFile(const QString filePath) override;
    ChapterItem *createChapterTree() const override;
    ChapterFeatures chapterFeatures() const override;
    Status importFromFile() override;
    Status writeToFile(ChapterItem *chapterRoot) override;
    Status exportToFile(ChapterItem *chapterRoot) override;

private:
    uint32_t m_duration = 0;
    QString m_file;
};
