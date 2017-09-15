#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QTimer>
#include <QHostInfo>
#include <QMessageBox>


#include <qmath.h>

#include "netsocket.h"
#include "textinput.h"
#include "peer.h"

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    ChatDialog();

public slots:
     void sendMyMsg2RandomPeer();
     void readPendingDatagrams();    
    void lookedUp(const QHostInfo &host);
private slots:
     void doAntiEntropy();
     void checkReply();
     void tryAddNewNeighbor();

signals:
     void finishLookUp();
private:
    quint16 myPort;
    quint16 myPortMin;
    quint16 myPortMax;
    QVector<Peer> neighbors;
    NetSocket* sock;
    QTimer* timer;

    QTextEdit* textview;
    TextInput* textinput;
    QTextEdit* onlineNeighbor;
    QLineEdit* neighborInput;
    QHostInfo hostInfo;


    QString identity; // a random number that identifies a peer.
    quint32 mySeqNo; // they latest ID od message.

    QVariantMap statusList; // a list of status. <"tiger",4>
    QMap<QString,QMap<quint32, QString>> messageList; // a list of received message; [<"tiger",[<1, "hello">, <2, "world">]]

    void processTheDatagram(const QByteArray& datagram, const QHostAddress& sender, const quint16& senderPort);
    void flipCoins();
    void sendStatus(const QHostAddress& sender, const quint16 senderPort,const QString& senderIdentity,const quint32& SeqNo);
    void sendStatusList(const QHostAddress& sender, const quint16 senderPort);
    void updateList(const QString& content,const QString& senderIdentity,const quint32& SeqNo);
    void brocastMessage(const QVariantMap& message);
    void bind();
    void checkInputNeighbor(const QString& address,const quint16& post);

    void creatLocalNeighbors();
    void addNeighbor(const Peer& newPeer);


};


#endif // CHATDIALOG_H
