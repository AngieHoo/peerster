#include "chatdialog.h"

ChatDialog::ChatDialog()
{
    setWindowTitle("Peerster");

    QTime t = QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    identity = QString::number(qrand());

    mySeqNo = 0;

    //make changes!!!!!!!
    //
    // Read-only text box where we display messages from everyone.
    // This widget expands both horizontally and vertically.
    textview = new QTextEdit(this);
    textview->setReadOnly(true);

    // Small text-entry box the user can enter messages.
    // This widget normally expands only horizontally,
    // leaving extra vertical space for the textview widget.
    //
    // You might change this into a read/write QTextEdit,
    // so that the user can easily enter multi-line messages.
    textinput = new TextInput(this);
    textinput->setFocus();

    // Lay out the widgets to appear in the main window.
    // For Qt widget and layout concepts see:
    // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(textview);
    layout->addWidget(textinput);
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

    if (!sock->bind())
        exit(1);
    timer->start();
    
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
    sock->sendMsg2RandomPeer(map);
    // clean my
    textinput->clear();
    QTimer::singleShot(2000, this, SLOT(checkReply()));
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

void ChatDialog::flipCoins(const QString& originID,const quint32& SeqNo){
    QTime t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int flag = qrand() % 2;
    if (flag == 0) return;
    QVariantMap message;
    message["ChatText"] = messageList[originID][SeqNo];
    message["Origin"] = originID;
    message["SeqNo"] = SeqNo;

    sock->sendMsg2RandomPeer(message);
    QTimer::singleShot(2000, this, SLOT(checkReply()));
    return;
}

void ChatDialog::brocastMessage(const QVariantMap& message) {
    sock->sendMsg2RandomPeer(message);
    QTimer::singleShot(2000, this, SLOT(checkReply()));
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

void ChatDialog::processTheDatagram(const QByteArray& datagram, const QHostAddress& sender, const quint16 senderPort) {
    // need to tell if it is a status message or a ordinary message.
    qDebug() << "receive message from" << sender << ", port:" << senderPort;
    QVariantMap message;
    QDataStream in(datagram);
    QString messageType;
    in >> messageType;
    if (messageType == "Want") {// a status message
        QVariant tmp;
        QVariantMap status;
        in >> tmp;
        status = tmp.toMap();
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
        }
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
          //return;
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
          }
    }
    else if (messageType == "CmpStatus") {
        QVariant tmp;
        QVariantMap status;
        in >> tmp;
        status = tmp.toMap();
        qDebug() << "message type is [CmpStatus]";
        for (QVariantMap::iterator it = statusList.begin(); it != statusList.end(); it++) {
            if (status[it.key()].toInt() < it.value().toInt()) {
                QVariantMap newMessage;
                newMessage["ChatText"] = messageList[it.key()][status[it.key()].toInt()];
                newMessage["Origin"] = it.key();
                newMessage["SeqNo"] = status[it.key()].toInt();
                sock->sendMessage(sender,senderPort, newMessage);
                QTimer::singleShot(2000, this, SLOT(checkReply()));
            }
        }
    }
    else {//TODO: to handle the information loss???
        qDebug() << "information dammaged!";
    }

    return;
}

void ChatDialog::doAntiEntropy(){
    qDebug() << "doAntiEntropy!!!!";
    QVariantMap status;
    status["CmpStatus"] = statusList;
//    for (QVariantMap::iterator it = statusList.begin(); it != statusList.end(); it++) {
//        status["CmpStatus"].toMap()[it.key()] = it.value().toInt() + 1;
//    }
    sock->sendMsg2RandomPeer(status);
    return;
}
