#include "control.h"


Control::Control(QObject *parent) : QObject(parent)
{
    forward = true;
    startDownloading = false;
    isSearching  = false;
    budget = 2;
    matchedNum = 0;
    downLoadDir = "./download/";

    QDir dir;
    if (!dir.exists("./download")) {
        qDebug() << "set up new download directory.";
        dir.mkdir("download");
    }

    sock = new NetSocket(this);
    connect(sock, SIGNAL(processTheDatagram(QByteArray,QHostAddress,quint16)), SLOT(processTheDatagram(QByteArray,QHostAddress,quint16)));

    model = new Model(this);

    fileManager = new FileManager(this);

    connect(model, SIGNAL(displayNewNeighbor(const QHostAddress&,const quint16&)), this, SIGNAL(displayNewNeighbor(const QHostAddress&,const quint16&)));

    timer = new QTimer(this);
    timer->setInterval(10000);

    timerRoute = new QTimer(this);
    timerRoute->setInterval(60000);

    timerFileReq = new QTimer(this);
    timerFileReq->setInterval(3000);

    timerSearchFile = new QTimer(this);
    timerSearchFile->setInterval(1000);


    connect(timer, SIGNAL(timeout()), this, SLOT(doAntiEntropy()));
    connect(timerRoute, SIGNAL(timeout()), this, SLOT(generateRouteMessage()));
    connect(timerFileReq, SIGNAL(timeout()), this, SLOT(sendDownLoadRequest()));
    connect(timerSearchFile, SIGNAL(timeout()), this, SLOT(sendSearchRequest()));

}

void Control::start(){
    bind();
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
    sendChatMsg2Peer(model->getNeighbor(IP, port), newMessage);
}

void Control::generateRouteMessage()
{
    QVariantMap routeMessage;
    model->addMyMessage();
    routeMessage[ORIGIN] = model->getIdentity();
    routeMessage[SEQ_NO] = model->getMySeqNo();
    qDebug() << "send routing message: " << routeMessage;
    forwardMessage2All(routeMessage);
}

//brocast rumor messsage include chat message and route message.
void Control::forwardMessageRandomly(const QVariantMap& message) {
    Peer* randomPeer = model->getPeerRandomly();
    if (!randomPeer) return;
    qDebug() << "brocast rumor message to" << randomPeer->getIP() << ":" << randomPeer->getPort();
    sendChatMsg2Peer(randomPeer, message);
}

void Control::forwardMessage2All(const QVariantMap &message)
{
    QVector<Peer*> neighbors = model->getNeighbors();
    for (auto &n : neighbors) {
        sendChatMsg2Peer(n, message);
    }

}

void Control::sendMyPrivateMessage(const QString &content)
{
    qDebug() << "sendMyPrivateMessage.";
    sendPrivateMessage(model->getPrivateChattingPeer(), model->getIdentity(), content, INIT_HOP_STEP);
}

void Control::uploadFiles(const QStringList &fileNameList)
{
    fileManager->upLoadFiles(fileNameList);
}



void Control::downloadMatchFile(const QString &fileNameNOriginID)
{
    qDebug() << "Control::downloadMatchFile";
    QStringList strs =  fileNameNOriginID.split(QRegExp("[:,]"), QString::SkipEmptyParts);
    if (strs.size() != 2) return;
    timerSearchFile->stop();
    downLoadCache.fileName = strs[0];
    downLoadCache.metaHashVal =  matchedFiles[strs[0]][strs[1]];
    requestBlocks.append(QPair<QByteArray, QString>(downLoadCache.metaHashVal, strs[1]));
    timerFileReq->start();
}

void Control::sendSearchRequest() {

    if (budget < 100 && matchedNum < 10)
        budget *= 2;
    if (budget > 100) budget = 100;

    QVariantMap message;
    message[ORIGIN] = model->getIdentity();
    message[SEARCH] = searchKeyWords;
    message[BUDGET] = budget;
    qDebug() << "send search file request" << message[SEARCH] <<  " to " << message[BUDGET];
    forwardMessage2All(message);
}

void Control::searchFiles(const QString &keyWords)
{
    searchKeyWords = keyWords;
    budget = 1;
    matchedFiles.clear();
    matchedNum = 0;
    timerSearchFile->start();
}

void Control::sendDownLoadRequest(){
    if (requestBlocks.empty()) return;
    QVariantMap message;
    message[DEST] = requestBlocks.front().second;
    message[ORIGIN] = model->getIdentity();
    message[HOP_LIMIT] = INIT_HOP_STEP;
    message[BLOCK_REQUEST] = requestBlocks.front().first;
    qDebug() << "send download request" << message[BLOCK_REQUEST] <<  " to " << message[DEST];
    sendP2PMessage(message);
}

void Control::sendP2PMessage(const QVariantMap &message)
{
    if (!model->hasDirectSender(message[DEST].toString())) return;
    QPair<QHostAddress, quint16> directSender(model->getDirectSender(message[DEST].toString()));
    sock->sendMessage(directSender.first, directSender.second, message);
}

