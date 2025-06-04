#include "Item.h"
#include <QDebug>
#include <QPainter> // For drawText and draw
#include <QRectF>   // For drawText and draw
#include <QColor>   // For draw method placeholder
#include <QDateTime> // For animation timing
#include "ItemManager.h" // Required for ItemTypeData access
#include "Brush.h"       // Required for Brush* return type and itemTypeData->brush
#include "TableBrush.h"  // Task 013: For getTableBrush()
#include "CarpetBrush.h" // Task 013: For getCarpetBrush()
#include "SpriteManager.h" // Task 54: For sprite integration
#include "GameSprite.h"    // Task 54: For GameSpriteData
#include <QDataStream>  // For unserializeOtbmAttributes
#include <QByteArray>   // For unserializeOtbmAttributes
#include <QIODevice>    // For QDataStream operations (usually included by QDataStream)
#include "Tile.h"        // For qobject_cast<Tile*> in setModified
#include "ItemRenderer.h" // Task 011: Extracted rendering logic
#include <QMetaType>     // For Qt6 QMetaType constants

#include "OtbmTypes.h" // For OTBM attribute enums

// TODO (Task51): Implement client version specific logic for item attribute deserialization.
// This might involve:
// - Different attribute IDs for the same property in older versions.
// - Different data types or interpretations for attribute values.
// - Conditional reading based on map's client version (passed in or accessed globally/via parent).
// Map* map = qobject_cast<Map*>(this->parent()->parent()); // Example if Item is child of Tile, child of Map
// if (map && map->getOtbmMajorVersion() < SOME_VERSION) { /* handle old way */ }

// Attribute Key Definitions
const QString Item::AttrCount = QStringLiteral("count");
const QString Item::AttrActionID = QStringLiteral("actionId");
const QString Item::AttrUniqueID = QStringLiteral("uniqueId");
const QString Item::AttrText = QStringLiteral("text");
const QString Item::AttrDescription = QStringLiteral("description");
const QString Item::AttrCharges = QStringLiteral("charges");
const QString Item::AttrDuration = QStringLiteral("duration");
const QString Item::AttrWriter = QStringLiteral("writer");
const QString Item::AttrArticle = QStringLiteral("article");
const QString Item::AttrTier = QStringLiteral("tier");
const QString Item::AttrTeleDestX = QStringLiteral("teleportDestX");
const QString Item::AttrTeleDestY = QStringLiteral("teleportDestY");
const QString Item::AttrTeleDestZ = QStringLiteral("teleportDestZ");
const QString Item::AttrDepotID = QStringLiteral("depotId");
const QString Item::AttrFluidType = QStringLiteral("fluidType");
const QString Item::AttrDecayTo = QStringLiteral("decayTo");
const QString Item::AttrDecayTime = QStringLiteral("decayTime");
const QString Item::AttrSplashType = QStringLiteral("splashType");
const QString Item::AttrRuneSpellName = QStringLiteral("runeSpellName");
const QString Item::AttrRuneLevel = QStringLiteral("runeLevel");
const QString Item::AttrRuneMagicLevel = QStringLiteral("runeMagicLevel");

// Task 48: Additional OTBM attribute constants
const QString Item::AttrRuneCharges = QStringLiteral("runeCharges");
const QString Item::AttrWrittenDate = QStringLiteral("writtenDate");
const QString Item::AttrSleeperGuid = QStringLiteral("sleeperGuid");
const QString Item::AttrSleepStart = QStringLiteral("sleepStart");
const QString Item::AttrDecayingState = QStringLiteral("decayingState");
const QString Item::AttrHouseDoorId = QStringLiteral("houseDoorId");
const QString Item::AttrPodiumOutfit = QStringLiteral("podiumOutfit");
const QString Item::AttrOwner = QStringLiteral("owner");
const QString Item::AttrSpecialDescription = QStringLiteral("specialDescription");
const QString Item::AttrCustomAttributes = QStringLiteral("customAttributes");

// Task 55: Advanced property attribute constants
const QString Item::AttrDoorId = QStringLiteral("doorId");
const QString Item::AttrDoorType = QStringLiteral("doorType");
const QString Item::AttrDoorOpen = QStringLiteral("doorOpen");
const QString Item::AttrDoorLocked = QStringLiteral("doorLocked");
const QString Item::AttrContainerCapacity = QStringLiteral("containerCapacity");
const QString Item::AttrContainerContents = QStringLiteral("containerContents");
const QString Item::AttrContainerRestrictions = QStringLiteral("containerRestrictions");
const QString Item::AttrBedSleeperId = QStringLiteral("bedSleeperId");
const QString Item::AttrBedSleepStart = QStringLiteral("bedSleepStart");
const QString Item::AttrBedRegenerationRate = QStringLiteral("bedRegenerationRate");
const QString Item::AttrPodiumDirection = QStringLiteral("podiumDirection");
const QString Item::AttrPodiumShowOutfit = QStringLiteral("podiumShowOutfit");
const QString Item::AttrPodiumShowMount = QStringLiteral("podiumShowMount");
const QString Item::AttrPodiumShowPlatform = QStringLiteral("podiumShowPlatform");
const QString Item::AttrPodiumOutfitLookType = QStringLiteral("podiumOutfitLookType");
const QString Item::AttrPodiumOutfitHead = QStringLiteral("podiumOutfitHead");
const QString Item::AttrPodiumOutfitBody = QStringLiteral("podiumOutfitBody");
const QString Item::AttrPodiumOutfitLegs = QStringLiteral("podiumOutfitLegs");
const QString Item::AttrPodiumOutfitFeet = QStringLiteral("podiumOutfitFeet");
const QString Item::AttrPodiumOutfitAddon = QStringLiteral("podiumOutfitAddon");
const QString Item::AttrBreakChance = QStringLiteral("breakChance");
const QString Item::AttrDuration = QStringLiteral("duration");
const QString Item::AttrMaxDuration = QStringLiteral("maxDuration");
const QString Item::AttrCreatureType = QStringLiteral("creatureType");
const QString Item::AttrCreatureName = QStringLiteral("creatureName");
const QString Item::AttrSpawnRadius = QStringLiteral("spawnRadius");
const QString Item::AttrSpawnInterval = QStringLiteral("spawnInterval");
const QString Item::AttrSpawnMaxCreatures = QStringLiteral("spawnMaxCreatures");

Item::Item(quint16 serverId, QObject *parent) : QObject(parent),
    serverId_(serverId),
    clientId_(0), // Typically set by ItemManager based on serverId_
    // name_ is default QString()
    // itemTypeName_ is default QString()
    // Boolean flags default to false as per their declaration in the header
    isMoveable_(false),
    isBlocking_(true), // Default most items to blocking as per typical RME behavior
    blocksMissiles_(true),
    blocksPathfind_(true),
    isStackable_(false),
    isGroundTile_(false),
    isAlwaysOnTop_(false),
    topOrder_(1), // Default topOrder (0=ground, 1=on_ground, 2=creature/top, 3=always_on_top)
    isTeleport_(false),
    isContainer_(false),
    isReadable_(false),
    canWriteText_(false),
    isPickupable_(true), // Most items are pickupable by default
    isRotatable_(false),
    isHangable_(false),
    hasHookSouth_(false),
    hasHookEast_(false),
    hasHeight_(false),
    // Initialize new dedicated members
    // Task 017: Removed description_ and charges_ - now using attributes_ map only
    editorSuffix_(""),
    itemGroup_(ITEM_GROUP_NONE),
    itemType_(ITEM_TYPE_NONE),
    weight_(0.0f),
    attack_(0),
    defense_(0),
    armor_(0),
    maxTextLen_(0),
    rotateTo_(0),
    volume_(0),
    slotPosition_(0),
    weaponType_(0), // Assuming 0 is WEAPON_NONE or similar default
    lightLevel_(0),
    lightColor_(0),
    classification_(0),
    height_(0),
    drawOffsetX_(0),
    drawOffsetY_(0),
    creationTime_(QDateTime::currentMSecsSinceEpoch())
{
    // Name, ClientID, and flags are often set by an ItemManager after creation
    // based on the serverId by reading an items.xml or similar definition file.
    // The new dedicated members would also typically be set by ItemManager.
}

Item::~Item() {
    // attributes_ QMap<QString, QVariant> manages its own memory.
    // QObject parent will handle deletion if parented.
}

