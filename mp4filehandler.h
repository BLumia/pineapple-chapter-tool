#pragma once

#include "filehandlerinterface.h"

class Mp4FileHandler : public FileHandlerInterface
{
public:
    Mp4FileHandler();
    ~Mp4FileHandler();

    // FileHandlerInterface interface
public:
    Status setFile(const QString filePath) override;
    ChapterItem *createChapterTree() const override;
    ChapterFeatures chapterFeatures() const override;
    Status importFromFile() override;
    Status writeToFile(ChapterItem *chapterRoot) override;

private:
    QString m_file;
};
