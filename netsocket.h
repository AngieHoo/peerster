#ifndef NETSOCKET_H
#define NETSOCKET_H

#include <QUdpSocket>
#include <QVariantMap>
#include <QApplication>
#include <QDebug>
#include <QDataStream>
#include <unistd.h>
#include <QTime>

//#include <QHostInfo>

class NetSocket : public QUdpSocket
{
    Q_OBJECT
private slots:
    //void readPendingDatagrams();

public:
    NetSocket(QObject *parent = 0);

    void sendMessage(const QHostAddress& sender, quint16 senderPort,const QVariantMap& message);
    bool bind(quint16 p);

};

#endif // NETSOCKET_H
