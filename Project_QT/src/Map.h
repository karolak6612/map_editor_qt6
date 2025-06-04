#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QList> // For QList members
#include <QVector3D> // For 3D coordinates
                 // Let's use a simple struct or a custom class for 3D points if needed,
                 // or separate x,y,z. For now, will use a QPoint for x,y and int z for pos.
                 // Re-evaluating: QPoint3D is not in Qt Core. Will use a simple Position struct or individual coords.
                 // For the sake of the request, I will assume a QPoint3D-like structure or use separate coords.
                 // The prompt explicitly asks for QPoint3D. If it's from a specific Qt module (e.g. Qt3D),
                 // it would need an include. Assuming it's a custom type or placeholder here.
                 // For now, I will use a simple custom struct for clarity if QPoint3D is not readily available.

// Let's define a simple 3D point structure if QPoint3D is not assumed to be a standard Qt type here.
// Or, if it's from a specific module like Qt3D, that include would be needed.
// For this task, I'll proceed as if QPoint3D is available or will be made available.
// If QPoint3D is from Qt3D, then #include <Qt3DCore/qentity.h> or similar might be needed,
// but that's a large dependency. Let's assume it's a simple type for now.
// For the purpose of this task, I will use separate x,y,z for the getTile overload,
// as QPoint3D is not part of core Qt widgets/gui.
// The prompt's example uses QPoint3D, so I will include it and assume it's defined elsewhere
// or is a type alias for something like QVector3D for the purpose of this skeleton.
// For a typical Qt Widgets application, QVector3D from QtGui could be an option.
// Given the context, I will use a simple struct for now.
#include <QDataStream> // For loadFromOTBM
#include <QReadWriteLock> // For thread safety
#include <QRect> // For region operations

// Forward declare MapIterator classes to avoid circular includes
class MapIterator;
class ConstMapIterator;

#include "MapPos.h" // Use MapPos typedef from Position.h

#include <QtGlobal> // For qHash

// Note: qHash for MapPos is provided by Position.h since MapPos is a typedef for Position


// Forward declarations
class Tile;
class Item;
class Creature;
class Spawn;
class House;
class Town; // Forward-declare Town
class Waypoint;
class Waypoints;
class Selection; // Forward-declare Selection
class MapIterator; // Forward-declare MapIterator
class MapIO; // Forward-declare MapIO for I/O operations
class MapEntityManager; // Task 011: Extracted entity management
// Add any other classes that Map might store by pointer and need forward declaration

class Map : public QObject {
    Q_OBJECT

public:
    explicit Map(QObject *parent = nullptr);
    Map(int width, int height, int floors, const QString& description = QString(), QObject *parent = nullptr);
    ~Map() override;

    // OTBM Version getters
    quint32 getOtbmMajorVersion() const { return m_otbmMajorVersion; }
    quint32 getOtbmMinorVersion() const { return m_otbmMinorVersion; }
    quint32 getOtbmBuildVersion() const { return m_otbmBuildVersion; }
    QString getOtbmVersionDescription() const { return m_otbmVersionDescription; }
    bool isModified() const { return m_modified; }
    void setModified(bool modified) {
        if (m_modified != modified) {
            m_modified = modified;
            // Potentially emit a signal here if needed: emit mapModifiedStatusChanged(m_modified);
        }
    }

    void initialize(int width, int height, int floors, const QString& description = QString());
    void clear(); // Deletes all tiles and resets map

    // Dimension and Metadata Accessors
    int width() const;
    int height() const;
    int floors() const; // Number of Z-layers
    QString description() const;
    void setDescription(const QString& description);

    // FIXED: Current floor tracking for brush operations
    int getCurrentFloor() const;
    void setCurrentFloor(int floor);

    // Tile Access and Manipulation
    Tile* getTile(int x, int y, int z) const;
    Tile* getTile(const MapPos& pos) const; // Convenience overload using custom MapPos struct
    // Tile* getTile(const QVector3D& pos) const; // Alternative using QVector3D
    bool setTile(int x, int y, int z, Tile* tile); // Map takes ownership if successful. Returns false if coords are invalid.
    Tile* createTile(int x, int y, int z); // Creates a new tile, Map owns it. Returns nullptr if coords invalid or tile alloc fails.

    // Stubs for map-wide entities
    void addSpawn(Spawn* spawn);
    void removeSpawn(Spawn* spawn); // Removes from list, does not delete the object
    const QList<Spawn*>& getSpawns() const;

    // Task 66: Enhanced house management
    void addHouse(House* house);
    void removeHouse(House* house); // Removes from list, does not delete the object
    void removeHouse(quint32 houseId);
    House* getHouse(quint32 houseId) const;
    const QList<House*>& getHouses() const;
    void clearHouses();
    quint32 getNextHouseId() const;
    
