#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QMap>
#include <QObject>
#include <QVector>
#include <QFile>
#include <QDebug>
#include <QtCrypto>
#include "lib.h"
//#include "fileinfo.h"

using namespace peerster;



class FileManager : public QObject
{
    Q_OBJECT
public:
    FileManager(const QStringList&, QObject *parent = 0);
    FileManager(QObject *parent = 0);
    void upLoadFiles(const QStringList&);
    QByteArray getBlockAt(const QByteArray&);
    QVector<QPair<QString, QByteArray>> findMatchedFiles(const QStringList &);


private:
    QMap<QByteArray ,FileInfo> fileInfoList;


};

#endif // FILEMANAGER_H
