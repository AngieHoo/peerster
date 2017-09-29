#include "control.h"

Control::Control(QObject *parent) : QObject(parent)
{
    sock = new NetSocket(this);
    connect(sock, SIGNAL(processTheDatagram(QByteArray,QHostAddress,quint16)), SLOT(processTheDatagram(QByteArray,QHostAddress,quint16)));

    model = new Model(this);

    connect(model, SIGNAL(displayNewNeighbor(const QString&,const QHostAddress&,const quint16&)), this, SIGNAL(displayNewNeighbor(const QString&,const QHostAddress&,const quint16&)));

    timer = new QTimer(this);
    timer->setInterval(10000);
    timerRoute = new QTimer(this);
    timerRoute->setInterval(6000);

    connect(timer, SIGNAL(timeout()), this, SLOT(doAntiEntropy()));
    connect(timer, SIGNAL(timeout()), this, SLOT(sendRouteMessage()));

}

void Control::start(){
     bind();
     model->creatLocalNeighbors();
     timer->start();
     timerRoute->start();

     QStringList commond = QCoreApplication::arguments();
     if (commond.size() > 1 && commond[1] != ""){
         QString originNeighbor = commond[1];
         QString address = originNeighbor.left(originNeighbor.indexOf(':'));
         quint16 port = originNeighbor.mid(originNeighbor.indexOf(':') + 1).toInt();
         checkInputNeighbor(address, port);
     }
     sendRouteMessage();
}

void Control::bind(){
    for (int p = model->getMyPortMin(); p <= model->getMyPortMax(); p++) {
        if (sock->bind(p)) {
            qDebug() << "bound to UDP port " << p;
            model->setMyPort(p);
            return;
        }
    }
    exit(1);
}

void Control::checkInputNeighbor(const QString& address,const quint16& port){
    QHostAddress self(QHostAddress::LocalHost);
    qDebug() << "Input Neighbor's Address: " ;
    QHostAddress testIP;
    if (testIP.setAddress(address)){ // is a IP address.
        QHostInfo host = QHostInfo::fromName(address);
        if (host.error() != QHostInfo::NoError) {
            qDebug() << "Lookup failed:" << host.errorString();
        }
        else {
            qDebug()  << address << ", Port: " << port;
            if (model->isValidNewComer(host.hostName(), testIP, port)) {
                Peer* peer = model->addNeighbor(host.hostName(), testIP, port);
                connect(peer, SIGNAL(timerOut(const Peer*)),this, SLOT(processNoReply(const Peer*)));
            }
        }
    }
    else {
        QEventLoop eventloop;
        connect(this, SIGNAL(finishLookUp()), &eventloop,SLOT(quit()));
        int id = QHostInfo::lookupHost(address, this, SLOT(lookedUp(QHostInfo)));
        eventloop.exec();
        if (hostInfo.addresses().size() > 0) {
            qDebug()  << hostInfo.addresses().at(0)  << ", Port: " << port;
            if (model->isValidNewComer(hostInfo.hostName(), hostInfo.addresses().at(0), port)) {
                Peer* peer = model->addNeighbor(hostInfo.hostName(), hostInfo.addresses().at(0), port);
                connect(peer, SIGNAL(timerOut(const Peer*)),this, SLOT(processNoReply(const Peer*)));
            }
        }
        else{
            QMessageBox::about(NULL, "Warning", "Invalid Address!");
        }
    }
}

void Control::sendMyMessage(const QString& content) {
    model->addMyMessage(content);
    QVariantMap message;
    message["ChatText"] = content;
    message["Origin"] = model->getIdentity();
    message["SeqNo"] = model->getMySeqNo();
    brocastMessage(message);
    sendRouteMessage();
    return;
}

void Control::sendOriginMessage(const QHostAddress& IP, const quint16& port, const QString& content,
                                const QString& originID,const quint32& seq){
    QVariantMap newMessage;
    newMessage["ChatText"] = content;
    newMessage["Origin"] = originID;
    newMessage["SeqNo"] = seq;
    qDebug() << "send to" << IP << port <<"," << content << "," << originID << "," << seq;
    sendMsg2Peer(model->getNeighbor(IP, port), newMessage);
}

