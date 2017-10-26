#include "chatdialog.h"

ChatDialog::ChatDialog()
{
    setWindowTitle("Peerster");

    textview = new QTextEdit(this);
    textview->setReadOnly(true);

    inputTextPrompt = new QLabel(this);
    inputTextPrompt->setText("Input chatting messages:");

    textinput = new TextInput(this);
    textinput->setFocus();

    QPushButton *addFileButtn = new QPushButton("send Files", this);


    onlineNeighbor = new QTextEdit(this);
    onlineNeighbor->setReadOnly(true);

    inputNeighborPrompt = new QLabel(this);
    inputNeighborPrompt->setText("Add new Neighbors:");

    neighborInput = new QLineEdit(this);

    QHBoxLayout *layout = new QHBoxLayout(this);

    QVBoxLayout *layoutText = new QVBoxLayout(this);
    layoutText->addWidget(textview);
    layoutText->addWidget(inputTextPrompt);
    layoutText->addWidget(textinput);
    layoutText->addWidget(addFileButtn);


    QVBoxLayout *layoutNeighbor = new QVBoxLayout(this);
    layoutNeighbor->addWidget(onlineNeighbor);
    layoutNeighbor->addWidget(inputNeighborPrompt);
    layoutNeighbor->addWidget(neighborInput);

    privateChatList = new QListWidget(this);

    layout->addLayout(layoutText);
    layout->addLayout(layoutNeighbor);
    layout->addWidget(privateChatList);


    setLayout(layout);

    connect(textinput, SIGNAL(returnPressed()), this, SLOT(sendMyMessage()));
    connect(neighborInput, SIGNAL(returnPressed()), this, SLOT(addNewNeighbor()));
    connect(privateChatList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(choosePeer(QListWidgetItem*)));

    privateDialog = new QInputDialog(this);
    privateDialog->setOkButtonText("Send");
    privateDialog->setCancelButtonText("Cancel");
    privateDialog->setLabelText("Input chatting message:");
    privateDialog->hide();

    connect(privateDialog, SIGNAL(accepted()), this, SLOT(sendPrivateMessage()));

    fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("Open files");
    fileDialog->setDirectory(".");
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->hide();

    connect(addFileButtn, SIGNAL(clicked(bool)), this, SLOT(chooseFiles()));
    //QStringList strPathList = fileDialog.selectedFiles();




    control = new Control(this);
    connect(control, SIGNAL(displayNewNeighbor(const QHostAddress&,const quint16&)), this, SLOT(displayNewNeighbor(const QHostAddress&,const quint16&)));
    connect(control, SIGNAL(displayNewMessage(QString)), this, SLOT(displayNewMessage(QString)));
    connect(control, SIGNAL(addNewRouitngnID(QString)), this, SLOT(addChatPeer(QString)));

    control->start();
}




//Private slots:
void ChatDialog::sendMyMessage() {
    QString content = textinput->toPlainText();
    if (content.size() == 0) {
        QMessageBox::about(NULL, "Warning", "Please input something!");
        return;
    }
    control->sendMyMessage(content);
    textview->append(control->getIdentity() + ":" + content);
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

void ChatDialog::choosePeer(QListWidgetItem *peer)
{
    control->addPrivateChatPeer(peer->text());
    privateDialog->show();

}

void ChatDialog::chooseFiles()
{
    fileDialog->show();
    QStringList strPathList;
    if (fileDialog->exec() == QDialog::Accepted)
    {
         strPathList = fileDialog->selectedFiles();
         control->uploadFiles(strPathList);
    }
    qDebug() << strPathList;
}


void ChatDialog::displayNewNeighbor(const QHostAddress& IP, const quint16& port){
    qDebug() << "ChatDialog::displayNewNeighbor";
    onlineNeighbor->append(IP.toString() + ":" + QString::number(port));
}

void ChatDialog::displayNewMessage(const QString& content){
    textview->append(content);
}

void ChatDialog::addChatPeer(const QString & originID)
{
    new QListWidgetItem(originID, privateChatList);

}

void ChatDialog::sendPrivateMessage()
{
    qDebug() << "ChatDialog::sendPrivteMessge";
    QString content= privateDialog->textValue();
    if (content == "") {
        QMessageBox::about(NULL, "Warning", "Invalid Address!");
        return;
    }
    control->sendMyPrivateMessage(content);
    privateDialog->setTextValue("");
}


