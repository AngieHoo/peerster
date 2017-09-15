#include "peer.h"

Peer::Peer(const QString& DNS, const QHostAddress& IP, const quint16& Port)
    : DNS(DNS), IP(IP), Port(Port){}

Peer::Peer(){
    DNS = QHostInfo::localHostName();
    IP = QHostAddress::LocalHost;
    Port = -1;
}

QString Peer::getDNS() const {
    return DNS;
}
QHostAddress Peer::getIP() const {
    return IP;
}

quint16 Peer::getPort() const {
    return Port;
}

void Peer::setDNS(const QString& DNS){
    this->DNS = DNS;
}

void Peer::setIP(const QHostAddress& IP){
    this->IP = IP;
}

void Peer::SetPort(const quint16& Port){
    this->Port = Port;
}



