#include "chatdialog.h"

ChatDialog::ChatDialog()
{
    setWindowTitle("Peerster");

    mySeqNo= 0;
    QTime t = QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    identity = QString::number(qrand());


    myPortMin = 32768 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 3;

    textview = new QTextEdit(this);
    textview->setReadOnly(true);

    textinput = new TextInput(this);
    textinput->setFocus();

    onlineNeighbor = new QTextEdit(this);
    onlineNeighbor->setReadOnly(true);

    neighborInput = new QLineEdit(this);

    QHBoxLayout *layout = new QHBoxLayout(this);

    QVBoxLayout *layoutText = new QVBoxLayout(this);
    layoutText->addWidget(textview);
    layoutText->addWidget(textinput);

    QVBoxLayout *layoutNeighbor = new QVBoxLayout(this);
    layoutNeighbor->addWidget(onlineNeighbor);
    layoutNeighbor->addWidget(neighborInput);

    layout->addLayout(layoutText);
    layout->addLayout(layoutNeighbor);


    setLayout(layout);

    timer = new QTimer(this);
    timer->setInterval(10000);
    //timer->setSingleShot(true);
    
    sock = new NetSocket(this);
    // Register a callback on the textinput's returnPressed signal
    // so that we can send the message entered by the user.
    connect(textinput, SIGNAL(returnPressed()), this, SLOT(sendMyMsg2RandomPeer()));
    connect(sock, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    connect(timer, SIGNAL(timeout()), this, SLOT(doAntiEntropy()));
    connect(neighborInput, SIGNAL(returnPressed()), this, SLOT(tryAddNewNeighbor()));

    qDebug() << "Localhost name:" << QHostInfo::localHostName();

    bind();
    creatLocalNeighbors();

    timer->start();
    
}

void ChatDialog::tryAddNewNeighbor(){
    QHostAddress self(QHostAddress::LocalHost);
    QString str = neighborInput->text();
    QString address = str.left(str.indexOf(':'));
    quint16 port = str.mid(str.indexOf(':') + 1).toInt();
    qDebug() << "Input Neighbor's Address: " << address << ", Port: " << port;
    QHostAddress testIP;
    if (testIP.setAddress(address)){ // is a IP address.
        QHostInfo host = QHostInfo::fromName(address);
        if (host.error() != QHostInfo::NoError) {
            qDebug() << "Lookup failed:" << host.errorString();
        }
        else {
            qDebug() << "Found hostName:" << host.hostName();
            if (addNeighbor(Peer(host.hostName(), testIP, port))) //check if it is myself or exist in neighbor list. if not add it.
                onlineNeighbor->append(host.hostName() + "(" + testIP .toString() + ")" + ":" + QString::number(port));
        }
    }
    else {
        QEventLoop eventloop;
        connect(this, SIGNAL(finishLookUp()), &eventloop,SLOT(quit()));
        int id = QHostInfo::lookupHost(address, this, SLOT(lookedUp(QHostInfo)));
        eventloop.exec();
        //if (hostInfo.error() != QHostInfo::NoError) {// success!
        if (hostInfo.addresses().size() > 0) {
            if (addNeighbor(Peer(hostInfo.hostName(), hostInfo.addresses().at(0), port)))
                onlineNeighbor->append(address + "(" + hostInfo.addresses().at(0).toString() + ")"+ ":" + QString::number(port));
        }
        else{
            QMessageBox::about(NULL, "Warning", "Invalid Address!");
        }
    }
    neighborInput->clear();
    return;
}


void ChatDialog::lookedUp(const QHostInfo &host)
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

void ChatDialog::creatLocalNeighbors(){
    if (myPort > myPortMin) {
        neighbors.push_back(Peer(QHostInfo::localHostName(),  QHostAddress::LocalHost, myPort - 1));
        onlineNeighbor->append(QHostInfo::localHostName() + "(" + "127.0.0.1" + ")"  + ":" + QString::number(myPort - 1));
    }
    if (myPort < myPortMax) {
        neighbors.push_back(Peer(QHostInfo::localHostName(),  QHostAddress::LocalHost, myPort + 1));
        onlineNeighbor->append(QHostInfo::localHostName() + "(" +  "127.0.0.1" + ")" + ":" + QString::number(myPort + 1));
    }
    return;
}

void ChatDialog::bind(){
    for (int p = myPortMin; p <= myPortMax; p++) {
        if (sock->bind(p)) {
            qDebug() << "bound to UDP port " << p;
            myPort = p;
            return;
        }
    }
    exit(1);
}

void ChatDialog::sendMyMsg2RandomPeer() {
    // i'll first update my list.
    mySeqNo++;
    statusList[identity] = mySeqNo;
    messageList[identity][mySeqNo] = textinput->toPlainText();
    // display the message;
    textview->append(textinput->toPlainText());
    // send the message;
    QVariantMap map;
    map["ChatText"] = textinput->toPlainText();
    map["Origin"] = identity;
    map["SeqNo"] = mySeqNo;

    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int pickNo = qrand() % neighbors.size();

    sock->sendMessage(neighbors[pickNo].getIP(),neighbors[pickNo].getPort(),map);
    // clean my
    textinput->clear();
    //QTimer::singleShot(2000, this, SLOT(checkReply()));
    return;
}


void ChatDialog::sendStatus(const QHostAddress& sender, const quint16 senderPort, const QString& originID,const quint32& SeqNo){
    QVariantMap status;
    status[originID] = SeqNo;
    QVariantMap want;
    want["Want"] = status;
    sock->sendMessage(sender, senderPort, want);
    return;
}

void ChatDialog::readPendingDatagrams() {

    while (sock->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(sock->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        sock->readDatagram(datagram.data(), datagram.size(),
                           &sender, &senderPort);
        processTheDatagram(datagram, sender, senderPort);
    }
    return;
}
void ChatDialog::flipCoins(){
    qDebug() << "flip coins!!!";
    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int flag = qrand() % 2;
    if (flag == 0) {
        qDebug() << "I am too tired! I wanna stop!!";
        return;
    }
    qDebug() << "I decide to send this message to a random peer!";
    QVariantMap statusl;
    int pickNo = qrand() % neighbors.size();
    sendStatusList(neighbors[pickNo].getIP(),neighbors[pickNo].getPort());
    return;
}


void ChatDialog::brocastMessage(const QVariantMap& message) {
    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int pickNo = qrand() % neighbors.size();
    sock->sendMessage(neighbors[pickNo].getIP(),neighbors[pickNo].getPort(),message);
    // QTimer::singleShot(2000, this, SLOT(checkReply()));
    return;
}

void ChatDialog::checkReply(){
    //TODO:..........
    return;
}

void ChatDialog::updateList(const QString& content,const QString& originID,const quint32& SeqNo) {
    statusList[originID] = SeqNo;
    messageList[originID][SeqNo] = content;
    return;
}

void ChatDialog::sendStatusList(const QHostAddress& sender, const quint16 senderPort) {

    QVariantMap statusl;
    //statusl["Want"] = statusList;
    for (QVariantMap::iterator it = statusList.begin(); it != statusList.end(); it++) {
        statusl[it.key()] = it.value().toInt() + 1;
    }
    QVariantMap want;
    want["Want"] = statusl;
    qDebug() << "send my statuslist:" << want;
    sock->sendMessage(sender, senderPort, want);
    return;
}

void ChatDialog::processTheDatagram(const QByteArray& datagram, const QHostAddress& sender, const quint16& senderPort) {
    qDebug() << "senderIP:" << sender.toIPv4Address() << ", port:" << senderPort;

    // check if the sender is my neighbor, if not, add it into my neighbor list.
    QHostInfo host = QHostInfo::fromName(sender.toString());
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }
    qDebug() << "check if the sender has been my neighbor." << host.hostName();
    if (addNeighbor(Peer(host.hostName(), sender, senderPort)))
        onlineNeighbor->append(host.hostName() + "(" + sender.toString() + ")" + ":" + QString::number(senderPort));


    // need to tell if it is a status message or a ordinary message.
    qDebug() << "receive message from" << sender << ", port:" << senderPort;
    QVariantMap message;
    QDataStream in(datagram);
    QString messageType;
    in >> messageType;
    if (messageType == "Want") {// a status message
        qDebug() << "the message type is [Status]";
        QVariant tmp;
        QVariantMap status;
        in >> tmp;
        status = tmp.toMap();
        qDebug() << "sender'status: " << status;
        bool flagNew = false; // flag if i have some newer status to send to the sender.
        for (QVariantMap::iterator it = statusList.begin(); it != statusList.end(); it++) {
            if (status[it.key()].toInt() <= it.value().toInt()) {
                flagNew = true;
                QVariantMap newMessage;
                newMessage["ChatText"] = messageList[it.key()][status[it.key()].toInt()];
                newMessage["Origin"] = it.key();
                newMessage["SeqNo"] = status[it.key()].toInt();
                sock->sendMessage(sender,senderPort, newMessage);
                //QTimer::singleShot(2000, this, SLOT(checkReply()));
            }
        }

        if (!flagNew){ // if I have nothing new for the sender, then check if the sender contains new message that i have not received
            bool flagOld = false;
            for (QVariantMap::iterator it = status.begin(); it != status.end(); it++) {
                if (statusList[it.key()].toInt() + 1 < it.value().toInt()) { // the sender's status is newer than mine.
                    flagOld = true;
                    qDebug() << "Status type 2";
                    sendStatusList(sender, senderPort);
                    break;
                }
            }
            if (!flagOld) { // we have exactly the same status
                flipCoins(); // I pick up a random neighbor to send my status to it.
            }
            else {
                qDebug() << "I ask him to send me some new message.";
            }
        }
        else {
            qDebug() << "I have send him some new messages he need";
        }



        /* version 1:
        for (QVariantMap::iterator it = status.begin(); it != status.end(); it++) {
            QString originID = it.key();
            quint32 SeqNo = it.value().toInt();
            qDebug() << "message type is [Status]: the original ID: " << originID << ", the seqNo: " << SeqNo;
            // find out is the sender's status is newer than mine (the required seqno is new than my requred seqno), if so send my status to it,
            // if not, send a corresponding message to sender to met his requirement.
            if (statusList[originID].toInt()  + 1 < SeqNo) { // the sender's status is newer than mine.
                qDebug() << "Status type 1";
                sendStatus(sender, senderPort, originID, statusList[originID].toInt() + 1);
            }
            else if (statusList[originID].toInt() + 1 > SeqNo){// my status is newer than the sender.
                qDebug() << "Status type 2";
                QVariantMap newMessage;
                newMessage["ChatText"] = messageList[originID][SeqNo];
                newMessage["Origin"] = originID;
                newMessage["SeqNo"] = SeqNo;
                sock->sendMessage(sender, senderPort, newMessage);
                QTimer::singleShot(2000, this, SLOT(checkReply()));
            }
            else {  // we have exactly the same state for message from originID. so i decide to flip a coins.
                qDebug() << "Status type 3";
                flipCoins(originID, SeqNo - 1);
            }
        }*/
    }
    else if (messageType == "ChatText") { // brocast message to a random neighbor
        QString content,originKey, SeqNoKey, originID;
        quint32 SeqNo;
        QVariant tmp;
        in >> tmp;
        content = tmp.toString();
        in >> originKey >> tmp;
        originID = tmp.toString();
        in >> SeqNoKey >> tmp;
        SeqNo = tmp.toInt();
        message[messageType] = content;
        message[originKey] = originID;
        message[SeqNoKey] = SeqNo;
        qDebug() << "message type is [ChatText]:" << content <<"," << originKey << ":"<< originID << ", " << SeqNoKey<< ":" << SeqNo;

        if ((statusList[originID].toInt() + 1 == SeqNo)) {// i have never received this message, and that is what i want. the sequence of this message is right for me.
            qDebug() << "I receive a new message with the right sequence!";
            textview->append(content); // display this message.
            updateList(content, originID, SeqNo);//i'll update my statuslist and my messagelist
            qDebug() << "Update my list.... the statuslise: " << statusList << ", the message list:" << messageList;
            brocastMessage(message); // send message to a random neighbor.
        }
        else if (statusList[originID].toInt() < SeqNo - 1){
            qDebug() << "I receive a useless new message....."    ;
            // i've never received this message, but its sequence is not what i want, so i requare sender to send me the right sequence of message.
            //i won't display this message. but i will still brocast it.
            brocastMessage(message); // send message to a random neighbor.
        }

        sendStatusList(sender, senderPort);// reply my statuslist;
        // version 1: only send one status:
        /*
          if (statusList[originID].toInt() >= SeqNo){ //i have receive this message before. so I require the sender to send me newer message.
              qDebug() << "ChatText type 1";
              sendStatus(sender, senderPort, originID, statusList[originID].toInt() + 1);
          }
          else if (statusList[originID].toInt() == SeqNo - 1) { // i have never received this message, and that is what i want. the sequence of this message is right for me.
              qDebug() << "ChatText type 2";
              textview->append(content); // display this message.
              updateList(content, originID, SeqNo);//i'll update my statuslist and my messagelist
              brocastMessage(message); //  i brocast this message.
              sendStatus(sender, senderPort, originID, SeqNo + 1); // I want newer message from sender.
          }
          else { // statusList[originID] < SeqNo - 1
              // i've never received this message, but its sequence is not what i want, so i requare sender to send me the right sequence of message.
              //i won't diplay this message. but i will still brocast it.
              qDebug() << "ChatText type 3";
              brocastMessage(message);
              sendStatus(sender, senderPort, originID, statusList[originID].toInt() + 1);
          }*/
    }

    else {//TODO: to handle the information loss???
        qDebug() << "information dammaged!";
    }

    return;
}

void ChatDialog::doAntiEntropy(){
    qDebug() << "doAntiEntropy!!!!";
    QVariantMap statusl;
    statusl["Want"] = statusList;
    for (QVariantMap::iterator it = statusList.begin(); it != statusList.end(); it++) {
        statusl["Want"].toMap()[it.key()] = it.value().toInt() + 1;
    }

    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int pickNo = qrand() % neighbors.size();

    sock->sendMessage(neighbors[pickNo].getIP(),neighbors[pickNo].getPort(),statusl);
    return;
}

bool ChatDialog::addNeighbor(const Peer& newPeer){
    QHostAddress self(QHostAddress::LocalHost);
    if (newPeer.getIP().toIPv4Address() == self.toIPv4Address() && newPeer.getPort() == myPort) {
        QMessageBox::about(NULL, "Warning", "Please don't add yourself!");
        return false;
    }
    for (QVector<Peer>::iterator it = neighbors.begin(); it != neighbors.end(); it++) {
        if (it->getIP().toIPv4Address() == newPeer.getIP().toIPv4Address() && it->getPort() == newPeer.getPort()) {
            //QMessageBox::about(NULL, "Warning", "You already have this neighbor!!");
            return false;
        }
    }
    qDebug() <<  "Congradulation! we sccessfully add the neighbor!!!!!!";
    qDebug() << "the neigbor's Address is:" << newPeer.getIP();
    neighbors.push_back(newPeer);
    return true;
}