void Item::setModified(bool modified) {
    if (m_modified != modified) {
        m_modified = modified;
        if (m_modified) { // Only propagate upwards if item becomes modified
            // Attempt to get Tile* from parent.
            // This assumes Item is directly parented to Tile.
            // If there's an intermediate QObject, this cast might fail or need adjustment.
            QObject* p = parent();
            if (p) {
                // Check if parent is a Tile. This requires RTTI if Tile is not QObject base.
                // For now, assume Tile is a QObject or inherits it.
                // #include "Tile.h" // Make sure Tile.h is included in Item.cpp
                // Tile* tile = qobject_cast<Tile*>(p);
                // For now, to avoid circular dependency issues if Tile.h isn't already included
                // for other reasons, we'll just log a placeholder or emit a generic signal.
                // A more robust solution might involve a signal/slot mechanism if direct casting is problematic,
                // or ensuring Tile is forward-declared and qobject_cast can work.
                // The prompt implies qobject_cast<Tile*>(parent()) should work.
                // This requires Tile to be a QObject and Tile.h to be included.
                // Assuming Tile.h will be included for qobject_cast<Tile*> to compile.
                // Let's add a placeholder for Tile.h inclusion if not present.
                // #include "Tile.h" // If not already included
                Tile* tile = qobject_cast<Tile*>(p); // Requires "Tile.h"
                if (tile) {
                    tile->setModified(true);
                } else {
                    // Optional: Log if parent is not a Tile, as propagation won't occur.
                    // qDebug() << "Item::setModified - Parent is not a Tile, modified state not propagated to Tile.";
                }
            }
        }
        // Optionally, emit a signal if needed: emit modifiedStatusChanged(m_modified);
    }
}

// Core Properties
quint16 Item::getServerId() const { return serverId_; }
void Item::setServerId(quint16 id) { 
    if (serverId_ != id) {
        quint16 oldId = serverId_;
        serverId_ = id;
        qDebug() << "Item serverId changed from" << oldId << "to" << serverId_ << "(Warning: This is unusual)";
        emit propertyChanged();
    }
}

quint16 Item::getClientId() const { return clientId_; }
void Item::setClientId(quint16 id) {
    if (clientId_ != id) {
        clientId_ = id;
        setModified(true);
        emit propertyChanged(); 
    }
}

QString Item::name() const { return name_; }
void Item::setName(const QString& name) {
    if (name_ != name) {
        name_ = name;
        setModified(true);
        emit propertyChanged();
    }
}

QString Item::typeName() const { return itemTypeName_; }
void Item::setTypeName(const QString& typeName) {
    if (itemTypeName_ != typeName) {
        itemTypeName_ = typeName;
        setModified(true);
        emit propertyChanged();
    }
}

// Generic Attribute System
void Item::setAttribute(const QString& key, const QVariant& value) {
    QVariant oldValue = attributes_.value(key);
    if (oldValue != value) {
        attributes_.insert(key, value);
        setModified(true);
        emit attributeChanged(key, value);
    }
}

QVariant Item::getAttribute(const QString& key, const QVariant& defaultValue) const {
    return attributes_.value(key, defaultValue);
}

bool Item::hasAttribute(const QString& key) const {
    return attributes_.contains(key);
}

void Item::clearAttribute(const QString& key) {
    if (attributes_.remove(key)) {
        setModified(true);
        emit attributeChanged(key, QVariant());
    }
}

const QMap<QString, QVariant>& Item::getAttributes() const {
    return attributes_;
}

// Specific Attribute Accessors
int Item::getCount() const {
    if (isStackable_) {
        return getAttribute(Item::AttrCount, 1).toInt();
    }
    return 1;
}

void Item::setCount(int count) {
    if (isStackable_) {
        if (count <= 0) count = 1;
        setAttribute(Item::AttrCount, count); // This emits attributeChanged
        // No direct member for stackable count other than through attributes_
    } else if (count != 1) {
        // qWarning() << "Item (ID:" << serverId_ << ", Name:" << name_ << ") is not stackable, cannot set count to" << count;
    }
}

QString Item::getText() const {
    return getAttribute(Item::AttrText).toString();
}
void Item::setText(const QString& text) {
    // This setter will update attributes_ and emit attributeChanged.
    // If there was a direct member like text_, it would be set here too.
    setAttribute(Item::AttrText, text);
}

int Item::getActionId() const {
    // Assuming actionId is primarily stored in attributes_
    return getAttribute(Item::AttrActionID).toInt();
}
void Item::setActionId(int id) {
    // This setter will update attributes_ and emit attributeChanged.
    // If there was a direct member like actionId_, it would be set here too.
    setAttribute(Item::AttrActionID, id);
}

int Item::getUniqueId() const {
    // Assuming uniqueId is primarily stored in attributes_
    return getAttribute(Item::AttrUniqueID).toInt();
}
void Item::setUniqueId(int id) {
    // This setter will update attributes_ and emit attributeChanged.
    // If there was a direct member like uniqueId_, it would be set here too.
    setAttribute(Item::AttrUniqueID, id);
}

// Boolean Flag Getters/Setters
#define ITEM_BOOL_PROPERTY_IMPL(PropNameCapital, MemberName) \
bool Item::is##PropNameCapital() const { return MemberName; } \
void Item::set##PropNameCapital(bool on) { if (MemberName != on) { MemberName = on; setModified(true); emit propertyChanged(); } }

ITEM_BOOL_PROPERTY_IMPL(Moveable, isMoveable_)
ITEM_BOOL_PROPERTY_IMPL(Blocking, isBlocking_)
ITEM_BOOL_PROPERTY_IMPL(Stackable, isStackable_)
ITEM_BOOL_PROPERTY_IMPL(GroundTile, isGroundTile_)
ITEM_BOOL_PROPERTY_IMPL(AlwaysOnTop, isAlwaysOnTop_)
ITEM_BOOL_PROPERTY_IMPL(Teleport, isTeleport_)
ITEM_BOOL_PROPERTY_IMPL(Container, isContainer_)
ITEM_BOOL_PROPERTY_IMPL(Readable, isReadable_)
ITEM_BOOL_PROPERTY_IMPL(CanWriteText, canWriteText_)
ITEM_BOOL_PROPERTY_IMPL(Pickupable, isPickupable_)
ITEM_BOOL_PROPERTY_IMPL(Rotatable, isRotatable_)
ITEM_BOOL_PROPERTY_IMPL(Hangable, isHangable_)
ITEM_BOOL_PROPERTY_IMPL(HasHookSouth, hasHookSouth_)
ITEM_BOOL_PROPERTY_IMPL(HasHookEast, hasHookEast_)
ITEM_BOOL_PROPERTY_IMPL(HasHeight, hasHeight_)

// These have different names or slightly different logic from the macro
bool Item::blocksMissiles() const { return blocksMissiles_; }
void Item::setBlocksMissiles(bool on) { if (blocksMissiles_ != on) { blocksMissiles_ = on; setModified(true); emit propertyChanged(); } }

bool Item::blocksPathfind() const { return blocksPathfind_; }
void Item::setBlocksPathfind(bool on) { if (blocksPathfind_ != on) { blocksPathfind_ = on; setModified(true); emit propertyChanged(); } }

int Item::getTopOrder() const { return topOrder_; }
void Item::setTopOrder(int order) { if (topOrder_ != order) { topOrder_ = order; setModified(true); emit propertyChanged(); } }

int Item::getStackPos() const {
    // Calculate stack position based on Tibia's layering rules
    // Lower values are drawn first (bottom), higher values drawn last (top)

    // Get item properties for additional flags
    const ItemProperties& props = ItemManager::getInstance().getItemProperties(serverId_);

    // Base stack position calculation:
    // 0-99: Ground items (always on bottom)
    // 100-199: Always on bottom items (borders, etc.)
    // 200-299: Normal items (sorted by topOrder)
    // 300-399: Creatures and top items
    // 400+: Always on top items

    if (isGroundTile()) {
        return 0; // Ground is always at the bottom
    }

    if (props.alwaysOnBottom) {
        return 100 + topOrder_; // Always on bottom items
    }

    if (isAlwaysOnTop()) {
        return 400 + topOrder_; // Always on top items
    }

    // Normal items use topOrder for positioning
    return 200 + topOrder_;
}

// Renamed setters for macro compatibility in Item.h, now implementing them directly for clarity
void Item::setIsTeleport(bool on) { setTeleport(on); }
void Item::setIsContainer(bool on) { setContainer(on); }


// Brush-related property implementations
bool Item::isTable() const {
    // Assuming ItemManager::getInstance() and getItemProperties() are thread-safe if used across threads.
    // For typical editor usage, this might be single-threaded.
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.isTable;
}

bool Item::isCarpet() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.isCarpet;
}

Brush* Item::getBrush() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.brush;
}

// Task 013: Specific brush type getters
TableBrush* Item::getTableBrush() const {
    if (!isTable()) {
        return nullptr;
    }

    Brush* brush = getBrush();
    return dynamic_cast<TableBrush*>(brush);
}

CarpetBrush* Item::getCarpetBrush() const {
    if (!isCarpet()) {
        return nullptr;
    }

    Brush* brush = getBrush();
    return dynamic_cast<CarpetBrush*>(brush);
}

