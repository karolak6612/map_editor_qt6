#include "ItemFactory.h"
#include "ItemManager.h"
#include "Item.h"
#include "TeleportItem.h"
#include "ContainerItem.h"
#include "DoorItem.h"
#include "DepotItem.h"
#include "PodiumItem.h"
#include <QDebug>

ItemFactory::ItemFactory(QObject* parent) : QObject(parent) {
    // Constructor implementation
}

ItemFactory::~ItemFactory() {
    // Destructor implementation
}

Item* ItemFactory::createItem(quint16 serverId, QObject* parent) {
    ItemManager* manager = ItemManager::instance();
    if (!manager->itemTypeExists(serverId)) {
        qWarning() << "Attempted to create item with unknown server ID:" << serverId;
        return nullptr;
    }

    const ItemProperties& props = manager->getItemProperties(serverId);
    
    // Create specialized item using factory pattern
    Item* newItem = createSpecializedItem(serverId, parent);
    if (!newItem) {
        return nullptr;
    }

    // Apply common properties
    applyItemProperties(newItem, props);
    
    return newItem;
}

Item* ItemFactory::createItemFromOTBM(quint16 serverId, quint16 subtype, QObject* parent) {
    // Create item using factory pattern
    Item* item = createItem(serverId, parent);
    if (!item) {
        return nullptr;
    }
    
    // Apply OTBM-specific initialization
    if (subtype != 0xFFFF) {
        ItemManager* manager = ItemManager::instance();
        const ItemProperties& props = manager->getItemProperties(serverId);
        applyOTBMSubtype(item, subtype, props);
    }
    
    return item;
}

void ItemFactory::applyItemProperties(Item* item, const ItemProperties& props) {
    if (!item) return;

    item->setClientId(props.clientId);
    item->setName(props.name);
    
    // Set new dedicated members using their setters
    item->setDescriptionText(props.description);
    item->setEditorSuffix(props.editorSuffix);
    item->setItemGroup(props.group);
    item->setItemType(props.type);
    item->setWeight(props.weight);
    item->setAttack(props.attack);
    item->setDefense(props.defense);
    item->setArmor(props.armor);
    item->setCharges(props.charges);
    item->setMaxTextLen(props.maxTextLen);
    item->setRotateTo(props.rotateTo);
    item->setVolume(props.volume);
    item->setSlotPosition(props.slotPosition);
    item->setWeaponType(props.weaponType);
    item->setLightLevel(props.lightLevel);
    item->setLightColor(props.lightColor);
    item->setClassification(props.classification);

    // Set boolean flags from ItemProperties
    item->setMoveable(props.isMoveable);
    item->setBlocking(props.isBlocking);
    item->setBlocksMissiles(props.blockMissiles);
    item->setBlocksPathfind(props.blockPathfind);
    item->setStackable(props.isStackable);
    item->setGroundTile(props.isGroundTile);
    item->setAlwaysOnTop(props.alwaysOnBottom);
    item->setTopOrder(props.topOrder);
    item->setIsTeleport(props.type == ITEM_TYPE_TELEPORT);
    item->setIsContainer(props.type == ITEM_TYPE_CONTAINER);
    item->setReadable(props.isReadable);
    item->setCanWriteText(props.canWriteText);
    item->setPickupable(props.isPickupable);
    item->setRotatable(props.isRotatable);
    item->setHangable(props.isHangable);
    item->setHasHookSouth(props.hasHookSouth);
    item->setHasHookEast(props.hasHookEast);
    item->setHasHeight(props.hasElevation);
    
    // Set attributes that are commonly instance-specific but might have type defaults
    if (props.isStackable) {
        item->setCount(1); // Default count for a newly created stackable item
    }
}

void ItemFactory::applyOTBMSubtype(Item* item, quint16 subtype, const ItemProperties& props) {
    if (!item || subtype == 0xFFFF) return;

    if (props.type == ITEM_TYPE_DOOR) {
        // For doors, subtype might indicate open/closed state
        DoorItem* door = qobject_cast<DoorItem*>(item);
        if (door) {
            // Subtype logic for doors would go here
            // This is simplified - actual logic depends on the specific implementation
        }
    } else if (props.isStackable && subtype > 0) {
        // For stackable items, subtype is the count
        item->setCount(subtype);
    } else if (props.charges > 0 && subtype > 0) {
        // For chargeable items, subtype might be remaining charges
        item->setCharges(subtype);
    }
}

Item* ItemFactory::createSpecializedItem(quint16 serverId, QObject* parent) {
    // Check for specialized types first
    if (TeleportItem::isTeleportType(serverId)) {
        return new TeleportItem(serverId, parent);
    } else if (DepotItem::isDepotType(serverId)) {
        return new DepotItem(serverId, parent);
    } else if (ContainerItem::isContainerType(serverId)) {
        return new ContainerItem(serverId, parent);
    } else if (DoorItem::isDoorType(serverId)) {
        return new DoorItem(serverId, parent);
    } else if (PodiumItem::isPodiumType(serverId)) {
        return new PodiumItem(serverId, parent);
    } else {
        // Default to base Item class
        return new Item(serverId, parent);
    }
}


