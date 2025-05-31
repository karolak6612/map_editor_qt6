#include "OtbmReader.h"
#include "Item.h"          // For Item class
#include "ItemManager.h"   // For ItemManager (used in readItem)
#include <QIODevice>       // For QIODevice::ReadOnly
#include <QDebug>

// Define consts if not already in OtbmTypes.h or make them class static consts
// For this subtask, assume they are defined as above in OtbmReader.h
// (They are indeed in OtbmReader.h as global consts for this scope)

OtbmReader::OtbmReader(QDataStream& stream) : m_stream(stream) {
    m_stream.setByteOrder(QDataStream::LittleEndian);
}

OtbmReader::~OtbmReader() {}

quint8 OtbmReader::peekByte() {
    quint8 byte = 0; // Initialize to a default value
    if (m_stream.device() && m_stream.device()->peek(reinterpret_cast<char*>(&byte), 1) == 1) {
        return byte;
    }
    // If device is null or peek fails, stream status might be set, or we return 0
    // Consider setting stream status here if peek fails unexpectedly not due to EOF
    return byte; // Will be 0 if peek failed or returned 0 bytes
}

bool OtbmReader::expectByte(quint8 expected) {
    quint8 byte;
    m_stream >> byte;
    if (m_stream.status() != QDataStream::Ok || byte != expected) {
        qWarning() << "OtbmReader: Expected byte" << Qt::hex << expected << "but got" << byte << "or stream error. Status:" << m_stream.status();
        if (m_stream.status() == QDataStream::Ok) { // If status was Ok but byte didn't match
            m_stream.setStatus(QDataStream::ReadCorruptData); // Mark stream as corrupt
        }
        return false;
    }
    return true;
}

bool OtbmReader::enterNode(quint8& nodeType) {
    if (!expectByte(OTBM_NODE_START)) return false;
    m_stream >> nodeType;
    if (m_stream.status() != QDataStream::Ok) {
        qWarning() << "OtbmReader: Failed to read node type after OTBM_NODE_START. Status:" << m_stream.status();
        return false;
    }
    m_nodeTypeStack.push_back(nodeType);
    // qDebug() << "OtbmReader: Entered node type" << Qt::hex << nodeType;
    return true;
}

bool OtbmReader::leaveNode() {
    if (m_nodeTypeStack.isEmpty()) {
        qWarning() << "OtbmReader: leaveNode called with empty node stack.";
        m_stream.setStatus(QDataStream::ReadCorruptData); // Indicates a structural error
        return false;
    }

    // This simplified leaveNode relies on the consuming logic to position the stream
    // correctly before OTBM_NODE_END. A more robust version might skip unknown children
    // or unread attributes until OTBM_NODE_END is found.
    if (!expectByte(OTBM_NODE_END)) {
        qWarning() << "OtbmReader: Failed to read OTBM_NODE_END for node type" << Qt::hex << m_nodeTypeStack.last();
        // Stream status is already set by expectByte on failure
        return false;
    }
    // qDebug() << "OtbmReader: Left node type" << Qt::hex << m_nodeTypeStack.last();
    m_nodeTypeStack.pop_back();
    return true;
}

bool OtbmReader::nextAttributeId(quint8& attrId) {
    if (m_stream.atEnd()) {
        // qDebug() << "OtbmReader::nextAttributeId - Stream at end.";
        return false;
    }

    quint8 next = peekByte();
    if (m_stream.status() != QDataStream::Ok && m_stream.status() != QDataStream::ReadPastEnd) {
        // An actual error occurred during peek
        qWarning() << "OtbmReader::nextAttributeId - Peek byte failed. Status:" << m_stream.status();
        return false;
    }

    if (next == OTBM_NODE_END || next == OTBM_NODE_START) {
        // These markers indicate end of attributes for the current node, or start of a child node.
        // qDebug() << "OtbmReader::nextAttributeId - Found node end/start marker:" << Qt::hex << next;
        return false;
    }

    m_stream >> attrId; // Consume the attribute ID
    if (m_stream.status() != QDataStream::Ok) {
        qWarning() << "OtbmReader::nextAttributeId - Failed to read attribute ID. Status:" << m_stream.status();
        return false;
    }
    return true;
}

// Primitive readers: these assume the stream is positioned right after an attribute ID.
// They first read a quint16 which is the length of the actual data that follows.
bool OtbmReader::readByte(quint8& value) {
    quint16 len;
    m_stream >> len;
    if (m_stream.status() != QDataStream::Ok) {
        qWarning() << "OtbmReader::readByte - Failed to read length. Status:" << m_stream.status();
        return false;
    }
    if (len != sizeof(quint8)) {
        qWarning() << "OtbmReader::readByte - Invalid length for byte:" << len << ". Expected:" << sizeof(quint8);
        m_stream.setStatus(QDataStream::ReadCorruptData); return false;
    }
    m_stream >> value;
    return m_stream.status() == QDataStream::Ok;
}