// Other methods
QString Item::getDescription() const {
    QString desc = name_;
    if (!desc.isEmpty()) {
        desc += " ";
    }
    desc += QString("(ID: %1").arg(serverId_);
    if (clientId_ != 0 && clientId_ != serverId_) {
        desc += QString(", ClientID: %1").arg(clientId_);
    }
    desc += ")";

    // Add weight information if available
    if (weight() > 0.0f) {
        desc += QString("\nWeight: %1 oz").arg(weight(), 0, 'f', 2);
    }

    // Add stackable count if applicable
    if (isStackable() && getCount() > 1) {
        desc += QString("\nCount: %1").arg(getCount());
    }

    // Add charges information if applicable
    if (isCharged() && charges() > 0) {
        desc += QString("\nCharges: %1").arg(charges());
    }

    // Add attack/defense/armor if applicable
    if (attack() > 0) {
        desc += QString("\nAttack: %1").arg(attack());
    }
    if (defense() > 0) {
        desc += QString("\nDefense: %1").arg(defense());
    }
    if (armor() > 0) {
        desc += QString("\nArmor: %1").arg(armor());
    }

    // Append text from AttrText if available
    if (hasAttribute(Item::AttrText)) {
        QString textAttr = getText(); // getText() now uses Item::AttrText
        if(!textAttr.isEmpty()){
             desc += "\n\"" + textAttr + "\"";
        }
    }

    // Append additional description from AttrDescription if available
    // Task 017: Description is now stored only in attributes_ map
    if (hasAttribute(Item::AttrDescription)) {
        desc += "\n" + getAttribute(Item::AttrDescription).toString();
    }

    // Add fluid type for fluid containers
    if (isFluidContainer() && hasAttribute(Item::AttrFluidType)) {
        int fluidType = getAttribute(Item::AttrFluidType).toInt();
        if (fluidType > 0) {
            desc += QString("\nFluid Type: %1").arg(fluidType);
        }
    }

    return desc;
}

void Item::drawText(QPainter* painter, const QRectF& targetRect, const QMap<QString, QVariant>& options) {
    // Task 011: Delegate to ItemRenderer for mandate M6 compliance
    ItemRenderer::drawText(this, painter, targetRect, options);
}

Item* Item::deepCopy() const {
    Item* newItem = new Item(this->serverId_); 
    
    newItem->clientId_ = this->clientId_;
    newItem->name_ = this->name_;
    newItem->itemTypeName_ = this->itemTypeName_;
    newItem->attributes_ = this->attributes_; 

    newItem->isMoveable_ = this->isMoveable_;
    newItem->isBlocking_ = this->isBlocking_;
    newItem->blocksMissiles_ = this->blocksMissiles_;
    newItem->blocksPathfind_ = this->blocksPathfind_;
    newItem->isStackable_ = this->isStackable_;
    newItem->isGroundTile_ = this->isGroundTile_;
    newItem->isAlwaysOnTop_ = this->isAlwaysOnTop_;
    newItem->topOrder_ = this->topOrder_;
    newItem->isTeleport_ = this->isTeleport_;
    newItem->isContainer_ = this->isContainer_;
    newItem->isReadable_ = this->isReadable_;
    newItem->canWriteText_ = this->canWriteText_;
    newItem->isPickupable_ = this->isPickupable_;
    newItem->isRotatable_ = this->isRotatable_;
    newItem->isHangable_ = this->isHangable_;
    newItem->hasHookSouth_ = this->hasHookSouth_;
    newItem->hasHookEast_ = this->hasHookEast_;
    newItem->hasHeight_ = this->hasHeight_;
    
    // Task 017: Removed description_ direct member - now handled by attributes_ map copy above
    newItem->editorSuffix_ = this->editorSuffix_;
    newItem->itemGroup_ = this->itemGroup_;
    newItem->itemType_ = this->itemType_;
    newItem->weight_ = this->weight_;
    newItem->attack_ = this->attack_;
    newItem->defense_ = this->defense_;
    newItem->armor_ = this->armor_;
    // Task 017: Removed charges_ direct member - now handled by attributes_ map copy above
    newItem->maxTextLen_ = this->maxTextLen_;
    newItem->rotateTo_ = this->rotateTo_;
    newItem->volume_ = this->volume_;
    newItem->slotPosition_ = this->slotPosition_;
    newItem->weaponType_ = this->weaponType_;
    newItem->lightLevel_ = this->lightLevel_;
    newItem->lightColor_ = this->lightColor_;
    newItem->classification_ = this->classification_;
    newItem->height_ = this->height_;
    newItem->drawOffsetX_ = this->drawOffsetX_;
    newItem->drawOffsetY_ = this->drawOffsetY_;
    newItem->creationTime_ = this->creationTime_;

    return newItem;
}

void Item::draw(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    // Task 011: Delegate to ItemRenderer for mandate M6 compliance
    ItemRenderer::draw(this, painter, targetRect, options);
}

// Task 011: Rendering methods moved to ItemRenderer for mandate M6 compliance

// drawSpriteLayer moved to ItemRenderer

// drawMultiTileSprite moved to ItemRenderer

int Item::calculateCurrentFrame(const DrawingOptions& options) const {
    // For animated items, calculate current frame based on time
    if (isAnimated() && options.enableAnimations) {
        SpriteManager* spriteManager = SpriteManager::getInstance();
        if (spriteManager) {
            QSharedPointer<const GameSpriteData> spriteData = spriteManager->getGameSpriteData(clientId_);
            if (spriteData && spriteData->frames > 1) {

                // Use synchronized animation timing for consistent behavior
                qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

                // Get animation properties from item type or use defaults
                int animationSpeed = getAnimationSpeed(); // milliseconds per frame
                bool isAsync = getAnimationAsync();
                int loopCount = getAnimationLoopCount();

                if (isAsync) {
                    // Asynchronous animation - each item animates independently
                    qint64 itemStartTime = creationTime_; // Use item creation time as start
                    qint64 elapsedTime = currentTime - itemStartTime;

                    // Calculate frame based on elapsed time and animation properties
                    if (loopCount == -1) {
                        // Ping-pong animation
                        return calculatePingPongFrame(elapsedTime, animationSpeed, spriteData->frames);
                    } else {
                        // Normal loop animation
                        qint64 totalAnimationTime = spriteData->frames * animationSpeed;
                        if (loopCount > 0) {
                            qint64 totalLoopTime = totalAnimationTime * loopCount;
                            if (elapsedTime >= totalLoopTime) {
                                return spriteData->frames - 1; // Stay on last frame
                            }
                        }
                        qint64 cycleTime = elapsedTime % totalAnimationTime;
                        return static_cast<int>(cycleTime / animationSpeed);
                    }
                } else {
                    // Synchronous animation - all items of this type animate together
                    qint64 totalAnimationTime = spriteData->frames * animationSpeed;
                    qint64 cycleTime = currentTime % totalAnimationTime;
                    return static_cast<int>(cycleTime / animationSpeed);
                }
            }
        }
    }

    // For non-animated items or if animation is disabled
    return 0;
}

int Item::calculatePingPongFrame(qint64 elapsedTime, int frameSpeed, int frameCount) const {
    if (frameCount <= 1) return 0;

    qint64 totalCycleTime = (frameCount * 2 - 2) * frameSpeed; // Time for one complete ping-pong cycle
    qint64 cycleTime = elapsedTime % totalCycleTime;
    int frameInCycle = static_cast<int>(cycleTime / frameSpeed);

    if (frameInCycle < frameCount) {
        return frameInCycle; // Forward direction
    } else {
        return (frameCount * 2 - 2) - frameInCycle; // Backward direction
    }
}

int Item::getAnimationSpeed() const {
    // Get animation speed from item properties or use default
    // This could be loaded from ItemManager or item attributes
    return attributes_.value("animationSpeed", 500).toInt();
}

bool Item::getAnimationAsync() const {
    // Get async animation setting from item properties
    return attributes_.value("animationAsync", false).toBool();
}

int Item::getAnimationLoopCount() const {
    // Get loop count from item properties
    // -1 = ping-pong, 0 = infinite, >0 = specific count
    return attributes_.value("animationLoopCount", 0).toInt();
}

void Item::calculatePatternCoordinates(int& patternX, int& patternY, int& patternZ,
                                      const DrawingOptions& options) const {
    Q_UNUSED(options);

    // Default pattern coordinates
    patternX = 0;
    patternY = 0;
    patternZ = 0;

    // Calculate pattern based on item state and properties

    // Pattern X: Often used for item variations or count-based patterns
    if (isStackable() && getCount() > 1) {
        // Use count to determine pattern (for stackable items with different visuals)
        int count = getCount();
        if (count >= 5) patternX = 2;
        else if (count >= 2) patternX = 1;
        else patternX = 0;
    }

    // Pattern Y: Often used for item state (open/closed, on/off, etc.)
    if (hasCharges() && getCharges() > 0) {
        // Use charges for pattern variation
        patternY = qMin(getCharges() - 1, 3); // Limit to available patterns
    }

    // Pattern Z: Often used for direction or orientation
    if (isRotatable()) {
        // Use rotation state if available
        // This would need to be stored in item attributes
        patternZ = attributes_.value("rotation", 0).toInt();
    }
}

bool Item::isAnimated() const {
    // Check if this item type supports animation
    // This could be determined from ItemType data or sprite properties
    SpriteManager* spriteManager = SpriteManager::getInstance();
    if (spriteManager) {
        QSharedPointer<const GameSpriteData> spriteData = spriteManager->getGameSpriteData(clientId_);
        if (spriteData) {
            return spriteData->frames > 1;
        }
    }

    return false;
}

// Task 55: Advanced property accessor implementations

