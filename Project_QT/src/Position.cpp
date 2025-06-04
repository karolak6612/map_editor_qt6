#include "Position.h"
#include <QDebug>
#include <QtMath>

// Static constants for common positions
const Position Position::INVALID_POSITION = Position(65535, 65535, 255);
const Position Position::ORIGIN = Position(0, 0, 0);

// Register the Position type with Qt's meta-object system
static int positionMetaTypeId = qRegisterMetaType<Position>("Position");

// Additional utility functions that don't fit in the header

/**
 * @brief Debug stream operator for Position
 */
QDebug operator<<(QDebug debug, const Position& pos) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Position(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
    return debug;
}

/**
 * @brief Data stream output operator for Position
 */
QDataStream& operator<<(QDataStream& stream, const Position& pos) {
    stream << pos.x << pos.y << pos.z;
    return stream;
}

/**
 * @brief Data stream input operator for Position
 */
QDataStream& operator>>(QDataStream& stream, Position& pos) {
    stream >> pos.x >> pos.y >> pos.z;
    return stream;
}
