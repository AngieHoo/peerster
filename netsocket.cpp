
#include "netsocket.h"


NetSocket::NetSocket(QObject *parent) : QUdpSocket(parent){}

bool NetSocket::bind(quint16 p)
{
    if (QUdpSocket::bind(p)) {
        return true;
    }
    return false;
}

void NetSocket::sendMessage(const QHostAddress& sender, const quint16 senderPort,const QVariantMap& message) {
    QByteArray data;
    QDataStream str(&data, QIODevice::WriteOnly);
    qDebug() <<"NetSocket::sendMessage to neighbor:" << sender << ":" << senderPort;
    for (QVariantMap::const_iterator it = message.begin(); it != message.end(); it++) {
        str << it.key();
        str << it.value();
        qDebug() << "NetSocket::content:" << it.key() << it.value();
    }
    writeDatagram(data,data.length(), sender, senderPort);
    return;
}
