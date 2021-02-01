#include "filehandlermanager.h"

#include "mpegfilehandler.h"
#include "vorbisfilehandler.h"
#include "opusfilehandler.h"
#include "mp4filehandler.h"
#include "cuefilehandler.h"

#include <QMultiMap>
#include <QMultiHash>

class FileHandlerManagerPrivate
{
public:
    explicit FileHandlerManagerPrivate(FileHandlerManager *qq);
    ~FileHandlerManagerPrivate();

    QMap<QString, std::function<FileHandlerInterface*()>> m_handlerMap;

    FileHandlerManager *q_ptr;
};

FileHandlerManagerPrivate::FileHandlerManagerPrivate(FileHandlerManager *qq)
    : q_ptr(qq)
{

}

FileHandlerManagerPrivate::~FileHandlerManagerPrivate()
{

}

// ------------------------- Item Get Border Line -------------------------

FileHandlerManager::FileHandlerManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new FileHandlerManagerPrivate(this))
{
    registerFileHandler<MpegFileHandler>("audio/mpeg");
    registerFileHandler<VorbisFileHandler>("audio/x-vorbis+ogg");
    registerFileHandler<OpusFileHandler>("audio/x-opus+ogg");
    registerFileHandler<Mp4FileHandler>("audio/mp4");
    registerFileHandler<CueFileHandler>("application/x-cue");
}

FileHandlerManager::~FileHandlerManager()
{

}

void FileHandlerManager::insertFileHandler(const QString & mimeTypeStr, std::function<FileHandlerInterface *()> fn)
{
    Q_D(FileHandlerManager);

    d->m_handlerMap[mimeTypeStr] = fn;
}

FileHandlerManager *FileHandlerManager::instance()
{
    static FileHandlerManager manager;

    return &manager;
}

FileHandlerInterface *FileHandlerManager::createHandlerByMimeType(const QMimeType &mimeType) const
{
    Q_D(const FileHandlerManager);

    for (const QString & mimeTypeStr : d->m_handlerMap.keys()) {
        if (mimeType.inherits(mimeTypeStr)) {
            return d->m_handlerMap[mimeTypeStr]();
        }
    }

    return nullptr;
}
