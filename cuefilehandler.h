#ifndef CUEFILEHANDLER_H
#define CUEFILEHANDLER_H

#include "filehandlerinterface.h"

class CueFileHandler : public FileHandlerInterface
{
public:
    CueFileHandler();
    ~CueFileHandler();

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

#endif // CUEFILEHANDLER_H
