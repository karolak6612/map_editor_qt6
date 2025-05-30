#include "Item.h"
#include <QDebug>
#include <QPainter> // For drawText and draw
#include <QRectF>   // For drawText and draw
#include <QColor>   // For draw method placeholder
// QMap and QVariant are included via Item.h
// DrawingOptions.h is included via Item.h

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
    // This is unusual to change after creation as it's the primary ID.
    // If it must be settable, ensure all systems depending on it are updated.
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
    if (oldValue != value) { // Only emit if value actually changes
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
        emit attributeChanged(key, QVariant()); // Emit with an invalid QVariant to signify removal
    }
}

const QMap<QString, QVariant>& Item::getAttributes() const {
    return attributes_;
}

// Specific Attribute Accessors
int Item::getCount() const {
    if (isStackable_) {
        // Stackable items store count in attributes, default to 1 if not set.
        return getAttribute(QStringLiteral("count"), 1).toInt(); 
    }
    return 1; // Non-stackable items always have a count of 1
}

void Item::setCount(int count) {
    if (isStackable_) {
        if (count <= 0) count = 1; // Stackable items should have at least 1
        setAttribute(QStringLiteral("count"), count);
    } else if (count != 1) {
        // It's a common operation to try to set count, even for non-stackables (e.g. from generic code).
        // Silently ignore or log a warning. For now, ignore.
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
#define ITEM_BOOL_PROPERTY_IMPL(PropName, MemberName) \
bool Item::is##PropName() const { return MemberName; } \
void Item::set##PropName(bool on) { if (MemberName != on) { MemberName = on; emit propertyChanged(); } }

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

// These have different names or slightly different logic
bool Item::blocksMissiles() const { return blocksMissiles_; }
void Item::setBlocksMissiles(bool on) { if (blocksMissiles_ != on) { blocksMissiles_ = on; emit propertyChanged(); } }

bool Item::blocksPathfind() const { return blocksPathfind_; }
void Item::setBlocksPathfind(bool on) { if (blocksPathfind_ != on) { blocksPathfind_ = on; emit propertyChanged(); } }

int Item::getTopOrder() const { return topOrder_; }
void Item::setTopOrder(int order) { if (topOrder_ != order) { topOrder_ = order; emit propertyChanged(); } }


// Other methods
QString Item::getDescription() const {
    QString desc = name_;
    if (!desc.isEmpty()) {
        desc += " ";
    }
    desc += QString("(ID: %1").arg(serverId_);
    if (clientId_ != 0 && clientId_ != serverId_) { // Only show client ID if different and non-zero
        desc += QString(", ClientID: %1").arg(clientId_);
    }
    desc += ")";

    if (hasAttribute(QStringLiteral("text"))) {
        QString textAttr = getText();
        if(!textAttr.isEmpty()){
             desc += "\n\"" + textAttr + "\"";
        }
    }
    if (hasAttribute(QStringLiteral("description"))) { // A specific description attribute
        desc += "\n" + getAttribute(QStringLiteral("description")).toString();
    }
    // Add more attributes to description as needed
    return desc;
}

void Item::drawText(QPainter* painter, const QRectF& targetRect, const QMap<QString, QVariant>& options) {
    // Placeholder for drawing item count or other text directly on the item
    if (painter && isStackable_ && getCount() > 1) {
        // Basic example: draw count in a corner.
        // Actual positioning and font will need refinement.
        QString countStr = QString::number(getCount());
        painter->save();
        // Example: small font, red color, bottom-right
        QFont font = painter->font();
        font.setPointSize(font.pointSize() - 2 > 0 ? font.pointSize() - 2 : 6); // Make it small
        painter->setFont(font);
        painter->setPen(Qt::red); 
        // painter->drawText(targetRect, Qt::AlignBottom | Qt::AlignRight, countStr);
        // For more control, calculate text rect:
        QRectF textRect = painter->fontMetrics().boundingRect(countStr);
        textRect.moveBottomRight(targetRect.bottomRight() - QPointF(1,1)); // Small offset
        painter->drawText(textRect, countStr);
        painter->restore();
        // qDebug() << "Item::drawText placeholder for item ID" << serverId_ << "Count:" << getCount();
    }
    // Other text drawing (e.g., from getText() if item is readable and text is short) could go here
}

Item* Item::deepCopy() const {
    // Create with same ID, parent will be set by caller (e.g., Tile or when placing on map)
    Item* newItem = new Item(this->serverId_); 
    
    newItem->clientId_ = this->clientId_;
    newItem->name_ = this->name_;
    newItem->itemTypeName_ = this->itemTypeName_;
    
    // Deep copy attributes: QMap<QString, QVariant> performs a deep copy if QVariants are simple types.
    // If QVariants store pointers, more care is needed for those specific attributes.
    newItem->attributes_ = this->attributes_; 

    // Copy boolean flags
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
    
    return newItem;
}

void Item::draw(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    if (!painter) return;
    
    // Placeholder: Draw a semi-transparent blueish rectangle for a generic item
    QColor itemColor = Qt::blue;
    // Use serverId_ for varied color, it's a quint16
    // To make it more visually distinct, ensure serverId_ doesn't map to similar hues too often.
    // A simple way is to multiply by a prime or a number that distributes well across 360 degrees.
    itemColor.setHsv(((serverId_ * 37) % 360), 200, 220); 
    
    painter->fillRect(targetRect, QColor(itemColor.red(), itemColor.green(), itemColor.blue(), 128)); // Semi-transparent
    painter->setPen(Qt::black); // Explicitly set pen for outline after fillRect might change it
    painter->drawRect(targetRect);

    if (options.drawDebugInfo) {
        painter->save();
        // Draw bounding box (already drawn above, but could be a different color for debug)
        QPen debugPen(Qt::magenta);
        debugPen.setStyle(Qt::DotLine);
        painter->setPen(debugPen);
        painter->drawRect(targetRect);

        // Draw Server ID
        QString idText = QString("ID:%1").arg(serverId_);
        QFont font = painter->font();
        font.setPointSize(8); // Small font for debug info
        painter->setFont(font);
        painter->setPen(Qt::white); // Ensure text is visible
        // Position text inside the box, e.g., top-left
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
    // Compare floats with a small epsilon if exact comparison is problematic
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
    if (this->volume_ != volume) { // Explicit this-> to avoid confusion with a potential local variable
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
