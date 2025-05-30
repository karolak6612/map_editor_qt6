#include "Item.h"
#include <QDebug>
#include <QPainter> // For drawText
#include <QRectF>   // For drawText
// QMap and QVariant are included via Item.h

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
    hasHeight_(false)
{
    // Name, ClientID, and flags are often set by an ItemManager after creation
    // based on the serverId by reading an items.xml or similar definition file.
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
