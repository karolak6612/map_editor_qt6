#include "ContainerItem.h"
#include "ItemManager.h"
#include <QDataStream>
#include <QDebug>

ContainerItem::ContainerItem(quint16 serverId, QObject *parent)
    : Item(serverId, parent) {
    // Set container-specific properties
    setIsContainer(true);
}

ContainerItem::~ContainerItem() {
    clearItems();
}

void ContainerItem::addItem(Item* item) {
    if (!item) return;
    
    if (canAddItem(item)) {
        contents_.append(item);
        item->setParent(this);
        setModified(true);
        
        emit itemAdded(item);
        emit contentsChanged();
        emit propertyChanged();
    }
}

void ContainerItem::removeItem(Item* item) {
    if (!item) return;
    
    int index = contents_.indexOf(item);
    if (index >= 0) {
        removeItem(index);
    }
}

void ContainerItem::removeItem(int index) {
    if (index >= 0 && index < contents_.size()) {
        Item* item = contents_.takeAt(index);
        item->setParent(nullptr);
        setModified(true);
        
        emit itemRemoved(item);
        emit contentsChanged();
        emit propertyChanged();
    }
}

Item* ContainerItem::getItem(int index) const {
    if (index >= 0 && index < contents_.size()) {
        return contents_.at(index);
    }
    return nullptr;
}

QList<Item*> ContainerItem::getItems() const {
    return contents_;
}

void ContainerItem::clearItems() {
    while (!contents_.isEmpty()) {
        Item* item = contents_.takeLast();
        delete item;
    }
    setModified(true);
    emit contentsChanged();
    emit propertyChanged();
}

int ContainerItem::getItemCount() const {
    return contents_.size();
}

int ContainerItem::getVolume() const {
    return contents_.size();
}

int ContainerItem::getMaxVolume() const {
    return volume();
}

double ContainerItem::getTotalWeight() const {
    double totalWeight = Item::getWeight();
    
    for (Item* item : contents_) {
        if (item) {
            totalWeight += item->getWeight();
        }
    }
    
    return totalWeight;
}

bool ContainerItem::isFull() const {
    return getVolume() >= getMaxVolume();
}

bool ContainerItem::canAddItem(Item* item) const {
    if (!item) return false;
    if (isFull()) return false;
    
    // Additional checks could be added here
    // (e.g., item type restrictions, weight limits)
    
    return true;
}

Item* ContainerItem::deepCopy() const {
    ContainerItem* copy = new ContainerItem(getServerId());
    
    // Copy base class properties
    copy->setClientId(getClientId());
    copy->setName(name());
    copy->setModified(isModified());
    
    // Copy all attributes
    const auto& attrs = getAttributes();
    for (auto it = attrs.constBegin(); it != attrs.constEnd(); ++it) {
        copy->setAttribute(it.key(), it.value());
    }
    
    // Deep copy container contents
    for (Item* item : contents_) {
        if (item) {
            Item* itemCopy = item->deepCopy();
            copy->addItem(itemCopy);
        }
    }
    
    return copy;
}

QString ContainerItem::getDescription() const {
    QString baseDesc = Item::getDescription();
    
    if (!baseDesc.isEmpty()) {
        baseDesc += "\n";
    }
    
    baseDesc += QString("Container (%1/%2 items)")
               .arg(getItemCount())
               .arg(getMaxVolume());
    
    if (!contents_.isEmpty()) {
        baseDesc += QString(", Weight: %1").arg(getTotalWeight(), 0, 'f', 2);
    }
    
    return baseDesc;
}

double ContainerItem::getWeight() const {
    return getTotalWeight();
}

bool ContainerItem::unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) {
    // First call base class implementation
    if (!Item::unserializeOtbmAttributes(stream, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }
    
    // Container-specific deserialization would be handled here
    // For now, contents are loaded separately by the map loader
    
    return true;
}

bool ContainerItem::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // Call base class implementation
    if (!Item::serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }
    
    // Container-specific serialization would be handled here
    // For now, contents are saved separately by the map saver
    
    return true;
}

bool ContainerItem::isContainerType(quint16 serverId) {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(serverId);
    return props.type == ITEM_TYPE_CONTAINER || props.group == ITEM_GROUP_CONTAINER;
}

#include "ContainerItem.moc"
