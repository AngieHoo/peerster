#include "control.h"

Control::Control(QObject *parent) : QObject(parent)
{
    forward = true;
    sock = new NetSocket(this);
    connect(sock, SIGNAL(processTheDatagram(QByteArray,QHostAddress,quint16)), SLOT(processTheDatagram(QByteArray,QHostAddress,quint16)));

    model = new Model(this);

    connect(model, SIGNAL(displayNewNeighbor(const QHostAddress&,const quint16&)), this, SIGNAL(displayNewNeighbor(const QHostAddress&,const quint16&)));

    timer = new QTimer(this);
    timer->setInterval(10000);
    timerRoute = new QTimer(this);
    timerRoute->setInterval(60000);

    connect(timer, SIGNAL(timeout()), this, SLOT(doAntiEntropy()));
    connect(timerRoute, SIGNAL(timeout()), this, SLOT(generateRouteMessage()));

}

void Control::start(){
    bind();
    //model->creatLocalNeighbors();
    timer->start();
    timerRoute->start();

    QStringList commond = QCoreApplication::arguments();
    int i = 1;
    while (i < commond.size()) {
        if (commond[i] == "-noforward") {
            qDebug() << "set forward: false!";
            forward = false;
        }
        else
        {
            QString originNeighbor = commond[i];
            QString address = originNeighbor.left(originNeighbor.indexOf(':'));
            quint16 port = originNeighbor.mid(originNeighbor.indexOf(':') + 1).toInt();
            checkInputNeighbor(address, port);
        }
        i++;
    }

    generateRouteMessage();
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
    //QHostAddress self(QHostAddress::LocalHost);
    qDebug() << "Input Neighbor's Address: " ;
    QHostAddress testIP;
    if (testIP.setAddress(address)){ // is a IP address.
        QHostInfo host = QHostInfo::fromName(address);
        if (host.error() != QHostInfo::NoError) {
            qDebug() << "Lookup failed:" << host.errorString();
        }
        else {
            qDebug()  << address << ", Port: " << port;
            if (model->isValidNewComer(testIP, port)) {
                Peer* peer = model->addNeighbor(testIP, port);
                connect(peer, SIGNAL(timerOut(Peer*)),this, SLOT(processNoReply(Peer*)));
            }
        }
    }
    else {
        QEventLoop eventloop;
        connect(this, SIGNAL(finishLookUp()), &eventloop,SLOT(quit()));
        QHostInfo::lookupHost(address, this, SLOT(lookedUp(QHostInfo)));
        eventloop.exec();
        if (hostInfo.addresses().size() > 0) {
            qDebug()  << hostInfo.addresses().at(0)  << ", Port: " << port;
            if (model->isValidNewComer(hostInfo.addresses().at(0), port)) {
                Peer* peer = model->addNeighbor(hostInfo.addresses().at(0), port);
                connect(peer, SIGNAL(timerOut(Peer*)),this, SLOT(processNoReply(Peer*)));
            }
        }
        else{
            QMessageBox::about(NULL, "Warning", "Invalid Address!");
        }
    }
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

void Control::sendMyMessage(const QString& content) {
    model->addMyMessage(content);
    QVariantMap message;
    message[CHAT_TEXT] = content;
    message[ORIGIN] = model->getIdentity();
    message[SEQ_NO] = model->getMySeqNo();
    qDebug() << "send my message" << message;
    forwardMessageRandomly(message);
    return;
}

void Control::sendOriginMessage(const QHostAddress& IP, const quint16& port, const QString& content,
                                const QString& originID,const quint32& seq){
    QVariantMap newMessage;
    newMessage[ORIGIN] = originID;
    newMessage[SEQ_NO] = seq;
    if (content.size() > 0) {
        newMessage[CHAT_TEXT] = content;
        qDebug() << "Control::sendOriginMessage:  I send him a chat message." << newMessage;
    }
    else qDebug() << "Control::sendOriginMessage:  I send him a rout message.";

    //qDebug() << "send to" << IP << port <<"," << content << "," << originID << "," << seq;
    sendMsg2Peer(model->getNeighbor(IP, port), newMessage);
}

void Control::generateRouteMessage()
{
    QVariantMap routeMessage;
    model->addMyMessage();
    routeMessage[ORIGIN] = model->getIdentity();
    routeMessage[SEQ_NO] = model->getMySeqNo();
    qDebug() << "send routing message: " << routeMessage;
    forwardMessageRandomly(routeMessage);
}

//brocast rumor messsage include chat message and route message.
void Control::forwardMessageRandomly(const QVariantMap& message) {
    //qDebug() << "brocast message";
    Peer* randomPeer = model->getPeerRandomly();
    if (!randomPeer) return;
    //qDebug() << "pick peer:" << randomPeer->getIP() << randomPeer->getPort();
    qDebug() << "brocast rumor message to" << randomPeer->getIP() << ":" << randomPeer->getPort();
    sendMsg2Peer(randomPeer, message);
}

void Control::forwardMessage2All(const QVariantMap &message)
{
    QVector<Peer*> neighbors = model->getNeighbors();
    for (auto &n : neighbors) {
        sendMsg2Peer(n, message);
    }

}

void Control::sendMyPrivateMessage(const QString &content)
{
    qDebug() << "sendMyPrivateMessage.";
    sendPrivateMessage(model->getPrivateChattingPeer(), model->getIdentity(), content, 10);
}

void Control::sendPrivateMessage(const QString &destinationID, const QString &originID, const QString &content, int hop)
{
    QVariantMap message;
    message[DEST] = destinationID;
    message[ORIGIN] = originID;
    message[CHAT_TEXT] = content;
    message[HOP_LIMIT] = hop;
    QHostAddress IP = model->getRoutingTable()[destinationID].first;
    quint16 port = model->getRoutingTable()[destinationID].second;
    qDebug() << "Send private message to " << destinationID << "OriginID:" << originID << ", IP:" << IP << " ,Port:" << port<<  ", content: " << content << "hotLimit" << hop;

    sock->sendMessage(IP, port, message);
}

void Control::processStatusMessage(const QVariantMap &message, const QHostAddress& IP, const quint16& port)
{
    qDebug() << "Receive a [statusList]" << message;

    // stop the sender's timer:
    Peer* peer = model->getNeighbor(IP, port);
    peer->stopTimer();

    QVariantMap myStatuslist = model->getStatusList();
    QVariantMap senderStatusList = message[WANT].toMap();
    bool flagNew = false; // flag if i have some newer message to send to the IP.

    for (QVariantMap::const_iterator it = myStatuslist.begin(); it != myStatuslist.end(); it++) {
        int seq = (senderStatusList.contains(it.key()) == false) ? 1 : senderStatusList[it.key()].toInt();
        if (seq <= it.value().toInt()) {
            //qDebug() << "i have some newer message to send to the IP.";
            flagNew = true;
            QString content = model->getMessagelist()[it.key()][seq];
            if (forward || !forward && content.size() == 0)
                sendOriginMessage(IP, port, content, it.key(), seq);
            qDebug() << "my status" << myStatuslist;
            //qDebug() << "I send him my new message. sender seq: " << seq << "my status:" << it.value().toInt();
        }
    }
    if (flagNew) return;
    else{
        for (QVariantMap::iterator it = senderStatusList.begin(); it != senderStatusList.end(); it++) {
            if (myStatuslist[it.key()].toInt() + 1 < it.value().toInt()) { // the IP's status is newer than mine.
                qDebug() << "send my status for newer message.";
                sendMyStatusList(IP, port);
                return;
            }
        }
        //qDebug() << "same status;";
        flipCoins(); //we have exactly the same status, I pick up a random neighbor to send my status to it.
    }
}

void Control::processRumorMessage(QVariantMap &message, const QHostAddress& IP, const quint16& port, messageType type)
{
    QString originID = message[ORIGIN].toString();
    quint32 SeqNo = message[SEQ_NO].toInt();
    QString content = (message.contains(CHAT_TEXT) ? message[CHAT_TEXT].toString() : "");

    bool direct = true;
    if (message.contains(LAST_IP) && message.contains(LAST_PORT)) {
        addNewNeighbor(QHostAddress(message[LAST_IP].toInt()), message[LAST_PORT].toInt());
        direct = false;
    }
    qDebug() << "Reveive a " << "[direct:" << direct << "]" << ((type == CHAT_MESSAGE) ? "[chat]" : "[route]") <<  "messasge from" << IP << "," << " OriginID:" << originID << ", SeqNo:" << SeqNo << "content" << content;

    message[LAST_IP] = IP.toIPv4Address();
    message[LAST_PORT] = port;

    if (direct) { // replace the indirect with the direct path
       model->updateRoutingTable(originID, IP, port);
    }

    //QVariantMap myStatuslist = model->getStatusList();
    if (model->getHighestSeq(originID) < SeqNo) {
        if (model->isValidNewRoutingID(originID)) {// if the originID is a new one, update the routing table.
            emit addNewRouitngnID(originID);
        }
        model->updateRoutingTable(originID, IP, port);
        if (model->getHighestSeq(originID) + 1 == SeqNo) {
            qDebug() << "right seq";
            model->addNewMessage(originID, IP, port, content); // update message list and status list
            if (type == CHAT_MESSAGE) {
                qDebug() << "It's a chat message: " << content;
                emit displayNewMessage(originID + ":" + content);
            }
        }
        if (type == ROUT_MESSAGE)
            forwardMessage2All(message);
        else if (type == CHAT_MESSAGE && forward)
            forwardMessageRandomly(message); // send message to a random neighbor.
    }
//  else if (model->getHighestSeq(originID) == SeqNo && direct) { // replace the indirect with the direct path
//       model->updateRoutingTable(originID, IP, port);
//    }

    qDebug() << "reply my status" << model->getStatusList();
    sendMyStatusList(IP, port);// reply my statuslist;
}


void Control::processPrivateMessage(const QVariantMap &message)
{
    QString destID = message[DEST].toString();
    QString originID = message[ORIGIN].toString();
    QString content = message[CHAT_TEXT].toString();
    int hopLimit = message[HOP_LIMIT].toInt();

    qDebug() << "received message type is privateMessage:" << content << ", DestID: " << destID << "OriginID:" << originID << "content: " << content << HOP_LIMIT << hopLimit;

    if (destID == model->getIdentity()) {
        emit displayNewMessage(originID + ":" + content);
        return;
    }

    if (hopLimit < 1) {
        return;
    }

    hopLimit--;

    if (!forward) return;
    qDebug() << "forward message to" << model->getRoutingTable()[destID];
    sendPrivateMessage(destID, originID, content, hopLimit);

}

void Control::addNewNeighbor(const QHostAddress &IP, const quint16 &port)
{
    if (model->isValidNewComer(IP, port)) {
        Peer* peer = model->addNeighbor(IP, port);
        connect(peer, SIGNAL(timerOut(Peer*)),this, SLOT(processNoReply(Peer*)));
    }
}

void Control::processTheDatagram(const QByteArray& datagram, const QHostAddress& IP, const quint16& port) {
    //qDebug() << "receive message from sender: " << IP.toString() << ", port:" << port;

    QHostInfo host = QHostInfo::fromName(IP.toString());
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }
    //qDebug() << "sender's DNS" << host.hostName();

    //check if it's a new comer
    addNewNeighbor(IP, port);

    QDataStream in(datagram);
    QVariantMap message;
    in >> message;

    if (message.contains(WANT)) {// a status message
        processStatusMessage(message, IP, port);
    }
    else if (message.contains(CHAT_TEXT) && message.contains(ORIGIN)
             && message.contains(SEQ_NO)) { // brocast message to a random neighbor
        processRumorMessage(message, IP, port, CHAT_MESSAGE);
    }
    else if (message.contains(ORIGIN) && message.contains(SEQ_NO)) {
        processRumorMessage(message, IP, port, ROUT_MESSAGE);
    }
    else if (message.contains(DEST) && message.contains(ORIGIN)
             && message.contains(CHAT_TEXT) && message.contains(HOP_LIMIT)) {
        processPrivateMessage(message);
    }
    else {
        qDebug() << message.begin().key();
        qDebug() << "information dammaged!";
    }
    return;
}

