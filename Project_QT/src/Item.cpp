#include "Item.h"
#include <QDebug>
#include <QPainter> // For drawText and draw
#include <QRectF>   // For drawText and draw
#include <QColor>   // For draw method placeholder
#include "ItemManager.h" // Required for ItemTypeData access
#include "Brush.h"       // Required for Brush* return type and itemTypeData->brush
#include <QDataStream>  // For unserializeOtbmAttributes
#include <QByteArray>   // For unserializeOtbmAttributes
#include <QIODevice>    // For QDataStream operations (usually included by QDataStream)
#include "Tile.h"        // For qobject_cast<Tile*> in setModified

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
    description_(""),
    editorSuffix_(""),
    itemGroup_(ITEM_GROUP_NONE),
    itemType_(ITEM_TYPE_NONE),
    weight_(0.0f),
    attack_(0),
    defense_(0),
    armor_(0),
    charges_(0),
    maxTextLen_(0),
    rotateTo_(0),
    volume_(0),
    slotPosition_(0),
    weaponType_(0), // Assuming 0 is WEAPON_NONE or similar default
    lightLevel_(0),
    lightColor_(0),
    classification_(0)
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

// Renamed setters for macro compatibility in Item.h, now implementing them directly for clarity
void Item::setIsTeleport(bool on) { setTeleport(on); }
void Item::setIsContainer(bool on) { setContainer(on); }


// Brush-related property implementations
bool Item::isTable() const {
    // Assuming ItemManager::getInstance() and getItemTypeData() are thread-safe if used across threads.
    // For typical editor usage, this might be single-threaded.
    const ItemTypeData* itemTypeData = ItemManager::getInstance().getItemTypeData(serverId_);
    if (itemTypeData) {
        return itemTypeData->isTable; // Assumes ItemTypeData struct has 'isTable'
    }
    return false;
}

bool Item::isCarpet() const {
    const ItemTypeData* itemTypeData = ItemManager::getInstance().getItemTypeData(serverId_);
    if (itemTypeData) {
        return itemTypeData->isCarpet; // Assumes ItemTypeData struct has 'isCarpet'
    }
    return false;
}

Brush* Item::getBrush() const {
    const ItemTypeData* itemTypeData = ItemManager::getInstance().getItemTypeData(serverId_);
    if (itemTypeData) {
        return itemTypeData->brush; // Assumes ItemTypeData struct has 'brush'
    }
    return nullptr;
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

    // Append text from AttrText if available
    if (hasAttribute(Item::AttrText)) {
        QString textAttr = getText(); // getText() now uses Item::AttrText
        if(!textAttr.isEmpty()){
             desc += "\n\"" + textAttr + "\"";
        }
    }
    // Append additional description from AttrDescription if available
    // This is different from the primary item look description stored in member description_
    if (hasAttribute(Item::AttrDescription)) {
        desc += "\n" + getAttribute(Item::AttrDescription).toString();
    }
    return desc;
}

void Item::drawText(QPainter* painter, const QRectF& targetRect, const QMap<QString, QVariant>& options) {
    if (painter && isStackable_ && getCount() > 1) {
        QString countStr = QString::number(getCount());
        painter->save();
        QFont font = painter->font();
        font.setPointSize(font.pointSize() - 2 > 0 ? font.pointSize() - 2 : 6);
        painter->setFont(font);
        painter->setPen(Qt::red); 
        QRectF textRect = painter->fontMetrics().boundingRect(countStr);
        textRect.moveBottomRight(targetRect.bottomRight() - QPointF(1,1));
        painter->drawText(textRect, countStr);
        painter->restore();
    }
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
    
    newItem->description_ = this->description_;
    newItem->editorSuffix_ = this->editorSuffix_;
    newItem->itemGroup_ = this->itemGroup_;
    newItem->itemType_ = this->itemType_;
    newItem->weight_ = this->weight_;
    newItem->attack_ = this->attack_;
    newItem->defense_ = this->defense_;
    newItem->armor_ = this->armor_;
    newItem->charges_ = this->charges_;
    newItem->maxTextLen_ = this->maxTextLen_;
    newItem->rotateTo_ = this->rotateTo_;
    newItem->volume_ = this->volume_;
    newItem->slotPosition_ = this->slotPosition_;
    newItem->weaponType_ = this->weaponType_;
    newItem->lightLevel_ = this->lightLevel_;
    newItem->lightColor_ = this->lightColor_;
    newItem->classification_ = this->classification_;

    return newItem;
}

void Item::draw(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    if (!painter) return;
    
    QColor itemColor = Qt::blue;
    itemColor.setHsv(((serverId_ * 37) % 360), 200, 220); 
    
    painter->fillRect(targetRect, QColor(itemColor.red(), itemColor.green(), itemColor.blue(), 128));
    painter->setPen(Qt::black);
    painter->drawRect(targetRect);

    if (options.drawDebugInfo) {
        painter->save();
        QPen debugPen(Qt::magenta);
        debugPen.setStyle(Qt::DotLine);
        painter->setPen(debugPen);
        painter->drawRect(targetRect);

        QString idText = QString("ID:%1").arg(serverId_);
        QFont font = painter->font();
        font.setPointSize(8);
        painter->setFont(font);
        painter->setPen(Qt::white);
        painter->drawText(targetRect.adjusted(2, 2, -2, -2), Qt::AlignTop | Qt::AlignLeft | Qt::TextDontClip, idText);
        painter->restore();
    }
}

// --- New Dedicated Property Getters and Setters ---
QString Item::descriptionText() const { return description_; }
void Item::setDescriptionText(const QString& description) {
    if (description_ != description) {
        description_ = description;
        setModified(true); // Also sets modified if direct member changes
        setAttribute(Item::AttrDescription, description_); // Keep attributes_ in sync
        emit propertyChanged();
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

quint16 Item::charges() const { return charges_; }
void Item::setCharges(quint16 charges) {
    if (charges_ != charges) {
        charges_ = charges;
        setModified(true); // Also sets modified if direct member changes
        setAttribute(Item::AttrCharges, charges_); // Keep attributes_ in sync
        emit propertyChanged();
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
            case OTBM_ATTR_RUNE_CHARGES: { // Typically quint8
                if (dataLength < sizeof(quint8)) { qWarning("ATTR_RUNE_CHARGES data too short"); break; }
                quint8 val; attributeValueStream >> val;
                setCharges(val); // Map to general charges
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
            // case OTBM_ATTR_WRITTENDATE: // Example: quint32
            // case OTBM_ATTR_HOUSEDOORID: // Example: quint8

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
    // For description, use the direct member description_ if available, otherwise from attributes map
    QString currentDescription = descriptionText(); // This getter accesses description_
    if (!currentDescription.isEmpty() || hasAttribute(Item::AttrDescription)) { // Check direct member or if it was set via generic attributes
         writeStringAttribute(OTBM_ATTR_DESC, currentDescription.isEmpty() ? getAttribute(Item::AttrDescription).toString() : currentDescription);
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

    // TODO: Serialize other generic attributes from m_attributes if they don't map to known OTBM types
    // This might involve using OTBM_ATTR_ATTRIBUTE_MAP for TFS 1.x+ style custom attributes.
    // For now, only known/typed attributes are serialized.

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
