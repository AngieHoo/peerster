#include "fileinfo.h"

 FileInfo::FileInfo(const QString& fileName, const QByteArray& metafile, const QByteArray& metaHashVal, quint64 size)
     : metafile(metafile), fileName(fileName), metaHashVal(metaHashVal), size(size)
 {}

QString FileInfo::getFileName() const
{
    return fileName;
}

QByteArray FileInfo::getMetafile() const
{
    return metafile;
}

QByteArray FileInfo::getMetaHashVal() const
{
    return metaHashVal;
}

quint64 FileInfo::getFileSize() const
{
    return size;
}
