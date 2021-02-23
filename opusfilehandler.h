#pragma once

#include "filehandlerinterface.h"

class OpusFileHandler : public FileHandlerInterface
{
public:
    OpusFileHandler();
    ~OpusFileHandler();

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

