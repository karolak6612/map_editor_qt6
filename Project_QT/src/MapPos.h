#ifndef MAPPOS_H
#define MAPPOS_H

#include <QtGlobal>   // For quint16, quint8, qHashMulti
#include <QMetaType>  // For Q_DECLARE_METATYPE
#include <QHash>      // For qHash functions (specifically qHashMulti)
#include <QDebug>     // For QDebug operator (optional but good practice)
#include <QPointF>    // For QPointF constructor

// Forward declaration if MapPos is used in QDebug operator before its full definition
// class QDebug; // Not strictly needed if QDebug include is before operator<<

struct MapPos {
    quint16 x;
    quint16 y;
    quint8 z;

    MapPos(quint16 x_ = 0, quint16 y_ = 0, quint8 z_ = 0) : x(x_), y(y_), z(z_) {}

    // Copy constructor (compiler-generated is usually fine)
    MapPos(const MapPos& other) = default;

    // Assignment operator (compiler-generated is usually fine)
    MapPos& operator=(const MapPos& other) = default;

    MapPos(const QPointF& point)
        : x(static_cast<quint16>(qFloor(point.x()))),
          y(static_cast<quint16>(qFloor(point.y()))),
          z(0) // QPointF is 2D, default z to 0. Using qFloor for safer conversion.
    {
        // If QPointF might have a z coordinate (e.g., if it's a typedef for QVector3D):
        // z(static_cast<quint8>(qFloor(point.z())))
        // For now, assuming z is determined by context or defaults to 0 from a 2D QPointF.
    }

    bool operator==(const MapPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    // Add other members/methods as they currently exist in your MapPos.h
    // e.g., isValid(), toString(), etc.
};

// qHash specialization MUST be in the global namespace or the same namespace as MapPos.
// It must be visible *after* MapPos is fully defined.
// This allows QSet<MapPos>, QHash<MapPos, ...>, etc., to work.
inline size_t qHash(const MapPos& pos, size_t seed = 0) noexcept {
    return qHashMulti(seed, pos.x, pos.y, pos.z);
}

// Optional: For QDebug << MapPos
// QDebug operator<<(QDebug debug, const MapPos& pos);
// Declaration if definition is in .cpp

// Q_DECLARE_METATYPE MUST come AFTER the full definition of MapPos
// and its qHash function (if MapPos is used as a key in QHash/QMap)
// or in QSet, or directly in signals/slots by value (though const ref is common).
Q_DECLARE_METATYPE(MapPos)

#endif // MAPPOS_H
