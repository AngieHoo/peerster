#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QVariantMap>
#include <QHostInfo>
#include "textinput.h"
#include "library.h"

class NetSocket;

class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog();

public slots:
     void sendMessage();

     void readPendingDatagrams();

private:
    void addMessage(QVariant message);
    void processTheDatagram(QByteArray datagram);
	QTextEdit *textview;
    TextInput *textinput;
    NetSocket *sock;

};



class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
    NetSocket(QObject *parent = 0);

	// Bind this socket to a Peerster-specific default port.
	bool bind();


public slots:
    void sendMessage(QVariantMap message);

private:
	int myPortMin, myPortMax;
};

#endif // PEERSTER_MAIN_HH
