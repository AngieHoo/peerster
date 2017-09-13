
#include "netsocket.h"


NetSocket::NetSocket(QObject *parent) : QUdpSocket(parent)
{
    // Pick a range of four UDP ports to try to allocate by default,
    // computed based on my Unix user ID.
    // This makes it trivial for up to four Peerster instances per user
    // to find each other on the same host,
    // barring UDP port conflicts with other applications
    // (which are quite possible).
    // We use the range from 32768 to 49151 for this purpose.
    myPortMin = 32768 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 3;
    myPort = -1;

}


void NetSocket::createNeighbors() {
    if (myPort > myPortMin)
        neighbors.push_back(myPort - 1);
    if (myPort < myPortMax)
        neighbors.push_back(myPort + 1);
    return;
}

bool NetSocket::bind()
{
    // Try to bind to each of the range myPortMin..myPortMax in turn.
    for (int p = myPortMin; p <= myPortMax; p++) {
        if (QUdpSocket::bind(p)) {
            myPort = p;
            createNeighbors();
            qDebug() << "bound to UDP port " << p;
            return true;
        }
    }

    qDebug() << "Oops, no ports in my default range " << myPortMin
             << "-" << myPortMax << " available";
    return false;
}
void NetSocket::sendMessage(const QHostAddress& sender, const quint16 senderPort,const QVariantMap& message) {
    QByteArray data;
    QDataStream str(&data, QIODevice::WriteOnly);
    for (QVariantMap::const_iterator it = message.begin(); it != message.end(); it++) {
        str << it.key();
        str << it.value();
        qDebug() << "NetSocket::sendMessage" << it.key() << it.value();
    }
    writeDatagram(data,data.length(), sender, senderPort);
    return;
}
void NetSocket::sendMsg2RandomPeer(const QVariantMap& message) {
    QByteArray data;
    QDataStream str(&data, QIODevice::WriteOnly);
    for (QVariantMap::const_iterator it = message.begin(); it != message.end(); it++) {
        str << it.key() << it.value();
        qDebug() << "NetSocket::sendMsg2RandomPeer" << it.key() << it.value();
    }
    // randomly pick up a neighbor to send message: QTime t;
    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int pickNo = neighbors.at(qrand() % neighbors.size());
    qDebug() << "send message to port" << pickNo;
    //qDebug() << "send:" << data ;
    writeDatagram(data, data.length(),QHostAddress::LocalHost, pickNo);
    return;
}
