#include "model.h"

Model::Model(QObject *parent) : QObject(parent)
{
    mySeqNo= 0;
    QTime t = QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    identity = QString::number(qrand());

    myPortMin = 32768 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 3;
    myPort = -1;
   // connect(this, SIGNAL(test()),this, SLOT(respondtest()));

}
const QVariantMap& Model::getStatusList() const{
    return statusList;
}
const QMap<QString,QMap<quint32, QString>>& Model::getMessagelist() const{
    return messageList;
}

const QString Model::getPrivateChattingPeer() const
{
    return privateChatDestID;
}
Peer* Model::getPeerRandomly() const{
    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int pickNo = qrand() % neighbors.size();
    return neighbors[pickNo];
}

const QString& Model::getIdentity() const{
    return identity;
}

quint32 Model::getMySeqNo() const{
    return mySeqNo;
}

const QHash<QString, QPair<QHostAddress, quint16> >& Model::getRoutingTable() const
{
    return routingTable;
}

quint16 Model::getMyPortMin() const{
    return myPortMin;
}
quint16 Model::getMyPort() const{
    return myPort;
}
quint16 Model::getMyPortMax() const{
    return myPortMax;
}
void Model::setMyPort(quint16 p){
    myPort = p;
}

void Model::setMyPortMin(quint16 p){
    myPortMin = p;
}

void Model::setMyPortMax(quint16 p){
    myPortMax = p;
}

void Model::setPrivateChattingPeer(const QString & pcp)
{
    privateChatDestID = pcp;
}

void Model::updateRoutingTable(const QString &originID, const QHostAddress &senderIP, quint16 senderPort)
{
    routingTable[originID].first = senderIP;
    routingTable[originID].second = senderPort;
}

bool Model::isValidNewComer(const QString& DNS, const QHostAddress& IP, const quint16& Port){
    QHostAddress self(QHostAddress::LocalHost);
    if (IP.toIPv4Address() == self.toIPv4Address() && Port == myPort) {
        QMessageBox::about(NULL, "Warning", "Please don't add yourself!");
        return false;
    }
    if (getNeighbor(IP, Port) != NULL) return false;
    return true;
}

Peer* Model::addNeighbor(const QString& DNS, const QHostAddress& IP, const quint16& Port){
    qDebug() <<  "Add the neighbor: " << IP << ": " << Port;
    Peer* peer = new Peer(DNS, IP, Port, this);
    neighbors.push_back(peer);
    emit displayNewNeighbor(DNS, IP, Port);
    qDebug() << "Model::displayNewNeighbor()";
    return peer;
}

Peer* Model::getNeighbor(const QHostAddress& IP, const quint16& Port){
    for (QVector<Peer*>::iterator it = neighbors.begin(); it != neighbors.end(); it++) {
        QHostAddress nIP = (*it)->getIP();
        quint16 np = (*it)->getPort();
        if (nIP.toIPv4Address() == IP.toIPv4Address() && np == Port) {
            return *it;
        }
    }
    return NULL;
}

void Model::addMyMessage(const QString& content){
    mySeqNo++;
    statusList[identity] = mySeqNo;
    messageList[identity][mySeqNo] = content;
    qDebug() << "update list:" << statusList << messageList;
}

void Model::receiveNewMessage(const QString& content, const QString& originID) {
    statusList[originID] = statusList[originID].toInt() + 1;
    messageList[originID][statusList[originID].toInt()] = content;
    qDebug() << "update list:" << statusList << messageList;
}

void Model::creatLocalNeighbors(){
    for (quint16 p = myPortMin; p <= myPortMax; p++) {
        if (myPort == p) continue;
        addNeighbor(QHostInfo::localHostName(), QHostAddress::LocalHost, p);
    }
}

//void Model::respondtest(){
//    qDebug() << "respond";
//}
