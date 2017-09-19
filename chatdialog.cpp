#include "chatdialog.h"

ChatDialog::ChatDialog()
{
    setWindowTitle("Peerster");



    textview = new QTextEdit(this);
    textview->setReadOnly(true);

    textinput = new TextInput(this);
    textinput->setFocus();

    onlineNeighbor = new QTextEdit(this);
    onlineNeighbor->setReadOnly(true);

    neighborInput = new QLineEdit(this);

    QHBoxLayout *layout = new QHBoxLayout(this);

    QVBoxLayout *layoutText = new QVBoxLayout(this);
    layoutText->addWidget(textview);
    layoutText->addWidget(textinput);

    QVBoxLayout *layoutNeighbor = new QVBoxLayout(this);
    layoutNeighbor->addWidget(onlineNeighbor);
    layoutNeighbor->addWidget(neighborInput);

    layout->addLayout(layoutText);
    layout->addLayout(layoutNeighbor);

    setLayout(layout);

    connect(textinput, SIGNAL(returnPressed()), this, SLOT(sendMyMessage()));
    connect(neighborInput, SIGNAL(returnPressed()), this, SLOT(addNewNeighbor()));


    control = new Control(this);
    connect(control, SIGNAL(displayNewNeighbor(const QString&,const QHostAddress&,const quint16&)), this, SLOT(displayNewNeighbor(const QString&,const QHostAddress&,const quint16&)));
    connect(control, SIGNAL(displayNewMessage(QString)), this, SLOT(displayNewMessage(QString)));

    control->start();
}

//private function:



//Private slots:
void ChatDialog::sendMyMessage() {
    control->sendMyMessage(textinput->toPlainText());
    textview->append(control->getIdentity() + ":" + textinput->toPlainText());
    textinput->clear();
    return;
}

void ChatDialog::addNewNeighbor(){
    QString str = neighborInput->text();
    QString address = str.left(str.indexOf(':'));
    quint16 port = str.mid(str.indexOf(':') + 1).toInt();
    control->checkInputNeighbor(address,port);
    neighborInput->clear();
    return;
}


//Public Slots:
void ChatDialog::displayNewNeighbor(const QString& DNS, const QHostAddress& IP, const quint16& port){
    qDebug() << "ChatDialog::displayNewNeighbor";
    onlineNeighbor->append(DNS + "(" + IP.toString() + ")"+ ":" + QString::number(port));
}

void ChatDialog::displayNewMessage(const QString& content){
   textview->append(content);
}



