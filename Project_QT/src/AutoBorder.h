#ifndef AUTOBORDER_H
#define AUTOBORDER_H

#include <QtGlobal>
#include <QStringList>
#include <QDomElement>
#include <QString>

// Forward declarations
class GroundBrush;
class Map;
class Item;

// Direct migration from wxwidgets/brush_enums.h BorderType enum
enum class BorderType {
    // Border types (also used for carpets) - migrated from wxwidgets
    BORDER_NONE = 0,
    NORTH_HORIZONTAL = 1,
    EAST_HORIZONTAL = 2,
    SOUTH_HORIZONTAL = 3,
    WEST_HORIZONTAL = 4,
    NORTHWEST_CORNER = 5,
    NORTHEAST_CORNER = 6,
    SOUTHWEST_CORNER = 7,
    SOUTHEAST_CORNER = 8,
    NORTHWEST_DIAGONAL = 9,
    NORTHEAST_DIAGONAL = 10,
    SOUTHEAST_DIAGONAL = 11,
    SOUTHWEST_DIAGONAL = 12,
    // Additional carpet type
    CARPET_CENTER = 13
};

/**
 * AutoBorder class - Direct 1:1 migration from wxwidgets AutoBorder
 * 
 * From wxwidgets/brush.h:
 * - Used by GroundBrush, should be transparent to users
 * - Contains tiles[13] array for border item IDs
 * - Has id, group, and ground properties
 * - Provides edgeNameToID static method for XML loading
 * - Provides load method for XML parsing
 * - Provides hasItemId and getItemId methods for border item lookup
 */
class AutoBorder {
public:
    // Direct migration from wxwidgets constructor
    explicit AutoBorder(quint32 id);
    ~AutoBorder();

    // Static method migrated from wxwidgets AutoBorder::edgeNameToID
    static BorderType edgeNameToID(const QString& edgename);
    
    // XML loading method migrated from wxwidgets AutoBorder::load
    bool load(const QDomElement& element, QStringList& warnings, GroundBrush* owner = nullptr, quint16 groundEquivalent = 0);

    // Check if this border contains the specified item ID (migrated from wxwidgets)
    bool hasItemId(quint16 itemId) const;

    // Get the item ID for a specific border alignment (migrated from wxwidgets)
    quint16 getItemId(quint32 alignment) const;
    
    // Get the item ID for a specific border type
    quint16 getItemId(BorderType borderType) const;
    
    // Set item ID for a specific border type
    void setItemId(BorderType borderType, quint16 itemId);
    
    // Clear all border item IDs
    void clearItemIds();

    // Getters for properties (migrated from wxwidgets public members)
    quint32 getId() const { return id; }
    quint16 getGroup() const { return group; }
    bool isGround() const { return ground; }
    
    // Setters for properties
    void setId(quint32 newId) { id = newId; }
    void setGroup(quint16 newGroup) { group = newGroup; }
    void setGround(bool isGround) { ground = isGround; }
    
    // Get border items to place (placeholder for future implementation)
    QList<Item*> getBorderItemsToPlace(Map* map) const;

private:
    // Direct migration from wxwidgets AutoBorder members
    quint32 tiles[13];  // Array of border item IDs for each border type (0-12)
    quint32 id;         // Border definition ID
    quint16 group;      // Border group ID
    bool ground;        // Whether this border defines ground equivalent
    
    // Helper method to convert BorderType to array index
    int borderTypeToIndex(BorderType borderType) const;
    
    // Helper method to validate array index
    bool isValidIndex(int index) const;
};

#endif // AUTOBORDER_H
