#ifndef ITEM_H
#define ITEM_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QtGlobal> // For quint16
#include <QDataStream> // For unserializeOtbmAttributes
#include <utility> // For std::pair

#include <QRectF> // For draw method targetRect
#include <QPainter> // For drawing methods
#include <QSharedPointer> // For sprite data
#include <QImage> // For sprite images
#include <QDateTime> // For animation timing
#include "DrawingOptions.h" // For draw method options

// Task 015: Include ItemManager.h to get enum definitions
#include "ItemManager.h"

// Forward declarations
struct GameSpriteData;
class SpriteManager;
class ItemManager;

// ITEMPROPERTY enum for wxWidgets compatibility
enum ITEMPROPERTY {
    BLOCKSOLID,
    HASHEIGHT,
    BLOCKPROJECTILE,
    BLOCKPATHFIND,
    PROTECTIONZONE,
    HOOK_SOUTH,
    HOOK_EAST,
    MOVEABLE,
    BLOCKINGANDNOTMOVEABLE,
    HASLIGHT
};

// Forward declarations
class QPainter;
class Brush; // Added forward declaration for Brush
// QRectF is included above via #include <QRectF>
// QVariantMap is typedef for QMap<QString, QVariant> - no need to forward declare if QMap is included

class Item : public QObject {
    Q_OBJECT

public:
    // Constructors & Destructor
    explicit Item(quint16 serverId, QObject *parent = nullptr);
    ~Item() override;

    // Modified State
    bool isModified() const { return m_modified; }
    void setModified(bool modified);

    // Core Properties
    quint16 getServerId() const;
    void setServerId(quint16 id); // Typically fixed after creation, but providing setter for flexibility

    quint16 getClientId() const;
    void setClientId(quint16 id);

    QString name() const;
    void setName(const QString& name);

    QString typeName() const; 
    void setTypeName(const QString& typeName);

    // Generic Attribute System
    void setAttribute(const QString& key, const QVariant& value);
    QVariant getAttribute(const QString& key, const QVariant& defaultValue = QVariant()) const;
    bool hasAttribute(const QString& key) const;
    void clearAttribute(const QString& key);
    const QMap<QString, QVariant>& getAttributes() const; 

    // Specific Attribute Accessors (examples, more can be added)
    int getCount() const;
    void setCount(int count);

    QString getText() const;
    void setText(const QString& text);

    int getActionId() const;
    void setActionId(int id);

    int getUniqueId() const;
    void setUniqueId(int id);

    // Task 55: Advanced property accessors
    // Door properties
    quint8 getDoorId() const;
    void setDoorId(quint8 doorId);
    bool isDoorOpen() const;
    void setDoorOpen(bool open);
    bool isDoorLocked() const;
    void setDoorLocked(bool locked);

    // Container properties
    int getContainerCapacity() const;
    void setContainerCapacity(int capacity);
    QVariantList getContainerContents() const;
    void setContainerContents(const QVariantList& contents);
    QStringList getContainerRestrictions() const;
    void setContainerRestrictions(const QStringList& restrictions);

    // Teleport properties (already partially implemented)
    QPoint getTeleportDestination() const;
    void setTeleportDestination(const QPoint& destination);
    void setTeleportDestination(int x, int y, int z);

    // Bed properties
    quint32 getBedSleeperId() const;
    void setBedSleeperId(quint32 sleeperId);
    quint32 getBedSleepStart() const;
    void setBedSleepStart(quint32 sleepStart);
    int getBedRegenerationRate() const;
    void setBedRegenerationRate(int rate);

    // Podium properties
    int getPodiumDirection() const;
    void setPodiumDirection(int direction);
    bool getPodiumShowOutfit() const;
    void setPodiumShowOutfit(bool show);
    bool getPodiumShowMount() const;
    void setPodiumShowMount(bool show);
    bool getPodiumShowPlatform() const;
    void setPodiumShowPlatform(bool show);

    // Podium outfit properties
    quint16 getPodiumOutfitLookType() const;
    void setPodiumOutfitLookType(quint16 lookType);
    quint8 getPodiumOutfitHead() const;
    void setPodiumOutfitHead(quint8 head);
    quint8 getPodiumOutfitBody() const;
    void setPodiumOutfitBody(quint8 body);
    quint8 getPodiumOutfitLegs() const;
    void setPodiumOutfitLegs(quint8 legs);
    quint8 getPodiumOutfitFeet() const;
    void setPodiumOutfitFeet(quint8 feet);
    quint8 getPodiumOutfitAddon() const;
    void setPodiumOutfitAddon(quint8 addon);

