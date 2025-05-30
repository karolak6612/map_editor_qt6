#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant> // For potential future use in ItemProperties if some attributes are generic
#include <QtGlobal> // For quint16, qint8, etc.

// Forward declaration
class Item;

// Define enums based on wxwidgets/items.h (can be moved to a common types file later)
// These are typically used as flags or simple type identifiers.
enum ItemGroup_t : quint8 { // Explicitly type to quint8 if they are small and used as such
    ITEM_GROUP_NONE = 0, 
    ITEM_GROUP_GROUND, 
    ITEM_GROUP_CONTAINER, 
    ITEM_GROUP_WEAPON,
    ITEM_GROUP_AMMUNITION, 
    ITEM_GROUP_ARMOR, 
    ITEM_GROUP_RUNE, 
    ITEM_GROUP_TELEPORT,
    ITEM_GROUP_MAGICFIELD, 
    ITEM_GROUP_WRITEABLE, 
    ITEM_GROUP_KEY, 
    ITEM_GROUP_SPLASH,
    ITEM_GROUP_FLUID, 
    ITEM_GROUP_DOOR, 
    ITEM_GROUP_DEPRECATED, // Still useful to know if an item is deprecated
    ITEM_GROUP_PODIUM, 
    ITEM_GROUP_LAST // Sentinel value
};

enum ItemTypes_t : quint8 { // Explicitly type to quint8
    ITEM_TYPE_NONE = 0, 
    ITEM_TYPE_DEPOT, 
    ITEM_TYPE_MAILBOX, 
    ITEM_TYPE_TRASHHOLDER,
    ITEM_TYPE_CONTAINER, 
    ITEM_TYPE_DOOR, 
    ITEM_TYPE_MAGICFIELD, 
    ITEM_TYPE_TELEPORT,
    ITEM_TYPE_BED, 
    ITEM_TYPE_KEY, 
    ITEM_TYPE_PODIUM, 
    ITEM_TYPE_LAST // Sentinel value
};


// Corresponds to wxwidgets ItemType and OTB item attributes
struct ItemProperties {
    // Core IDs & Name
    quint16 serverId = 0;
    quint16 clientId = 0; // Sprite ID
    QString name;
    QString description; // Often empty, can be derived or from XML
    QString editorSuffix; 

    // Flags (mirroring ItemPropertyFlag concepts and wxItemType booleans)
    bool isBlocking = true;       
    bool blockMissiles = false;
    bool blockPathfind = false;   // Often same as isBlocking for server, but can differ for client/editor
    bool hasElevation = false;
    bool isUseable = true;        // Most items are useable in some way by default
    bool isPickupable = false;    // Default to not pickupable, override for those that are
    bool isMoveable = false;      // Default to not moveable
    bool isStackable = false;
    bool isGroundTile = false;    // Is this a "ground" item like terrain
    bool alwaysOnBottom = false;  // If true, rendered first (like ground tiles)
    bool isReadable = false;      
    bool isRotatable = false;     
    bool isHangable = false;      
    bool hasHookEast = false;     
    bool hasHookSouth = false;    
    bool cannotDecay = false;     
    bool allowDistRead = false;   
    bool clientCharges = false;   // "isClientCharged" / has charges that client should display
    bool ignoreLook = false;      
    
    // Additional flags from wxItemType not directly in itemflags_t
    bool isOptionalBorder = false; // For automagic bordering
    bool isWall = false;
    bool isBrushDoor = false;     // If it's a door type for brush placement
    bool isOpen = false;          // For doors/windows, usually runtime state but can be default
    bool isLocked = false;        // For doors, usually runtime state
    bool isTable = false;         // For rendering order/behavior
    bool isCarpet = false;        // For rendering order/behavior
    bool isMetaItem = false;      // Special editor items not for game
    bool floorChangeDown = false;
    bool floorChangeNorth = false;
    bool floorChangeSouth = false;
    bool floorChangeEast = false;
    bool floorChangeWest = false;
    bool floorChange = false;     // True if any floorChange* is true

    // Other properties
    ItemGroup_t group = ITEM_GROUP_NONE; 
    ItemTypes_t type = ITEM_TYPE_NONE;   
    
    quint16 maxTextLen = 0;      // For writable items
    quint16 lightLevel = 0;
    quint16 lightColor = 0;
    qint8 topOrder = 1;          // Rendering order: 0 (ground), 1 (items), 2 (creatures/top), 3 (on top always)
    quint8 weaponType = 0;       // WEAPON_NONE
    quint32 slotPosition = 0;     // SlotPositionBits (for equipment)
    quint16 charges = 0;          // Default charges if applicable
    bool extraChargeable = false; // Has charges attribute in OTB/XML (not client_charges)
    quint16 rotateTo = 0;         // Item ID it rotates to
    float weight = 0.0f;          // Item weight
    qint16 attack = 0;
    qint16 defense = 0;
    qint16 armor = 0;
    quint16 volume = 0;           // For containers
    quint16 classification = 0;   // For Tibia 12.81+ items

    // Default constructor to initialize with sensible defaults
    ItemProperties() = default; 
};


class ItemManager : public QObject {
    Q_OBJECT

public:
    static ItemManager* instance(); 

    bool loadDefinitions(const QString& otbPath, const QString& xmlPath = QString());
    const ItemProperties& getItemProperties(quint16 serverId) const;
    bool itemTypeExists(quint16 serverId) const;
    Item* createItem(quint16 serverId, QObject* parent = nullptr); // Returns Item pointer, parent for Qt ownership
    void clearDefinitions();
    bool isLoaded() const;
    quint16 getMaxServerId() const;


signals:
    void definitionsLoaded();
    void definitionsCleared();

private:
    explicit ItemManager(QObject *parent = nullptr); 
    ItemManager(const ItemManager&) = delete; 
    ItemManager& operator=(const ItemManager&) = delete; 
    ~ItemManager() override;

    bool parseOtb(const QString& filePath);
    bool parseXml(const QString& filePath); 

    QMap<quint16, ItemProperties> itemPropertiesMap_;
    bool loaded_ = false;
    quint16 maxServerId_ = 0;

    static ItemManager* s_instance;
    static ItemProperties defaultProperties_; // For returning on unknown ID, or if ID 0 is requested
};

#endif // ITEMMANAGER_H
