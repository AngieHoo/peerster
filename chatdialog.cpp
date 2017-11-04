#include "chatdialog.h"

QByteArray fromStringToQByteArray(const QString &s);

ChatDialog::ChatDialog()
{
    setWindowTitle("Peerster");
    QHBoxLayout *layout = new QHBoxLayout(this);

//---------------------------------------------------------
    QLabel* chatLabel = new QLabel(this);
    chatLabel->setText("Chat messages:");

    textview = new QTextEdit(this);
    textview->setReadOnly(true);

    QLabel* inputChatLabel = new QLabel(this);
    inputChatLabel->setText("Input messages:");

    textinput = new TextInput(this);


    QVBoxLayout *layoutText = new QVBoxLayout(this);
    layoutText->addWidget(chatLabel, 1);
    layoutText->addWidget(textview,20);
    layoutText->addWidget(inputChatLabel,1);
    layoutText->addWidget(textinput,10);

//---------------------------------------------------------
    QLabel* neighborLabel = new QLabel(this);
    neighborLabel->setText("Neighbor:");

    onlineNeighbor = new QTextEdit(this);
    onlineNeighbor->setReadOnly(true);

    QLabel* inputNeighborPrompt = new QLabel(this);
    inputNeighborPrompt->setText("Add new Neighbors:");

    neighborInput = new QLineEdit(this);

    QVBoxLayout *layoutNeighbor = new QVBoxLayout(this);

    layoutNeighbor->addWidget(neighborLabel);
    layoutNeighbor->addWidget(onlineNeighbor);
    layoutNeighbor->addWidget(inputNeighborPrompt);
    layoutNeighbor->addWidget(neighborInput);
//---------------------------------------------------------
    QLabel* peersLable = new QLabel(this);
    peersLable->setText("Origin Peers:");

    privateChatList = new QListWidget(this);

    QVBoxLayout *layoutPeer = new QVBoxLayout(this);
    layoutPeer->addWidget(peersLable);
    layoutPeer->addWidget(privateChatList);

//---------------------------------------------------------
    QPushButton *addFileButtn = new QPushButton("Upload Files", this);
    addFileButtn->setFocusPolicy(Qt::NoFocus);

    QLabel* lableUploadFile = new QLabel("Uploaded file list:",this);
    upLoadFiles = new QTextEdit(this);
    QVBoxLayout *layoutSearch = new QVBoxLayout(this);
    searchInput = new QLineEdit(this);
    QPushButton* searchButton = new QPushButton("search", this);
    searchButton->setFocusPolicy(Qt::NoFocus);
    matchFileList = new QListWidget(this);

    QHBoxLayout *subSearchLayout = new QHBoxLayout(this);
    subSearchLayout->addWidget(searchInput);
    subSearchLayout->addWidget(searchButton);

    layoutSearch->addWidget(lableUploadFile);
    layoutSearch->addWidget(upLoadFiles);
    layoutSearch->addWidget(addFileButtn);
    layoutSearch->addLayout(subSearchLayout);
    layoutSearch->addWidget(matchFileList);

//---------------------------------------------------------
    layout->addLayout(layoutText);
    layout->addLayout(layoutNeighbor);
    layout->addLayout(layoutPeer);
    layout->addLayout(layoutSearch);
//---------------------------------------------------------

    textinput->setFocus();
    setLayout(layout);

    //connect(downLoadButtn, SIGNAL(clicked(bool)), this, SLOT(sendDownLoadRequest()));
    connect(textinput, SIGNAL(returnPressed()), this, SLOT(sendMyMessage()));
    connect(neighborInput, SIGNAL(returnPressed()), this, SLOT(addNewNeighbor()));
    connect(privateChatList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(choosePeer(QListWidgetItem*)));
    connect(searchButton, SIGNAL(clicked(bool)), this, SLOT(searchFiles()));
    connect(matchFileList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(chooseMatchFile(QListWidgetItem*)));
    connect(addFileButtn, SIGNAL(clicked(bool)), this, SLOT(chooseFiles()));


    privateDialog = new QInputDialog(this);
    privateDialog->setOkButtonText("Send");
    privateDialog->setCancelButtonText("Cancel");
    privateDialog->setLabelText("Input chatting message:");
    privateDialog->hide();

    connect(privateDialog, SIGNAL(accepted()), this, SLOT(sendPrivateMessage()));


    //QStringList strPathList = fileDialog.selectedFiles();


    control = new Control(this);
    connect(control, SIGNAL(displayNewNeighbor(const QHostAddress&,const quint16&)), this, SLOT(displayNewNeighbor(const QHostAddress&,const quint16&)));
    connect(control, SIGNAL(displayNewMessage(QString)), this, SLOT(displayNewMessage(QString)));
    connect(control, SIGNAL(addNewRouitngnID(QString)), this, SLOT(addChatPeer(QString)));
    connect(control, SIGNAL(addNewMatchedFiles(QString,QString)), this, SLOT(addNewMatchedFiles(QString, QString)));

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

void ChatDialog::chooseMatchFile(QListWidgetItem *file)
{
    control->downloadMatchFile(file->text());
}

void ChatDialog::chooseFiles()
{
    QFileDialog fileDialog;
    fileDialog.setWindowTitle("Open files");
    fileDialog.setDirectory(".");
    fileDialog.setFileMode(QFileDialog::ExistingFiles);

    QStringList strPathList;
    if (fileDialog.exec() == QDialog::Accepted)
    {
         strPathList = fileDialog.selectedFiles();
         control->uploadFiles(strPathList);
         for (auto p : strPathList) {
             upLoadFiles->append(p);
         }
    }
    qDebug() << strPathList;
}


void ChatDialog::searchFiles()
{
    if (searchInput->text().size() == 0) {
        QMessageBox::about(NULL, "Warning", "Please input key words!");
        return;
    }
    matchFileList->clear();
    control->searchFiles(searchInput->text());
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

void ChatDialog::addNewMatchedFiles(const QString& fileName,const QString& originID)
{
    new QListWidgetItem(fileName + ":" + originID, matchFileList);
}

QByteArray fromStringToQByteArray(const QString& s) {
    QByteArray ret;
    bool ok;
    for (int i = 0; i < s.size(); i += 2) {
        quint16 input = s.mid(i, 2).toInt(&ok, 16);
        //qDebug() << char(input & 0xff);
        ret.append(char(input & 0xff));
    }

    qDebug() << "from " << s << " to" << ret << "size:" << ret.size();

    return ret;
}
