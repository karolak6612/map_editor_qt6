#include "DepotItem.h"
#include "ItemManager.h"
#include <QDataStream>
#include <QDebug>

DepotItem::DepotItem(quint16 serverId, QObject *parent)
    : ContainerItem(serverId, parent), depotId_(0) {
    // Depot-specific initialization
}

DepotItem::~DepotItem() {
    // No special cleanup needed
}

quint32 DepotItem::getDepotId() const {
    return depotId_;
}

void DepotItem::setDepotId(quint32 depotId) {
    if (depotId_ != depotId) {
        depotId_ = depotId;
        setModified(true);
        
        // Update attribute for serialization
        setAttribute(AttrDepotID, depotId);
        
        emit propertyChanged();
    }
}

Item* DepotItem::deepCopy() const {
    DepotItem* copy = new DepotItem(getServerId());
    
    // Copy base class properties (ContainerItem handles container contents)
    ContainerItem* containerCopy = static_cast<ContainerItem*>(copy);
    ContainerItem* baseCopy = static_cast<ContainerItem*>(ContainerItem::deepCopy());
    
    // Copy container contents
    for (Item* item : getItems()) {
        if (item) {
            copy->addItem(item->deepCopy());
        }
    }
    
    // Copy depot-specific properties
    copy->setDepotId(depotId_);
    
    delete baseCopy; // Clean up temporary copy
    return copy;
}

QString DepotItem::getDescription() const {
    QString baseDesc = ContainerItem::getDescription();
    
    if (depotId_ != 0) {
        if (!baseDesc.isEmpty()) {
            baseDesc += "\n";
        }
        baseDesc += QString("Depot ID: %1").arg(depotId_);
    }
    
    return baseDesc;
}

bool DepotItem::unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) {
    // First call base class implementation
    if (!ContainerItem::unserializeOtbmAttributes(stream, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }
    
    // Extract depot ID from attributes
    depotId_ = getAttribute(AttrDepotID, 0).toUInt();
    
    return true;
}

bool DepotItem::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // Update attributes before serialization
    if (depotId_ != 0) {
        const_cast<DepotItem*>(this)->setAttribute(AttrDepotID, depotId_);
    }
    
    // Call base class implementation
    return ContainerItem::serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion);
}

bool DepotItem::isDepotType(quint16 serverId) {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(serverId);
    return props.type == ITEM_TYPE_DEPOT;
}

#include "DepotItem.moc"
