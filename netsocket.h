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

public:
    NetSocket(QObject *parent = 0);

    // Bind this socket to a Peerster-specific default port.
    bool bind();
    void sendMessage(const QHostAddress& sender, const quint16 senderPort,const QVariantMap& message);
    void sendMsg2RandomPeer(const QVariantMap& message);

private:
    int myPortMin, myPortMax, myPort;
    QVector<int> neighbors;

    void createNeighbors();
};

#endif // NETSOCKET_H