// Door properties
quint8 Item::getDoorId() const {
    return getAttribute(AttrDoorId, 0).toUInt();
}

void Item::setDoorId(quint8 doorId) {
    setAttribute(AttrDoorId, doorId);
}

bool Item::isDoorOpen() const {
    return getAttribute(AttrDoorOpen, false).toBool();
}

void Item::setDoorOpen(bool open) {
    setAttribute(AttrDoorOpen, open);
}

bool Item::isDoorLocked() const {
    return getAttribute(AttrDoorLocked, false).toBool();
}

void Item::setDoorLocked(bool locked) {
    setAttribute(AttrDoorLocked, locked);
}

// Container properties
int Item::getContainerCapacity() const {
    return getAttribute(AttrContainerCapacity, 20).toInt(); // Default capacity
}

void Item::setContainerCapacity(int capacity) {
    setAttribute(AttrContainerCapacity, capacity);
}

QVariantList Item::getContainerContents() const {
    return getAttribute(AttrContainerContents, QVariantList()).toList();
}

void Item::setContainerContents(const QVariantList& contents) {
    setAttribute(AttrContainerContents, contents);
}

QStringList Item::getContainerRestrictions() const {
    return getAttribute(AttrContainerRestrictions, QStringList()).toStringList();
}

void Item::setContainerRestrictions(const QStringList& restrictions) {
    setAttribute(AttrContainerRestrictions, restrictions);
}

// Teleport properties
QPoint Item::getTeleportDestination() const {
    QVariantList coords = getAttribute(AttrDestination, QVariantList()).toList();
    if (coords.size() >= 2) {
        return QPoint(coords[0].toInt(), coords[1].toInt());
    }
    return QPoint(0, 0);
}

void Item::setTeleportDestination(const QPoint& destination) {
    setTeleportDestination(destination.x(), destination.y(), 0);
}

void Item::setTeleportDestination(int x, int y, int z) {
    QVariantList coords;
    coords << x << y << z;
    setAttribute(AttrDestination, coords);
}

// Bed properties
quint32 Item::getBedSleeperId() const {
    return getAttribute(AttrBedSleeperId, 0).toUInt();
}

void Item::setBedSleeperId(quint32 sleeperId) {
    setAttribute(AttrBedSleeperId, sleeperId);
}

quint32 Item::getBedSleepStart() const {
    return getAttribute(AttrBedSleepStart, 0).toUInt();
}

void Item::setBedSleepStart(quint32 sleepStart) {
    setAttribute(AttrBedSleepStart, sleepStart);
}

int Item::getBedRegenerationRate() const {
    return getAttribute(AttrBedRegenerationRate, 1).toInt();
}

void Item::setBedRegenerationRate(int rate) {
    setAttribute(AttrBedRegenerationRate, rate);
}

// Podium properties
int Item::getPodiumDirection() const {
    return getAttribute(AttrPodiumDirection, 0).toInt();
}

void Item::setPodiumDirection(int direction) {
    setAttribute(AttrPodiumDirection, direction);
}

bool Item::getPodiumShowOutfit() const {
    return getAttribute(AttrPodiumShowOutfit, true).toBool();
}

void Item::setPodiumShowOutfit(bool show) {
    setAttribute(AttrPodiumShowOutfit, show);
}

bool Item::getPodiumShowMount() const {
    return getAttribute(AttrPodiumShowMount, false).toBool();
}

void Item::setPodiumShowMount(bool show) {
    setAttribute(AttrPodiumShowMount, show);
}

bool Item::getPodiumShowPlatform() const {
    return getAttribute(AttrPodiumShowPlatform, false).toBool();
}

void Item::setPodiumShowPlatform(bool show) {
    setAttribute(AttrPodiumShowPlatform, show);
}

// Podium outfit properties
quint16 Item::getPodiumOutfitLookType() const {
    return getAttribute(AttrPodiumOutfitLookType, 0).toUInt();
}

void Item::setPodiumOutfitLookType(quint16 lookType) {
    setAttribute(AttrPodiumOutfitLookType, lookType);
}

quint8 Item::getPodiumOutfitHead() const {
    return getAttribute(AttrPodiumOutfitHead, 0).toUInt();
}

void Item::setPodiumOutfitHead(quint8 head) {
    setAttribute(AttrPodiumOutfitHead, head);
}

quint8 Item::getPodiumOutfitBody() const {
    return getAttribute(AttrPodiumOutfitBody, 0).toUInt();
}

void Item::setPodiumOutfitBody(quint8 body) {
    setAttribute(AttrPodiumOutfitBody, body);
}

quint8 Item::getPodiumOutfitLegs() const {
    return getAttribute(AttrPodiumOutfitLegs, 0).toUInt();
}

void Item::setPodiumOutfitLegs(quint8 legs) {
    setAttribute(AttrPodiumOutfitLegs, legs);
}

quint8 Item::getPodiumOutfitFeet() const {
    return getAttribute(AttrPodiumOutfitFeet, 0).toUInt();
}

void Item::setPodiumOutfitFeet(quint8 feet) {
    setAttribute(AttrPodiumOutfitFeet, feet);
}

quint8 Item::getPodiumOutfitAddon() const {
    return getAttribute(AttrPodiumOutfitAddon, 0).toUInt();
}

void Item::setPodiumOutfitAddon(quint8 addon) {
    setAttribute(AttrPodiumOutfitAddon, addon);
}

// Item durability and timing properties
int Item::getBreakChance() const {
    return getAttribute(AttrBreakChance, 0).toInt();
}

void Item::setBreakChance(int chance) {
    setAttribute(AttrBreakChance, chance);
}

quint32 Item::getDuration() const {
    return getAttribute(AttrDuration, 0).toUInt();
}

void Item::setDuration(quint32 duration) {
    setAttribute(AttrDuration, duration);
}

quint32 Item::getMaxDuration() const {
    return getAttribute(AttrMaxDuration, 0).toUInt();
}

void Item::setMaxDuration(quint32 maxDuration) {
    setAttribute(AttrMaxDuration, maxDuration);
}

// Creature/Spawn properties
QString Item::getCreatureType() const {
    return getAttribute(AttrCreatureType, QString()).toString();
}

void Item::setCreatureType(const QString& type) {
    setAttribute(AttrCreatureType, type);
}

QString Item::getCreatureName() const {
    return getAttribute(AttrCreatureName, QString()).toString();
}

void Item::setCreatureName(const QString& name) {
    setAttribute(AttrCreatureName, name);
}

int Item::getSpawnRadius() const {
    return getAttribute(AttrSpawnRadius, 3).toInt();
}

void Item::setSpawnRadius(int radius) {
    setAttribute(AttrSpawnRadius, radius);
}

quint32 Item::getSpawnInterval() const {
    return getAttribute(AttrSpawnInterval, 10000).toUInt(); // 10 seconds default
}

void Item::setSpawnInterval(quint32 interval) {
    setAttribute(AttrSpawnInterval, interval);
}

int Item::getSpawnMaxCreatures() const {
    return getAttribute(AttrSpawnMaxCreatures, 3).toInt();
}

void Item::setSpawnMaxCreatures(int maxCreatures) {
    setAttribute(AttrSpawnMaxCreatures, maxCreatures);
}

// --- Enhanced Placeholder Rendering Methods ---

// drawPlaceholder moved to ItemRenderer

// drawDebugInfo moved to ItemRenderer

// drawBoundingBox moved to ItemRenderer

// Helper methods moved to ItemRenderer for mandate M6 compliance

// --- New Dedicated Property Getters and Setters ---
// Task 017: Read description from attributes map only
QString Item::descriptionText() const {
    return getAttribute(Item::AttrDescription).toString();
}
// Task 017: Write description to attributes map only
void Item::setDescriptionText(const QString& description) {
    QString currentDescription = getAttribute(Item::AttrDescription).toString();
    if (currentDescription != description) {
        setAttribute(Item::AttrDescription, description); // This calls setModified() and emits signals
    }
}

QString Item::editorSuffix() const { return editorSuffix_; }
void Item::setEditorSuffix(const QString& suffix) {
    if (editorSuffix_ != suffix) {
        editorSuffix_ = suffix;
        setModified(true);
        emit propertyChanged();
    }
}

ItemGroup_t Item::itemGroup() const { return itemGroup_; }
void Item::setItemGroup(ItemGroup_t group) {
    if (itemGroup_ != group) {
        itemGroup_ = group;
        setModified(true);
        emit propertyChanged();
    }
}

ItemTypes_t Item::itemType() const { return itemType_; }
void Item::setItemType(ItemTypes_t type) {
    if (itemType_ != type) {
        itemType_ = type;
        setModified(true);
        emit propertyChanged();
    }
}

float Item::weight() const { return weight_; }
void Item::setWeight(float weight) {
    if (qAbs(weight_ - weight) > 0.0001f) { 
        weight_ = weight;
        setModified(true);
        emit propertyChanged();
    }
}

qint16 Item::attack() const { return attack_; }
void Item::setAttack(qint16 attack) {
    if (attack_ != attack) {
        attack_ = attack;
        setModified(true);
        emit propertyChanged();
    }
}