void Control::sendMsg2Peer(Peer* peer, const QVariantMap& message) {
    sock->sendMessage(peer->getIP(), peer->getPort(), message);
    peer->setMessage(message);
    peer->startTimer();
}

void Control::sendMyStatusList(const QHostAddress& sender, const quint16 senderPort) {
    QVariantMap statusList, my = model->getStatusList();
    for (QVariantMap::iterator it = my.begin(); it != my.end(); it++) {
        statusList[it.key()] = it.value().toInt() + 1;
    }
    QVariantMap want;
    want[WANT] = statusList;
    //qDebug() << "send my statuslist:" << want;
    sock->sendMessage(sender, senderPort, want);
    return;
}

void Control::flipCoins(){
    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int flag = qrand() % FLIP_COIN; // 1/5 rate
    if (flag || !forward) return;
    qDebug() << "flip coins!!!";
    Peer* peer = model->getPeerRandomly();
    if (peer)
        sendMyStatusList(peer->getIP(), peer->getPort());
}


void Control::doAntiEntropy(){
    Peer* peer = model->getPeerRandomly();
    if (peer) {
        //qDebug() << "send antientropy to" << peer->getIP();
        sendMyStatusList(peer->getIP(), peer->getPort());
    }
}


void Control::processNoReply(Peer* peer){
    qDebug() << "process no reply!";
    peer->stopTimer();
    QVariantMap message = peer->getMessage();
    Peer* p = model->getPeerRandomly();

    if (peer)
        sendMsg2Peer(p, message);
}

void Control::addPrivateChatPeer(const QString & p)
{
    model->setPrivateChattingPeer(p);
}


QString Control::getIdentity(){
    return model->getIdentity();
}
