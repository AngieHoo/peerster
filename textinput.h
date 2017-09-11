#ifndef TEXTINPUT_H
#define TEXTINPUT_H
#include <QTextEdit>
#include <QKeyEvent>

class TextInput : public QTextEdit
{
Q_OBJECT

public:
    explicit TextInput(QWidget *parent = 0);
private:
    void keyPressEvent(QKeyEvent *event);
    ~TextInput();

signals:
    void returnPressed();
};

#endif // TEXTINPUT_H