    // Item durability and timing properties
    int getBreakChance() const;
    void setBreakChance(int chance);
    quint32 getDuration() const;
    void setDuration(quint32 duration);
    quint32 getMaxDuration() const;
    void setMaxDuration(quint32 maxDuration);

    // Creature/Spawn properties
    QString getCreatureType() const;
    void setCreatureType(const QString& type);
    QString getCreatureName() const;
    void setCreatureName(const QString& name);
    int getSpawnRadius() const;
    void setSpawnRadius(int radius);
    quint32 getSpawnInterval() const;
    void setSpawnInterval(quint32 interval);
    int getSpawnMaxCreatures() const;
    void setSpawnMaxCreatures(int maxCreatures);

    // Add more specific attribute getters/setters as identified from wxItem
    // e.g., charges, duration, fluidType etc.

    // Boolean Flags (derived from ItemType, set by ItemManager or specific subclasses)
    // Getters
    bool isMoveable() const;
    bool isBlocking() const;      
    bool blocksMissiles() const;  
    bool blocksPathfind() const;  
    bool isStackable() const;
    bool isGroundTile() const;
    bool isAlwaysOnTop() const;
    int getTopOrder() const;
    int getStackPos() const;      // Z-ordering position within tile
    bool isTeleport() const;
    bool isContainer() const;     
    bool isReadable() const;      
    bool canWriteText() const;    
    bool isPickupable() const;
    bool isRotatable() const;
    bool isHangable() const;
    bool hasHookSouth() const;
    bool hasHookEast() const;
    bool hasHeight() const;       
    // Add more as per ItemPropertyFlag in wxItem and ItemType properties

    // Brush-related properties
    bool isTable() const;
    bool isCarpet() const; // <-- ADDED THIS LINE
    bool isWall() const;
    bool isBorder() const;
    // Add isCarpet(), isWall() etc. here if they follow the same pattern of querying ItemTypeData

    // Selection methods
    bool isSelected() const;
    void select();
    void deselect();
    void setSelected(bool selected);

    // Memory and utility methods
    quint32 memsize() const;
    bool hasProperty(int property) const;
    quint8 getMiniMapColor() const;

    // Setters for these flags (mainly for ItemManager/subclass setup)
    void setMoveable(bool on);
    void setBlocking(bool on);
    void setBlocksMissiles(bool on);
    void setBlocksPathfind(bool on);
    void setStackable(bool on);
    void setGroundTile(bool on);
    void setAlwaysOnTop(bool on);
    void setTopOrder(int order);
    void setIsTeleport(bool on); 
    void setIsContainer(bool on); 
    void setReadable(bool on);
    void setCanWriteText(bool on);
    void setPickupable(bool on);
    void setRotatable(bool on);
    void setHangable(bool on);
    void setHasHookSouth(bool on);
    void setHasHookEast(bool on);
    void setHasHeight(bool on);


