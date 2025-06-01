#include "AutoBorder.h"
#include "GroundBrush.h"
#include "Map.h"
#include "Item.h"
#include <QDebug>
#include <QDomElement>

// Direct migration from wxwidgets AutoBorder constructor
AutoBorder::AutoBorder(quint32 _id)
    : id(_id), group(0), ground(false) {
    // Initialize tiles array to 0 (migrated from wxwidgets)
    for (int i = 0; i < 13; i++) {
        tiles[i] = 0;
    }
}

AutoBorder::~AutoBorder() {
    // No cleanup needed for simple types
}

// Direct migration from wxwidgets AutoBorder::edgeNameToID
BorderType AutoBorder::edgeNameToID(const QString& edgename) {
    if (edgename == "n") {
        return BorderType::NORTH_HORIZONTAL;
    } else if (edgename == "w") {
        return BorderType::WEST_HORIZONTAL;
    } else if (edgename == "s") {
        return BorderType::SOUTH_HORIZONTAL;
    } else if (edgename == "e") {
        return BorderType::EAST_HORIZONTAL;
    } else if (edgename == "cnw") {
        return BorderType::NORTHWEST_CORNER;
    } else if (edgename == "cne") {
        return BorderType::NORTHEAST_CORNER;
    } else if (edgename == "csw") {
        return BorderType::SOUTHWEST_CORNER;
    } else if (edgename == "cse") {
        return BorderType::SOUTHEAST_CORNER;
    } else if (edgename == "dnw") {
        return BorderType::NORTHWEST_DIAGONAL;
    } else if (edgename == "dne") {
        return BorderType::NORTHEAST_DIAGONAL;
    } else if (edgename == "dsw") {
        return BorderType::SOUTHWEST_DIAGONAL;
    } else if (edgename == "dse") {
        return BorderType::SOUTHEAST_DIAGONAL;
    }
    return BorderType::BORDER_NONE;
}

// XML loading method - migrated structure from wxwidgets AutoBorder::load
bool AutoBorder::load(const QDomElement& element, QStringList& warnings, GroundBrush* owner, quint16 groundEquivalent) {
    Q_UNUSED(owner);        // May be used in future for validation
    Q_UNUSED(groundEquivalent); // May be used in future for ground type validation

    // Load border ID
    bool ok;
    quint32 borderId = element.attribute("id", "0").toUInt(&ok);
    if (ok) {
        id = borderId;
    } else {
        warnings.append(QString("Invalid border id: %1").arg(element.attribute("id")));
        return false;
    }

    // Load group (optional)
    quint16 borderGroup = element.attribute("group", "0").toUShort(&ok);
    if (ok) {
        group = borderGroup;
    }

    // Load ground flag (optional)
    ground = element.attribute("ground", "false") == "true";

    // Load border items from <borderitem edge="..." item="..."/> elements (correct wxwidgets format)
    QDomElement borderItem = element.firstChildElement("borderitem");
    while (!borderItem.isNull()) {
        QString edgeName = borderItem.attribute("edge");
        QString itemIdStr = borderItem.attribute("item");

        if (edgeName.isEmpty()) {
            warnings.append("Missing edge attribute in borderitem");
            borderItem = borderItem.nextSiblingElement("borderitem");
            continue;
        }

        if (itemIdStr.isEmpty()) {
            warnings.append(QString("Missing item attribute in borderitem for edge %1").arg(edgeName));
            borderItem = borderItem.nextSiblingElement("borderitem");
            continue;
        }

        // Convert edge name to border type
        BorderType borderType = edgeNameToID(edgeName);
        if (borderType != BorderType::BORDER_NONE) {
            quint16 itemId = itemIdStr.toUShort(&ok);
            if (ok && itemId > 0) {
                setItemId(borderType, itemId);
                qDebug() << "AutoBorder::load: Set edge" << edgeName << "to item ID" << itemId;
            } else {
                warnings.append(QString("Invalid item ID for border edge %1: %2").arg(edgeName).arg(itemIdStr));
            }
        } else {
            warnings.append(QString("Unknown border edge name: %1").arg(edgeName));
        }

        borderItem = borderItem.nextSiblingElement("borderitem");
    }

    qDebug() << "AutoBorder::load: Loaded border ID" << id << "group" << group << "ground" << ground;
    return true;
}

// Direct migration from wxwidgets AutoBorder::hasItemId
bool AutoBorder::hasItemId(quint16 itemId) const {
    for (int i = 0; i < 13; ++i) {
        if (tiles[i] == itemId) {
            return true;
        }
    }
    return false;
}

// Direct migration from wxwidgets AutoBorder::getItemId
quint16 AutoBorder::getItemId(quint32 alignment) const {
    // For now, just return the first matching tile
    if (alignment < 13 && tiles[alignment] != 0) {
        return tiles[alignment];
    }
    
    // If no direct match, return the first non-zero tile
    for (int i = 0; i < 13; ++i) {
        if (tiles[i] != 0) {
            return tiles[i];
        }
    }
    
    return 0; // No valid item ID found
}

// Get item ID for specific border type
quint16 AutoBorder::getItemId(BorderType borderType) const {
    int index = borderTypeToIndex(borderType);
    if (isValidIndex(index)) {
        return tiles[index];
    }
    return 0;
}

// Set item ID for specific border type
void AutoBorder::setItemId(BorderType borderType, quint16 itemId) {
    int index = borderTypeToIndex(borderType);
    if (isValidIndex(index)) {
        tiles[index] = itemId;
    }
}

// Clear all border item IDs
void AutoBorder::clearItemIds() {
    for (int i = 0; i < 13; i++) {
        tiles[i] = 0;
    }
}

// Get border items to place (placeholder implementation)
QList<Item*> AutoBorder::getBorderItemsToPlace(Map* map) const {
    Q_UNUSED(map);
    
    // Placeholder implementation - would create actual Item objects
    // This would be implemented when Item creation and Map integration is complete
    QList<Item*> items;
    
    qDebug() << "AutoBorder::getBorderItemsToPlace: Placeholder implementation";
    return items;
}

// Helper method to convert BorderType to array index
int AutoBorder::borderTypeToIndex(BorderType borderType) const {
    return static_cast<int>(borderType);
}

// Helper method to validate array index
bool AutoBorder::isValidIndex(int index) const {
    return index >= 0 && index < 13;
}
