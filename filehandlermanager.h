#pragma once

#include <typeinfo>

#include <QMimeType>
#include <QObject>

class FileHandlerInterface;
class FileHandlerManagerPrivate;
class FileHandlerManager : public QObject
{
    Q_OBJECT

public:
    static FileHandlerManager *instance();

    template <class T>
    void registerFileHandler() {
        insertFileHandler(typeid(T).name(), [=](){
            return static_cast<FileHandlerInterface*>(new T());
        });
    }

    template <class T>
    void registerMimeTypeHandler(const QString & mimeTypeStr,
                                 bool asReader, bool asWriter) {
        insertMimeTypeHandler(mimeTypeStr, typeid(T).name(), asReader, asWriter);
    }

    template <class T>
    void registerExporter(const QString & suffix, const QString & userFilterCopywriting) {
        insertExporter(suffix, userFilterCopywriting, typeid(T).name());
    }

    void insertMimeTypeHandler(const QString & mimeTypeStr, const QString & handlerId,
                                 bool asReader, bool asWriter);
    void insertExporter(const QString & suffix, const QString & userFilterCopywriting,
                        const QString & handlerId);

    FileHandlerInterface *createReadHandlerByMimeType(const QMimeType &mimeType) const;
    FileHandlerInterface *createWriteHandlerByMimeType(const QMimeType &mimeType) const;
    FileHandlerInterface *createExportHandlerBySuffix(const QString &suffix) const;
    QList<QPair<QString, QString> > exporterList() const; // <suffix, copywriting>

signals:

private:
    explicit FileHandlerManager(QObject *parent = nullptr);
    ~FileHandlerManager();

    void insertFileHandler(const QString & handlerId, std::function<FileHandlerInterface *()> fn);

    QScopedPointer<FileHandlerManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(FileHandlerManager)
};
