#ifndef CONTROL_H
#define CONTROL_H

#include <QTimer>
#include <QHostInfo>
#include <QMessageBox>
#include <QVector>

#include "model.h"
#include "netsocket.h"
#include "lib.h"

using namespace peerster;

class Control : public QObject
{
    Q_OBJECT
public:
    explicit Control(QObject *parent = 0);
    void checkInputNeighbor(const QString& address,const quint16& post);//done
    void sendMyMessage(const QString& content);
    void start();
    void addPrivateChatPeer(const QString&);
    void sendMyPrivateMessage(const QString& content);
    QString getIdentity();

public slots:
    void processTheDatagram(const QByteArray&, const QHostAddress&, const quint16&);
    void processNoReply(const Peer*);


private slots:
     void lookedUp(const QHostInfo &host);
     void doAntiEntropy();
     void generateRouteMessage();

signals:
    void displayNewNeighbor(const QString&, const QHostAddress&, const quint16&); // check
    void displayNewMessage(const QString&);//check
    void finishLookUp();
    void addNewRouitngnID(const QString&);

private:
     Model* model;
     NetSocket* sock;
     QTimer* timer;
     QTimer* timerRoute;
     QHostInfo hostInfo;
     bool forward;


     void bind();

     void sendMsg2Peer(Peer*, const QVariantMap&); // check
     void sendOriginMessage(const QHostAddress&, const quint16&, const QString&, const QString&,const quint32&);//
     void sendMyStatusList(const QHostAddress& sender, const quint16 senderPort);//    
     void brocastMessage(const QVariantMap& message);

     void sendPrivateMessage(const QString& destinationID, const QString &originID, const QString& content, int hop);

     void flipCoins();

     void processStatusMessage(const QVariantMap &message, const QHostAddress& IP, const quint16& port);
     void processRumorMessage(const QVariantMap &message, const QHostAddress& IP, const quint16& port, messageType type);
     void processPrivateMessage(const QVariantMap &message, const QHostAddress& IP, const quint16& port);
     void processRouteMessage(const QVariantMap &message, const QHostAddress& IP, const quint16& port);

};

#endif // CONTROL_H
