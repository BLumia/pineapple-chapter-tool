#include "filehandlermanager.h"

#include "mpegfilehandler.h"
#include "vorbisfilehandler.h"
#include "opusfilehandler.h"
#include "mp4filehandler.h"
#include "cuefilehandler.h"
#include "ytdlfilehandler.h"

#include <QMultiMap>
#include <QMultiHash>

class FileHandlerManagerPrivate
{
public:
    explicit FileHandlerManagerPrivate(FileHandlerManager *qq);
    ~FileHandlerManagerPrivate();

    QMap<QString, std::function<FileHandlerInterface*()>> m_handlerIdMap; // <HandlerName, creator>
    QMap<QString, QString> m_readerMap; // <MimeTypeStr, HandlerName>
    QMap<QString, QString> m_writerMap; // <MimeTypeStr, HandlerName>
    QMap<QString, QString> m_textImporterMap; // <TxtTypeIdentifier, HandlerName>
    QMap<QString, QPair<QString, QString> > m_exporterMap; // <SuffixStr, <HandlerName, Copywriting> >

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
    registerFileHandler<MpegFileHandler>();
    registerFileHandler<VorbisFileHandler>();
    registerFileHandler<OpusFileHandler>();
    registerFileHandler<Mp4FileHandler>();
    registerFileHandler<CueFileHandler>();
    registerFileHandler<YtdlFileHandler>();

    registerMimeTypeHandler<MpegFileHandler>("audio/mpeg", true, true);
    registerMimeTypeHandler<VorbisFileHandler>("audio/x-vorbis+ogg", true, true);
    registerMimeTypeHandler<OpusFileHandler>("audio/x-opus+ogg", true, true);
    registerMimeTypeHandler<Mp4FileHandler>("audio/mp4", true, true);
    registerMimeTypeHandler<CueFileHandler>("application/x-cue", true, true);
    registerMimeTypeHandler<YtdlFileHandler>("application/json", true, false);

    registerExporter<OpusFileHandler>("*.ogm.txt", tr("OGM Style TXT Files"));
    registerExporter<YtdlFileHandler>("*.info.json", tr("youtube-dl Style Json Files"));
}

FileHandlerManager::~FileHandlerManager()
{

}

void FileHandlerManager::insertFileHandler(const QString & handlerId, std::function<FileHandlerInterface *()> fn)
{
    Q_D(FileHandlerManager);

    d->m_handlerIdMap[handlerId] = fn;
}

FileHandlerManager *FileHandlerManager::instance()
{
    static FileHandlerManager manager;

    return &manager;
}

void FileHandlerManager::insertMimeTypeHandler(const QString &mimeTypeStr, const QString &handlerId, bool asReader, bool asWriter)
{
    Q_D(FileHandlerManager);

    if (asReader) d->m_readerMap[mimeTypeStr] = handlerId;
    if (asWriter) d->m_writerMap[mimeTypeStr] = handlerId;
}

void FileHandlerManager::insertExporter(const QString &suffix, const QString & userFilterCopywriting,
                                          const QString &handlerId)
{
    Q_D(FileHandlerManager);

    d->m_exporterMap[suffix] = qMakePair(handlerId, userFilterCopywriting);
}

FileHandlerInterface *FileHandlerManager::createReadHandlerByMimeType(const QMimeType &mimeType) const
{
    Q_D(const FileHandlerManager);

    for (const QString & mimeTypeStr : d->m_readerMap.keys()) {
        if (mimeType.inherits(mimeTypeStr)) {
            return d->m_handlerIdMap[d->m_readerMap[mimeTypeStr]]();
        }
    }

    return nullptr;
}

FileHandlerInterface *FileHandlerManager::createWriteHandlerByMimeType(const QMimeType &mimeType) const
{
    Q_D(const FileHandlerManager);

    for (const QString & mimeTypeStr : d->m_writerMap.keys()) {
        if (mimeType.inherits(mimeTypeStr)) {
            return d->m_handlerIdMap[d->m_writerMap[mimeTypeStr]]();
        }
    }

    return nullptr;
}

FileHandlerInterface *FileHandlerManager::createExportHandlerBySuffix(const QString &suffix) const
{
    Q_D(const FileHandlerManager);

    return d->m_handlerIdMap[d->m_exporterMap[suffix].first]();
}

QList<QPair<QString, QString> > FileHandlerManager::exporterList() const
{
    Q_D(const FileHandlerManager);

    QList<QPair<QString, QString> > result;

    for (const QString & suffix : d->m_exporterMap.keys()) {
        result.append(qMakePair(suffix,
                                QString("%1 (%2)").arg(d->m_exporterMap[suffix].second,
                                                       suffix)));
    }

    return result;
}
