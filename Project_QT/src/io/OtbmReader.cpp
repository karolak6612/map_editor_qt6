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


Item* OtbmReader::readItem(ItemManager* itemManager, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) {
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

    quint8 initialSubtype = 0;
    bool initialSubtypeWasRead = false;

    // Handle OTBMv1 initial subtype reading (mapOtbmFormatVersion == 0 means OTBM_VERSION_1)
    // Note: Assuming mapOtbmFormatVersion is 0 for OTBM_VERSION_1.
    // This value comes from the OTBM_ROOT_ATTR_VERSION_MAJOR attribute of the map file itself.
    if (mapOtbmFormatVersion == 0) {
        const ItemProperties& iType = itemManager->getItemProperties(itemId);
        if (iType.isStackable || iType.group == ITEM_GROUP_SPLASH || iType.group == ITEM_GROUP_FLUID) {
            m_stream >> initialSubtype; // Read the subtype byte
            if (m_stream.status() != QDataStream::Ok) {
                qWarning() << "OtbmReader::readItem - Failed to read initial subtype for OTBMv1 item ID:" << itemId << "Stream status:" << m_stream.status();
                // Don't return nullptr yet, try to read attributes if item can be created
                // but log this potential corruption.
                m_stream.setStatus(QDataStream::ReadCorruptData); // Mark as corrupt
            } else {
                initialSubtypeWasRead = true;
                // qDebug() << "OtbmReader::readItem - Read initial subtype" << initialSubtype << "for OTBMv1 item ID:" << itemId;
            }
        }
    }

    Item* item = itemManager->createItem(itemId);
    if (!item) {
        qWarning() << "OtbmReader::readItem - ItemManager failed to create item with ID:" << itemId;
        // Need to skip attributes if item creation fails but stream is ok.
        // This is complex. For now, assume if createItem fails, it's a critical issue.
        if (m_stream.status() == QDataStream::Ok) m_stream.setStatus(QDataStream::ReadCorruptData);
        return nullptr;
    }

    // If an initial subtype was read for OTBMv1, apply it now.
    if (initialSubtypeWasRead) {
        const ItemProperties& iType = itemManager->getItemProperties(itemId); // Get props again for safety
        if (iType.isStackable) {
            // For stackable items, OTBMv1 subtype 0 often meant 100, but here we treat it as actual count.
            // If subtype is 0, it might mean default count (e.g. 1 for non-ammo). RME saved 0 as 0.
            // Let's assume initialSubtype is the actual count or becomes it.
            // If initialSubtype is 0 for a stackable, it might mean count 1 or indicate an issue.
            // For now, if stackable and subtype is 0, use 1, else use subtype.
            item->setCount(initialSubtype == 0 ? 1 : initialSubtype);
        } else if (iType.group == ITEM_GROUP_SPLASH || iType.group == ITEM_GROUP_FLUID) {
            item->setCharges(initialSubtype); // Fluids/splashes can have subtype 0 (empty)
        }
        // Item's modified flag should be false after this setup
        item->setModified(false);
    }

    if (!item->unserializeOtbmAttributes(m_stream, otbItemsMajorVersion, otbItemsMinorVersion)) {
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
