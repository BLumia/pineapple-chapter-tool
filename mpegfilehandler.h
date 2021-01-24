#pragma once

#include "filehandlerinterface.h"

class MpegFileHandler : public FileHandlerInterface
{
public:
    MpegFileHandler();
    ~MpegFileHandler();

    // FileHandlerInterface interface
public:
    Status setFile(const QString filePath) override;
    ChapterItem *createChapterTree() const override;
    ChapterFeatures chapterFeatures() const override;
    Status importFromFile() override;
    Status writeToFile(ChapterItem *chapterRoot) override;

private:
    bool loadDuration();
    void fillAllEndTimeMs(ChapterItem * chapterRoot);

private:
    uint32_t m_duration = 0;
    QString m_file;
};