qint16 Item::defense() const { return defense_; }
void Item::setDefense(qint16 defense) {
    if (defense_ != defense) {
        defense_ = defense;
        setModified(true);
        emit propertyChanged();
    }
}

qint16 Item::armor() const { return armor_; }
void Item::setArmor(qint16 armor) {
    if (armor_ != armor) {
        armor_ = armor;
        setModified(true);
        emit propertyChanged();
    }
}

// Task 017: Read charges from attributes map only
quint16 Item::charges() const {
    return getAttribute(Item::AttrCharges).toUInt();
}
// Task 017: Write charges to attributes map only
void Item::setCharges(quint16 charges) {
    quint16 currentCharges = getAttribute(Item::AttrCharges).toUInt();
    if (currentCharges != charges) {
        setAttribute(Item::AttrCharges, charges); // This calls setModified() and emits signals
    }
}

quint16 Item::maxTextLen() const { return maxTextLen_; }
void Item::setMaxTextLen(quint16 len) {
    if (maxTextLen_ != len) {
        maxTextLen_ = len;
        setModified(true);
        emit propertyChanged();
    }
}

quint16 Item::rotateTo() const { return rotateTo_; }
void Item::setRotateTo(quint16 id) {
    if (rotateTo_ != id) {
        rotateTo_ = id;
        setModified(true);
        emit propertyChanged();
    }
}

quint16 Item::volume() const { return volume_; }
void Item::setVolume(quint16 volume) {
    if (this->volume_ != volume) {
        this->volume_ = volume;
        setModified(true);
        emit propertyChanged();
    }
}

quint32 Item::slotPosition() const { return slotPosition_; }
void Item::setSlotPosition(quint32 slotPos) {
    if (slotPosition_ != slotPos) {
        slotPosition_ = slotPos;
        setModified(true);
        emit propertyChanged();
    }
}

quint8 Item::weaponType() const { return weaponType_; }
void Item::setWeaponType(quint8 type) {
    if (weaponType_ != type) {
        weaponType_ = type;
        setModified(true);
        emit propertyChanged();
    }
}

quint16 Item::lightLevel() const { return lightLevel_; }
void Item::setLightLevel(quint16 level) {
    if (lightLevel_ != level) {
        lightLevel_ = level;
        setModified(true);
        emit propertyChanged();
    }
}

quint16 Item::lightColor() const { return lightColor_; }
void Item::setLightColor(quint16 color) {
    if (lightColor_ != color) {
        lightColor_ = color;
        setModified(true);
        emit propertyChanged();
    }
}

quint16 Item::classification() const { return classification_; }
void Item::setClassification(quint16 classification) {
    if (classification_ != classification) {
        classification_ = classification;
        setModified(true); // Also sets modified if direct member changes
        setAttribute(Item::AttrTier, classification_); // Keep attributes_ in sync using AttrTier
        emit propertyChanged();
    }
}

// Helper methods
bool Item::isFluidContainer() const {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(getServerId());
    return props.group == ITEM_GROUP_FLUID;
}

bool Item::isSplash() const {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(getServerId());
    return props.group == ITEM_GROUP_SPLASH;
}

bool Item::isCharged() const {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(getServerId());
    return props.clientCharges || props.extraChargeable;
}

// Note: The local OtbmAttr namespace has been removed. Constants are now from OtbmTypes.h

bool Item::unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) {
    // TODO: Use otbItemsMajorVersion for OTBMv1 subtype logic if not handled in OtbmReader
    // Q_UNUSED(otbItemsMajorVersion); // mapOtbmFormatVersion is not available here, only OTB item versions.
                                     // This was a mistake in plan. OtbmReader handles OTBMv1 subtype with mapOtbmFormatVersion.
                                     // Here, we only have OTB item versions.
    // Q_UNUSED(otbItemsMinorVersion); // Will be used for OTBM_ATTR_CHARGES
    if (stream.atEnd()) {
        // No attributes to read, which is valid.
        return true;
    }

    stream.setByteOrder(QDataStream::LittleEndian);

    while (!stream.atEnd()) {
        quint8 attributeId;
        stream >> attributeId;
        if (stream.status() != QDataStream::Ok) {
            qWarning() << "Item::unserializeOtbmAttributes - Failed to read attribute ID. Stream status:" << stream.status();
            return false;
        }

        quint16 dataLength;
        stream >> dataLength;
        if (stream.status() != QDataStream::Ok) {
            qWarning() << "Item::unserializeOtbmAttributes - Failed to read data length for attribute ID" << attributeId << ". Stream status:" << stream.status();
            return false;
        }

        QByteArray attributeDataBytes;
        if (dataLength > 0) { // Only resize and read if dataLength > 0
            attributeDataBytes.resize(dataLength);
            if (stream.readRawData(attributeDataBytes.data(), dataLength) != dataLength) {
                qWarning() << "Item::unserializeOtbmAttributes - Failed to read attribute data for ID" << attributeId << "Expected length:" << dataLength;
                return false;
            }
        }

        QDataStream attributeValueStream(attributeDataBytes);
        attributeValueStream.setByteOrder(QDataStream::LittleEndian);

        const ItemProperties& iType = ItemManager::instance()->getItemProperties(getServerId());
        // TODO (Task51-OTBMv1): Handle OTBMv1 initial subtype reading if not done in OtbmReader.
        // This might involve a special check here if otbItemsMajorVersion (or map OTBM version) indicates OTBMv1
        // and reading a subtype/count if the item is stackable, fluid, or splash.

        switch (static_cast<OTBM_ItemAttribute>(attributeId)) {
            case OTBM_ATTR_DESCRIPTION:
            case OTBM_ATTR_DESC: { // OTBM_ATTR_DESC is often the primary one
                setDescriptionText(QString::fromUtf8(attributeDataBytes));
                break;
            }
            case OTBM_ATTR_TEXT: {
                setText(QString::fromUtf8(attributeDataBytes));
                break;
            }
            case OTBM_ATTR_WRITTENBY: { // Corrected enum name
                // Assuming no dedicated member for writer yet, just use attribute
                setAttribute(Item::AttrWriter, QString::fromUtf8(attributeDataBytes));
                break;
            }
            case OTBM_ATTR_COUNT: { // quint8. OTBMv1 uses this for subtype for some items.
                                    // This logic assumes OtbmReader already handled initial subtype for OTBMv1.
                                    // So, if COUNT appears for OTBMv1 here, it's likely an actual count override or error.
                if (dataLength < sizeof(quint8)) { qWarning("ATTR_COUNT data too short"); break; }
                quint8 val; attributeValueStream >> val;
                if (iType.isStackable) {
                    setCount(val);
                } else {
                    // For non-stackables, COUNT attribute is unusual post-OTBMv1 subtype handling.
                    // Could be charges for some specific old items or custom server usage.
                    qDebug() << "Item ID" << getServerId() << "is not stackable but received OTBM_ATTR_COUNT:" << val << ". Treating as charges.";
                    setCharges(val);
                }
                break;
            }
            case OTBM_ATTR_RUNE_CHARGES: { // Task 015: Version-dependent handling
                // For older OTBM versions (< 7.8), this is quint8
                // For newer versions, this is quint16 (handled in Task 48 section below)
                if (otbItemsMajorVersion < 7 || (otbItemsMajorVersion == 7 && otbItemsMinorVersion < 8)) {
                    if (dataLength < sizeof(quint8)) { qWarning("ATTR_RUNE_CHARGES data too short for old version"); break; }
                    quint8 val; attributeValueStream >> val;
                    setCharges(val); // Map to general charges for old versions
                } else {
                    if (dataLength < sizeof(quint16)) { qWarning("ATTR_RUNE_CHARGES data too short for new version"); break; }
                    quint16 val; attributeValueStream >> val;
                    setAttribute(Item::AttrRuneCharges, val); // Use dedicated attribute for new versions
                }
                break;
            }
            case OTBM_ATTR_CHARGES: { // Typically quint16
                if (dataLength < sizeof(quint16)) { qWarning("ATTR_CHARGES data too short"); break; }
                quint16 val; attributeValueStream >> val;
                // Use OTB item minor version (ClientVersionID). CLIENT_VERSION_820 corresponds to value 10.
                // TODO (Task51-ClientVer): Confirm '10' is the correct enum/define for CLIENT_VERSION_820 from client_version.h or similar.
                if (otbItemsMinorVersion >= 10) { // CLIENT_VERSION_820 or newer
                    if (iType.clientCharges || iType.extraChargeable) { // Check if item type actually supports charges
                        setCharges(val);
                    } else {
                        qDebug() << "Item ID" << getServerId() << "received ATTR_CHARGES but type is not client-chargeable by OTB version" << otbItemsMinorVersion;
                        // Optionally store as generic attribute if needed: setAttribute(Item::AttrCharges, val);
                    }
                } else { // Older clients might use ATTR_CHARGES differently or not at all for some items
                    qDebug() << "Item ID" << getServerId() << "ATTR_CHARGES (" << val << ") encountered for older OTB version " << otbItemsMinorVersion << ". Applying directly.";
                    setCharges(val); // Default handling: apply if present
                }
                break;
            }
            case OTBM_ATTR_ACTION_ID: {
                if (dataLength < sizeof(quint16)) break;
                quint16 val;
                attributeValueStream >> val;
                setActionId(val);
                break;
            }
            case OTBM_ATTR_UNIQUE_ID: {
                if (dataLength < sizeof(quint16)) break;
                quint16 val;
                attributeValueStream >> val;
                setUniqueId(val);
                break;
            }
            case OTBM_ATTR_DURATION: {
                if (dataLength < sizeof(quint32)) break;
                quint32 val;
                attributeValueStream >> val;
                setAttribute(Item::AttrDuration, val);
                break;
            }
            case OTBM_ATTR_DEPOT_ID: {
                if (dataLength < sizeof(quint16)) break;
                quint16 val;
                attributeValueStream >> val;
                setAttribute(Item::AttrDepotID, val);
                break;
            }
            case OTBM_ATTR_TELE_DEST: { // Corrected enum name
                if (dataLength < (sizeof(quint16) * 2 + sizeof(quint8))) break;
                quint16 x, y;
                quint8 z;
                attributeValueStream >> x >> y >> z;
                if (attributeValueStream.status() == QDataStream::Ok) {
                    setAttribute(Item::AttrTeleDestX, x);
                    setAttribute(Item::AttrTeleDestY, y);
                    setAttribute(Item::AttrTeleDestZ, z);
                } else {
                     qWarning() << "Item::unserializeOtbmAttributes - Failed to read TELEPORT_DEST components.";
                }
                break;
            }
             case OTBM_ATTR_TIER: {
                if (dataLength < sizeof(quint16)) break;
                quint16 val;
                attributeValueStream >> val;
                setClassification(val); // This will also set AttrTier via the setter
                break;
            }

            // Task 48: Additional OTBM attributes
            // Task 015: OTBM_ATTR_RUNE_CHARGES now handled above with version-dependent logic
            case OTBM_ATTR_WRITTENDATE: { // quint32 - Written date timestamp
                if (dataLength < sizeof(quint32)) { qWarning("ATTR_WRITTENDATE data too short"); break; }
                quint32 val; attributeValueStream >> val;
                setAttribute(Item::AttrWrittenDate, val);
                break;
            }
            case OTBM_ATTR_HOUSEDOORID: { // quint8 - House door ID
                if (dataLength < sizeof(quint8)) { qWarning("ATTR_HOUSEDOORID data too short"); break; }
                quint8 val; attributeValueStream >> val;
                setAttribute(Item::AttrHouseDoorId, val);
                break;
            }
            case OTBM_ATTR_SLEEPERGUID: { // quint32 - Sleeper GUID
                if (dataLength < sizeof(quint32)) { qWarning("ATTR_SLEEPERGUID data too short"); break; }
                quint32 val; attributeValueStream >> val;
                setAttribute(Item::AttrSleeperGuid, val);
                break;
            }
            case OTBM_ATTR_SLEEPSTART: { // quint32 - Sleep start timestamp
                if (dataLength < sizeof(quint32)) { qWarning("ATTR_SLEEPSTART data too short"); break; }
                quint32 val; attributeValueStream >> val;
                setAttribute(Item::AttrSleepStart, val);
                break;
            }
            case OTBM_ATTR_DECAYING_STATE: { // quint8 - Decaying state
                if (dataLength < sizeof(quint8)) { qWarning("ATTR_DECAYING_STATE data too short"); break; }
                quint8 val; attributeValueStream >> val;
                setAttribute(Item::AttrDecayingState, val);
                break;
            }
            case OTBM_ATTR_PODIUMOUTFIT: { // Complex data - Podium outfit
                // Store raw bytes for complex podium outfit data
                setAttribute(Item::AttrPodiumOutfit, QVariant(attributeDataBytes));
                break;
            }
            case OTBM_ATTR_ATTRIBUTE_MAP: { // Complex data - TFS 1.x+ style custom attributes
                // Parse custom attribute map (Task 48 requirement)
                if (!unserializeCustomAttributeMap(attributeDataBytes)) {
                    qWarning() << "Failed to parse OTBM_ATTR_ATTRIBUTE_MAP";
                }
                break;
            }

            default:
                qDebug() << "Item::unserializeOtbmAttributes - Unhandled attribute ID:" << Qt::hex << attributeId << "Length:" << dataLength;
                QString unknownKey = QString("otbm_attr_%1").arg(attributeId, 2, 16, QChar('0'));
                setAttribute(unknownKey, QVariant(attributeDataBytes)); // Store raw bytes
                break;
        }

        // Check stream status for operations that read from attributeValueStream
        bool isStringType = (attributeId == OTBM_ATTR_DESCRIPTION ||
                             attributeId == OTBM_ATTR_DESC ||
                             attributeId == OTBM_ATTR_TEXT ||
                             attributeId == OTBM_ATTR_WRITTENBY);

        if (!isStringType && attributeValueStream.status() != QDataStream::Ok) {
             // If dataLength was 0, status might be Ok but nothing was read, which is fine.
            if (dataLength > 0) {
                qWarning() << "Item::unserializeOtbmAttributes - Error reading value for attribute ID" << attributeId
                           << "Stream status:" << attributeValueStream.status();
                // Depending on strictness, might return false here.
                // For now, we'll continue and try to read other attributes.
            }
        }
    }
    this->setModified(false); // Item state reflects persistent storage
    return true; // Successfully read all available attributes
}