void Control::brocastMessage(const QVariantMap& message) {
    qDebug() << "brocast message";
    Peer* randomPeer = model->getPeerRandomly();
    qDebug() << "pick peer:" << randomPeer->getIP() << randomPeer->getPort();
    sendMsg2Peer(randomPeer, message);
    return;
}

void Control::sendMyPrivateMessage(const QString &content)
{
    sendPrivateMessage(model->getPrivateChattingPeer(), model->getIdentity(), content, 10);
}

void Control::sendPrivateMessage(const QString &destinationID, const QString &originID, const QString &content, int hop)
{
    QVariantMap message;
    message["Dest"] = destinationID;
    message["Origin"] = originID;
    message["ChatText"] = content;
    message["HopLimit"] = hop;
    QHostAddress IP = model->getRoutingTable()[destinationID].first;
    quint16 port = model->getRoutingTable()[destinationID].second;
    sock->sendMessage(IP, port, message);
}

void Control::processStatusMessage(const QVariantMap &myStatuslist, const QVariantMap &message, const QHostAddress& IP, const quint16& port)
{
    qDebug() << "the message type is [Status]";

    // stop the sender's timer:
    Peer* peer = model->getNeighbor(IP, port);
    peer->stopTimer();

    QVariantMap senerStatusList;
    senerStatusList = message["Want"].toMap();
    qDebug() << "sender status: " << senerStatusList;
    bool flagNew = false; // flag if i have some newer status to send to the IP.
    for (QVariantMap::const_iterator it = myStatuslist.begin(); it != myStatuslist.end(); it++) {
        int seq = senerStatusList[it.key()].toInt() == 0 ? 1 : senerStatusList[it.key()].toInt();
        if (seq <= it.value().toInt()) {
            sendOriginMessage(IP, port, model->getMessagelist()[it.key()][seq], it.key(), seq);
            qDebug() << "I send him my new message. my status:" << myStatuslist;
        }
    }
    if (flagNew) return;
    else{ // if I have nothing new for the IP, then check if the IP contains new message that i have not received
        for (QVariantMap::iterator it = senerStatusList.begin(); it != senerStatusList.end(); it++) {
            if (myStatuslist[it.key()].toInt() + 1 < it.value().toInt()) { // the IP's status is newer than mine.
                qDebug() << "send my status for newer message.";
                sendMyStatusList(IP, port);
                return;
            }
        }
        flipCoins(); //we have exactly the same status, I pick up a random neighbor to send my status to it.
    }
}

void Control::processGroupChatMessage(const QVariantMap &myStatuslist, const QVariantMap &message, const QHostAddress& IP, const quint16& port)
{
    QString originID = message["Origin"].toString();
    quint32 SeqNo = message["SeqNo"].toInt();
    QString content = message["ChatText"].toString();
    qDebug() << "message type is ChatText:" << content <<",OriginID:" << originID << ", SeqNo:" << SeqNo;
    if (myStatuslist[originID].toInt() + 1 <= SeqNo) {
        if (!model->getRoutingTable().count(originID)  && originID != model->getIdentity()) // new orignID
            emit comeNewOriginID(originID);
        model->updateRoutingTable(originID, IP, port);
        brocastMessage(message); // send message to a random neighbor.
        if (myStatuslist[originID].toInt() + 1 == SeqNo) {
            emit displayNewMessage(originID + ":" + content);
            model->receiveNewMessage(content, originID);
            qDebug() << "I receive a new message with the right sequence!";
        }
    }
    sendMyStatusList(IP, port);// reply my statuslist;
}

void Control::processPrivateMessage(const QVariantMap &myStatuslist, const QVariantMap &message, const QHostAddress& IP, const quint16& port)
{

    QString destID = message["Dest"].toString();
    QString originID = message["Origin"].toString();
    QString content = message["ChatText"].toString();
    int hotLimit = message["HotLimit"].toInt();

    qDebug() << "message type is privateMessage:" << content << ", DestID: " << destID << "OriginID:" << originID;

    if (destID == model->getIdentity()) {
        emit displayNewMessage(originID + ":" + content);
        return;
    }

    if (hotLimit < 1) return;

    hotLimit--;
    sendPrivateMessage(destID, originID, content, hotLimit);

}

