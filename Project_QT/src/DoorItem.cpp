#include "DoorItem.h"
#include "ItemManager.h"
#include <QDataStream>
#include <QDebug>

DoorItem::DoorItem(quint16 serverId, QObject *parent)
    : Item(serverId, parent), isOpen_(false), isLocked_(false), 
      doorId_(serverId), openDoorId_(0), closedDoorId_(0) {
    // Set door-specific properties
    setBlocking(true); // Doors are typically blocking
}

DoorItem::~DoorItem() {
    // No special cleanup needed
}

bool DoorItem::isOpen() const {
    return isOpen_;
}

void DoorItem::setOpen(bool open) {
    if (isOpen_ != open) {
        isOpen_ = open;
        setModified(true);
        
        // Update blocking state based on door state
        setBlocking(!open);
        
        emit doorStateChanged(open);
        emit propertyChanged();
    }
}

bool DoorItem::isLocked() const {
    return isLocked_;
}

void DoorItem::setLocked(bool locked) {
    if (isLocked_ != locked) {
        isLocked_ = locked;
        setModified(true);
        emit propertyChanged();
    }
}

quint16 DoorItem::getDoorId() const {
    return doorId_;
}

void DoorItem::setDoorId(quint16 doorId) {
    if (doorId_ != doorId) {
        doorId_ = doorId;
        setModified(true);
        emit propertyChanged();
    }
}

quint16 DoorItem::getOpenDoorId() const {
    return openDoorId_;
}

void DoorItem::setOpenDoorId(quint16 openDoorId) {
    if (openDoorId_ != openDoorId) {
        openDoorId_ = openDoorId;
        setModified(true);
        emit propertyChanged();
    }
}

quint16 DoorItem::getClosedDoorId() const {
    return closedDoorId_;
}

void DoorItem::setClosedDoorId(quint16 closedDoorId) {
    if (closedDoorId_ != closedDoorId) {
        closedDoorId_ = closedDoorId;
        setModified(true);
        emit propertyChanged();
    }
}

void DoorItem::toggle() {
    setOpen(!isOpen_);
}

quint16 DoorItem::getAlternateDoorId() const {
    if (isOpen_) {
        return closedDoorId_ != 0 ? closedDoorId_ : doorId_;
    } else {
        return openDoorId_ != 0 ? openDoorId_ : doorId_;
    }
}

Item* DoorItem::deepCopy() const {
    DoorItem* copy = new DoorItem(getServerId());
    
    // Copy base class properties
    copy->setClientId(getClientId());
    copy->setName(name());
    copy->setModified(isModified());
    
    // Copy all attributes
    const auto& attrs = getAttributes();
    for (auto it = attrs.constBegin(); it != attrs.constEnd(); ++it) {
        copy->setAttribute(it.key(), it.value());
    }
    
    // Copy door-specific properties
    copy->setOpen(isOpen_);
    copy->setLocked(isLocked_);
    copy->setDoorId(doorId_);
    copy->setOpenDoorId(openDoorId_);
    copy->setClosedDoorId(closedDoorId_);
    
    return copy;
}

QString DoorItem::getDescription() const {
    QString baseDesc = Item::getDescription();
    
    if (!baseDesc.isEmpty()) {
        baseDesc += "\n";
    }
    
    QString stateDesc = isOpen_ ? "Open" : "Closed";
    if (isLocked_) {
        stateDesc += " (Locked)";
    }
    
    baseDesc += QString("Door: %1").arg(stateDesc);
    
    if (doorId_ != getServerId()) {
        baseDesc += QString(" [ID: %1]").arg(doorId_);
    }
    
    return baseDesc;
}

bool DoorItem::unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) {
    // First call base class implementation
    if (!Item::unserializeOtbmAttributes(stream, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }
    
    // Door-specific deserialization
    // Door state is typically determined by the item ID itself
    // Additional door properties could be loaded from attributes here
    
    return true;
}

bool DoorItem::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // Call base class implementation
    if (!Item::serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }
    
    // Door-specific serialization
    // Door state is typically saved as the item ID itself
    // Additional door properties could be saved to attributes here
    
    return true;
}

bool DoorItem::isDoorType(quint16 serverId) {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(serverId);
    return props.type == ITEM_TYPE_DOOR || props.group == ITEM_GROUP_DOOR || props.isBrushDoor;
}

#include "DoorItem.moc"
