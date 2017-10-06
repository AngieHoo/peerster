#ifndef MODEL_H
#define MODEL_H

#include <QTimer>
#include <QHostInfo>
#include <QMessageBox>
#include <QVector>
#include <unistd.h>
#include <QTime>

#include "peer.h"
#include "lib.h"

using namespace peerster;

class Model : public QObject
{
    Q_OBJECT
public:
    explicit Model(QObject *parent = 0);
    quint16 getMyPortMin() const;
    quint16 getMyPort() const;
    quint16 getMyPortMax() const;
    quint32 getMySeqNo() const;
    Peer* getPeerRandomly() const;
    const QString& getIdentity() const;
    const QVariantMap& getStatusList() const;
    const QHash<QString, QPair<QHostAddress,quint16>>&  getRoutingTable() const;
    const QMap<QString,QMap<quint32, QString>>& getMessagelist() const;
    const QVector<Peer*> getNeighbors() const;
    const int getHighestSeq(const QString& originID) const;
    const QString getPrivateChattingPeer() const;
    void setMyPort(quint16);
    void setMyPortMin(quint16);
    void setMyPortMax(quint16);
    void setPrivateChattingPeer(const QString&);

    void updateRoutingTable(const QString& originID, const QHostAddress& senderIP, quint16 senderPort);
    Peer* addNeighbor(const QHostAddress&, const quint16&);
    Peer* getNeighbor(const QHostAddress& IP, const quint16& Port);
    bool isValidNewRoutingID(const QString& originID);
    bool isValidNewComer(const QHostAddress&, const quint16&);
    void addMyMessage(const QString& content = "");
    void addNewMessage(const QString& originID, const QHostAddress& IP, const quint16& port,const QString& content = "") ;
    void creatLocalNeighbors();

signals:
    void displayNewNeighbor(const QHostAddress&, const quint16&);
    //void test();

public slots:
    //void respondtest();
private:
    QHostAddress myIP;
    quint16 myPort;
    quint16 myPortMin;
    quint16 myPortMax;
    QVector<Peer*> neighbors;
    QString privateChatDestID;

    QString identity; // a random number that identifies a peer.
    quint32 mySeqNo; // they latest ID od message.

    QVariantMap statusList; // a list of status. <"tiger",4>
    QMap<QString,QMap<quint32, QString>> messageList; // a list of received message; [<"tiger",[<1, "hello">, <2, "world">]]  
    QHash<QString, QPair<QHostAddress,quint16>> routingTable;

};

#endif // MODEL_H
