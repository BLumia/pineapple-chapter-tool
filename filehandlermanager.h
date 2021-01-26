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
    void registerFileHandler(const QString &mimeTypeStr) {
        // TODO: to be implement
    }

    FileHandlerInterface *createHandlerByMimeType(const QMimeType &mimeType) const;

signals:

private:
    explicit FileHandlerManager(QObject *parent = nullptr);

    Q_DECLARE_PRIVATE(FileHandlerManager)
};
