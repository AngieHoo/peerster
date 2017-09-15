#ifndef PEER_H
#define PEER_H
#include <QVector>
#include <QHostAddress>
#include <QHostInfo>

class Peer
{
public:
    Peer(const QString& DNS, const QHostAddress& IP, const quint16& Port);
    Peer();
    QString getDNS() const;
    QHostAddress getIP() const;
    quint16 getPort() const;
    void setDNS(const QString&);
    void setIP(const QHostAddress&);
    void SetPort(const quint16&);
private:
    QString DNS;
    QHostAddress IP;
    quint16 Port;

};

#endif // PEER_H
