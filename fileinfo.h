#ifndef FILEINFO_H
#define FILEINFO_H
#include <QByteArray>
#include <QString>

class FileInfo
{
public:
    FileInfo(const QString&, const QByteArray&, const QByteArray&, quint64);
    FileInfo(FileInfo& );
    FileInfo(FileInfo&& );
    QString getFileName() const;
    QByteArray getMetafile() const;
    QByteArray getMetaHashVal() const;
    quint64 getFileSize() const;

private:
    QByteArray metafile;
    QString fileName;
    QByteArray metaHashVal;
    quint32 size;

};

#endif // FILEINFO_H
