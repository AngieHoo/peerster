#include "peer.h"

Peer::Peer(const QHostAddress& IP, const quint16& Port, QObject *parent, const QString& DNS)
    : QObject(parent), DNS(DNS), IP(IP), Port(Port){
    timer.setSingleShot(true);
    timer.setInterval(2000);
    connect(&timer, SIGNAL(timeout()), this, SLOT(sendTimeOutMsg()));
}

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


void Peer::sendTimeOutMsg(){
    emit timerOut(this);
}
void Peer::startTimer(){
    timer.start();
}

void Peer::stopTimer(){
    timer.stop();
}

void Peer::setMessage(const QVariantMap& message){
    this->message = message;
}

QVariantMap Peer::getMessage() const{
    return message;
}




