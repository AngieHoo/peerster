#ifndef LIB_H
#define LIB_H

namespace peerster{

const QString DEST = "Dest";
const QString ORIGIN = "Origin";
const QString CHAT_TEXT= "ChatText";
const QString HOP_LIMIT = "HopLimit";
const QString SEQ_NO = "SeqNo";
const QString WANT = "Want";
const QString LAST_IP = "LastIP";
const QString LAST_PORT = "LastPort";

const int FLIP_COIN = 5;

enum messageType {
    CHAT_MESSAGE,
    ROUT_MESSAGE,
    STATUS_MESSAGE
};


}
#endif // LIB_H
