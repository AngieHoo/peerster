#ifndef MODEL_H
#define MODEL_H

#include <QTimer>
#include <QHostInfo>
#include <QMessageBox>
#include <QVector>
#include <unistd.h>
#include <QTime>

#include "peer.h"

class Model : public QObject
{
    Q_OBJECT
public:
    explicit Model(QObject *parent = 0);
    quint16 getMyPortMin() const;
    quint16 getMyPort() const;
    quint16 getMyPortMax() const;
    QString getIdentity() const;
    quint32 getMySeqNo() const;
    Peer* getPeerRandomly() const;
    QVariantMap getStatusList() const;
    QMap<QString,QMap<quint32, QString>> getMessagelist() const;
    void setMyPort(quint16);
    void setMyPortMin(quint16);
    void setMyPortMax(quint16);
    Peer* addNeighbor(const QString&, const QHostAddress&, const quint16&);
    Peer* getNeighbor(const QHostAddress& IP, const quint16& Port);
    bool isValidNewComer(const QString&, const QHostAddress&, const quint16&);
    void sendMyMessage(const QString&);
    void receiveNewMessage(const QString&, const QString&);
    void creatLocalNeighbors();

signals:
    void displayNewNeighbor(const QString&, const QHostAddress&, const quint16&);
    //void test();

public slots:
    //void respondtest();
private:
    quint16 myPort;
    quint16 myPortMin;
    quint16 myPortMax;
    QVector<Peer*> neighbors;


    QString identity; // a random number that identifies a peer.
    quint32 mySeqNo; // they latest ID od message.

    QVariantMap statusList; // a list of status. <"tiger",4>
    QMap<QString,QMap<quint32, QString>> messageList; // a list of received message; [<"tiger",[<1, "hello">, <2, "world">]]

};

#endif // MODEL_H