    // Task 71: Enhanced waypoint management (using Waypoints collection)
    Waypoints& waypoints() { return *waypoints_; }
    const Waypoints& waypoints() const { return *waypoints_; }

    // Legacy waypoint methods for compatibility
    void addWaypoint(Waypoint* waypoint);
    void removeWaypoint(Waypoint* waypoint);
    void removeWaypoint(const QString& name);
    Waypoint* getWaypoint(const QString& name) const;
    const QList<Waypoint*> getWaypoints() const; // Returns all waypoints as list

    // Task 71: Additional waypoint methods for full functionality
    Waypoint* findWaypoint(const QString& name) const;
    Waypoint* findWaypointAt(const MapPos& position) const;
    Waypoint* findWaypointAt(int x, int y, int z) const;
    QList<Waypoint*> findWaypointsInArea(const QRect& area, int z = 0) const;
    bool hasWaypoint(const QString& name) const;
    bool hasWaypointAt(const MapPos& position) const;
    int getWaypointCount() const;
    void clearWaypoints();

    // Task 71: Waypoint validation and utilities
    bool isValidWaypointName(const QString& name) const;
    QString generateUniqueWaypointName(const QString& baseName = "Waypoint") const;
    QStringList getWaypointNames() const;

    // Task 71: Waypoint navigation and interaction
    bool centerOnWaypoint(const QString& name);
    bool centerOnWaypoint(Waypoint* waypoint);
    QList<Waypoint*> getWaypointsInRadius(const MapPos& center, int radius) const;

    // Task 66: Enhanced town management
    void addTown(Town* town);
    void removeTown(Town* town);
    void removeTown(quint32 townId);
    Town* getTown(quint32 townId) const;
    Town* getTown(const QString& townName) const;
    const QList<Town*>& getTowns() const { return m_towns; }
    void clearTowns();
    quint32 getNextTownId() const;

    QString getExternalSpawnFile() const { return m_externalSpawnFile; }
    void setExternalSpawnFile(const QString& fileName) {
        if (m_externalSpawnFile != fileName) {
            m_externalSpawnFile = fileName;
            setModified(true);
        }
    }

    QString getExternalHouseFile() const { return m_externalHouseFile; }
    void setExternalHouseFile(const QString& fileName) {
        if (m_externalHouseFile != fileName) {
            m_externalHouseFile = fileName;
            setModified(true);
        }
    }

    quint32 getOtbItemsMajorVersion() const { return m_otbItemsMajorVersion; }
    quint32 getOtbItemsMinorVersion() const { return m_otbItemsMinorVersion; }
    // void setOtbItemsVersions(quint32 major, quint32 minor); // Optional setter

    // Selection methods
    Selection* getSelection() const;
    void updateSelection(const QSet<MapPos>& newSelection); // Example: takes a set of positions

    // Iterator methods for high-performance tile traversal
    MapIterator begin();
    MapIterator end();
    ConstMapIterator begin() const;
    ConstMapIterator end() const;

    // Performance utilities
    quint64 getTileCount() const; // Count of non-null tiles
    Tile* swapTile(int x, int y, int z, Tile* newTile); // Swap and return old tile
    Tile* swapTile(const MapPos& pos, Tile* newTile);

    // Advanced tile operations
    void clearTile(int x, int y, int z); // Clear tile without deleting
    void clearTile(const MapPos& pos);
    bool hasTile(int x, int y, int z) const; // Check if tile exists
    bool hasTile(const MapPos& pos) const;

    // New methods for commands and brushes
    Tile* getOrCreateTile(const QPointF& pos);
    Tile* getOrCreateTile(int x, int y, int z); // Overload
    void removeTile(const QPointF& pos); // If it becomes empty
    void removeTile(int x, int y, int z);   // Overload

    // Ground operations taking item ID
    void setGround(const QPointF& pos, quint16 groundItemId);
    void removeGround(const QPointF& pos);

    // Task 52: Enhanced border/wall update requests with Qt integration
    void requestBorderUpdate(const QPointF& tilePos);
    void requestBorderUpdate(const QList<QPointF>& tilePositions);
    void requestBorderUpdate(const QRect& area);
    void requestWallUpdate(const QPointF& tilePos);

    // Map cleanup and optimization utilities
    quint32 cleanDuplicateItems(const QVector<QPair<quint16, quint16>>& ranges = QVector<QPair<quint16, quint16>>());
    void optimizeMemory(); // Compact tile storage and free unused memory
    void rebuildTileIndex(); // Rebuild internal tile indexing if needed

    // Thread safety utilities
    void lockForReading() const;
    void lockForWriting();
    void unlock() const;
    void unlockWrite();

