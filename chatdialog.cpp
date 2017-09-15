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

    QStringList commond = QCoreApplication::arguments();
    QString str;
    foreach(str, commond) {
        qDebug() << str;
    }
    if (commond.size() > 1 && commond[1] != ""){
        QString originNeighbor = commond[1];
        QString address = originNeighbor.left(str.indexOf(':'));
        quint16 port = originNeighbor.mid(str.indexOf(':') + 1).toInt();
        checkInputNeighbor(address, port);
    }

    timer->start();

}

void ChatDialog::checkInputNeighbor(const QString& address,const quint16& port){
    QHostAddress self(QHostAddress::LocalHost);
    qDebug() << "Input Neighbor's Address: " << address << ", Port: " << port;
    QHostAddress testIP;
    if (testIP.setAddress(address)){ // is a IP address.
        QHostInfo host = QHostInfo::fromName(address);
        if (host.error() != QHostInfo::NoError) {
            qDebug() << "Lookup failed:" << host.errorString();
        }
        else {
            qDebug() << "Found hostName:" << host.hostName();
            addNeighbor(Peer(host.hostName(), testIP, port)); //check if it is myself or exist in neighbor list. if not add it.
                //onlineNeighbor->append(host.hostName() + "(" + testIP .toString() + ")" + ":" + QString::number(port));
        }
    }
    else {
        QEventLoop eventloop;
        connect(this, SIGNAL(finishLookUp()), &eventloop,SLOT(quit()));
        int id = QHostInfo::lookupHost(address, this, SLOT(lookedUp(QHostInfo)));
        eventloop.exec();
        //if (hostInfo.error() != QHostInfo::NoError) {// success!
        if (hostInfo.addresses().size() > 0) {
            addNeighbor(Peer(hostInfo.hostName(), hostInfo.addresses().at(0), port));
                //onlineNeighbor->append(address + "(" + hostInfo.addresses().at(0).toString() + ")"+ ":" + QString::number(port));
        }
        else{
            QMessageBox::about(NULL, "Warning", "Invalid Address!");
        }
    }
}

void ChatDialog::tryAddNewNeighbor(){

    QString str = neighborInput->text();
    QString address = str.left(str.indexOf(':'));
    quint16 port = str.mid(str.indexOf(':') + 1).toInt();
    qDebug() << "Input Neighbor's Address: " << address << ", Port: " << port;
    checkInputNeighbor(address,port);
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
    for (quint16 p = myPortMin; p <= myPortMax; p++) {
        if (myPort == p) continue;
        neighbors.push_back(Peer(QHostInfo::localHostName(),  QHostAddress::LocalHost, p));
        onlineNeighbor->append(QHostInfo::localHostName() + "(" + "127.0.0.1" + ")"  + ":" + QString::number(p));
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
    addNeighbor(Peer(host.hostName(), sender, senderPort));
        //onlineNeighbor->append(host.hostName() + "(" + sender.toString() + ")" + ":" + QString::number(senderPort));

    // need to tell if it is a status message or a ordinary message.
    qDebug() << "receive message from" << sender << ", port:" << senderPort;

    //QVariant tmp;
    QDataStream in(datagram);
    //in >> tmp;
    //QVariantMap message = tmp.toMap();
    QVariantMap message;
    in >> message;
    qDebug() << "complete message:" << message;
    if (message.contains("Want")) {// a status message
        qDebug() << "the message type is [Status]";
        QVariantMap status;     
        status = message["Want"].toMap();
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

    }
    else if (message.contains("ChatText") && message.contains("Origin") && message.contains("SeqNo")) { // brocast message to a random neighbor
        QString originID = message["Origin"].toString();
        quint32 SeqNo = message["SeqNo"].toInt();
        QString content = message["ChatText"].toString();
        qDebug() << "message type is ChatText:" << content <<",OriginID:" << originID << ", SeqNo:" << SeqNo;
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
    }

    else {//TODO: to handle the information loss???
        qDebug() << message.begin().key();
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

void ChatDialog::addNeighbor(const Peer& newPeer){
    QHostAddress self(QHostAddress::LocalHost);
    if (newPeer.getIP().toIPv4Address() == self.toIPv4Address() && newPeer.getPort() == myPort) {
        QMessageBox::about(NULL, "Warning", "Please don't add yourself!");
        return;
    }
    for (QVector<Peer>::iterator it = neighbors.begin(); it != neighbors.end(); it++) {
        if (it->getIP().toIPv4Address() == newPeer.getIP().toIPv4Address() && it->getPort() == newPeer.getPort()) {
            //QMessageBox::about(NULL, "Warning", "You already have this neighbor!!");
            return;
        }
    }
    qDebug() <<  "Congradulation! we sccessfully add the neighbor!!!!!!";
    qDebug() << "the neigbor's Address is:" << newPeer.getIP();
    neighbors.push_back(newPeer);
    onlineNeighbor->append(newPeer.getDNS() + "(" + newPeer.getIP().toString() + ")" + ":" + QString::number(newPeer.getPort()));
    return;
}
