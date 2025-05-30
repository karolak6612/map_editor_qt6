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
    // Q_FLAG(TileMapFlags) // Q_FLAG is for use with Q_PROPERTY, not strictly needed for Q_DECLARE_FLAGS alone

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
    // Q_FLAG(TileStateFlags)

public:
    explicit Tile(int x, int y, int z, QObject *parent = nullptr);
    ~Tile() override;

    // Coordinate getters
    int x() const { return x_; }
    int y() const { return y_; }
    int z() const { return z_; }
    MapPos mapPos() const; 

    // Item/Creature Management
    void addItem(Item* item); // Appends to items_. Tile takes ownership if not already parented.
    bool removeItem(Item* item); // Removes specific item, deletes it. Returns true if found and removed.
    Item* removeItem(int index); // Removes item at specific index from items_, returns it (caller takes ownership).
    
    void setGround(Item* groundItem); // Deletes old ground, sets new. Tile takes ownership.
    Item* getGround() const;

    const QVector<Item*>& items() const; // Const ref to items_ (non-ground items)
    QVector<Item*>& items();             // Non-const ref to items_ for internal map operations
    
    Creature* creature() const;
    void setCreature(Creature* creature); // Deletes old, sets new. Tile takes ownership.

    Spawn* spawn() const;
    void setSpawn(Spawn* spawn); // Does NOT take ownership. Spawn is linked, not owned.

    int itemCount() const; // Number of items (ground + other items)
    int creatureCount() const; // 0 or 1
    bool isEmpty() const; // Checks ground, items, creature

    Item* getTopLookItem() const; 
    Item* getTopUseItem() const;  
    Item* getTopSelectableItem() const; 

    // Flag/Property Management
    void setMapFlag(TileMapFlag flag, bool on = true);
    bool hasMapFlag(TileMapFlag flag) const;
    TileMapFlags getMapFlags() const; // Renamed from mapFlags to avoid conflict with potential future Q_PROPERTY

    void setStateFlag(TileStateFlag flag, bool on = true);
    bool hasStateFlag(TileStateFlag flag) const;
    TileStateFlags getStateFlags() const; // Renamed from stateFlags

    bool isBlocking() const; 
    bool isPZ() const; void setPZ(bool on);
    bool isNoPVP() const; void setNoPVP(bool on);
    bool isNoLogout() const; void setNoLogout(bool on);
    bool isPVPZone() const; void setPVPZone(bool on);
    // Add more for other map flags as needed

    bool isModified() const; void setModified(bool on = true);
    bool isSelected() const; void setSelected(bool on = true);


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

    // Method to draw the tile and its contents
    void draw(QPainter* painter, const QRectF& targetScreenRect, const DrawingOptions& options) const;

signals:
    void tileChanged(int x, int y, int z); 
    void visualChanged(int x, int y, int z); 

private:
    int x_, y_, z_;

    Item* ground_ = nullptr;        // Tile owns this
    QVector<Item*> items_;          // Tile owns these (non-ground items)
    Creature* creature_ = nullptr;  // Tile owns this
    Spawn* spawn_ = nullptr;        // Tile does NOT own this, it's a link
    quint32 houseId_ = 0;

    TileMapFlags mapFlags_ = TileMapFlag::NoFlag;
    TileStateFlags stateFlags_ = TileStateFlag::NoState;
    QVector<quint16> zoneIds_;
    
    // quint8 minimapColor_ = 0xFF; // To be calculated by update()
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Tile::TileMapFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Tile::TileStateFlags)

#endif // TILE_H
