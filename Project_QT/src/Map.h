#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QList> // For QList members
#include <QPoint> // For QPoint3D, though QPoint itself is 2D. QPoint3D is not standard Qt.
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

struct MapPos {
    int x = 0;
    int y = 0;
    int z = 0;
    MapPos(int x_ = 0, int y_ = 0, int z_ = 0) : x(x_), y(y_), z(z_) {}

    bool operator==(const MapPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

#include <QtGlobal> // For qHash

// qHash function for MapPos
inline uint qHash(const MapPos& pos, uint seed = 0) {
    return qHash(pos.x, seed) ^ qHash(pos.y, seed << 1) ^ qHash(pos.z, seed << 2);
}
// And include QVector3D as a common Qt type for 3D points if QPoint3D is a placeholder.
#include <QVector3D> // As a more standard Qt 3D point, can be used if QPoint3D is not defined.


// Forward declarations
class Tile;
class Item;
class Creature;
class Spawn;
class House;
class Town; // Forward-declare Town
class Waypoint;
class Selection; // Forward-declare Selection
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

    void addHouse(House* house);
    void removeHouse(House* house); // Removes from list, does not delete the object
    const QList<House*>& getHouses() const;
    
    void addWaypoint(Waypoint* waypoint);
    // void removeWaypoint(Waypoint* waypoint); // Old one, effectively removed by new m_waypoints
    // const QList<Waypoint*>& getWaypoints() const; // Old one, replaced by new m_waypoints getter

    const QList<Town*>& getTowns() const { return m_towns; }
    // void addTown(Town* town); // To be implemented with load logic
    // void removeTown(Town* town); // To be implemented with load logic

    const QList<Waypoint*>& getWaypoints() const { return m_waypoints; } // New getter for m_waypoints
    // void addWaypoint(Waypoint* waypoint); // To be implemented with load logic
    // void removeWaypoint(Waypoint* waypoint); // To be implemented with load logic

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

    // New methods for commands and brushes
    Tile* getOrCreateTile(const QPointF& pos);
    Tile* getOrCreateTile(int x, int y, int z); // Overload
    void removeTile(const QPointF& pos); // If it becomes empty
    void removeTile(int x, int y, int z);   // Overload

    // Ground operations taking item ID
    void setGround(const QPointF& pos, quint16 groundItemId);
    void removeGround(const QPointF& pos);

    // Border/Wall update requests
    void requestBorderUpdate(const QPointF& tilePos);
    void requestWallUpdate(const QPointF& tilePos);

    // Stubs for loading/saving
    bool load(const QString& path);
    bool save(const QString& path) const;
    bool loadFromOTBM(QDataStream& stream); // Added
    bool saveToOTBM(QDataStream& stream) const; // Added

signals:
    void mapChanged(); // Example signal
    void dimensionsChanged(int newWidth, int newHeight, int newFloors);
    void tileChanged(int x, int y, int z); // Example for more granular updates

private:
    int getTileIndex(int x, int y, int z) const;
    bool isCoordValid(int x, int y, int z) const; // Helper for coordinate validation

    QString description_;
    int width_ = 0;
    int height_ = 0;
    int floors_ = 0;
    QVector<Tile*> tiles_; // Flat list of all tiles, Map owns these Tile objects

    // Placeholder containers for map-wide entities
    // Ownership: If these are created *by* the Map (e.g. map.createNewHouse()), Map owns.
    // If they are added from outside (e.g. map.addHouse(existingHouse)), ownership depends on design.
    // For now, assuming Map takes ownership if it creates them, or shared ownership if added.
    // The remove methods only remove from list, not delete.
    QList<Spawn*> spawns_;     
    QList<House*> houses_;     
    Selection* selection_ = nullptr;

    QList<Town*> m_towns;
    QList<Waypoint*> m_waypoints; // Re-declaring here to ensure it's part of the new block
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
};

#endif // MAP_H
