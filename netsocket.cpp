
#include "netsocket.h"


NetSocket::NetSocket(QObject *parent) : QUdpSocket(parent){
    connect(this, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

bool NetSocket::bind(quint16 p)
{
    if (QUdpSocket::bind(p)) {
        return true;
    }
    return false;
}

//void NetSocket::readPendingDatagrams() {
//    while (sock->hasPendingDatagrams()) {
//        QByteArray datagram;
//        datagram.resize(sock->pendingDatagramSize());
//        QHostAddress sender;
//        quint16 senderPort;
//        sock->readDatagram(datagram.data(), datagram.size(),
//                           &sender, &senderPort);
//        processTheDatagram(datagram, sender, senderPort);
//    }
//    return;
//}

void NetSocket::sendMessage(const QHostAddress& sender, quint16 senderPort,const QVariantMap& message) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << message;
    qDebug() <<"NetSocket::sendMessage to neighbor:" << sender << ":" << senderPort;
    qDebug() << "NetSocket::Massage content:" << message;
    writeDatagram(data,sender, senderPort);
    return;
}
