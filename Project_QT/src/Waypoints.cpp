#include "Waypoints.h"
#include "Waypoint.h"
#include "Map.h"
#include "Tile.h"
#include <QDebug>

Waypoints::Waypoints(Map& map, QObject* parent)
    : QObject(parent), map_(map) {
    // Initialize empty waypoint collection
}

Waypoints::~Waypoints() {
    clear();
}

void Waypoints::addWaypoint(Waypoint* waypoint) {
    if (!waypoint) {
        qWarning() << "Waypoints::addWaypoint called with null waypoint";
        return;
    }
    
    QString normalizedName = normalizeWaypointName(waypoint->name());
    if (normalizedName.isEmpty()) {
        qWarning() << "Waypoints::addWaypoint called with waypoint with empty name";
        return;
    }
    
    // Remove existing waypoint with same name if it exists
    removeWaypoint(normalizedName);
    
    // Update tile waypoint count if waypoint has valid position
    updateTileWaypointCount(waypoint, true);
    
    // Add waypoint to collection
    waypoints_.insert(normalizedName, waypoint);
    waypoint->setParent(this); // Take ownership
    
    emit waypointAdded(waypoint);
    qDebug() << "Waypoint added:" << waypoint->name() << "at position" 
             << waypoint->position().x << waypoint->position().y << waypoint->position().z;
}

void Waypoints::removeWaypoint(const QString& name) {
    QString normalizedName = normalizeWaypointName(name);
    auto it = waypoints_.find(normalizedName);
    if (it == waypoints_.end()) {
        return; // Waypoint not found
    }
    
    Waypoint* waypoint = it.value();
    
    // Update tile waypoint count
    updateTileWaypointCount(waypoint, false);
    
    // Remove from collection
    waypoints_.erase(it);
    
    emit waypointRemoved(waypoint->name());
    qDebug() << "Waypoint removed:" << waypoint->name();
    
    // Delete waypoint
    delete waypoint;
}

void Waypoints::removeWaypoint(Waypoint* waypoint) {
    if (!waypoint) {
        return;
    }
    removeWaypoint(waypoint->name());
}

void Waypoints::clear() {
    if (!waypoints_.isEmpty()) {
        // Update tile waypoint counts for all waypoints
        for (auto it = waypoints_.begin(); it != waypoints_.end(); ++it) {
            updateTileWaypointCount(it.value(), false);
            delete it.value();
        }
        
        waypoints_.clear();
        emit waypointsCleared();
        qDebug() << "All waypoints cleared";
    }
}

// Query methods
Waypoint* Waypoints::getWaypoint(const QString& name) const {
    QString normalizedName = normalizeWaypointName(name);
    auto it = waypoints_.find(normalizedName);
    return (it != waypoints_.end()) ? it.value() : nullptr;
}

Waypoint* Waypoints::getWaypoint(Tile* tile) const {
    if (!tile) {
        return nullptr;
    }
    
    // Find waypoint by position (linear search - could be optimized with position hash)
    for (auto it = waypoints_.begin(); it != waypoints_.end(); ++it) {
        Waypoint* waypoint = it.value();
        if (waypoint && waypoint->position() == tile->getPosition()) {
            return waypoint;
        }
    }
    
    return nullptr;
}

bool Waypoints::hasWaypoint(const QString& name) const {
    return getWaypoint(name) != nullptr;
}

int Waypoints::count() const {
    return waypoints_.size();
}

bool Waypoints::isEmpty() const {
    return waypoints_.isEmpty();
}

// Collection access
QList<Waypoint*> Waypoints::getAllWaypoints() const {
    return waypoints_.values();
}

QStringList Waypoints::getWaypointNames() const {
    QStringList names;
    for (auto it = waypoints_.begin(); it != waypoints_.end(); ++it) {
        names.append(it.value()->name());
    }
    return names;
}

// Iterator support
QMap<QString, Waypoint*>::iterator Waypoints::begin() {
    return waypoints_.begin();
}

QMap<QString, Waypoint*>::iterator Waypoints::end() {
    return waypoints_.end();
}

QMap<QString, Waypoint*>::const_iterator Waypoints::begin() const {
    return waypoints_.begin();
}

QMap<QString, Waypoint*>::const_iterator Waypoints::end() const {
    return waypoints_.end();
}

QMap<QString, Waypoint*>::const_iterator Waypoints::find(const QString& name) const {
    QString normalizedName = normalizeWaypointName(name);
    return waypoints_.find(normalizedName);
}

quint32 Waypoints::memsize() const {
    quint32 size = sizeof(Waypoints);
    for (auto it = waypoints_.begin(); it != waypoints_.end(); ++it) {
        size += it.key().size() * sizeof(QChar);
        if (it.value()) {
            size += it.value()->memsize();
        }
    }
    return size;
}

// Helper methods
QString Waypoints::normalizeWaypointName(const QString& name) const {
    return Waypoint::normalizeName(name);
}

void Waypoints::updateTileWaypointCount(Waypoint* waypoint, bool increment) {
    if (!waypoint) {
        return;
    }
    
    // Get tile at waypoint position
    Tile* tile = map_.getTile(waypoint->position().x, waypoint->position().y, waypoint->position().z);
    if (!tile) {
        // Create tile if it doesn't exist and we're adding a waypoint
        if (increment) {
            tile = map_.createTile(waypoint->position().x, waypoint->position().y, waypoint->position().z);
        }
        if (!tile) {
            return;
        }
    }
    
    // Update waypoint count on tile
    if (increment) {
        tile->increaseWaypointCount();
    } else {
        tile->decreaseWaypointCount();
    }
}

#include "Waypoints.moc"
