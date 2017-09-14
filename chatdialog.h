#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QVBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QTimer>

#include <qmath.h>

#include "netsocket.h"
#include "textinput.h"

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    ChatDialog();

public slots:
     void sendMyMsg2RandomPeer();
     void readPendingDatagrams();    

private slots:
     void doAntiEntropy();
     void checkReply();

private:
    QTextEdit* textview;
    TextInput* textinput;
    NetSocket* sock;
    QTimer* timer;

    QString identity; // a random number that identifies a peer.
    quint32 mySeqNo; // they latest ID od message.

    QVariantMap statusList; // a list of status. <"tiger",4>
    QMap<QString,QMap<quint32, QString>> messageList; // a list of received message; [<"tiger",[<1, "hello">, <2, "world">]]

    void processTheDatagram(const QByteArray& datagram, const QHostAddress& sender, const quint16& senderPort);
    void flipCoins(const QString& originID,const quint32& SeqNo);
    void flipCoins();
    void sendStatus(const QHostAddress& sender, const quint16 senderPort,const QString& senderIdentity,const quint32& SeqNo);
    void sendStatusList(const QHostAddress& sender, const quint16 senderPort);
    void updateList(const QString& content,const QString& senderIdentity,const quint32& SeqNo);
    void brocastMessage(const QVariantMap& message);

};


#endif // CHATDIALOG_H
