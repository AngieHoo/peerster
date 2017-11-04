#include "filemanager.h"
//QString fromQByteArrayToQString_16(const QByteArray& ba);

FileManager::FileManager(const QStringList& fileList, QObject *parent)
    :QObject(parent)
{
   upLoadFiles(fileList);
}


FileManager::FileManager(QObject *parent)
    :QObject(parent)
{
}

void FileManager::upLoadFiles(const QStringList & fileList)
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
        qDebug() << "fileName:" << name << ", size: " << fileSize << "metaFile:";
        while (leftSize) {
            quint64 loadSize = leftSize > BLOCK_BYTE_SIZE ? BLOCK_BYTE_SIZE : leftSize;
            QByteArray block = file.read(loadSize);
            leftSize -= loadSize;
            shaHash.update(block);
            QByteArray hashResult = shaHash.final().toByteArray();
            metafile.append(hashResult);
            //fileBlocks[hashResult] = block;
            qDebug() << "size:" << hashResult.size() << "\""<< hashResult << "\"";
            shaHash.clear();
        }
        shaHash.update(metafile);
        QByteArray metaHash = shaHash.final().toByteArray();
        qDebug() << "metaFile Hash Value:" << metaHash << " toHex: " << metaHash.toHex();
        fileInfoList[metaHash] = (FileInfo(name, metafile, metaHash, fileSize));
    }

}


QByteArray FileManager::getBlockAt(const QByteArray& key)
{
   QByteArray res;
   qDebug() << "fileList:" << fileInfoList.size();
    for (auto file : fileInfoList) {
        qDebug() << "name:" << file.fileName << ",metaHashValue" << file.metaHashVal << "size:" << file.size;
        qDebug() << "block count" << file.size / BLOCK_BYTE_SIZE << ", " << file.metafile.size() / HASH_SIZE;
        if (file.metaHashVal == key) return file.metafile;
        int index = 0;
        QFile localFile(file.fileName);
        if (!localFile.open(QFile::ReadOnly)) {
            qDebug() << "Fail to open the file!";
            continue;
        }
        while (index < localFile.size()){
            qDebug() << index;
            int len = (file.size - index >= BLOCK_BYTE_SIZE) ? BLOCK_BYTE_SIZE : file.size - index;
            qDebug() << "readSize:" << len;
            QByteArray blockHash = file.metafile.mid(index / BLOCK_BYTE_SIZE * HASH_SIZE, HASH_SIZE);
            res = localFile.read(len);
            qDebug() << res;
            if (blockHash == key) {
                qDebug() << "found the flie block!!!!!!!!";
                localFile.close();
                return res;
            }
            index += BLOCK_BYTE_SIZE;
        }
    }
    return res;
}

QVector<QPair<QString, QByteArray>> FileManager::findMatchedFiles(const QStringList &keyWords)
{
    QVector<QPair<QString, QByteArray>> res;
    for (auto file : fileInfoList) {
        for (auto kw : keyWords) {
            if (file.fileName.contains(kw)){
                QString newName = file.fileName.mid(file.fileName.lastIndexOf(QRegExp("[\\/]")) + 1);
                res.push_back(QPair<QString, QByteArray>(newName, file.metaHashVal));
                qDebug() << "find matched files : " << newName;
                break;
            }
        }
    }

    return res;
}