bool OtbmReader::readU16(quint16& value) {
    quint16 len;
    m_stream >> len;
    if (m_stream.status() != QDataStream::Ok) {
        qWarning() << "OtbmReader::readU16 - Failed to read length. Status:" << m_stream.status();
        return false;
    }
    if (len != sizeof(quint16)) {
         qWarning() << "OtbmReader::readU16 - Invalid length for U16:" << len << ". Expected:" << sizeof(quint16);
         m_stream.setStatus(QDataStream::ReadCorruptData); return false;
    }
    m_stream >> value;
    return m_stream.status() == QDataStream::Ok;
}

bool OtbmReader::readU32(quint32& value) {
    quint16 len;
    m_stream >> len;
    if (m_stream.status() != QDataStream::Ok) {
        qWarning() << "OtbmReader::readU32 - Failed to read length. Status:" << m_stream.status();
        return false;
    }
    if (len != sizeof(quint32)) {
        qWarning() << "OtbmReader::readU32 - Invalid length for U32:" << len << ". Expected:" << sizeof(quint32);
        m_stream.setStatus(QDataStream::ReadCorruptData); return false;
    }
    m_stream >> value;
    return m_stream.status() == QDataStream::Ok;
}

bool OtbmReader::readString(QString& value) {
    quint16 len; // This is the length of the string data itself, not a "data length" for the length field
    m_stream >> len;
    if (m_stream.status() != QDataStream::Ok) {
        qWarning() << "OtbmReader::readString - Failed to read string data length. Status:" << m_stream.status();
        return false;
    }
    if (len == 0) {
        value = QString();
        return true;
    }
    QByteArray ba(len, Qt::Uninitialized); // Resize to string data length
    if (m_stream.readRawData(ba.data(), len) != len) {
        qWarning() << "OtbmReader::readString - Failed to read string data of length:" << len << ". Status:" << m_stream.status();
        m_stream.setStatus(QDataStream::ReadCorruptData);
        return false;
    }
    value = QString::fromUtf8(ba);
    return true;
}

QByteArray OtbmReader::readData(quint16 length) {
    // This method is for when the length is already known (e.g. after reading an attribute's dataLength field)
    // and we just need to read that many bytes.
    if (length == 0) return QByteArray();

    QByteArray ba(length, Qt::Uninitialized);
    if (m_stream.readRawData(ba.data(), length) != length) {
        qWarning() << "OtbmReader::readData - Failed to read data of length:" << length << ". Status:" << m_stream.status();
        m_stream.setStatus(QDataStream::ReadCorruptData);
        return QByteArray(); // Return empty on failure
    }
    return ba;
}


Item* OtbmReader::readItem(ItemManager* itemManager) {
    // This method assumes the OTBM_ITEM node type has been read by enterNode,
    // or if called standalone for a single item, the stream is at the item's server ID.
    // For now, this OtbmReader is general. A map loader would call enterNode(OTBM_ITEM) first.
    // Let's assume for this specific method, it's called when an OTBM_ITEM node is expected.
    // The Item's ServerID is part of the node properties, not an attribute.

    quint16 itemId;
    m_stream >> itemId; // Read Item Server ID (part of OTBM_ITEM node properties, not an attribute)
    if (m_stream.status() != QDataStream::Ok) {
        qWarning() << "OtbmReader::readItem - Failed to read item ID. Status:" << m_stream.status();
        return nullptr;
    }

    if (!itemManager) {
        qWarning() << "OtbmReader::readItem - ItemManager is null, cannot create item with ID:" << itemId;
        // To skip this item's attributes robustly, we'd need to parse them until OTBM_NODE_END
        // or a size is known. This is complex if we can't even create the item.
        // For now, consider this a fatal error for this item reading process.
        m_stream.setStatus(QDataStream::ReadCorruptData);
        return nullptr;
    }

    Item* item = itemManager->createItem(itemId);
    if (!item) {
        qWarning() << "OtbmReader::readItem - ItemManager failed to create item with ID:" << itemId;
        // Similar to above, skipping attributes is hard without parsing.
        m_stream.setStatus(QDataStream::ReadCorruptData);
        return nullptr;
    }

    // Item::unserializeOtbmAttributes is responsible for reading all attributes
    // until it encounters an OTBM_NODE_END or another indicator.
    // It will use this OtbmReader's stream.
    if (!item->unserializeOtbmAttributes(m_stream)) { // Pass the stream by reference
        qWarning() << "OtbmReader::readItem - Failed to unserialize attributes for item ID:" << itemId;
        delete item; // Clean up partially created item
        // The stream status should already be set by unserializeOtbmAttributes or its callees on error
        return nullptr;
    }

    // Note: After attributes, the OTBM_NODE_END for the item itself is expected.
    // This should be handled by the caller of readItem, typically in a loop that calls leaveNode().
    // Example:
    // if (reader.enterNode(nodeType) && nodeType == OTBM_ITEM) {
    //    Item* item = reader.readItem(&itemManager);
    //    if (item) { /* store item */ }
    //    if (!reader.leaveNode()) { /* error */ }
    // }

    return item;
}
