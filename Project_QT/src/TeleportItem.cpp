#include "TeleportItem.h"
#include "ItemManager.h"
#include <QDataStream>
#include <QDebug>

TeleportItem::TeleportItem(quint16 serverId, QObject *parent)
    : Item(serverId, parent), destination_(0, 0, 0) {
    // Set teleport-specific properties
    setIsTeleport(true);
}

TeleportItem::~TeleportItem() {
    // No special cleanup needed
}

MapPos TeleportItem::getDestination() const {
    return destination_;
}

void TeleportItem::setDestination(const MapPos& destination) {
    if (destination_ != destination) {
        destination_ = destination;
        setModified(true);
        
        // Update attributes for serialization
        setAttribute(AttrTeleDestX, destination.x);
        setAttribute(AttrTeleDestY, destination.y);
        setAttribute(AttrTeleDestZ, destination.z);
        
        emit propertyChanged();
    }
}

void TeleportItem::setDestination(int x, int y, int z) {
    setDestination(MapPos(x, y, z));
}

int TeleportItem::getDestX() const {
    return destination_.x;
}

int TeleportItem::getDestY() const {
    return destination_.y;
}

int TeleportItem::getDestZ() const {
    return destination_.z;
}

Item* TeleportItem::deepCopy() const {
    TeleportItem* copy = new TeleportItem(getServerId());
    
    // Copy base class properties
    copy->setClientId(getClientId());
    copy->setName(name());
    copy->setModified(isModified());
    
    // Copy all attributes
    const auto& attrs = getAttributes();
    for (auto it = attrs.constBegin(); it != attrs.constEnd(); ++it) {
        copy->setAttribute(it.key(), it.value());
    }
    
    // Copy teleport-specific properties
    copy->setDestination(destination_);
    
    return copy;
}

QString TeleportItem::getDescription() const {
    QString baseDesc = Item::getDescription();
    if (destination_.x != 0 || destination_.y != 0 || destination_.z != 0) {
        if (!baseDesc.isEmpty()) {
            baseDesc += "\n";
        }
        baseDesc += QString("Teleport destination: %1, %2, %3")
                   .arg(destination_.x)
                   .arg(destination_.y)
                   .arg(destination_.z);
    }
    return baseDesc;
}

bool TeleportItem::unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) {
    // First call base class implementation
    if (!Item::unserializeOtbmAttributes(stream, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }
    
    // Extract teleport destination from attributes
    int x = getAttribute(AttrTeleDestX, 0).toInt();
    int y = getAttribute(AttrTeleDestY, 0).toInt();
    int z = getAttribute(AttrTeleDestZ, 0).toInt();
    
    if (x != 0 || y != 0 || z != 0) {
        destination_ = MapPos(x, y, z);
    }
    
    return true;
}

bool TeleportItem::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // Update attributes before serialization
    if (destination_.x != 0 || destination_.y != 0 || destination_.z != 0) {
        const_cast<TeleportItem*>(this)->setAttribute(AttrTeleDestX, destination_.x);
        const_cast<TeleportItem*>(this)->setAttribute(AttrTeleDestY, destination_.y);
        const_cast<TeleportItem*>(this)->setAttribute(AttrTeleDestZ, destination_.z);
    }
    
    // Call base class implementation
    return Item::serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion);
}

bool TeleportItem::isTeleportType(quint16 serverId) {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(serverId);
    return props.type == ITEM_TYPE_TELEPORT || props.group == ITEM_GROUP_TELEPORT;
}


