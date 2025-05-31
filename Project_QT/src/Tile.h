#ifndef TILE_H
#define TILE_H

#include <QObject>
#include <QVector>
#include <QFlags>     // Required for QFlags
#include "Map.h"      // For MapPos definition (defined in Map.h in Task 11)
                      // This also makes Map class known, though not strictly needed by Tile itself.
#include <QtGlobal>   // For quint16, quint32

#include <QRectF> // For draw method targetRect
#include "DrawingOptions.h" // For draw method options

// Forward declarations
class Item;
class Creature;
class Spawn;
class QPainter; // For draw method
// MapPos is now included via Map.h
// class Map; // Already forward declared via Map.h inclusion or defined if Map.h is fully included.

class Tile : public QObject {
    Q_OBJECT

public:
    // Based on TILESTATE_ enums from wxwidgets/tile.h
    enum class TileMapFlag : quint16 {
        NoFlag              = 0x0000,
        ProtectionZone      = 0x0001, // TILESTATE_PROTECTIONZONE
        // Deprecated       = 0x0002, // TILESTATE_DEPRECATED - Skip
        NoPVP               = 0x0004, // TILESTATE_NOPVP
        NoLogout            = 0x0008, // TILESTATE_NOLOGOUT
        PVPZone             = 0x0010, // TILESTATE_PVPZONE
        Refresh             = 0x0020, // TILESTATE_REFRESH - Used for client refresh, might be transient
        ZoneBrush           = 0x0040  // TILESTATE_ZONE_BRUSH (RME specific, for temporary display)
    };
    Q_DECLARE_FLAGS(TileMapFlags, TileMapFlag)

    enum class TileStateFlag : quint16 {
        NoState             = 0x0000,
        Selected            = 0x0001, // TILESTATE_SELECTED
        Unique              = 0x0002, // TILESTATE_UNIQUE (has unique item)
        Blocking            = 0x0004, // TILESTATE_BLOCKING
        OptionalBorder      = 0x0008, // TILESTATE_OP_BORDER
        HasTable            = 0x0010, // TILESTATE_HAS_TABLE (RME specific, for rendering)
        HasCarpet           = 0x0020, // TILESTATE_HAS_CARPET (RME specific, for rendering)
        Modified            = 0x0040  // TILESTATE_MODIFIED
    };
    Q_DECLARE_FLAGS(TileStateFlags, TileStateFlag)

public:
    explicit Tile(int x, int y, int z, QObject *parent = nullptr);
    ~Tile() override;

    // Coordinate getters
    int x() const { return x_; }
    int y() const { return y_; }
    int z() const { return z_; }
    MapPos mapPos() const; 

    // Item/Creature Management
    void addItem(Item* item);
    bool removeItem(Item* item);
    Item* removeItem(int index);
    
    void setGround(Item* groundItem);
    Item* getGround() const;

    const QVector<Item*>& items() const;
    QVector<Item*>& items();
    
    Creature* creature() const;
    void setCreature(Creature* creature);

    Spawn* spawn() const;
    void setSpawn(Spawn* spawn);

    int itemCount() const;
    int creatureCount() const;
    bool isEmpty() const;

    Item* getTopLookItem() const; 
    Item* getTopUseItem() const;  
    Item* getTopSelectableItem() const; 

    // Flag/Property Management
    void setMapFlag(TileMapFlag flag, bool on = true);
    bool hasMapFlag(TileMapFlag flag) const;
    TileMapFlags getMapFlags() const;

    void setStateFlag(TileStateFlag flag, bool on = true);
    bool hasStateFlag(TileStateFlag flag) const;
    TileStateFlags getStateFlags() const;

    bool isBlocking() const; 
    bool isPZ() const; void setPZ(bool on);
    bool isNoPVP() const; void setNoPVP(bool on);
    bool isNoLogout() const; void setNoLogout(bool on);
    bool isPVPZone() const; void setPVPZone(bool on);

    bool isModified() const; void setModified(bool on = true);
    bool isSelected() const; void setSelected(bool on = true);

    // Table specific methods
    bool hasTable() const;
    Item* getTable() const;
    void cleanTables(Map* map, bool dontDelete = false);
    void tableize(Map* map);

    // Carpet specific methods
    bool hasCarpet() const;
    Item* getCarpet() const; // Gets the first item that is a carpet
    void cleanCarpets(Map* map, bool dontDelete = false); // Removes carpet items
    void carpetize(Map* map); // Applies carpet connection logic

    // House ID
    quint32 getHouseId() const;
    void setHouseId(quint32 id);
    bool isHouseTile() const;

    // Zone IDs
    void addZoneId(quint16 zoneId);
    bool removeZoneId(quint16 zoneId); 
    void clearZoneIds();
    const QVector<quint16>& getZoneIds() const;
    bool hasZoneId(quint16 zoneId) const;
    
    void update(); 

    void draw(QPainter* painter, const QRectF& targetScreenRect, const DrawingOptions& options) const;

    QList<Item*> getWallItems() const;
    void clearWalls();
    void addWallItemById(quint16 wallItemId);
    void removeGround();
    void setGroundById(quint16 groundItemId);

signals:
    void tileChanged(int x, int y, int z); 
    void visualChanged(int x, int y, int z); 

private:
    int x_, y_, z_;

    Item* ground_ = nullptr;
    QVector<Item*> items_;
    Creature* creature_ = nullptr;
    Spawn* spawn_ = nullptr;
    quint32 houseId_ = 0;

    TileMapFlags mapFlags_ = TileMapFlag::NoFlag;
    TileStateFlags stateFlags_ = TileStateFlag::NoState;
    QVector<quint16> zoneIds_;
    
    Map* getMap() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Tile::TileMapFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Tile::TileStateFlags)

#endif // TILE_H
