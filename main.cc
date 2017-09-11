
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

#include "main.hh"

ChatDialog::ChatDialog()
{
    setWindowTitle("Peerster");
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

    sock = new NetSocket(this);
    // Register a callback on the textinput's returnPressed signal
    // so that we can send the message entered by the user.
    connect(textinput, SIGNAL(returnPressed()),
            this, SLOT(sendMessage()));
    //todo:.........
    connect(sock, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    if (!sock->bind())
        exit(1);
}


void ChatDialog::sendMessage() {
    QVariant text(textinput->toPlainText());
    QVariantMap map;
    map["ChatText"] = text;
    sock->sendMessage(map);
    textinput->clear();
}

void ChatDialog::readPendingDatagrams() {

    while (sock->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(sock->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        sock->readDatagram(datagram.data(), datagram.size(),
                           &sender, &senderPort);
        processTheDatagram(datagram);
    }
    return;
}

void ChatDialog::processTheDatagram(QByteArray datagram) {
    QDataStream in(&datagram, QIODevice::ReadOnly);
    QString messageType;
    in >> messageType;
    QVariant content;
    in >> content;
    qDebug() << "ChatDialog::processTheDatagram: receive message!!!!";
    qDebug() << "message type:" << messageType << ", [content:" << content << "]";
    textview->append(content.toString());


}

void ChatDialog::addMessage(QVariant message) {
    textview->append(message.toString());
    return;
}

NetSocket::NetSocket(QObject *parent) : QUdpSocket(parent)
{
    // Pick a range of four UDP ports to try to allocate by default,
    // computed based on my Unix user ID.
    // This makes it trivial for up to four Peerster instances per user
    // to find each other on the same host,
    // barring UDP port conflicts with other applications
    // (which are quite possible).
    // We use the range from 32768 to 49151 for this purpose.
    myPortMin = 32768 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 3;

}


bool NetSocket::bind()
{
    // Try to bind to each of the range myPortMin..myPortMax in turn.
    for (int p = myPortMin; p <= myPortMax; p++) {
        if (QUdpSocket::bind(p)) {
            qDebug() << "bound to UDP port " << p;
            return true;
        }
    }

    qDebug() << "Oops, no ports in my default range " << myPortMin
             << "-" << myPortMax << " available";
    return false;
}

void NetSocket::sendMessage(QVariantMap message) {
    QByteArray data;
    QDataStream str(&data, QIODevice::WriteOnly);
    for (QVariantMap::iterator it = message.begin(); it != message.end(); it++) {
        str << it.key();
        str << it.value();
        qDebug() << "NetSocket::sendMessage" << it.value();
    }

    for (int p = myPortMin; p <= myPortMax; p++) {
        writeDatagram(data,data.length(),QHostAddress::LocalHost, p);
    }
    return;
}

int main(int argc, char **argv)
{
    // Initialize Qt toolkit
    QApplication app(argc,argv);

    // Create an initial chat dialog window
    ChatDialog dialog;
    dialog.show();

    return app.exec();
}

