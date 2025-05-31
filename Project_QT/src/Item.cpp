#include "Item.h"
#include <QDebug>
#include <QPainter> // For drawText and draw
#include <QRectF>   // For drawText and draw
#include <QColor>   // For draw method placeholder
#include "ItemManager.h" // Required for ItemTypeData access
#include "Brush.h"       // Required for Brush* return type and itemTypeData->brush

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
        return getAttribute(QStringLiteral("count"), 1).toInt(); 
    }
    return 1;
}

void Item::setCount(int count) {
    if (isStackable_) {
        if (count <= 0) count = 1;
        setAttribute(QStringLiteral("count"), count);
    } else if (count != 1) {
        // qWarning() << "Item (ID:" << serverId_ << ", Name:" << name_ << ") is not stackable, cannot set count to" << count;
    }
}

QString Item::getText() const {
    return getAttribute(QStringLiteral("text")).toString();
}
void Item::setText(const QString& text) {
    setAttribute(QStringLiteral("text"), text);
}

int Item::getActionId() const {
    return getAttribute(QStringLiteral("actionId")).toInt();
}
void Item::setActionId(int id) {
    setAttribute(QStringLiteral("actionId"), id);
}

int Item::getUniqueId() const {
    return getAttribute(QStringLiteral("uniqueId")).toInt();
}
void Item::setUniqueId(int id) {
    setAttribute(QStringLiteral("uniqueId"), id);
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
ITEM_BOOL_PROPERTY_IMPL(Teleport, isTeleport_) // Renamed setIsTeleport to setTeleport for macro
ITEM_BOOL_PROPERTY_IMPL(Container, isContainer_) // Renamed setIsContainer to setContainer for macro
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
    const ItemTypeData* itemTypeData = ItemManager::getInstance().getItemTypeData(serverId_);
    if (itemTypeData) {
        return itemTypeData->isTable; // Assumes ItemTypeData has a public bool member 'isTable'
    }
    return false;
}

Brush* Item::getBrush() const {
    const ItemTypeData* itemTypeData = ItemManager::getInstance().getItemTypeData(serverId_);
    if (itemTypeData) {
        return itemTypeData->brush; // Assumes ItemTypeData has a public Brush* member 'brush'
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

    if (hasAttribute(QStringLiteral("text"))) {
        QString textAttr = getText();
        if(!textAttr.isEmpty()){
             desc += "\n\"" + textAttr + "\"";
        }
    }
    if (hasAttribute(QStringLiteral("description"))) {
        desc += "\n" + getAttribute(QStringLiteral("description")).toString();
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
    
    // Copy new dedicated members
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
        emit propertyChanged();
    }
}
