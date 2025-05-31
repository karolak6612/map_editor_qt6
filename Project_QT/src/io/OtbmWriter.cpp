#include "OtbmWriter.h"
#include "Item.h"        // For Item class, needed for writeItemNode
#include <QIODevice>     // For QIODevice::WriteOnly (not strictly needed for QDataStream operations)
#include <QByteArray>
#include <QDebug>

// Ensure constants are accessible. They are defined in OtbmWriter.h for this scope.
// const quint8 OTBM_NODE_START_W = 0xFC;
// const quint8 OTBM_NODE_END_W = 0xFD;

OtbmWriter::OtbmWriter(QDataStream& stream) : m_stream(stream) {
    m_stream.setByteOrder(QDataStream::LittleEndian);
}

OtbmWriter::~OtbmWriter() {}

// Use OTBM_NodeTypes_t as defined in OtbmTypes.h
void OtbmWriter::beginNode(OTBM_NodeTypes_t nodeType) {
    m_stream << OTBM_NODE_START_W;
    m_stream << static_cast<quint8>(nodeType);
    // qDebug() << "OtbmWriter: Began node type" << static_cast<quint8>(nodeType);
}

void OtbmWriter::endNode() {
    m_stream << OTBM_NODE_END_W;
    // qDebug() << "OtbmWriter: Ended node.";
}

// Primitive writers
void OtbmWriter::writeByte(quint8 value) {
    m_stream << value;
}

void OtbmWriter::writeU16(quint16 value) {
    m_stream << value;
}

void OtbmWriter::writeU32(quint32 value) {
    m_stream << value;
}

void OtbmWriter::writeString(const QString& value) {
    QByteArray utf8Value = value.toUtf8();
    m_stream << static_cast<quint16>(utf8Value.length());
    m_stream.writeRawData(utf8Value.constData(), utf8Value.length());
}

void OtbmWriter::writeData(const QByteArray& data) {
    m_stream.writeRawData(data.constData(), data.length());
}

// Attribute writers
void OtbmWriter::writeAttributeByte(OTBM_ItemAttribute attrId, quint8 value) {
    m_stream << static_cast<quint8>(attrId);
    m_stream << static_cast<quint16>(sizeof(quint8)); // Length of data
    m_stream << value;
}

void OtbmWriter::writeAttributeU16(OTBM_ItemAttribute attrId, quint16 value) {
    m_stream << static_cast<quint8>(attrId);
    m_stream << static_cast<quint16>(sizeof(quint16)); // Length of data
    m_stream << value;
}

void OtbmWriter::writeAttributeU32(OTBM_ItemAttribute attrId, quint32 value) {
    m_stream << static_cast<quint8>(attrId);
    m_stream << static_cast<quint16>(sizeof(quint32)); // Length of data
    m_stream << value;
}

void OtbmWriter::writeAttributeString(OTBM_ItemAttribute attrId, const QString& value) {
    m_stream << static_cast<quint8>(attrId);
    QByteArray utf8Value = value.toUtf8();
    m_stream << static_cast<quint16>(utf8Value.length()); // This is the DataLength for the attribute
    m_stream.writeRawData(utf8Value.constData(), utf8Value.length()); // This is the Data
}

void OtbmWriter::writeAttributeData(OTBM_ItemAttribute attrId, const QByteArray& data) {
    m_stream << static_cast<quint8>(attrId);
    m_stream << static_cast<quint16>(data.length());
    m_stream.writeRawData(data.constData(), data.length());
}


// Higher-level object writing
void OtbmWriter::writeItemNode(const Item* item) {
    if (!item) {
        qWarning() << "OtbmWriter::writeItemNode - Attempted to write null item.";
        return;
    }
    // Item::serializeOtbmNode is responsible for writing its node type (OTBM_ITEM),
    // its server ID, and then calling its own serializeOtbmAttributes method.
    // The OtbmWriter's beginNode/endNode are for higher-level nodes (MapData, TileArea, Tile etc.)
    // that would wrap item nodes or other data.
    item->serializeOtbmNode(m_stream);
}
