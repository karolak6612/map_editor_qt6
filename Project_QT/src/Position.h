#ifndef POSITION_H
#define POSITION_H

#include <QtGlobal>
#include <QMetaType>
#include <QHash>

/**
 * @brief Represents a 3D position in the map
 * 
 * This class represents a position in the 3D map coordinate system.
 * Coordinates use the standard Tibia coordinate system:
 * - X: West to East (0-65535)
 * - Y: North to South (0-65535) 
 * - Z: Surface to underground (0-15, where 7 is ground level)
 */
class Position {
public:
    quint16 x;  ///< X coordinate (West-East)
    quint16 y;  ///< Y coordinate (North-South)
    quint8 z;   ///< Z coordinate (Surface-Underground)

    /**
     * @brief Default constructor - creates position at (0,0,0)
     */
    Position() : x(0), y(0), z(0) {}

    /**
     * @brief Constructor with coordinates
     * @param x_ X coordinate
     * @param y_ Y coordinate
     * @param z_ Z coordinate
     */
    Position(quint16 x_, quint16 y_, quint8 z_) : x(x_), y(y_), z(z_) {}

    /**
     * @brief Copy constructor
     */
    Position(const Position& other) = default;

    /**
     * @brief Assignment operator
     */
    Position& operator=(const Position& other) = default;

    /**
     * @brief Equality comparison
     */
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    /**
     * @brief Less than comparison (for use in containers)
     */
    bool operator<(const Position& other) const {
        if (z != other.z) return z < other.z;
        if (y != other.y) return y < other.y;
        return x < other.x;
    }

    /**
     * @brief Addition operator
     */
    Position operator+(const Position& other) const {
        return Position(x + other.x, y + other.y, z + other.z);
    }

    /**
     * @brief Subtraction operator
     */
    Position operator-(const Position& other) const {
        return Position(x - other.x, y - other.y, z - other.z);
    }

    /**
     * @brief Check if position is valid
     * @return True if coordinates are within valid ranges
     */
    bool isValid() const {
        return z <= 15; // X and Y are always valid for quint16
    }

    /**
     * @brief Get distance to another position (2D distance on same floor)
     * @param other Other position
     * @return Distance in tiles
     */
    double distanceTo(const Position& other) const {
        if (z != other.z) return -1.0; // Different floors
        
        qint32 dx = static_cast<qint32>(x) - static_cast<qint32>(other.x);
        qint32 dy = static_cast<qint32>(y) - static_cast<qint32>(other.y);
        
        return qSqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Get Manhattan distance to another position
     * @param other Other position
     * @return Manhattan distance in tiles
     */
    quint32 manhattanDistanceTo(const Position& other) const {
        if (z != other.z) return UINT32_MAX; // Different floors
        
        quint32 dx = (x > other.x) ? (x - other.x) : (other.x - x);
        quint32 dy = (y > other.y) ? (y - other.y) : (other.y - y);
        
        return dx + dy;
    }

    /**
     * @brief Check if position is adjacent to another position
     * @param other Other position
     * @return True if positions are adjacent (including diagonally)
     */
    bool isAdjacentTo(const Position& other) const {
        if (z != other.z) return false;
        
        qint32 dx = qAbs(static_cast<qint32>(x) - static_cast<qint32>(other.x));
        qint32 dy = qAbs(static_cast<qint32>(y) - static_cast<qint32>(other.y));
        
        return dx <= 1 && dy <= 1 && (dx + dy) > 0;
    }

    /**
     * @brief Get position moved in a direction
     * @param dx X offset
     * @param dy Y offset
     * @param dz Z offset
     * @return New position
     */
    Position moved(qint16 dx, qint16 dy, qint8 dz = 0) const {
        return Position(
            static_cast<quint16>(qMax(0, qMin(65535, static_cast<qint32>(x) + dx))),
            static_cast<quint16>(qMax(0, qMin(65535, static_cast<qint32>(y) + dy))),
            static_cast<quint8>(qMax(0, qMin(15, static_cast<qint32>(z) + dz)))
        );
    }

    /**
     * @brief Convert to string representation
     * @return String in format "(x,y,z)"
     */
    QString toString() const {
        return QString("(%1,%2,%3)").arg(x).arg(y).arg(z);
    }

    /**
     * @brief Create position from string
     * @param str String in format "(x,y,z)" or "x,y,z"
     * @return Position object, or invalid position if parsing fails
     */
    static Position fromString(const QString& str) {
        QString cleaned = str.trimmed();
        if (cleaned.startsWith('(') && cleaned.endsWith(')')) {
            cleaned = cleaned.mid(1, cleaned.length() - 2);
        }
        
        QStringList parts = cleaned.split(',');
        if (parts.size() != 3) {
            return Position(); // Invalid
        }
        
        bool ok1, ok2, ok3;
        quint16 px = parts[0].trimmed().toUShort(&ok1);
        quint16 py = parts[1].trimmed().toUShort(&ok2);
        quint8 pz = static_cast<quint8>(parts[2].trimmed().toUShort(&ok3));
        
        if (ok1 && ok2 && ok3) {
            return Position(px, py, pz);
        }
        
        return Position(); // Invalid
    }

    /**
     * @brief Hash function for use in QHash
     */
    friend size_t qHash(const Position& pos, size_t seed = 0) noexcept {
        return qHash(static_cast<quint64>(pos.x) << 32 |
                    static_cast<quint64>(pos.y) << 16 |
                    static_cast<quint64>(pos.z), seed);
    }

    // Static constants
    static const Position INVALID_POSITION;
    static const Position ORIGIN;
};

// Stream operators
QDebug operator<<(QDebug debug, const Position& pos);
QDataStream& operator<<(QDataStream& stream, const Position& pos);
QDataStream& operator>>(QDataStream& stream, Position& pos);

// Register with Qt's meta-object system
Q_DECLARE_METATYPE(Position)

#endif // POSITION_H