    // QGraphicsView integration helpers
    QVector<Tile*> getTilesInRegion(const QRect& region, int floor) const;
    QVector<Tile*> getTilesInRadius(const MapPos& center, int radius) const;
    void invalidateRegion(const QRect& region, int floor); // Mark region for redraw

    // Task 51: Enhanced serialization methods
    bool load(const QString& path);
    bool save(const QString& path) const;

    // OTBM format support (primary)
    bool loadFromOTBM(QDataStream& stream);
    bool saveToOTBM(QDataStream& stream) const;

    // XML format support (for components and full map)
    bool loadFromXML(const QString& path);
    bool saveToXML(const QString& path) const;
    bool loadSpawnsFromXML(const QString& path);
    bool saveSpawnsToXML(const QString& path) const;
    bool loadHousesFromXML(const QString& path);
    bool saveHousesToXML(const QString& path) const;
    bool loadWaypointsFromXML(const QString& path);
    bool saveWaypointsToXML(const QString& path) const;

    // JSON format support (for modern serialization)
    bool loadFromJSON(const QString& path);
    bool saveToJSON(const QString& path) const;

    // Format detection and routing
    QString detectFileFormat(const QString& path) const;
    bool loadByFormat(const QString& path, const QString& format);
    bool saveByFormat(const QString& path, const QString& format) const;

signals:
    void mapChanged(); // Example signal
    void dimensionsChanged(int newWidth, int newHeight, int newFloors);
    void tileChanged(int x, int y, int z); // Example for more granular updates

    // Task 66: House and town change signals
    void houseAdded(House* house);
    void houseRemoved(quint32 houseId);
    void houseDataChanged(House* house);
    void townAdded(Town* town);
    void townRemoved(quint32 townId);
    void townDataChanged(Town* town);

    // Task 71: Waypoint change signals
    void waypointAdded(Waypoint* waypoint);
    void waypointRemoved(const QString& name);
    void waypointRemoved(Waypoint* waypoint);
    void waypointModified(Waypoint* waypoint);
    void waypointMoved(Waypoint* waypoint, const MapPos& oldPosition, const MapPos& newPosition);
    void waypointsCleared();
    void waypointsChanged();
    void waypointCenterRequested(Waypoint* waypoint);

    // Task 52: Enhanced signals for Qt rendering integration
    void tilesChanged(const QList<QVector3D>& updatedTileCoords);
    void borderUpdateRequested(const QList<QVector3D>& affectedTiles);
    void visualUpdateNeeded(const QRect& area);

private:
    int getTileIndex(int x, int y, int z) const;
    bool isCoordValid(int x, int y, int z) const; // Helper for coordinate validation

    QString description_;
    int width_ = 0;
    int height_ = 0;
    int floors_ = 0;
    int currentFloor_ = 7;  // FIXED: Default to ground level (7 is standard ground floor)
    QVector<Tile*> tiles_; // Flat list of all tiles, Map owns these Tile objects

    // Placeholder containers for map-wide entities
    // Ownership: If these are created *by* the Map (e.g. map.createNewHouse()), Map owns.
    // If they are added from outside (e.g. map.addHouse(existingHouse)), ownership depends on design.
    // For now, assuming Map takes ownership if it creates them, or shared ownership if added.
    // The remove methods only remove from list, not delete.
    QList<Spawn*> spawns_;
    QList<House*> houses_;
    Selection* selection_ = nullptr;
    Waypoints* waypoints_ = nullptr; // Waypoints collection
    MapIO* mapIO_ = nullptr; // I/O operations handler

    // Task 011: Extracted entity management for mandate M6 compliance
    MapEntityManager* entityManager_ = nullptr;

    QList<Town*> m_towns;
    QList<Waypoint*> m_waypoints; // Legacy waypoint list for OTBM compatibility
    QString m_externalSpawnFile;
    QString m_externalHouseFile;

    // OTB Item Versioning information (from items.otb, typically stored in map)
    quint32 m_otbItemsMajorVersion = 0;
    quint32 m_otbItemsMinorVersion = 0;

    // OTBM Versioning information
    quint32 m_otbmMajorVersion = 0;
    quint32 m_otbmMinorVersion = 0;
    quint32 m_otbmBuildVersion = 0; // Using quint32 for build, can be string too
    QString m_otbmVersionDescription;
    mutable bool m_modified = false; // Add modified flag, default to false

    // Performance tracking
    mutable quint64 tileCount_ = 0; // Cache of non-null tile count
    mutable bool tileCountDirty_ = true; // Whether tileCount_ needs recalculation

    // Thread safety (using mutable for const methods that need locking)
    mutable QReadWriteLock mapLock_; // For thread-safe access to map data
};

#endif // MAP_H