bool Item::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // TODO (Task51): Implement client version specific logic for item attribute serialization.
    // This might involve:
    // - Writing different attribute IDs based on target client version.
    // TODO: Use otbItemsMajorVersion for OTBMv1 subtype logic if not handled in OtbmWriter
    // Q_UNUSED(otbItemsMajorVersion); // mapOtbmFormatVersion will be used for this
    // Q_UNUSED(otbItemsMinorVersion); // Will be used
    // - Converting values to older formats if necessary.
    // Map* map = qobject_cast<Map*>(this->parent()->parent()); // Example
    // if (map && map->getOtbmMajorVersion() < SOME_VERSION) { /* save old way */ }

    stream.setByteOrder(QDataStream::LittleEndian);

    // Helper lambda to write string attributes
    auto writeStringAttribute = [&](quint8 attrId, const QString& value) {
        if (!value.isEmpty()) {
            stream << attrId;
            QByteArray utf8Value = value.toUtf8();
            stream << static_cast<quint16>(utf8Value.length());
            stream.writeRawData(utf8Value.constData(), utf8Value.length());
        }
    };

    // Helper lambda for numeric attributes
    template<typename T>
    auto writeNumericAttribute = [&](quint8 attrId, T value, T defaultValue = 0) {
        if (value != defaultValue) {
            stream << attrId;
            stream << static_cast<quint16>(sizeof(T));
            stream << value;
        }
    };

    const ItemProperties& iType = ItemManager::instance()->getItemProperties(getServerId());
    // TODO (Task51-OTBMv1): Handle OTBMv1 initial subtype writing if not done in OtbmWriter.
    // This might involve a special check here if otbItemsMajorVersion (or map OTBM version) indicates OTBMv1
    // and writing a subtype/count if the item is stackable, fluid, or splash.

    // Write known attributes
    // Task 017: Description now comes from attributes map only
    QString currentDescription = descriptionText(); // This getter now accesses attributes_ map
    if (!currentDescription.isEmpty()) {
         writeStringAttribute(OTBM_ATTR_DESC, currentDescription);
    }

    if (hasAttribute(Item::AttrText)) { // Text is purely from attributes map via getText()
        writeStringAttribute(OTBM_ATTR_TEXT, getText());
    }
    if (hasAttribute(Item::AttrWriter)) {
        writeStringAttribute(OTBM_ATTR_WRITTENBY, getAttribute(Item::AttrWriter).toString());
    }

    // Charges / Count
    // Charges / Count
    if (mapOtbmFormatVersion == 0) { // OTBM Version 1 (0-indexed from map's m_otbmMajorVersion)
        // For OTBMv1, stackable, splash, and fluid items write their subtype/count using OTBM_ATTR_COUNT.
        // This is *after* the initial subtype byte has already been written by OtbmWriter::writeItemNode
        // IF this initial subtype logic is chosen for OtbmWriter.
        // The current plan has OtbmReader handle initial subtype, so OtbmWriter might not need to write it separately.
        // This attribute serialization should reflect what's left or what's standard for attributes.
        // Based on wxwidgets Item::serializeAttributes, for OTBMv1, it does NOT write count/charges here again
        // if it was already handled by the initial subtype byte.
        // So, for OTBMv1, we might skip writing COUNT/CHARGES here if they were part of that initial byte.
        // This needs careful alignment with OtbmReader's OTBMv1 handling.
        // For now, let's assume if mapOtbmFormatVersion == 0, the initial subtype was the *only* place for these.
        // So we do NOT write OTBM_ATTR_COUNT or OTBM_ATTR_CHARGES/OTBM_ATTR_RUNE_CHARGES for OTBMv1 here.
        // TODO (Task51-OTBMv1): Confirm this interpretation. If OTBMv1 items can *also* have these as attributes beyond initial subtype.
    } else { // Later OTBM Versions (mapOtbmFormatVersion >= 1, i.e., OTBM v2+)
        if (iType.isStackable) {
            if (getCount() > 0) {
                stream << static_cast<quint8>(OTBM_ATTR_COUNT);
                stream << static_cast<quint16>(sizeof(quint8));
                stream << static_cast<quint8>(getCount());
            }
        }
        // For non-stackable charged items (runes, amulets, etc.)
        if (!iType.isStackable && (iType.clientCharges || iType.extraChargeable)) {
            // Use OTB item minor version (ClientVersionID). CLIENT_VERSION_820 is 10.
            // TODO (Task51-ClientVer): Confirm '10' for CLIENT_VERSION_820.
            if (otbItemsMinorVersion >= 10) { // CLIENT_VERSION_820 or newer
                if (getCharges() > 0) {
                    stream << static_cast<quint8>(OTBM_ATTR_CHARGES); // Use OTBM_ATTR_CHARGES (u16)
                    stream << static_cast<quint16>(sizeof(quint16));
                    stream << static_cast<quint16>(getCharges());
                }
            } else { // Older clients might use OTBM_ATTR_RUNE_CHARGES (u8) or omit for some items.
                if (iType.group == ITEM_GROUP_RUNE && getCharges() > 0) { // Example: only write for runes on old clients
                   stream << static_cast<quint8>(OTBM_ATTR_RUNE_CHARGES);
                   stream << static_cast<quint16>(sizeof(quint8));
                   stream << static_cast<quint8>(getCharges());
                }
                // Other non-rune charged items on old clients might not save charges attribute.
            }
        }
    }
    // Note: OTBM_ATTR_RUNE_CHARGES could also be handled if needed for newer versions if distinct from general charges.

    if (getActionId() > 0) { // getActionId() retrieves from AttrActionID
        writeNumericAttribute<quint16>(OTBM_ATTR_ACTION_ID, getActionId());
    }
    if (getUniqueId() > 0) { // getUniqueId() retrieves from AttrUniqueID
        writeNumericAttribute<quint16>(OTBM_ATTR_UNIQUE_ID, getUniqueId());
    }
    if (classification_ > 0) { // Tier/Classification - direct member
        writeNumericAttribute<quint16>(OTBM_ATTR_TIER, classification_);
    }

    if (hasAttribute(Item::AttrDuration) && getAttribute(Item::AttrDuration).toUInt() > 0) {
        writeNumericAttribute<quint32>(OTBM_ATTR_DURATION, getAttribute(Item::AttrDuration).toUInt());
    }
    if (hasAttribute(Item::AttrDepotID) && getAttribute(Item::AttrDepotID).toUInt() > 0) {
        writeNumericAttribute<quint16>(OTBM_ATTR_DEPOT_ID, static_cast<quint16>(getAttribute(Item::AttrDepotID).toUInt()));
    }

    if (hasAttribute(Item::AttrTeleDestX)) {
        stream << static_cast<quint8>(OTBM_ATTR_TELE_DEST);
        stream << static_cast<quint16>(sizeof(quint16) * 2 + sizeof(quint8)); // Size of x, y, z
        stream << static_cast<quint16>(getAttribute(Item::AttrTeleDestX).toUInt());
        stream << static_cast<quint16>(getAttribute(Item::AttrTeleDestY).toUInt());
        stream << static_cast<quint8>(getAttribute(Item::AttrTeleDestZ).toUInt());
    }

    // Task 48: Serialize additional OTBM attributes
    if (hasAttribute(Item::AttrRuneCharges) && getAttribute(Item::AttrRuneCharges).toUInt() > 0) {
        writeNumericAttribute<quint16>(OTBM_ATTR_RUNE_CHARGES, getAttribute(Item::AttrRuneCharges).toUInt());
    }
    if (hasAttribute(Item::AttrWrittenDate) && getAttribute(Item::AttrWrittenDate).toUInt() > 0) {
        writeNumericAttribute<quint32>(OTBM_ATTR_WRITTENDATE, getAttribute(Item::AttrWrittenDate).toUInt());
    }
    if (hasAttribute(Item::AttrHouseDoorId) && getAttribute(Item::AttrHouseDoorId).toUInt() > 0) {
        writeNumericAttribute<quint8>(OTBM_ATTR_HOUSEDOORID, getAttribute(Item::AttrHouseDoorId).toUInt());
    }
    if (hasAttribute(Item::AttrSleeperGuid) && getAttribute(Item::AttrSleeperGuid).toUInt() > 0) {
        writeNumericAttribute<quint32>(OTBM_ATTR_SLEEPERGUID, getAttribute(Item::AttrSleeperGuid).toUInt());
    }
    if (hasAttribute(Item::AttrSleepStart) && getAttribute(Item::AttrSleepStart).toUInt() > 0) {
        writeNumericAttribute<quint32>(OTBM_ATTR_SLEEPSTART, getAttribute(Item::AttrSleepStart).toUInt());
    }
    if (hasAttribute(Item::AttrDecayingState) && getAttribute(Item::AttrDecayingState).toUInt() > 0) {
        writeNumericAttribute<quint8>(OTBM_ATTR_DECAYING_STATE, getAttribute(Item::AttrDecayingState).toUInt());
    }
    if (hasAttribute(Item::AttrPodiumOutfit)) {
        QByteArray podiumData = getAttribute(Item::AttrPodiumOutfit).toByteArray();
        if (!podiumData.isEmpty()) {
            stream << static_cast<quint8>(OTBM_ATTR_PODIUMOUTFIT);
            stream << static_cast<quint16>(podiumData.size());
            stream.writeRawData(podiumData.constData(), podiumData.size());
        }
    }

    // Serialize custom attributes using OTBM_ATTR_ATTRIBUTE_MAP for TFS 1.x+ style custom attributes
    QByteArray customAttributeData = serializeCustomAttributeMap();
    if (!customAttributeData.isEmpty()) {
        stream << static_cast<quint8>(OTBM_ATTR_ATTRIBUTE_MAP);
        stream << static_cast<quint16>(customAttributeData.size());
        stream.writeRawData(customAttributeData.constData(), customAttributeData.size());
    }

    return stream.status() == QDataStream::Ok;
}

