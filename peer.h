#ifndef PEER_H
#define PEER_H
#include <QVector>
#include <QHostAddress>
#include <QHostInfo>
#include <QTimer>

class Peer : public QObject
{
    Q_OBJECT
public:
    Peer(const QHostAddress& IP, const quint16& Port, QObject *parent, const QString& DNS = "");
    Peer();
    Peer(const Peer&);
    QString getDNS() const;
    QHostAddress getIP() const;
    quint16 getPort() const;
    QVariantMap getMessage() const;
    void setMessage(const QVariantMap& message);
    void stopTimer();
    void startTimer();

    void setDNS(const QString&);
    void setIP(const QHostAddress&);
    void SetPort(const quint16&);

signals:
    void timerOut(const Peer* self);


private slots:

    void sendTimeOutMsg();

private:
    QTimer timer;
    QVariantMap message;
    QString DNS;
    QHostAddress IP;
    quint16 Port;


};

#endif // PEER_H
