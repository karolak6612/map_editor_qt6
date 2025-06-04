#ifndef NETWORKMESSAGE_H
#define NETWORKMESSAGE_H

#include <QByteArray>
#include <QString>
#include <QDataStream>
#include <QIODevice>
#include <QColor>
#include <stdexcept>

// Forward declarations
class Position;

/**
 * @brief Network message class for packet serialization/deserialization
 * 
 * This class provides a Qt-based implementation of the NetworkMessage functionality
 * from the original wxwidgets LiveSocket system. It handles binary packet creation,
 * reading, and writing with automatic buffer management.
 * 
 * Key features:
 * - Binary packet serialization using QDataStream
 * - Automatic buffer expansion
 * - Type-safe read/write operations
 * - Position tracking for sequential access
 * - Compatibility with original packet format
 */
class NetworkMessage {
public:
    /**
     * @brief Construct a new NetworkMessage
     */
    NetworkMessage();

    /**
     * @brief Copy constructor
     */
    NetworkMessage(const NetworkMessage& other);

    /**
     * @brief Assignment operator
     */
    NetworkMessage& operator=(const NetworkMessage& other);

    /**
     * @brief Destructor
     */
    ~NetworkMessage() = default;

    /**
     * @brief Clear the message buffer and reset position
     */
    void clear();

    /**
     * @brief Get the current buffer size
     * @return Size of the buffer in bytes
     */
    qint32 size() const;

    /**
     * @brief Get the current read/write position
     * @return Current position in the buffer
     */
    qint32 position() const;

    /**
     * @brief Set the read/write position
     * @param pos New position
     */
    void setPosition(qint32 pos);

    /**
     * @brief Check if there's more data to read
     * @return True if more data is available
     */
    bool hasMoreData() const;

    /**
     * @brief Get the raw buffer data
     * @return Const reference to the buffer
     */
    const QByteArray& buffer() const;

    /**
     * @brief Set the buffer data
     * @param data New buffer data
     */
    void setBuffer(const QByteArray& data);

    /**
     * @brief Prepare message for sending (writes size header)
     */
    void prepareForSending();

    /**
     * @brief Generic read function for basic types
     * @tparam T Type to read
     * @return Value of type T
     */
    template<typename T>
    T read() {
        if (m_position + static_cast<qint32>(sizeof(T)) > m_buffer.size()) {
            throw std::runtime_error("Buffer underflow - attempted to read past buffer end");
        }
        
        QDataStream stream(m_buffer);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.device()->seek(m_position);
        
        T value;
        stream >> value;
        m_position += sizeof(T);
        
        return value;
    }

    /**
     * @brief Generic write function for basic types
     * @tparam T Type to write
     * @param value Value to write
     */
    template<typename T>
    void write(const T& value) {
        expandBuffer(sizeof(T));
        
        QDataStream stream(&m_buffer, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.device()->seek(m_position);
        
        stream << value;
        m_position += sizeof(T);
    }

    // Specialized read functions
    QString readString();
    Position readPosition();
    QColor readColor();

    // Specialized write functions
    void writeString(const QString& value);
    void writePosition(const Position& value);
    void writeColor(const QColor& value);

    // Convenience functions for common types
    quint8 readU8() { return read<quint8>(); }
    quint16 readU16() { return read<quint16>(); }
    quint32 readU32() { return read<quint32>(); }
    qint8 readI8() { return read<qint8>(); }
    qint16 readI16() { return read<qint16>(); }
    qint32 readI32() { return read<qint32>(); }

    void writeU8(quint8 value) { write<quint8>(value); }
    void writeU16(quint16 value) { write<quint16>(value); }
    void writeU32(quint32 value) { write<quint32>(value); }
    void writeI8(qint8 value) { write<qint8>(value); }
    void writeI16(qint16 value) { write<qint16>(value); }
    void writeI32(qint32 value) { write<qint32>(value); }

private:
    /**
     * @brief Expand buffer to accommodate additional data
     * @param additionalSize Additional bytes needed
     */
    void expandBuffer(qint32 additionalSize);

    QByteArray m_buffer;    ///< Message buffer
    qint32 m_position;      ///< Current read/write position
};

#endif // NETWORKMESSAGE_H
