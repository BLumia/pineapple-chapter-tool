#pragma once

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
    void registerFileHandler(const QString & mimeTypeStr) {
        insertFileHandler(mimeTypeStr, [=](){
            return static_cast<FileHandlerInterface*>(new T());
        });
    }

    FileHandlerInterface *createHandlerByMimeType(const QMimeType &mimeType) const;

signals:

private:
    explicit FileHandlerManager(QObject *parent = nullptr);
    ~FileHandlerManager();

    void insertFileHandler(const QString & mimeTypeStr, std::function<FileHandlerInterface *()> fn);

    QScopedPointer<FileHandlerManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(FileHandlerManager)
};
