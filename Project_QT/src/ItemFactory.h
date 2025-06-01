#ifndef ITEMFACTORY_H
#define ITEMFACTORY_H

#include <QObject>

// Forward declarations
class Item;
class ItemManager;
struct ItemProperties;

/**
 * @brief Factory class for creating specialized Item instances
 * 
 * This class handles the creation of specialized item types based on
 * item properties and server IDs. It implements the factory pattern
 * to ensure the correct item subclass is instantiated.
 */
class ItemFactory : public QObject {
    Q_OBJECT

public:
    explicit ItemFactory(QObject* parent = nullptr);
    ~ItemFactory();

    /**
     * @brief Create an item instance using factory pattern
     * @param serverId The server ID of the item to create
     * @param parent Qt parent object for memory management
     * @return Pointer to created Item (or specialized subclass), nullptr on failure
     */
    static Item* createItem(quint16 serverId, QObject* parent = nullptr);

    /**
     * @brief Create an item from OTBM data with subtype handling
     * @param serverId The server ID of the item
     * @param subtype Additional type information (count, charges, state, etc.)
     * @param parent Qt parent object for memory management
     * @return Pointer to created Item with OTBM-specific initialization
     */
    static Item* createItemFromOTBM(quint16 serverId, quint16 subtype = 0xFFFF, QObject* parent = nullptr);

    /**
     * @brief Apply common properties to any item instance
     * @param item The item to configure
     * @param props The properties to apply
     */
    static void applyItemProperties(Item* item, const ItemProperties& props);

    /**
     * @brief Apply OTBM-specific initialization to an item
     * @param item The item to configure
     * @param subtype The subtype value from OTBM data
     * @param props The item properties
     */
    static void applyOTBMSubtype(Item* item, quint16 subtype, const ItemProperties& props);

private:
    /**
     * @brief Determine the appropriate item class for a server ID
     * @param serverId The server ID to check
     * @return Pointer to new specialized item instance
     */
    static Item* createSpecializedItem(quint16 serverId, QObject* parent);
};

#endif // ITEMFACTORY_H
