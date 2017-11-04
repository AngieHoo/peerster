#ifndef CONTROL_H
#define CONTROL_H

#include <QTimer>
#include <QHostInfo>
#include <QMessageBox>
#include <QVector>
#include <QQueue>
#include <QDir>

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
    void uploadFiles(const QStringList &fileNameList);
    QString getIdentity();
    //void addDownLoadRequest(const QString&, const QByteArray&);
    void downloadMatchFile(const QString&);
    void searchFiles(const QString&);

public slots:
    void processTheDatagram(const QByteArray&, const QHostAddress&, const quint16&);
    void processNoReply(Peer*);


private slots:
     void lookedUp(const QHostInfo &host);
     void doAntiEntropy();
     void generateRouteMessage();
     void sendDownLoadRequest();
     void sendSearchRequest();

signals:
    void displayNewNeighbor(const QHostAddress&, const quint16&); // check
    void displayNewMessage(const QString&);//check
    void finishLookUp();
    void addNewRouitngnID(const QString&);
    void addNewMatchedFiles(const QString&, const QString&);

private:
     Model* model;
     NetSocket* sock;
     QTimer* timer;
     QTimer* timerRoute;
     QTimer* timerFileReq;
     QTimer* timerSearchFile;
     QHostInfo hostInfo;
     FileManager* fileManager;
     bool forward;
     QString downLoadDir;

     QQueue<QPair<QByteArray, QString>> requestBlocks;
     FileInfo downLoadCache;
     bool startDownloading;
     QMap<QString, QMap<QString, QByteArray>> matchedFiles;
     int matchedNum;
     bool isSearching;
     QString searchKeyWords;
     quint32 budget;

     void bind();

     void sendChatMsg2Peer(Peer*, const QVariantMap&); // check
     void sendOriginMessage(const QHostAddress&, const quint16&, const QString&, const QString&,const quint32&);//
     void sendMyStatusList(const QHostAddress& sender, const quint16 senderPort);//    
     void forwardMessageRandomly(const QVariantMap& message);
     void forwardMessage2All(const QVariantMap& message);
     void forwardMessage2Peer(Peer*, const QVariantMap&);
     void sendPrivateMessage(const QString& destinationID, const QString &originID, const QString& content, int hop);
     void sendP2PMessage(const QVariantMap& message);


     void flipCoins();

     void processStatusMessage(const QVariantMap &message, const QHostAddress& IP, const quint16& port);
     void processRumorMessage(QVariantMap &message, const QHostAddress& IP, const quint16& port, messageType type);
     void processRouteMessage(const QVariantMap &message, const QHostAddress& IP, const quint16& port);
     void processP2PMessage(const QVariantMap &message);
     void processBlockRequest(const QVariantMap &message);
     void processBlockReply(const QVariantMap &message);
     void processSearchRequest(QVariantMap &message);
     void processSearchReply(const QVariantMap &message);
     void addNewNeighbor( const QHostAddress& IP, const quint16& port);

};


#endif // CONTROL_H