bool Item::serializeOtbmNode(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << static_cast<quint8>(OTBM_ITEM); // Use OTBM_NodeTypes_t enum value
    stream << getServerId(); // quint16 serverId_ member

    // Write attributes. The OTBM format implies attributes follow directly.
    // The overall length of the item node (including attributes) is typically handled
    // by the calling OtbmWriter when it finalizes the node.
    if (!serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }

    // After attributes, for OTBM, an end byte (0xFF) is typically written by the OtbmWriter
    // to signify the end of the current node's properties/attributes, before writing children.
    // This method only serializes the item's direct properties. Node structure is for OtbmWriter.

    return stream.status() == QDataStream::Ok;
}

// --- Additional Brush-related Properties ---

bool Item::isWall() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.isWall;
}

bool Item::isBorder() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.isBorder;
}

// --- Selection Methods ---

bool Item::isSelected() const {
    return isSelected_;
}

void Item::select() {
    if (!isSelected_) {
        isSelected_ = true;
        setModified(true);
        emit propertyChanged();
    }
}

void Item::deselect() {
    if (isSelected_) {
        isSelected_ = false;
        setModified(true);
        emit propertyChanged();
    }
}

// --- Memory and Utility Methods ---

quint32 Item::memsize() const {
    quint32 size = sizeof(Item);

    // Add size of string members
    size += name_.size() * sizeof(QChar);
    size += itemTypeName_.size() * sizeof(QChar);
    // Task 017: Removed description_ - now stored in attributes_ map (counted below)
    size += editorSuffix_.size() * sizeof(QChar);

    // Add size of attributes map
    for (auto it = attributes_.begin(); it != attributes_.end(); ++it) {
        size += it.key().size() * sizeof(QChar);
        size += sizeof(QVariant); // Approximate size of QVariant
    }

    return size;
}

bool Item::hasProperty(int property) const {
    // Enhanced property checking based on ITEMPROPERTY enum values
    switch (static_cast<ITEMPROPERTY>(property)) {
        case BLOCKSOLID:
            return isBlocking_;
        case HASHEIGHT:
            return hasHeight_;
        case BLOCKPROJECTILE:
            return blocksMissiles_;
        case BLOCKPATHFIND:
            return blocksPathfind_;
        case PROTECTIONZONE:
            // Protection zone is typically a tile property, not item property
            return false;
        case HOOK_SOUTH:
            return hasHookSouth_;
        case HOOK_EAST:
            return hasHookEast_;
        case MOVEABLE:
            return isMoveable_ && getUniqueId() == 0;
        case BLOCKINGANDNOTMOVEABLE:
            return isBlocking_ && (!isMoveable_ || getUniqueId() != 0);
        case HASLIGHT:
            return hasLight();
        default:
            return false;
    }
}

quint8 Item::getMiniMapColor() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.minimapColor;
}

