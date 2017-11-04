#ifndef LIB_H
#define LIB_H
#include <QString>

namespace peerster{

const QString DEST = "Dest";
const QString ORIGIN = "Origin";
const QString CHAT_TEXT= "ChatText";
const QString HOP_LIMIT = "HopLimit";
const QString SEQ_NO = "SeqNo";
const QString WANT = "Want";
const QString LAST_IP = "LastIP";
const QString LAST_PORT = "LastPort";
const QString BLOCK_REQUEST = "BlockRequest";
const QString BLOCK_REPLY = "BlockReply";
const QString DATA = "Data";
const QString SEARCH = "Search";
const QString BUDGET = "Budget";
const QString SEARCH_REPLY = "SearchReply";
const QString MATCH_ID_S = "MatchIDs";
const QString MATCH_NAME= "MatchNames";


const int FLIP_COIN = 2;
const quint64 BLOCK_BYTE_SIZE = 8 * 1024;
const int INIT_HOP_STEP = 10;
const int HASH_SIZE = 20;

enum messageType {
    CHAT_MESSAGE,
    ROUT_MESSAGE,
    STATUS_MESSAGE
};
struct FileInfo {
    FileInfo(const QString& fileName, const QByteArray& metafile, const QByteArray& metaHashVal, quint64 size)
         : metafile(metafile), fileName(fileName), metaHashVal(metaHashVal), size(size){}
    FileInfo(){}
    QByteArray metafile;
    QString fileName;
    QByteArray metaHashVal;
    quint32 size;
};



}
#endif // LIB_H
