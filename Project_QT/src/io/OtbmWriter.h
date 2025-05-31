#ifndef OTBMWRITER_H
#define OTBMWRITER_H

#include <QDataStream>
#include <QString>
#include "OtbmTypes.h" // From previous step

// Forward declarations
class Item; // For writeItemNode method

// OTBM structure markers (ensure these are consistent with OtbmReader.h)
const quint8 OTBM_NODE_START_W = 0xFC; // Suffix _W to avoid redefinition if in same scope as reader's
const quint8 OTBM_NODE_END_W = 0xFD;
const quint8 OTBM_ESCAPE_CHAR_W = 0xFE;


class OtbmWriter {
public:
    explicit OtbmWriter(QDataStream& stream);
    ~OtbmWriter();

    // Node operations
    void beginNode(OtbmNodeType nodeType); // Changed from OTBM_NodeTypes_t to OtbmNodeType for consistency if enum class was used
                                         // Keeping OTBM_NodeTypes_t as per OtbmTypes.h content
    void beginNode(OTBM_NodeTypes_t nodeType);
    void endNode();

    // Primitive data type writing (raw data, no attribute ID or length prefix here)
    void writeByte(quint8 value);
    void writeU16(quint16 value);
    void writeU32(quint32 value);
    void writeString(const QString& value); // Writes u16 length + UTF-8 data
    void writeData(const QByteArray& data); // Writes raw byte array

    // Attribute writing methods (writes AttributeID, DataLength, then Data)
    void writeAttributeByte(OTBM_ItemAttribute attrId, quint8 value);
    void writeAttributeU16(OTBM_ItemAttribute attrId, quint16 value);
    void writeAttributeU32(OTBM_ItemAttribute attrId, quint32 value);
    void writeAttributeString(OTBM_ItemAttribute attrId, const QString& value);
    void writeAttributeData(OTBM_ItemAttribute attrId, const QByteArray& data);

    // Higher-level object writing
    // This method will handle the OTBM_ITEM node itself.
    void writeItemNode(const Item* item, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion);

    QDataStream& stream() { return m_stream; }

private:
    QDataStream& m_stream;
};

#endif // OTBMWRITER_H
