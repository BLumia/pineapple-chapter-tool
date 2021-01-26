#include "filehandlermanager.h"

#include "mpegfilehandler.h"
#include "vorbisfilehandler.h"
#include "opusfilehandler.h"
#include "mp4filehandler.h"

#include <QMultiMap>
#include <QMultiHash>

class FileHandlerManagerPrivate
{
public:
    explicit FileHandlerManagerPrivate(FileHandlerManager *qq);

    FileHandlerManager *q_ptr;
};

FileHandlerManagerPrivate::FileHandlerManagerPrivate(FileHandlerManager *qq)
    : q_ptr(qq)
{

}

// ------------------------- Item Get Border Line -------------------------

FileHandlerManager::FileHandlerManager(QObject *parent) : QObject(parent)
{
    registerFileHandler<MpegFileHandler>(QStringLiteral("audio/mpeg"));
    registerFileHandler<VorbisFileHandler>(QStringLiteral("audio/x-vorbis+ogg"));
    registerFileHandler<OpusFileHandler>(QStringLiteral("audio/x-opus+ogg"));
    registerFileHandler<Mp4FileHandler>(QStringLiteral("audio/mp4"));
}

FileHandlerManager *FileHandlerManager::instance()
{
    static FileHandlerManager manager;

    return &manager;
}

FileHandlerInterface *FileHandlerManager::createHandlerByMimeType(const QMimeType &mimeType) const
{
    // TODO: should be replaced to actual factory logic here.
    if (mimeType.inherits("audio/mpeg")) {
        return new MpegFileHandler;
    } else if (mimeType.inherits("audio/x-vorbis+ogg")) {
        return new VorbisFileHandler;
    } else if (mimeType.inherits("audio/x-opus+ogg")) {
        return new OpusFileHandler;
    } else if (mimeType.inherits("audio/mp4")) {
        return new VorbisFileHandler;
    }

    return nullptr;
}
