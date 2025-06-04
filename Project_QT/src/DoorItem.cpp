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

        // Update attribute for serialization (Task 24, Task 55)
        setAttribute(AttrDoorOpen, open);

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

        // Update attribute for serialization (Task 24, Task 55)
        setAttribute(AttrDoorLocked, locked);

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

        // Update attribute for serialization (Task 24, Task 55)
        setAttribute(AttrHouseDoorId, doorId);

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

    // Door-specific deserialization (Task 24, Task 55)
    // Extract door ID from attributes (matching wxwidgets OTBM_ATTR_HOUSEDOORID)
    QVariant doorIdData = getAttribute(AttrHouseDoorId);
    if (doorIdData.isValid()) {
        doorId_ = static_cast<quint16>(doorIdData.toUInt());
    }

    // Extract door state attributes
    QVariant doorOpenData = getAttribute(AttrDoorOpen);
    if (doorOpenData.isValid()) {
        isOpen_ = doorOpenData.toBool();
        setBlocking(!isOpen_); // Update blocking state
    }

    QVariant doorLockedData = getAttribute(AttrDoorLocked);
    if (doorLockedData.isValid()) {
        isLocked_ = doorLockedData.toBool();
    }

    // Extract alternate door IDs if available
    QVariant openDoorIdData = getAttribute(AttrDoorId);
    if (openDoorIdData.isValid()) {
        openDoorId_ = static_cast<quint16>(openDoorIdData.toUInt());
    }

    return true;
}

bool DoorItem::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // Door-specific serialization (Task 24, Task 55)
    // Update attributes before calling base class

    // Save door ID (matching wxwidgets OTBM_ATTR_HOUSEDOORID)
    if (doorId_ != 0 && doorId_ != getServerId()) {
        const_cast<DoorItem*>(this)->setAttribute(AttrHouseDoorId, doorId_);
    }

    // Save door state attributes
    const_cast<DoorItem*>(this)->setAttribute(AttrDoorOpen, isOpen_);
    const_cast<DoorItem*>(this)->setAttribute(AttrDoorLocked, isLocked_);

    // Save alternate door IDs if available
    if (openDoorId_ != 0) {
        const_cast<DoorItem*>(this)->setAttribute(AttrDoorId, openDoorId_);
    }

    // Call base class implementation
    if (!Item::serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }

    return true;
}

bool DoorItem::isDoorType(quint16 serverId) {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(serverId);
    return props.type == ITEM_TYPE_DOOR || props.group == ITEM_GROUP_DOOR || props.isBrushDoor;
}


