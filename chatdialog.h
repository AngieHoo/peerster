#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QLineEdit>



#include <qmath.h>

#include "textinput.h"
#include "control.h"

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    ChatDialog();

public slots:
     void displayNewNeighbor(const QString&, const QHostAddress&, const quint16&);
     void displayNewMessage(const QString&);

private slots: 
     void sendMyMessage();
     void addNewNeighbor();

signals:
     void finishLookUp();

private:
    Control* control;
    QTextEdit* textview;
    TextInput* textinput;
    QTextEdit* onlineNeighbor;
    QLineEdit* neighborInput;



};


#endif // CHATDIALOG_H