    // Other methods
    virtual QString getDescription() const; // Now a dedicated member, getter remains.
    virtual void drawText(QPainter* painter, const QRectF& targetRect, const QMap<QString, QVariant>& options); // Changed QVariantMap to QMap
    virtual void draw(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    virtual Item* deepCopy() const;

    // Enhanced placeholder rendering methods
    void drawPlaceholder(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    void drawDebugInfo(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    void drawBoundingBox(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;

    // Task 54: Full sprite integration methods
    void drawWithSprites(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    void drawSpriteLayer(QPainter* painter, const QRectF& targetRect,
                        QSharedPointer<const GameSpriteData> spriteData,
                        int frame, int patternX, int patternY, int patternZ,
                        int layer, const DrawingOptions& options) const;
    void drawMultiTileSprite(QPainter* painter, const QRectF& baseRect,
                            const QImage& frameImage,
                            QSharedPointer<const GameSpriteData> spriteData,
                            const DrawingOptions& options) const;

    // Task 54: Animation and pattern calculation
    int calculateCurrentFrame(const DrawingOptions& options) const;
    void calculatePatternCoordinates(int& patternX, int& patternY, int& patternZ,
                                   const DrawingOptions& options) const;

    // Animation helper methods
    int calculatePingPongFrame(qint64 elapsedTime, int frameSpeed, int frameCount) const;
    int getAnimationSpeed() const;
    bool getAnimationAsync() const;
    int getAnimationLoopCount() const;

    // Task 76: Special item flag rendering
    void drawSpecialFlags(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    void drawSelectionHighlight(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    void drawLockedDoorHighlight(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    void drawWallHookIndicator(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    void drawBlockingIndicator(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    bool isAnimated() const;

    // Helper methods for placeholder rendering
    QColor getPlaceholderColor() const;
    void drawItemIdText(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    void drawTypeIndicator(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;

    // Task 76: Helper methods for special flag detection
    bool isDoor() const { return hasAttribute(AttrDoorId) || hasAttribute(AttrDoorType); }
    bool isLocked() const { return isDoorLocked(); }
    bool isWallHook() const { return hasHookSouth() || hasHookEast(); }

    // New dedicated property getters
    QString descriptionText() const; // Renamed to avoid conflict with virtual getDescription
    QString editorSuffix() const;
    ItemGroup_t itemGroup() const;
    ItemTypes_t itemType() const;
    float weight() const;
    qint16 attack() const;
    qint16 defense() const;
    qint16 armor() const;
    quint16 charges() const;
    quint16 maxTextLen() const;
    quint16 rotateTo() const;
    quint16 volume() const;
    quint32 slotPosition() const;
    quint8 weaponType() const;
    quint16 lightLevel() const;
    quint16 lightColor() const;
    quint16 classification() const;

    class Brush* getBrush() const; // Returns the brush associated with this item's type

    // Task 013: Specific brush type getters
    class TableBrush* getTableBrush() const;   // Returns TableBrush if isTable(), otherwise nullptr
    class CarpetBrush* getCarpetBrush() const; // Returns CarpetBrush if isCarpet(), otherwise nullptr

    // New dedicated property setters
    void setDescriptionText(const QString& description); // Renamed
    void setEditorSuffix(const QString& suffix);
    void setItemGroup(ItemGroup_t group);
    void setItemType(ItemTypes_t type);
    void setWeight(float weight);
    void setAttack(qint16 attack);
    void setDefense(qint16 defense);
    void setArmor(qint16 armor);
    void setCharges(quint16 charges);
    void setMaxTextLen(quint16 len);
    void setRotateTo(quint16 id);
    void setVolume(quint16 volume);
    void setSlotPosition(quint32 slotPos);
    void setWeaponType(quint8 type);
    void setLightLevel(quint16 level);
    void setLightColor(quint16 color);
    void setClassification(quint16 classification);
    void setHeight(int height);
    void setDrawOffset(int x, int y);

    // Helper methods
    bool isFluidContainer() const;
    bool isSplash() const;
    bool isCharged() const;
    bool isClientCharged() const;
    bool isExtraCharged() const;
    bool canHoldText() const;
    bool canHoldDescription() const;
    bool hasLight() const;
    bool isAlwaysOnBottom() const;

    // Drawing and height methods
    int getHeight() const;
    std::pair<int, int> getDrawOffset() const;

    // Enhanced weight calculation
    virtual double getWeight() const; // Dynamic weight calculation for stackable items

    // Returns true if successful, false on error (e.g., stream error)
    // Stream should be positioned at the start of the item's attribute block.
    // This method will read all attributes for this item.
    // TODO (Task51): Consider if client version is needed for attribute interpretation.
    // If so, this method might need access to Map's version information.
    virtual bool unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion); // Signature from previous step, no change here.

    // Returns true if successful, false on stream error.
    // TODO (Task51): Consider if client version affects how attributes are written.
    virtual bool serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const; // UPDATED
    bool serializeOtbmNode(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const; // UPDATED

    // Task 48: Complex data handling
    bool unserializeCustomAttributeMap(const QByteArray& data);
    QByteArray serializeCustomAttributeMap() const;

public:
    // Attribute Keys
    static const QString AttrCount;
    static const QString AttrActionID;
    static const QString AttrUniqueID;
    static const QString AttrText;
    static const QString AttrDescription; // Note: distinct from member description_ for "look" text
    static const QString AttrCharges;     // Note: distinct from member charges_
    static const QString AttrDuration;
    static const QString AttrWriter;
    static const QString AttrArticle;
    static const QString AttrTier;
    static const QString AttrTeleDestX;
    static const QString AttrTeleDestY;
    static const QString AttrTeleDestZ;
    static const QString AttrDepotID;
    static const QString AttrFluidType;
    static const QString AttrDecayTo;
    static const QString AttrDecayTime;
    static const QString AttrSplashType;
    static const QString AttrRuneSpellName;
    static const QString AttrRuneLevel;
    static const QString AttrRuneMagicLevel;

    // Task 48: Additional OTBM attribute constants
    static const QString AttrRuneCharges;
    static const QString AttrWrittenDate;
    static const QString AttrSleeperGuid;
    static const QString AttrSleepStart;
    static const QString AttrDecayingState;
    static const QString AttrHouseDoorId;
    static const QString AttrPodiumOutfit;
    static const QString AttrOwner;
    static const QString AttrSpecialDescription;
    static const QString AttrCustomAttributes;

    // Task 55: Advanced property attribute constants
    static const QString AttrDoorId;
    static const QString AttrDoorType;
    static const QString AttrDoorOpen;
    static const QString AttrDoorLocked;
    static const QString AttrContainerCapacity;
    static const QString AttrContainerContents;
    static const QString AttrContainerRestrictions;
    static const QString AttrBedSleeperId;
    static const QString AttrBedSleepStart;
    static const QString AttrBedRegenerationRate;
    static const QString AttrPodiumDirection;
    static const QString AttrPodiumShowOutfit;
    static const QString AttrPodiumShowMount;
    static const QString AttrPodiumShowPlatform;
    static const QString AttrPodiumOutfitLookType;
    static const QString AttrPodiumOutfitHead;
    static const QString AttrPodiumOutfitBody;
    static const QString AttrPodiumOutfitLegs;
    static const QString AttrPodiumOutfitFeet;
    static const QString AttrPodiumOutfitAddon;
    static const QString AttrBreakChance;
    static const QString AttrDuration;
    static const QString AttrMaxDuration;
    static const QString AttrCreatureType;
    static const QString AttrCreatureName;
    static const QString AttrSpawnRadius;
    static const QString AttrSpawnInterval;
    static const QString AttrSpawnMaxCreatures;

signals:
    void attributeChanged(const QString& key, const QVariant& newValue);
    void propertyChanged(); 

private:
    quint16 serverId_ = 0;
    quint16 clientId_ = 0;
    QString name_;
    QString itemTypeName_; 

    QMap<QString, QVariant> attributes_;

    // Direct member bool flags for common properties
    bool isMoveable_ = false;
    bool isBlocking_ = false;
    bool blocksMissiles_ = false;
    bool blocksPathfind_ = false;
    bool isStackable_ = false;
    bool isGroundTile_ = false;
    bool isAlwaysOnTop_ = false; 
    int topOrder_ = 1;          
    bool isTeleport_ = false;   
    bool isContainer_ = false;  
    bool isReadable_ = false;
    bool canWriteText_ = false;
    bool isPickupable_ = false;
    bool isRotatable_ = false;
    bool isHangable_ = false;
    bool hasHookSouth_ = false;
    bool hasHookEast_ = false;
    bool hasHeight_ = false;
    bool isSelected_ = false; // Selection state

    // New dedicated members
    // Task 017: Removed description_ - now using AttrDescription in attributes_ map only
    QString editorSuffix_;
    ItemGroup_t itemGroup_ = ITEM_GROUP_NONE;
    ItemTypes_t itemType_ = ITEM_TYPE_NONE;

    float weight_ = 0.0f;
    qint16 attack_ = 0;
    qint16 defense_ = 0;
    qint16 armor_ = 0;
    // Task 017: Removed charges_ - now using AttrCharges in attributes_ map only
    quint16 maxTextLen_ = 0;
    quint16 rotateTo_ = 0; 
    quint16 volume_ = 0;   
    quint32 slotPosition_ = 0; 
    quint8 weaponType_ = 0;  
    quint16 lightLevel_ = 0;
    quint16 lightColor_ = 0;
    quint16 classification_ = 0;

    // Additional properties for enhanced functionality
    int height_ = 0;
    int drawOffsetX_ = 0;
    int drawOffsetY_ = 0;

    // Animation timing
    qint64 creationTime_ = 0; // For asynchronous animation timing

    mutable bool m_modified = false;
};

#endif // ITEM_H
