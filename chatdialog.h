#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QInputDialog>
#include <QPushButton>
#include <QFileDialog>


#include <qmath.h>

#include "textinput.h"
#include "control.h"
using namespace peerster;

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    ChatDialog();

public slots:
     void displayNewNeighbor(const QHostAddress&, const quint16&);
     void displayNewMessage(const QString&);
     void addChatPeer(const QString&);
     void sendPrivateMessage();
     void addNewMatchedFiles(const QString&, const QString&);

private slots: 
     void sendMyMessage();
     void addNewNeighbor();
     void choosePeer(QListWidgetItem* peer);
     void chooseMatchFile(QListWidgetItem* file);
     void chooseFiles();
     //void sendDownLoadRequest();
     void searchFiles();

signals:
     void finishLookUp();


private:
    Control* control;
    QTextEdit* textview;
    TextInput* textinput;
    QTextEdit* onlineNeighbor;
    QLineEdit* neighborInput;

    QListWidget* privateChatList;
    QInputDialog* privateDialog;
    QLineEdit* downLoadInput;
    QLineEdit* downLoadNodeID;
    QLineEdit* searchInput;
    QListWidget* matchFileList;
    QTextEdit* upLoadFiles;


};


#endif // CHATDIALOG_H
