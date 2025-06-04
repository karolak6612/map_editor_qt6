#include "NetworkMessage.h"
#include "LivePackets.h"
#include "../Position.h"
#include <QDebug>

NetworkMessage::NetworkMessage() : m_position(LiveNetworkConstants::HEADER_SIZE) {
    clear();
}

NetworkMessage::NetworkMessage(const NetworkMessage& other) 
    : m_buffer(other.m_buffer), m_position(other.m_position) {
}

NetworkMessage& NetworkMessage::operator=(const NetworkMessage& other) {
    if (this != &other) {
        m_buffer = other.m_buffer;
        m_position = other.m_position;
    }
    return *this;
}

void NetworkMessage::clear() {
    m_buffer.clear();
    m_buffer.resize(LiveNetworkConstants::HEADER_SIZE);
    m_position = LiveNetworkConstants::HEADER_SIZE;
}

qint32 NetworkMessage::size() const {
    return m_buffer.size();
}

qint32 NetworkMessage::position() const {
    return m_position;
}

void NetworkMessage::setPosition(qint32 pos) {
    if (pos >= 0 && pos <= m_buffer.size()) {
        m_position = pos;
    }
}

bool NetworkMessage::hasMoreData() const {
    return m_position < m_buffer.size();
}

const QByteArray& NetworkMessage::buffer() const {
    return m_buffer;
}

void NetworkMessage::setBuffer(const QByteArray& data) {
    m_buffer = data;
    m_position = LiveNetworkConstants::HEADER_SIZE;
}

void NetworkMessage::prepareForSending() {
    // Write the message size (excluding header) at the beginning
    qint32 messageSize = m_buffer.size() - LiveNetworkConstants::HEADER_SIZE;
    
    QDataStream stream(&m_buffer, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.device()->seek(0);
    stream << static_cast<quint32>(messageSize);
}

void NetworkMessage::expandBuffer(qint32 additionalSize) {
    qint32 requiredSize = m_position + additionalSize;
    if (requiredSize > m_buffer.size()) {
        m_buffer.resize(requiredSize);
    }
}

QString NetworkMessage::readString() {
    quint16 length = read<quint16>();
    
    if (m_position + length > m_buffer.size()) {
        throw std::runtime_error("Buffer underflow - string length exceeds remaining buffer size");
    }
    
    QByteArray stringData = m_buffer.mid(m_position, length);
    m_position += length;
    
    return QString::fromUtf8(stringData);
}

void NetworkMessage::writeString(const QString& value) {
    QByteArray utf8Data = value.toUtf8();
    quint16 length = static_cast<quint16>(utf8Data.size());
    
    // Ensure string doesn't exceed maximum length
    if (length > LiveNetworkConstants::MAX_MESSAGE_LENGTH) {
        qWarning() << "String length exceeds maximum allowed length, truncating";
        utf8Data = utf8Data.left(LiveNetworkConstants::MAX_MESSAGE_LENGTH);
        length = LiveNetworkConstants::MAX_MESSAGE_LENGTH;
    }
    
    write<quint16>(length);
    
    expandBuffer(length);
    m_buffer.replace(m_position, length, utf8Data);
    m_position += length;
}

Position NetworkMessage::readPosition() {
    Position pos;
    pos.x = read<quint16>();
    pos.y = read<quint16>();
    pos.z = read<quint8>();
    return pos;
}

void NetworkMessage::writePosition(const Position& value) {
    write<quint16>(value.x);
    write<quint16>(value.y);
    write<quint8>(value.z);
}

QColor NetworkMessage::readColor() {
    quint8 r = read<quint8>();
    quint8 g = read<quint8>();
    quint8 b = read<quint8>();
    quint8 a = read<quint8>();
    return QColor(r, g, b, a);
}

void NetworkMessage::writeColor(const QColor& value) {
    write<quint8>(static_cast<quint8>(value.red()));
    write<quint8>(static_cast<quint8>(value.green()));
    write<quint8>(static_cast<quint8>(value.blue()));
    write<quint8>(static_cast<quint8>(value.alpha()));
}
