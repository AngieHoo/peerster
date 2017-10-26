#include "fileinfo.h"

 FileInfo::FileInfo(const QString& fileName, const QByteArray& metafile, const QByteArray& metaHashVal, quint64 size)
     : metafile(metafile), fileName(fileName), metaHashVal(metaHashVal), size(size)
 {}

 FileInfo::FileInfo(FileInfo & f)
 {
     fileName = f.fileName;
     metafile = f.metafile;
     metaHashVal = f.metaHashVal;
     size = f.size;
 }

 FileInfo::FileInfo(FileInfo && mf)
 {
     fileName = std::move(mf.fileName);
     metafile = std::move(mf.metafile);
     metaHashVal = std::move(mf.metaHashVal);
     size = std::move(mf.size);
 }


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
