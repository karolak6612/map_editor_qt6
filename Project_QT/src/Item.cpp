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
        emit propertyChanged(); 
    }
}

QString Item::name() const { return name_; }
void Item::setName(const QString& name) {
    if (name_ != name) {
        name_ = name;
        emit propertyChanged();
    }
}

QString Item::typeName() const { return itemTypeName_; }
void Item::setTypeName(const QString& typeName) {
    if (itemTypeName_ != typeName) {
        itemTypeName_ = typeName;
        emit propertyChanged();
    }
}

// Generic Attribute System
void Item::setAttribute(const QString& key, const QVariant& value) {
    QVariant oldValue = attributes_.value(key);
    if (oldValue != value) {
        attributes_.insert(key, value);
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
void Item::set##PropNameCapital(bool on) { if (MemberName != on) { MemberName = on; emit propertyChanged(); } }

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
void Item::setBlocksMissiles(bool on) { if (blocksMissiles_ != on) { blocksMissiles_ = on; emit propertyChanged(); } }

bool Item::blocksPathfind() const { return blocksPathfind_; }
void Item::setBlocksPathfind(bool on) { if (blocksPathfind_ != on) { blocksPathfind_ = on; emit propertyChanged(); } }

int Item::getTopOrder() const { return topOrder_; }
void Item::setTopOrder(int order) { if (topOrder_ != order) { topOrder_ = order; emit propertyChanged(); } }

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
        setAttribute(Item::AttrDescription, description_); // Keep attributes_ in sync
        emit propertyChanged();
    }
}

QString Item::editorSuffix() const { return editorSuffix_; }
void Item::setEditorSuffix(const QString& suffix) {
    if (editorSuffix_ != suffix) {
        editorSuffix_ = suffix;
        emit propertyChanged();
    }
}

ItemGroup_t Item::itemGroup() const { return itemGroup_; }
void Item::setItemGroup(ItemGroup_t group) {
    if (itemGroup_ != group) {
        itemGroup_ = group;
        emit propertyChanged();
    }
}

ItemTypes_t Item::itemType() const { return itemType_; }
void Item::setItemType(ItemTypes_t type) {
    if (itemType_ != type) {
        itemType_ = type;
        emit propertyChanged();
    }
}

float Item::weight() const { return weight_; }
void Item::setWeight(float weight) {
    if (qAbs(weight_ - weight) > 0.0001f) { 
        weight_ = weight;
        emit propertyChanged();
    }
}

qint16 Item::attack() const { return attack_; }
void Item::setAttack(qint16 attack) {
    if (attack_ != attack) {
        attack_ = attack;
        emit propertyChanged();
    }
}

qint16 Item::defense() const { return defense_; }
void Item::setDefense(qint16 defense) {
    if (defense_ != defense) {
        defense_ = defense;
        emit propertyChanged();
    }
}

qint16 Item::armor() const { return armor_; }
void Item::setArmor(qint16 armor) {
    if (armor_ != armor) {
        armor_ = armor;
        emit propertyChanged();
    }
}

quint16 Item::charges() const { return charges_; }
void Item::setCharges(quint16 charges) {
    if (charges_ != charges) {
        charges_ = charges;
        setAttribute(Item::AttrCharges, charges_); // Keep attributes_ in sync
        emit propertyChanged();
    }
}

quint16 Item::maxTextLen() const { return maxTextLen_; }
void Item::setMaxTextLen(quint16 len) {
    if (maxTextLen_ != len) {
        maxTextLen_ = len;
        emit propertyChanged();
    }
}

quint16 Item::rotateTo() const { return rotateTo_; }
void Item::setRotateTo(quint16 id) {
    if (rotateTo_ != id) {
        rotateTo_ = id;
        emit propertyChanged();
    }
}

quint16 Item::volume() const { return volume_; }
void Item::setVolume(quint16 volume) {
    if (this->volume_ != volume) {
        this->volume_ = volume;
        emit propertyChanged();
    }
}

quint32 Item::slotPosition() const { return slotPosition_; }
void Item::setSlotPosition(quint32 slotPos) {
    if (slotPosition_ != slotPos) {
        slotPosition_ = slotPos;
        emit propertyChanged();
    }
}

quint8 Item::weaponType() const { return weaponType_; }
void Item::setWeaponType(quint8 type) {
    if (weaponType_ != type) {
        weaponType_ = type;
        emit propertyChanged();
    }
}

quint16 Item::lightLevel() const { return lightLevel_; }
void Item::setLightLevel(quint16 level) {
    if (lightLevel_ != level) {
        lightLevel_ = level;
        emit propertyChanged();
    }
}

quint16 Item::lightColor() const { return lightColor_; }
void Item::setLightColor(quint16 color) {
    if (lightColor_ != color) {
        lightColor_ = color;
        emit propertyChanged();
    }
}

quint16 Item::classification() const { return classification_; }
void Item::setClassification(quint16 classification) {
    if (classification_ != classification) {
        classification_ = classification;
        setAttribute(Item::AttrTier, classification_); // Keep attributes_ in sync using AttrTier
        emit propertyChanged();
    }
}

