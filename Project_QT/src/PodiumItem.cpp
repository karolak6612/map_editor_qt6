#include "PodiumItem.h"
#include "ItemManager.h"
#include <QDataStream>
#include <QDebug>

PodiumItem::PodiumItem(quint16 serverId, QObject *parent)
    : Item(serverId, parent), hasOutfit_(false), isVisible_(true) {
    // Podium-specific initialization
}

PodiumItem::~PodiumItem() {
    // No special cleanup needed
}

bool PodiumItem::hasOutfit() const {
    return hasOutfit_;
}

Outfit PodiumItem::getOutfit() const {
    return outfit_;
}

void PodiumItem::setOutfit(const Outfit& outfit) {
    outfit_ = outfit;
    hasOutfit_ = true;
    setModified(true);
    
    emit outfitChanged(outfit);
    emit propertyChanged();
}

void PodiumItem::clearOutfit() {
    if (hasOutfit_) {
        outfit_ = Outfit();
        hasOutfit_ = false;
        setModified(true);
        
        emit outfitChanged(outfit_);
        emit propertyChanged();
    }
}

bool PodiumItem::isVisible() const {
    return isVisible_;
}

void PodiumItem::setVisible(bool visible) {
    if (isVisible_ != visible) {
        isVisible_ = visible;
        setModified(true);
        emit propertyChanged();
    }
}

Item* PodiumItem::deepCopy() const {
    PodiumItem* copy = new PodiumItem(getServerId());
    
    // Copy base class properties
    copy->setClientId(getClientId());
    copy->setName(name());
    copy->setModified(isModified());
    
    // Copy all attributes
    const auto& attrs = getAttributes();
    for (auto it = attrs.constBegin(); it != attrs.constEnd(); ++it) {
        copy->setAttribute(it.key(), it.value());
    }
    
    // Copy podium-specific properties
    if (hasOutfit_) {
        copy->setOutfit(outfit_);
    }
    copy->setVisible(isVisible_);
    
    return copy;
}

QString PodiumItem::getDescription() const {
    QString baseDesc = Item::getDescription();
    
    if (!baseDesc.isEmpty()) {
        baseDesc += "\n";
    }
    
    if (hasOutfit_) {
        baseDesc += QString("Podium displaying outfit: %1").arg(outfit_.lookType);
    } else {
        baseDesc += "Empty podium";
    }
    
    if (!isVisible_) {
        baseDesc += " (Hidden)";
    }
    
    return baseDesc;
}

bool PodiumItem::unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) {
    // First call base class implementation
    if (!Item::unserializeOtbmAttributes(stream, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }
    
    // Podium-specific deserialization
    // Outfit data would be loaded from attributes here
    // This is a simplified implementation
    
    return true;
}

bool PodiumItem::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // Call base class implementation
    if (!Item::serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }
    
    // Podium-specific serialization
    // Outfit data would be saved to attributes here
    // This is a simplified implementation
    
    return true;
}

bool PodiumItem::isPodiumType(quint16 serverId) {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(serverId);
    return props.type == ITEM_TYPE_PODIUM || props.group == ITEM_GROUP_PODIUM;
}

#include "PodiumItem.moc"
