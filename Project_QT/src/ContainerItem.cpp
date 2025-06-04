#include "ContainerItem.h"
#include "ItemManager.h"
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

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

    // Container-specific deserialization (Task 24, Task 55)
    // Extract container contents from attributes if available
    QVariant contentsData = getAttribute(AttrContainerContents);
    if (contentsData.isValid() && !contentsData.toString().isEmpty()) {
        // Parse container contents from JSON format
        if (!deserializeContainerContents(contentsData.toString())) {
            qWarning() << "ContainerItem::unserializeOtbmAttributes - Failed to deserialize container contents";
        }
    }

    // Extract container capacity if available
    QVariant capacityData = getAttribute(AttrContainerCapacity);
    if (capacityData.isValid()) {
        // Container capacity is typically handled by ItemProperties, but can be overridden
        qDebug() << "ContainerItem: Custom capacity found:" << capacityData.toInt();
    }

    return true;
}

bool ContainerItem::serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const {
    // Container-specific serialization (Task 24, Task 55)
    // Serialize container contents to attributes before calling base class
    if (!contents_.isEmpty()) {
        QString contentsJson = serializeContainerContents();
        if (!contentsJson.isEmpty()) {
            const_cast<ContainerItem*>(this)->setAttribute(AttrContainerContents, contentsJson);
        }
    }

    // Serialize container capacity if different from default
    int defaultCapacity = getMaxVolume();
    if (defaultCapacity > 0) {
        const_cast<ContainerItem*>(this)->setAttribute(AttrContainerCapacity, defaultCapacity);
    }

    // Call base class implementation
    if (!Item::serializeOtbmAttributes(stream, mapOtbmFormatVersion, otbItemsMajorVersion, otbItemsMinorVersion)) {
        return false;
    }

    return true;
}

bool ContainerItem::isContainerType(quint16 serverId) {
    const ItemProperties& props = ItemManager::instance()->getItemProperties(serverId);
    return props.type == ITEM_TYPE_CONTAINER || props.group == ITEM_GROUP_CONTAINER;
}

// Private helper methods for container contents serialization (Task 24, Task 55)
QString ContainerItem::serializeContainerContents() const {
    if (contents_.isEmpty()) {
        return QString();
    }

    QJsonArray contentsArray;
    for (const Item* item : contents_) {
        if (item) {
            QJsonObject itemObj;
            itemObj["serverId"] = static_cast<int>(item->getServerId());
            itemObj["count"] = static_cast<int>(item->getCount());
            // Store slot position instead of X/Y/Z coordinates
            itemObj["slotPosition"] = static_cast<int>(item->slotPosition());

            // Include basic attributes that are commonly needed
            if (item->hasAttribute(AttrText)) {
                itemObj["text"] = item->getAttribute(AttrText).toString();
            }
            if (item->hasAttribute(AttrActionID)) {
                itemObj["actionId"] = item->getAttribute(AttrActionID).toInt();
            }
            if (item->hasAttribute(AttrUniqueID)) {
                itemObj["uniqueId"] = item->getAttribute(AttrUniqueID).toInt();
            }

            contentsArray.append(itemObj);
        }
    }

    QJsonDocument doc(contentsArray);
    return doc.toJson(QJsonDocument::Compact);
}

bool ContainerItem::deserializeContainerContents(const QString& contentsJson) {
    if (contentsJson.isEmpty()) {
        return true; // Empty contents is valid
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(contentsJson.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "ContainerItem::deserializeContainerContents - JSON parse error:" << error.errorString();
        return false;
    }

    if (!doc.isArray()) {
        qWarning() << "ContainerItem::deserializeContainerContents - Expected JSON array";
        return false;
    }

    // Clear existing contents
    clearItems();

    QJsonArray contentsArray = doc.array();
    for (const QJsonValue& value : contentsArray) {
        if (!value.isObject()) {
            continue;
        }

        QJsonObject itemObj = value.toObject();
        quint16 serverId = static_cast<quint16>(itemObj["serverId"].toInt());
        if (serverId == 0) {
            continue;
        }

        // Create new item
        Item* item = new Item(serverId, this);
        item->setCount(static_cast<quint16>(itemObj["count"].toInt(1)));
        // Restore slot position instead of X/Y/Z coordinates
        if (itemObj.contains("slotPosition")) {
            item->setSlotPosition(static_cast<quint32>(itemObj["slotPosition"].toInt(0)));
        }

        // Restore basic attributes
        if (itemObj.contains("text")) {
            item->setAttribute(AttrText, itemObj["text"].toString());
        }
        if (itemObj.contains("actionId")) {
            item->setAttribute(AttrActionID, itemObj["actionId"].toInt());
        }
        if (itemObj.contains("uniqueId")) {
            item->setAttribute(AttrUniqueID, itemObj["uniqueId"].toInt());
        }

        // Add to container (this will set parent and emit signals)
        addItem(item);
    }

    return true;
}


