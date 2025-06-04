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

    // Update attributes for serialization (Task 24, Task 55)
    setAttribute(AttrPodiumOutfitLookType, outfit.lookType);
    setAttribute(AttrPodiumOutfitHead, outfit.lookHead);
    setAttribute(AttrPodiumOutfitBody, outfit.lookBody);
    setAttribute(AttrPodiumOutfitLegs, outfit.lookLegs);
    setAttribute(AttrPodiumOutfitFeet, outfit.lookFeet);
    setAttribute(AttrPodiumOutfitAddon, outfit.lookAddon);

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

        // Update attribute for serialization (Task 24, Task 55)
        setAttribute(AttrPodiumShowOutfit, visible);

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

    // Podium-specific deserialization (Task 24, Task 55)
    // Extract outfit data from attributes (matching wxwidgets OTBM_ATTR_PODIUMOUTFIT)
    QVariant outfitData = getAttribute(AttrPodiumOutfit);
    if (outfitData.isValid() && !outfitData.toByteArray().isEmpty()) {
        if (!deserializePodiumOutfit(outfitData.toByteArray())) {
            qWarning() << "PodiumItem::unserializeOtbmAttributes - Failed to deserialize podium outfit";
        }
    }

    // Extract individual outfit attributes if available (alternative format)
    QVariant lookTypeData = getAttribute(AttrPodiumOutfitLookType);
    if (lookTypeData.isValid()) {
        outfit_.lookType = lookTypeData.toInt();
        hasOutfit_ = true;

        // Extract color attributes
        outfit_.lookHead = getAttribute(AttrPodiumOutfitHead, 0).toInt();
        outfit_.lookBody = getAttribute(AttrPodiumOutfitBody, 0).toInt();
        outfit_.lookLegs = getAttribute(AttrPodiumOutfitLegs, 0).toInt();
        outfit_.lookFeet = getAttribute(AttrPodiumOutfitFeet, 0).toInt();
        outfit_.lookAddon = getAttribute(AttrPodiumOutfitAddon, 0).toInt();
    }

    // Extract visibility state
    QVariant showOutfitData = getAttribute(AttrPodiumShowOutfit);
    if (showOutfitData.isValid()) {
        isVisible_ = showOutfitData.toBool();
    }

    return true;
}

bool PodiumItem::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // Podium-specific serialization (Task 24, Task 55)
    // Update attributes before calling base class

    if (hasOutfit_) {
        // Serialize outfit data in wxwidgets-compatible format
        QByteArray outfitData = serializePodiumOutfit();
        if (!outfitData.isEmpty()) {
            const_cast<PodiumItem*>(this)->setAttribute(AttrPodiumOutfit, outfitData);
        }

        // Also save individual attributes for compatibility
        const_cast<PodiumItem*>(this)->setAttribute(AttrPodiumOutfitLookType, outfit_.lookType);
        const_cast<PodiumItem*>(this)->setAttribute(AttrPodiumOutfitHead, outfit_.lookHead);
        const_cast<PodiumItem*>(this)->setAttribute(AttrPodiumOutfitBody, outfit_.lookBody);
        const_cast<PodiumItem*>(this)->setAttribute(AttrPodiumOutfitLegs, outfit_.lookLegs);
        const_cast<PodiumItem*>(this)->setAttribute(AttrPodiumOutfitFeet, outfit_.lookFeet);
        const_cast<PodiumItem*>(this)->setAttribute(AttrPodiumOutfitAddon, outfit_.lookAddon);
    }

    // Save visibility state
    const_cast<PodiumItem*>(this)->setAttribute(AttrPodiumShowOutfit, isVisible_);

    // Call base class implementation
    if (!Item::serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }

    return true;
}

bool PodiumItem::isPodiumType(quint16 serverId) {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(serverId);
    return props.type == ITEM_TYPE_PODIUM || props.group == ITEM_GROUP_PODIUM;
}

// Private helper methods for podium outfit serialization (Task 24, Task 55)
QByteArray PodiumItem::serializePodiumOutfit() const {
    if (!hasOutfit_) {
        return QByteArray();
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Serialize outfit data matching wxwidgets format
    // Based on wxwidgets serializeItemAttributes_OTBM implementation
    quint8 flags = 0;
    if (isVisible_) flags |= 0x01; // PODIUM_SHOW_OUTFIT
    // Additional flags could be added for mount, platform, etc.

    stream << flags;                           // quint8 flags
    stream << static_cast<quint8>(0);         // quint8 direction (default 0)
    stream << static_cast<quint16>(outfit_.lookType);  // quint16 lookType
    stream << static_cast<quint8>(outfit_.lookHead);   // quint8 lookHead
    stream << static_cast<quint8>(outfit_.lookBody);   // quint8 lookBody
    stream << static_cast<quint8>(outfit_.lookLegs);   // quint8 lookLegs
    stream << static_cast<quint8>(outfit_.lookFeet);   // quint8 lookFeet
    stream << static_cast<quint8>(outfit_.lookAddon);  // quint8 lookAddon

    // Mount data (if available)
    stream << static_cast<quint16>(outfit_.lookMount);     // quint16 lookMount
    stream << static_cast<quint8>(outfit_.lookMountHead);  // quint8 lookMountHead
    stream << static_cast<quint8>(outfit_.lookMountBody);  // quint8 lookMountBody
    stream << static_cast<quint8>(outfit_.lookMountLegs);  // quint8 lookMountLegs
    stream << static_cast<quint8>(outfit_.lookMountFeet);  // quint8 lookMountFeet

    return data;
}

bool PodiumItem::deserializePodiumOutfit(const QByteArray& data) {
    if (data.isEmpty()) {
        return true; // Empty data is valid
    }

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Deserialize outfit data matching wxwidgets format
    quint8 flags;
    quint8 direction;
    quint16 lookType;
    quint8 lookHead, lookBody, lookLegs, lookFeet, lookAddon;
    quint16 lookMount;
    quint8 lookMountHead, lookMountBody, lookMountLegs, lookMountFeet;

    stream >> flags >> direction >> lookType >> lookHead >> lookBody >> lookLegs >> lookFeet >> lookAddon;

    // Check if we have mount data
    if (!stream.atEnd()) {
        stream >> lookMount >> lookMountHead >> lookMountBody >> lookMountLegs >> lookMountFeet;
    } else {
        lookMount = 0;
        lookMountHead = lookMountBody = lookMountLegs = lookMountFeet = 0;
    }

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "PodiumItem::deserializePodiumOutfit - Stream error during deserialization";
        return false;
    }

    // Update outfit data
    outfit_.lookType = static_cast<int>(lookType);
    outfit_.lookHead = static_cast<int>(lookHead);
    outfit_.lookBody = static_cast<int>(lookBody);
    outfit_.lookLegs = static_cast<int>(lookLegs);
    outfit_.lookFeet = static_cast<int>(lookFeet);
    outfit_.lookAddon = static_cast<int>(lookAddon);
    outfit_.lookMount = static_cast<int>(lookMount);
    outfit_.lookMountHead = static_cast<int>(lookMountHead);
    outfit_.lookMountBody = static_cast<int>(lookMountBody);
    outfit_.lookMountLegs = static_cast<int>(lookMountLegs);
    outfit_.lookMountFeet = static_cast<int>(lookMountFeet);

    hasOutfit_ = (lookType != 0);
    isVisible_ = (flags & 0x01) != 0; // PODIUM_SHOW_OUTFIT flag

    return true;
}


