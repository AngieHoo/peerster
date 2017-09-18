#ifndef CONTROL_H
#define CONTROL_H

#include <QTimer>
#include <QHostInfo>
#include <QMessageBox>
#include <Qvector>

#include "model.h"
#include "netsocket.h"

class Control : public QObject
{
    Q_OBJECT
public:
    explicit Control(QObject *parent = 0);
    void checkInputNeighbor(const QString& address,const quint16& post);//done
    void sendMyMessage(const QString& content);
    void start();

public slots:
    void processTheDatagram(const QByteArray&, const QHostAddress&, const quint16&);
    void processNoReply(const Peer*);

private slots:
     void lookedUp(const QHostInfo &host);
     void doAntiEntropy();


signals:
    void displayNewNeighbor(const QString&, const QHostAddress&, const quint16&); // check
    void displayNewMessage(const QString&);//check
    void finishLookUp();

private:
     Model* model;
     NetSocket* sock;
     QTimer* timer;
     QHostInfo hostInfo;
     void bind();

     void sendMsg2Peer(Peer*, const QVariantMap&); // check
     void sendOriginMessage(const QHostAddress&, const quint16&, const QString&, const QString&,const quint32&);//
     void sendMyStatusList(const QHostAddress& sender, const quint16 senderPort);//
     void flipCoins();
     void brocastMessage(const QVariantMap& message);

};

#endif // CONTROL_H
