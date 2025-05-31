#ifndef OTBMREADER_H
#define OTBMREADER_H

#include <QDataStream>
#include <QString>
#include <QList>
#include <QScopedPointer>
#include "OtbmTypes.h"

// Forward declarations
class Item;
class ItemManager;

// OTBM structure markers (common values)
const quint8 OTBM_NODE_START = 0xFC;
const quint8 OTBM_NODE_END = 0xFD;
const quint8 OTBM_ESCAPE_CHAR = 0xFE;

class OtbmReader {
public:
    explicit OtbmReader(QDataStream& stream);
    ~OtbmReader();

    // Node operations
    bool enterNode(quint8& nodeType);
    bool leaveNode();

    // Primitive data type reading (from current node's properties)
    // These assume the stream is positioned right after an attribute ID byte.
    // They read the data length (quint16) then the data.
    bool readByte(quint8& value);
    bool readU16(quint16& value);
    bool readU32(quint32& value);
    bool readString(QString& value);
    QByteArray readData(quint16 length);

    // Attribute reading (reads attribute ID, then its value using above methods)
    // Returns false if attribute ID cannot be read or if value reading fails.
    bool nextAttributeId(quint8& attrId);

    // Higher-level object reading
    Item* readItem(ItemManager* itemManager, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion);

    QDataStream& stream() { return m_stream; }

private:
    bool expectByte(quint8 expected);
    quint8 peekByte();

    QDataStream& m_stream;
    QList<quint8> m_nodeTypeStack;
};

#endif // OTBMREADER_H
