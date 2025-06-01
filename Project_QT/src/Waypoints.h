#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>

// Forward declarations
class Waypoint;
class Map;
class Tile;

/**
 * Waypoints - Collection class for managing waypoints on a map
 * This class manages all waypoints and provides methods for adding/removing/finding waypoints
 * Similar to the wxWidgets Waypoints class but with Qt integration
 */
class Waypoints : public QObject {
    Q_OBJECT

public:
    explicit Waypoints(Map& map, QObject* parent = nullptr);
    ~Waypoints();

    // Waypoint management
    void addWaypoint(Waypoint* waypoint);
    void removeWaypoint(const QString& name);
    void removeWaypoint(Waypoint* waypoint);
    void clear();
    
    // Query methods
    Waypoint* getWaypoint(const QString& name) const;
    Waypoint* getWaypoint(Tile* tile) const;
    bool hasWaypoint(const QString& name) const;
    int count() const;
    bool isEmpty() const;
    
    // Collection access
    QList<Waypoint*> getAllWaypoints() const;
    QStringList getWaypointNames() const;
    
    // Iterator support (Qt-style)
    QMap<QString, Waypoint*>::iterator begin();
    QMap<QString, Waypoint*>::iterator end();
    QMap<QString, Waypoint*>::const_iterator begin() const;
    QMap<QString, Waypoint*>::const_iterator end() const;
    QMap<QString, Waypoint*>::const_iterator find(const QString& name) const;
    
    // Utility methods
    quint32 memsize() const;

signals:
    void waypointAdded(Waypoint* waypoint);
    void waypointRemoved(const QString& name);
    void waypointsCleared();

private:
    Map& map_;
    QMap<QString, Waypoint*> waypoints_; // Case-insensitive name -> waypoint mapping
    
    // Helper methods
    QString normalizeWaypointName(const QString& name) const;
    void updateTileWaypointCount(Waypoint* waypoint, bool increment);
};

#endif // WAYPOINTS_H
