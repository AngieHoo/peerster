#ifndef LIB_H
#define LIB_H

namespace peerster{

const QString DEST = "Dest";
const QString ORIGIN = "Origin";
const QString CHAT_TEXT= "ChatText";
const QString HOP_LIMIT = "HopLimit";
const QString SEQ_NO = "SeqNo";
const QString WANT = "Want";

enum messageType {
    CHAT_MESSAGE,
    ROUT_MESSAGE,
    STATUS_MESSAGE
} ;


}
#endif // LIB_H