// --- Enhanced Helper Methods ---

bool Item::isClientCharged() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.clientCharges;
}

bool Item::isExtraCharged() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.extraChargeable;
}

bool Item::canHoldText() const {
    return isReadable() || canWriteText();
}

bool Item::canHoldDescription() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.allowDistRead;
}

bool Item::hasLight() const {
    return lightLevel_ > 0;
}

bool Item::isAlwaysOnBottom() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    return itemProps.alwaysOnBottom;
}

int Item::getHeight() const {
    return height_;
}

std::pair<int, int> Item::getDrawOffset() const {
    return std::make_pair(drawOffsetX_, drawOffsetY_);
}

double Item::getWeight() const {
    const ItemProperties& itemProps = ItemManager::getInstance().getItemProperties(serverId_);
    if (isStackable()) {
        return itemProps.weight * getCount();
    }
    return itemProps.weight;
}

// --- Additional Setters ---

void Item::setHeight(int height) {
    if (height_ != height) {
        height_ = height;
        setModified(true);
        emit propertyChanged();
    }
}

void Item::setDrawOffset(int x, int y) {
    if (drawOffsetX_ != x || drawOffsetY_ != y) {
        drawOffsetX_ = x;
        drawOffsetY_ = y;
        setModified(true);
        emit propertyChanged();
    }
}

// Task 48: Complex data handling methods
bool Item::unserializeCustomAttributeMap(const QByteArray& data)
{
    if (data.isEmpty()) {
        return true; // Empty attribute map is valid
    }

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    try {
        while (!stream.atEnd()) {
            // Read attribute key (string)
            QString key;
            stream >> key;

            if (stream.status() != QDataStream::Ok) {
                qWarning() << "Item::unserializeCustomAttributeMap: Failed to read attribute key";
                return false;
            }

            // Read attribute type
            quint8 type;
            stream >> type;

            if (stream.status() != QDataStream::Ok) {
                qWarning() << "Item::unserializeCustomAttributeMap: Failed to read attribute type for key:" << key;
                return false;
            }

            // Read attribute value based on type
            QVariant value;
            switch (type) {
                case 1: { // String
                    QString stringValue;
                    stream >> stringValue;
                    value = stringValue;
                    break;
                }
                case 2: { // Integer (32-bit)
                    qint32 intValue;
                    stream >> intValue;
                    value = intValue;
                    break;
                }
                case 3: { // Float
                    float floatValue;
                    stream >> floatValue;
                    value = floatValue;
                    break;
                }
                case 4: { // Boolean
                    quint8 boolValue;
                    stream >> boolValue;
                    value = (boolValue != 0);
                    break;
                }
                case 5: { // Double
                    double doubleValue;
                    stream >> doubleValue;
                    value = doubleValue;
                    break;
                }
                default:
                    qWarning() << "Item::unserializeCustomAttributeMap: Unknown attribute type:" << type << "for key:" << key;
                    return false;
            }

            if (stream.status() != QDataStream::Ok) {
                qWarning() << "Item::unserializeCustomAttributeMap: Failed to read attribute value for key:" << key;
                return false;
            }

            // Store the custom attribute with a prefix to distinguish from standard attributes
            QString customKey = QString("custom_%1").arg(key);
            setAttribute(customKey, value);

            qDebug() << "Item::unserializeCustomAttributeMap: Loaded custom attribute:" << customKey << "=" << value;
        }

        return true;

    } catch (...) {
        qWarning() << "Item::unserializeCustomAttributeMap: Exception occurred while parsing custom attributes";
        return false;
    }
}

QByteArray Item::serializeCustomAttributeMap() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Find all custom attributes (those with "custom_" prefix)
    QMap<QString, QVariant>::const_iterator it;
    for (it = attributes_.constBegin(); it != attributes_.constEnd(); ++it) {
        const QString& key = it.key();
        if (!key.startsWith("custom_")) {
            continue; // Skip non-custom attributes
        }

        // Remove the "custom_" prefix for serialization
        QString originalKey = key.mid(7); // Remove "custom_" prefix
        const QVariant& value = it.value();

        // Write attribute key
        stream << originalKey;

        // Determine and write attribute type, then value
        if (value.typeId() == QMetaType::QString) {
            stream << static_cast<quint8>(1); // String type
            stream << value.toString();
        } else if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::LongLong) {
            stream << static_cast<quint8>(2); // Integer type
            stream << value.toInt();
        } else if (value.typeId() == QMetaType::Double) {
            double doubleVal = value.toDouble();
            if (doubleVal == static_cast<float>(doubleVal)) {
                stream << static_cast<quint8>(3); // Float type
                stream << static_cast<float>(doubleVal);
            } else {
                stream << static_cast<quint8>(5); // Double type
                stream << doubleVal;
            }
        } else if (value.typeId() == QMetaType::Bool) {
            stream << static_cast<quint8>(4); // Boolean type
            stream << static_cast<quint8>(value.toBool() ? 1 : 0);
        } else {
            // For unknown types, convert to string
            stream << static_cast<quint8>(1); // String type
            stream << value.toString();
        }

        qDebug() << "Item::serializeCustomAttributeMap: Serialized custom attribute:" << originalKey << "=" << value;
    }

    return data;
}

// Task 76: Special item flag rendering implementation
void Item::drawSpecialFlags(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    if (!painter) return;

    // Draw selection highlight if item is selected
    if (isSelected() && options.highlightSelectedTile) {
        drawSelectionHighlight(painter, targetRect, options);
    }

    // Draw locked door highlight if enabled
    if (options.highlightLockedDoors && isDoor() && isLocked()) {
        drawLockedDoorHighlight(painter, targetRect, options);
    }

    // Draw wall hook indicator if enabled
    if (options.showWallHooks && isWallHook()) {
        drawWallHookIndicator(painter, targetRect, options);
    }

    // Draw blocking indicator if enabled
    if (options.showBlocking && isBlocking()) {
        drawBlockingIndicator(painter, targetRect, options);
    }
}

void Item::drawSelectionHighlight(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    if (!painter) return;

    painter->save();

    // Draw animated selection highlight
    QColor highlightColor(0, 255, 255, 100); // Cyan highlight

    // Add pulsing animation effect
    if (options.showPreview) {
        qreal time = QTime::currentTime().msecsSinceStartOfDay() / 1000.0;
        qreal pulse = (qSin(time * 3.0) + 1.0) * 0.5; // 0.0 to 1.0
        highlightColor.setAlpha(50 + static_cast<int>(pulse * 100));
    }

    // Draw highlight border
    QPen highlightPen(highlightColor.lighter(150), 2);
    painter->setPen(highlightPen);
    painter->setBrush(QBrush(highlightColor));
    painter->drawRect(targetRect);

    painter->restore();
}

void Item::drawLockedDoorHighlight(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    if (!painter) return;

    painter->save();

    // Draw red highlight for locked doors
    QColor lockColor(255, 0, 0, 80); // Red highlight
    QPen lockPen(lockColor.darker(150), 2);
    painter->setPen(lockPen);
    painter->setBrush(QBrush(lockColor));
    painter->drawRect(targetRect);

    // Draw lock icon if there's space
    if (targetRect.width() > 16 && targetRect.height() > 16) {
        QRectF iconRect = targetRect.adjusted(2, 2, -2, -2);
        painter->setPen(QPen(Qt::white, 1));
        painter->setBrush(Qt::NoBrush);

        // Simple lock icon
        QRectF lockBody = iconRect.adjusted(iconRect.width() * 0.3, iconRect.height() * 0.4,
                                           -iconRect.width() * 0.3, -iconRect.height() * 0.1);
        QRectF lockShackle = iconRect.adjusted(iconRect.width() * 0.35, iconRect.height() * 0.1,
                                              -iconRect.width() * 0.35, -iconRect.height() * 0.5);

        painter->drawRect(lockBody);
        painter->drawArc(lockShackle, 0, 180 * 16); // Half circle for shackle
    }

    painter->restore();
}

void Item::drawWallHookIndicator(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    if (!painter) return;

    painter->save();

    // Draw wall hook indicator
    QColor hookColor(255, 255, 0, 120); // Yellow indicator
    QPen hookPen(hookColor.darker(150), 1);
    painter->setPen(hookPen);

    // Draw hook shape
    QPointF center = targetRect.center();
    qreal radius = qMin(targetRect.width(), targetRect.height()) * 0.3;

    // Draw hook as a small arc
    QRectF hookRect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
    painter->drawArc(hookRect, 45 * 16, 180 * 16); // Quarter circle hook

    painter->restore();
}

void Item::drawBlockingIndicator(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    if (!painter) return;

    painter->save();

    // Draw blocking indicator with cross pattern
    QColor blockColor(255, 128, 0, 100); // Orange indicator
    QPen blockPen(blockColor.darker(150), 2);
    painter->setPen(blockPen);

    // Draw X pattern to indicate blocking
    painter->drawLine(targetRect.topLeft(), targetRect.bottomRight());
    painter->drawLine(targetRect.topRight(), targetRect.bottomLeft());

    painter->restore();
}
