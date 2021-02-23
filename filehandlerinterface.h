#pragma once

#include <QFlags>

#include "chaptertreemodel.h"

class FileHandlerInterface {
public:
    enum ChapterFeature {
        StartTimeMs     = 0b0000,
        EndTimeMs       = 0b0001,
        Title           = 0b0010,
        Url             = 0b0100,
    };
    Q_DECLARE_FLAGS(ChapterFeatures, ChapterFeature)

    enum Status {
        SUCCESS,
        NO_FILE_LOADED, // forget to call setFile()
        FILE_STAT_ERROR, // any error during file loading.
        IMPORT_NOT_SUPPORTED,
        WRITE_NOT_SUPPORTED,
        EXPORT_NOT_SUPPORTED,
        DURATION_REQUIRED, // need to set duration manually.
    };

    virtual enum Status setFile(const QString filePath) = 0;
    virtual ChapterItem * createChapterTree() const = 0;
    virtual ChapterFeatures chapterFeatures() const = 0;
    virtual enum Status importFromFile() = 0;
    // it is possible that the handler modified the chapter tree.
    // Or maybe we need a pre-write check interface?
    virtual enum Status writeToFile(ChapterItem * chapterRoot) = 0;
    virtual enum Status exportToFile(ChapterItem * chapterRoot) = 0;
};
