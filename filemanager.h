#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "lib.h"
#include "fileinfo.h"
#include <QMap>
#include <QObject>
#include <QVector>
#include <QFile>
#include <QDebug>
#include <QtCrypto>

using namespace peerster;

class FileManager : public QObject
{
    Q_OBJECT
public:
    FileManager(const QStringList&, QObject *parent = 0);
    FileManager(QObject *parent = 0);
    void addFiles(const QStringList&);
    const QByteArray& getBlockAt(const QByteArray&);


private:
    QMap<QByteArray, QByteArray> fileBlocks;
    QVector<FileInfo> fileInfoList;
    //QStringList<string> fileList;

};

#endif // FILEMANAGER_H
