#include "textinput.h"


TextInput::TextInput(QWidget *parent): QTextEdit(parent){}

void TextInput::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return)
        emit returnPressed();
    else QTextEdit::keyPressEvent(event);
    return;
}
 TextInput::~TextInput(){}
