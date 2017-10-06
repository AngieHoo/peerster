#include "model.h"

Model::Model(QObject *parent) : QObject(parent)
{
    QHostInfo info=QHostInfo::fromName(QHostInfo::localHostName());
    myIP = info.addresses().first();
    mySeqNo= 0;
    QTime t = QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    identity = "Angie_" + QString::number(qrand());

    myPortMin = 32768 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 3;
    myPort = -1;
    // connect(this, SIGNAL(test()),this, SLOT(respondtest()));

    qDebug() << "initialize: identity: " << identity << ", IP:" << myIP;

}

const QVariantMap& Model::getStatusList() const{
    return statusList;
}

const QMap<QString,QMap<quint32, QString>>& Model::getMessagelist() const{
    return messageList;
}

const QVector<Peer *> Model::getNeighbors() const
{
    return neighbors;
}

const QString Model::getPrivateChattingPeer() const
{
    return privateChatDestID;
}

Peer* Model::getPeerRandomly() const{
    if (!neighbors.size()) return NULL;
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

 const int Model::getHighestSeq(const QString& originID) const {
     if (originID == identity) return mySeqNo;
     return statusList[originID].toInt();

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
    if (originID == identity || routingTable.contains(originID) && routingTable[originID].first == senderIP && routingTable[originID].second == senderPort) return;
    routingTable[originID].first = senderIP;
    routingTable[originID].second = senderPort;
    qDebug() << "Update routingTable:" << routingTable;
}

bool Model::isValidNewRoutingID(const QString& originID) {
    qDebug() << "routing table:" << routingTable;
    if (routingTable.contains(originID) || originID == identity) return false;
    return true;
}

bool Model::isValidNewComer(const QHostAddress& IP, const quint16& Port) {
    QHostAddress self(QHostAddress::LocalHost);
    if ((IP.toIPv4Address() == myIP.toIPv4Address() || self.toIPv4Address() == IP.toIPv4Address())&& Port == myPort) {
        //QMessageBox::about(NULL, "Warning", "Please don't add yourself!");
        return false;
    }
    if (getNeighbor(IP, Port) != NULL) return false;
    return true;
}

Peer* Model::addNeighbor(const QHostAddress& IP, const quint16& Port){
    qDebug() <<  "Add the neighbor: " << IP << ": " << Port;
    Peer* peer = new Peer(IP, Port, this);
    neighbors.push_back(peer);
    emit displayNewNeighbor(IP, Port);
    //qDebug() << "Model::displayNewNeighbor()";
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
    //qDebug() << "update list:" << statusList << messageList;
}

void Model::addNewMessage(const QString& originID, const QHostAddress& IP, const quint16& port, const QString& content) {
    statusList[originID] = statusList[originID].toInt() + 1;
    messageList[originID][statusList[originID].toInt()] = content;
    //qDebug() << "update list:" << statusList << messageList;
}

void Model::creatLocalNeighbors(){
    for (quint16 p = myPortMin; p <= myPortMax; p++) {
        if (myPort == p) continue;
        addNeighbor(QHostAddress::LocalHost, p);
    }
}

//void Model::respondtest(){
//    qDebug() << "respond";
//}