void Control::processRouteMessage(const QVariantMap &myStatuslist, const QVariantMap &message, const QHostAddress &IP, const quint16 &port)
{
    QString originID = message["Origin"].toString();
    quint32 SeqNo = message["SeqNo"].toInt();
    qDebug() << "message type is RouteMessage:" << "OriginID:" << originID << ", SeqNo:" << SeqNo;
    if (myStatuslist[originID].toInt() + 1 <= SeqNo) {
        if (!model->getRoutingTable().count(originID) && originID != model->getIdentity()) // new orignID
            emit comeNewOriginID(originID);
        model->updateRoutingTable(originID, IP, port);
        brocastMessage(message); // brocast route message
    }
}

void Control::sendMsg2Peer(Peer* peer, const QVariantMap& message) {
    sock->sendMessage(peer->getIP(), peer->getPort(), message);
    peer->setMessage(message);
    peer->startTimer();
}

void Control::sendMyStatusList(const QHostAddress& sender, const quint16 senderPort) {
    QVariantMap myStatusList = model->getStatusList();
    for (QVariantMap::iterator it = myStatusList.begin(); it != myStatusList.end(); it++) {
        myStatusList[it.key()] = it.value().toInt() + 1;
    }
    QVariantMap want;
    want["Want"] = myStatusList;
    qDebug() << "send my statuslist:" << want;
    sock->sendMessage(sender, senderPort, want);
    return;
}

void Control::flipCoins(){
    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int flag = qrand() % 2;
    if (flag) return;
    qDebug() << "flip coins!!!";
    Peer* peer = model->getPeerRandomly();
    sendMyStatusList(peer->getIP(), peer->getPort());
}

void Control::lookedUp(const QHostInfo &host)
{
    hostInfo = host;
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        emit finishLookUp();
        return;
    }
    hostInfo = host;
    emit finishLookUp();
}

void Control::doAntiEntropy(){
    Peer* peer = model->getPeerRandomly();
    sendMyStatusList(peer->getIP(), peer->getPort());
}

void Control::sendRouteMessage()
{
    QVariantMap routeMessage;
    routeMessage["Origin"] = model->getIdentity();
    routeMessage["SeqNo"] = model->getMySeqNo() + 1;
    brocastMessage(routeMessage);
}

void Control::processNoReply(const Peer* peer){
    QVariantMap message = peer->getMessage();
    Peer* p = model->getPeerRandomly();
    sendMsg2Peer(p, message);
}

void Control::addPrivateChatPeer(const QString & p)
{
    model->setPrivateChattingPeer(p);
}

void Control::processTheDatagram(const QByteArray& datagram, const QHostAddress& IP, const quint16& port) {
    qDebug() << "receive message from sender: " << IP.toString() << ", port:" << port;

    QHostInfo host = QHostInfo::fromName(IP.toString());
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }
    qDebug() << "sender's DNS" << host.hostName();

    //check if it's a new comer

    if (model->isValidNewComer(host.hostName(), IP, port)) {
        Peer* peer = model->addNeighbor(host.hostName(), IP, port);
        connect(peer, SIGNAL(timerOut(const Peer*)),this, SLOT(processNoReply(const Peer*)));
    }

    QDataStream in(datagram);
    QVariantMap message, myStatuslist = model->getStatusList();
    in >> message;

    if (message.contains("Want")) {// a status message
       processStatusMessage(myStatuslist,message, IP, port);
    }
    else if (message.contains("ChatText") && message.contains("Origin")
             && message.contains("SeqNo")) { // brocast message to a random neighbor
       processGroupChatMessage(myStatuslist,message, IP, port);
    }
    else if (message.contains("Origin") && message.contains("SeqNo")) {
        processRouteMessage(myStatuslist,message, IP, port);
    }
    else if (message.contains("Dest") && message.contains("Origin")
             && message.contains("ChatText") && message.contains("HopLimit")) {
        processPrivateMessage(myStatuslist,message, IP, port);
    }
    else {
        qDebug() << message.begin().key();
        qDebug() << "information dammaged!";
    }
    return;
}

QString Control::getIdentity(){
    return model->getIdentity();
}
