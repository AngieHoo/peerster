#include "filemanager.h"

FileManager::FileManager(const QStringList& fileList, QObject *parent)
    :QObject(parent)
{
   addFiles(fileList);
}


FileManager::FileManager(QObject *parent)
    :QObject(parent)
{
}

void FileManager::addFiles(const QStringList & fileList)
{
    for (auto name : fileList) {
        QFile file(name);
        if (!file.open(QFile::ReadOnly)) {
            qDebug() << "Fail to open the file!";
            continue;
        }
        quint64 fileSize = file.size(), leftSize = fileSize;
        QByteArray metafile;
        QCA::Hash shaHash("sha1");
        while (leftSize) {
            quint64 loadSize = leftSize > BLOCK_BYTE_SIZE ? BLOCK_BYTE_SIZE : leftSize;
            QByteArray block = file.read(loadSize);
            leftSize -= loadSize;
            shaHash.update(block);
            QByteArray hashResult = shaHash.final().toByteArray();
            metafile.append(hashResult);
            fileBlocks[hashResult] = block;
        }
        shaHash.update(metafile);
        fileInfoList.push_back(FileInfo(name, metafile, shaHash.final().toByteArray(), fileSize));
    }

}


const QByteArray &FileManager::getBlockAt(const QByteArray & key)
{
    return fileBlocks[key];
}