// OTBM Attribute IDs (common values, may need adjustment based on specific OTBM version target)
namespace OtbmAttr {
    static const quint8 DESCRIPTION      = 0x01; // Often description or special text
    static const quint8 EXT_FILE         = 0x02; // Not typically handled as a direct item attribute for storage
    static const quint8 TILE_FLAGS       = 0x03; // Usually for Tile objects, not item attributes directly
    static const quint8 ACTION_ID        = 0x04;
    static const quint8 UNIQUE_ID        = 0x05;
    // static const QString ItemAttrTextKey = Item::AttrText; // This was a note, Item::AttrText is used directly
    static const quint8 TEXT             = 0x06; // For item text
    static const quint8 WRITTEN_DATE     = 0x07;
    static const quint8 WRITTEN_BY       = 0x08;
    static const quint8 DESCRIPTION_ALT  = 0x09; // Often the primary "description" or "look text"
    static const quint8 CHARGES_COUNT    = 0x0D;
    static const quint8 DURATION         = 0x0F;
    static const quint8 DEPOT_ID         = 0x10;
    static const quint8 TELEPORT_DEST    = 0x12;
    static const quint8 HOUSE_DOOR_ID    = 0x16; // If item is a house door, this is its house-unique ID
    static const quint8 TIER             = 0x1A; // Example for Tier, might be different
}

bool Item::unserializeOtbmAttributes(QDataStream& stream) {
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

        switch (attributeId) {
            case OtbmAttr::DESCRIPTION:
            case OtbmAttr::DESCRIPTION_ALT: {
                setDescriptionText(QString::fromUtf8(attributeDataBytes));
                break;
            }
            case OtbmAttr::TEXT: {
                setText(QString::fromUtf8(attributeDataBytes));
                break;
            }
            case OtbmAttr::WRITTEN_BY: {
                // Assuming no dedicated member for writer yet, just use attribute
                setAttribute(Item::AttrWriter, QString::fromUtf8(attributeDataBytes));
                break;
            }
            case OtbmAttr::CHARGES_COUNT: {
                if (dataLength < sizeof(quint16)) { // Basic validation
                     qWarning() << "Item::unserializeOtbmAttributes - CHARGES_COUNT dataLength too short:" << dataLength;
                     // Skip this attribute or return false depending on strictness
                     break;
                }
                quint16 val;
                attributeValueStream >> val;
                // This could be charges or stack count.
                // If this item is stackable, set its count.
                if (isStackable()) {
                    setCount(val);
                } else {
                    // For non-stackable items, this typically represents charges.
                    setCharges(val);
                }
                // Note: setCount and setCharges should ideally handle their respective Attr keys.
                // If AttrCount and AttrCharges can be distinct for some items even if not stackable,
                // then more nuanced logic or direct setAttribute calls might be needed here based on item type.
                // For now, this directs CHARGES_COUNT to either stackable count or dedicated charges.
                break;
            }
            case OtbmAttr::ACTION_ID: {
                if (dataLength < sizeof(quint16)) break;
                quint16 val;
                attributeValueStream >> val;
                setActionId(val);
                break;
            }
            case OtbmAttr::UNIQUE_ID: {
                if (dataLength < sizeof(quint16)) break;
                quint16 val;
                attributeValueStream >> val;
                setUniqueId(val);
                break;
            }
            case OtbmAttr::DURATION: {
                if (dataLength < sizeof(quint32)) break;
                quint32 val;
                attributeValueStream >> val;
                setAttribute(Item::AttrDuration, val);
                break;
            }
            case OtbmAttr::DEPOT_ID: {
                if (dataLength < sizeof(quint16)) break;
                quint16 val;
                attributeValueStream >> val;
                setAttribute(Item::AttrDepotID, val);
                break;
            }
            case OtbmAttr::TELEPORT_DEST: {
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
             case OtbmAttr::TIER: {
                if (dataLength < sizeof(quint16)) break;
                quint16 val;
                attributeValueStream >> val;
                setClassification(val); // This will also set AttrTier via the setter
                break;
            }
            // case OtbmAttr::WRITTEN_DATE: // Example: quint32
            // case OtbmAttr::HOUSE_DOOR_ID: // Example: quint8

            default:
                qDebug() << "Item::unserializeOtbmAttributes - Unhandled attribute ID:" << Qt::hex << attributeId << "Length:" << dataLength;
                QString unknownKey = QString("otbm_attr_%1").arg(attributeId, 2, 16, QChar('0'));
                setAttribute(unknownKey, QVariant(attributeDataBytes)); // Store raw bytes
                break;
        }

        // Check stream status for operations that read from attributeValueStream
        bool isStringType = (attributeId == OtbmAttr::DESCRIPTION ||
                             attributeId == OtbmAttr::DESCRIPTION_ALT ||
                             attributeId == OtbmAttr::TEXT ||
                             attributeId == OtbmAttr::WRITTEN_BY);

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
    return true; // Successfully read all available attributes
}