void Control::forwardMessage2Peer(Peer *peer, const QVariantMap &message)
{
     sock->sendMessage(peer->getIP(), peer->getPort(), message);
}


void Control::sendPrivateMessage(const QString &destinationID, const QString &originID, const QString &content, int hop)
{
    QVariantMap message;
    message[DEST] = destinationID;
    message[ORIGIN] = originID;
    message[CHAT_TEXT] = content;
    message[HOP_LIMIT] = hop;
    sendP2PMessage(message);
}

void Control::processStatusMessage(const QVariantMap &message, const QHostAddress& IP, const quint16& port)
{
    //qDebug() << "Receive a [statusList]" << message;

    // stop the sender's timer:
    Peer* peer = model->getNeighbor(IP, port);
    peer->stopTimer();

    QVariantMap myStatuslist = model->getStatusList();
    QVariantMap senderStatusList = message[WANT].toMap();
    bool flagNew = false; // flag if i have some newer message to send to the IP.

    for (QVariantMap::const_iterator it = myStatuslist.begin(); it != myStatuslist.end(); it++) {
        int seq = (senderStatusList.contains(it.key()) == false) ? 1 : senderStatusList[it.key()].toInt();
        if (seq <= it.value().toInt()) {
            flagNew = true;
            QString content = model->getMessagelist()[it.key()][seq];
            if (forward || !forward && content.size() == 0)
                sendOriginMessage(IP, port, content, it.key(), seq);
            qDebug() << "my status" << myStatuslist;
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

    if (direct) { // replace the indirect with the dßirect path
        if (model->isValidNewRoutingID(originID)) {// if the originID is a new one, update the routing table.
            emit addNewRouitngnID(originID);
        }
       model->updateRoutingTable(originID, IP, port);
    }
    if (model->getHighestSeq(originID) < SeqNo) {
        if (!direct) {
            if (model->isValidNewRoutingID(originID)) {// if the originID is a new one, update the routing table.
                emit addNewRouitngnID(originID);
            }
            model->updateRoutingTable(originID, IP, port);
        }
        if (model->getHighestSeq(originID) + 1 == SeqNo) {
            //qDebug() << "right seq";
            model->addNewMessage(originID, IP, port, content); // update message list and status list
            if (type == CHAT_MESSAGE) {
                //qDebug() << "It's a chat message: " << content;
                emit displayNewMessage(originID + ":" + content);
            }
        }
        if (type == ROUT_MESSAGE)
            forwardMessage2All(message);
        else if (type == CHAT_MESSAGE && forward)
            forwardMessageRandomly(message); // send message to a random neighbor.
    }

   // qDebug() << "reply my status" << model->getStatusList();
    sendMyStatusList(IP, port);// reply my statuslist;
}

void Control::processP2PMessage(const QVariantMap &message)
{
    qDebug() << "received message type is p2p message:" << ", DestID: " << message[DEST].toString() << "OriginID:" <<  message[ORIGIN].toString() <<  "HOP_LIMIT" << message[HOP_LIMIT].toInt();

    if (message[DEST].toString() == model->getIdentity()) {// receiving the message intended for me;
        if (message.contains(CHAT_TEXT)) { // private chating message.
             emit displayNewMessage("(private)" + message[ORIGIN].toString() + ":" + message[CHAT_TEXT].toString());
        }
        else if (message.contains(BLOCK_REQUEST)) { // file downloading request
            processBlockRequest(message);
        }
        else if (message.contains(BLOCK_REPLY)) {
            processBlockReply(message);
        }
        else if (message.contains(SEARCH_REPLY)){
            processSearchReply(message);
        }
       return;
    }

    if (message[HOP_LIMIT].toUInt() < 1) {
        return;
    }

    QVariantMap newMessage(message);

    newMessage[HOP_LIMIT] = newMessage[HOP_LIMIT].toInt() - 1;

    if (!forward) return;
    qDebug() << "forward message to" << model->getRoutingTable()[message[DEST].toString()];
    //sendPrivateMessage(destID, originID, content, hopLimit);
    sendP2PMessage(newMessage);

}


void Control::processBlockRequest(const QVariantMap &message)
{

    QByteArray blockHash = message[BLOCK_REQUEST].toByteArray();
    qDebug() << "processBlockRequest!!!"  << "block hash：" << blockHash;
    QByteArray block = fileManager->getBlockAt(blockHash);
    if (block.size() > 0) {
        qDebug() << "get the block!!!!" << block;
        QVariantMap replyMessage;
        replyMessage[DEST] = message[ORIGIN];
        replyMessage[ORIGIN] = model->getIdentity();
        replyMessage[HOP_LIMIT] = INIT_HOP_STEP;
        replyMessage[BLOCK_REPLY] = blockHash;
        replyMessage[DATA] = block;
        qDebug() << "send my block reply: hashValue:" << replyMessage[BLOCK_REPLY] << ", data:" << replyMessage[DATA];
        sendP2PMessage(replyMessage);
    }

}

void Control::processBlockReply(const QVariantMap &message)
{
    qDebug() << "processBlockReply!!! block data：" << message[DATA].toByteArray() << "; hash valueL:" << message[BLOCK_REPLY];
    if (!(requestBlocks.front().first == message[BLOCK_REPLY].toByteArray())) // not the right sequence
        return;
    QCA::Hash shaHash("sha1");
    shaHash.update(message[DATA].toByteArray());
    QByteArray check = shaHash.final().toByteArray();
    if (check != message[BLOCK_REPLY].toByteArray()) // the content does not match
        return;
    requestBlocks.pop_front();//pop the request;

    if (startDownloading) {
        downLoadCache.size += message[DATA].toByteArray().size();
        QFile file(downLoadDir + downLoadCache.fileName);
        if (file.open(QFile::WriteOnly | QIODevice::Append)) {
            qDebug() << "write the file" << message[DATA].toByteArray();
            file.write(message[DATA].toByteArray());
        }
        if (requestBlocks.empty()) { // finish downloading
            startDownloading = false;
            timerFileReq->stop();
        }
    }
    else{
            downLoadCache.metafile = message[DATA].toByteArray();
            int index = 0, size = downLoadCache.metafile.size();
            if (size % HASH_SIZE == 0) {
                qDebug() << "right size.";
                while (index < size){
                    QByteArray blockHash = downLoadCache.metafile.mid(index, HASH_SIZE);
                    requestBlocks.push_back(QPair<QByteArray, QString>(blockHash, message[ORIGIN].toString()));
                    index += HASH_SIZE;
                }
            }
        startDownloading = true;
    }
    sendDownLoadRequest();
}

void Control::processSearchRequest(QVariantMap &message)
{
    if (message[ORIGIN] == model->getIdentity()) return;
    QStringList keyWords = message[SEARCH].toString().split(' ', QString::SkipEmptyParts);
    QVector<QPair<QString, QByteArray>> matchedLocalFiles = fileManager->findMatchedFiles(keyWords);
    if (matchedLocalFiles.size() > 0) {
        QVariantList matchedNames, matchedIDs;
        for (auto f : matchedLocalFiles) {
            matchedNames.push_back(f.first);
            matchedIDs.push_back(f.second);
        }

        QVariantMap matchedFilesMessage;
        matchedFilesMessage[DEST] = message[ORIGIN];
        matchedFilesMessage[ORIGIN] = model->getIdentity();
        matchedFilesMessage[HOP_LIMIT] = INIT_HOP_STEP;
        matchedFilesMessage[SEARCH_REPLY] = message[SEARCH];
        matchedFilesMessage[MATCH_NAME] = matchedNames;
        matchedFilesMessage[MATCH_ID_S] = matchedIDs;

        qDebug() << "replay my local file" << matchedLocalFiles;
        sendP2PMessage(matchedFilesMessage);
    }

    if (message[BUDGET].toInt() < 2) return;

    message[BUDGET] = message[BUDGET].toInt() - 1;

    QVector<Peer*> peers = model->getKNeighborsRandomly(message[BUDGET].toInt());
    message[BUDGET] = message[BUDGET].toInt() / peers.size();
    int left = message[BUDGET].toInt() % peers.size();
    for (auto &peer : peers) {
        if (left--) {
            message[BUDGET] = message[BUDGET].toInt() + 1;
            forwardMessage2Peer(peer, message);
            message[BUDGET] = message[BUDGET].toInt() - 1;
        }
        else forwardMessage2Peer(peer, message);
    }

}

void Control::processSearchReply(const QVariantMap &message)
{
    //has receive this file before:
    qDebug() << "process Search reply" << message[MATCH_NAME].toList() << ", " << message[MATCH_ID_S] << "," << message[ORIGIN].toString();
    QString originID = message[ORIGIN].toString();
    QVariantList fileNames = message[MATCH_NAME].toList(), matchIDs =  message[MATCH_ID_S].toList();
    for (size_t i = 0 ; i < fileNames.size() && i < matchIDs.size(); i++) {
        QByteArray fileID = matchIDs.at(i).toByteArray();
        QString name = fileNames.at(i).toString();
        if (matchedFiles.contains(name) && matchedFiles[name].contains(originID)
                && matchedFiles[name][originID] == fileID)
            return;
        matchedFiles[name][originID] = fileID;
        qDebug() << "add match file succesfully!";
        emit addNewMatchedFiles(name, originID);
    }


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

    else if (message.contains(DEST) && message.contains(ORIGIN) && message.contains(HOP_LIMIT)) {
        processP2PMessage(message);
    }
    else if (message.contains(SEARCH) && message.contains(BUDGET)) {
        processSearchRequest(message);
    }
    else {
        qDebug() << message.begin().key();
        qDebug() << "information dammaged!";
    }
    return;
}

void Control::sendChatMsg2Peer(Peer* peer, const QVariantMap& message) {
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
        sendChatMsg2Peer(p, message);
}

void Control::addPrivateChatPeer(const QString & p)
{
    model->setPrivateChattingPeer(p);
}


QString Control::getIdentity(){
    return model->getIdentity();
}

