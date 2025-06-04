#include "GroundBrush.h"
#include "Map.h"                    // For Map* type hint, though not directly used in stubs
#include "SetGroundItemCommand.h"   // For SetGroundItemCommand class
#include "MapView.h"                // For MapView* parameter in mouse events
#include <QUndoCommand>             // For QUndoCommand* return type
#include <QUndoStack>               // For QUndoStack* parameter in mouse events
#include <QDebug>
#include <QMouseEvent>              // For event->buttons() and QMouseEvent type
#include <QObject>                  // For tr()

// Constructor
GroundBrush::GroundBrush(QObject *parent)
    : TerrainBrush(parent),
      currentGroundItemId_(0),
      hasOptionalBorder_(false),
      useSoloOptionalBorder_(false),
      isReRandomizable_(false) {
    setSpecificName(tr("Ground Brush")); // Set the specific name for this brush type
    // setLookID(SOME_DEFAULT_GROUND_ITEM_ID); // Optionally set a default lookId_
}

// Constructor with ground item ID (Task 38 requirement)
GroundBrush::GroundBrush(quint16 groundItemId, QObject *parent)
    : TerrainBrush(parent),
      currentGroundItemId_(groundItemId),
      hasOptionalBorder_(false),
      useSoloOptionalBorder_(false),
      isReRandomizable_(false) {
    setSpecificName(QString("Ground %1").arg(groundItemId));
    // Task 016: Don't call setLookID() - currentGroundItemId_ is now the primary storage
    qDebug() << "GroundBrush: Created with ground item ID" << groundItemId << "name:" << specificName();
}

// Destructor
GroundBrush::~GroundBrush() {
    // Perform any specific cleanup for GroundBrush if needed
}

// Type identification
Brush::Type GroundBrush::type() const {
    return Brush::Type::Ground;
}

// Core action method stubs (enhanced for Task 38)
bool GroundBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext);

    if (!map) {
        qWarning() << "GroundBrush::canDraw: Map pointer is null.";
        return false;
    }

    quint16 groundItemId = getCurrentGroundItemId();
    if (groundItemId == 0) {
        qWarning() << "GroundBrush::canDraw: No ground item ID set.";
        return false;
    }

    // Check ground equivalent restrictions (Task 38 requirement)
    if (!canPlaceGroundAt(map, tilePos, groundItemId)) {
        return false;
    }

    return true;
}

QUndoCommand* GroundBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext); // Not used in this basic implementation

    if (!map) {
        qWarning() << "GroundBrush::applyBrush: Map pointer is null.";
        return nullptr;
    }

    quint16 groundItemIdToPlace = getCurrentGroundItemId();

    if (groundItemIdToPlace == 0) {
        qWarning() << "GroundBrush::applyBrush: currentGroundItemId_ is 0. Brush may not be configured to place a specific ground. No action taken.";
        return nullptr;
    }

    // Check if we can place the ground at this position (Task 38 requirement)
    if (!canPlaceGroundAt(map, tilePos, groundItemIdToPlace)) {
        qDebug() << "GroundBrush::applyBrush: Cannot place ground ID" << groundItemIdToPlace << "at" << tilePos << "due to ground equivalent restrictions.";
        return nullptr;
    }

    qDebug() << "GroundBrush::applyBrush: Attempting to place ground ID" << groundItemIdToPlace << "at" << tilePos;

    // Request border update for this position and neighbors (Task 38 requirement - placeholder for Task 52)
    requestBorderUpdate(map, tilePos);

    return new SetGroundItemCommand(map, tilePos, groundItemIdToPlace, parentCommand);
}

QUndoCommand* GroundBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!map) {
        qWarning() << "GroundBrush::removeBrush: Map pointer is null.";
        return nullptr;
    }

    qDebug() << "GroundBrush::removeBrush: Attempting to remove ground at" << tilePos;

    // Request border update for this position and neighbors (Task 38 requirement - placeholder for Task 52)
    requestBorderUpdate(map, tilePos);

    return new SetGroundItemCommand(map, tilePos, 0, parentCommand); // 0 signifies ground removal
}

// Brush geometry methods
int GroundBrush::getBrushSize() const {
    return 0;
}

Brush::BrushShape GroundBrush::getBrushShape() const {
    return Brush::BrushShape::Square;
}

// --- Methods for managing the ground item ID ---
void GroundBrush::setCurrentGroundItemId(quint16 itemId) {
    currentGroundItemId_ = itemId;
    qDebug() << "GroundBrush: Set currentGroundItemId to" << currentGroundItemId_;
}

quint16 GroundBrush::getCurrentGroundItemId() const {
    return currentGroundItemId_;
}

// Task 016: Override getLookID to use currentGroundItemId_ as primary storage
int GroundBrush::getLookID() const {
    return static_cast<int>(currentGroundItemId_);
}

// Optional border support (Task 38 requirement)
bool GroundBrush::hasOptionalBorder() const {
    return hasOptionalBorder_;
}

bool GroundBrush::useSoloOptionalBorder() const {
    return useSoloOptionalBorder_;
}

bool GroundBrush::isReRandomizable() const {
    return isReRandomizable_;
}

// Cancel operation
void GroundBrush::cancel() {
    qDebug() << "GroundBrush::cancel called";
}

// Mouse event handlers
QUndoCommand* GroundBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                         Map* map, QUndoStack* undoStack,
                                         bool shiftPressed, bool ctrlPressed, bool altPressed,
                                         QUndoCommand* parentCommand) {
    Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (!canDraw(map, mapPos, nullptr )) {
        return nullptr;
    }

    if (ctrlPressed) {
        return removeBrush(map, mapPos, nullptr , parentCommand);
    } else {
        return applyBrush(map, mapPos, nullptr , parentCommand);
    }
}

QUndoCommand* GroundBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (event->buttons() & Qt::LeftButton) {
        if (!canDraw(map, mapPos, nullptr )) {
            return nullptr;
        }
        if (ctrlPressed) {
            return removeBrush(map, mapPos, nullptr , parentCommand);
        } else {
            return applyBrush(map, mapPos, nullptr , parentCommand);
        }
    }
    return nullptr;
}

QUndoCommand* GroundBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                           Map* map, QUndoStack* undoStack,
                                           bool shiftPressed, bool ctrlPressed, bool altPressed,
                                           QUndoCommand* parentCommand) {
    Q_UNUSED(mapPos); Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(map); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(ctrlPressed); Q_UNUSED(altPressed); Q_UNUSED(parentCommand);
    return nullptr;
}

// Convenience method
bool GroundBrush::isGround() const {
    return true;
}

// Border system hooks (Task 38 requirement - placeholders for Task 52)
void GroundBrush::requestBorderUpdate(Map* map, const QPointF& tilePos) const {
    if (!map) {
        return;
    }

    // Placeholder for border system integration (Task 52)
    // This will request border recalculation for the affected tile and its neighbors
    map->requestBorderUpdate(tilePos);

    qDebug() << "GroundBrush::requestBorderUpdate: Requested border update for tile at" << tilePos;
}

bool GroundBrush::checkGroundEquivalent(Map* map, const QPointF& tilePos, quint16 groundItemId) const {
    Q_UNUSED(map);
    Q_UNUSED(tilePos);

    // Check if the ground item is in our equivalent group
    if (groundEquivalentGroup_.isEmpty()) {
        return true; // No restrictions if no equivalent group is defined
    }

    return groundEquivalentGroup_.contains(groundItemId);
}

// Ground placement validation (Task 38 requirement)
bool GroundBrush::canPlaceGroundAt(Map* map, const QPointF& tilePos, quint16 groundItemId) const {
    if (!map) {
        return false;
    }

    // Check ground equivalent restrictions
    if (!checkGroundEquivalent(map, tilePos, groundItemId)) {
        qDebug() << "GroundBrush::canPlaceGroundAt: Ground ID" << groundItemId << "not in equivalent group for position" << tilePos;
        return false;
    }

    // Additional placement validation can be added here
    // For example, checking neighboring tiles for compatibility

    return true;
}

// XML loading support (structure for Task 81)
bool GroundBrush::load(const QDomElement& element, QStringList& warnings) {
    // Call base class load first
    if (!TerrainBrush::load(element, warnings)) {
        return false;
    }

    // Load ground-specific properties
    bool ok;

    // Load ground item ID
    quint16 groundId = element.attribute("groundId", "0").toUShort(&ok);
    if (ok && groundId > 0) {
        setCurrentGroundItemId(groundId);
        // Task 016: Don't call setLookID() - currentGroundItemId_ is now the primary storage
        setSpecificName(QString("Ground %1").arg(groundId));
    }

    // Load border properties
    hasOptionalBorder_ = element.attribute("hasOptionalBorder", "false") == "true";
    useSoloOptionalBorder_ = element.attribute("useSoloOptionalBorder", "false") == "true";
    isReRandomizable_ = element.attribute("isReRandomizable", "false") == "true";

    // Load ground equivalent group
    QString equivalentGroup = element.attribute("groundEquivalentGroup");
    if (!equivalentGroup.isEmpty()) {
        QStringList idStrings = equivalentGroup.split(",", Qt::SkipEmptyParts);
        groundEquivalentGroup_.clear();
        for (const QString& idStr : idStrings) {
            quint16 id = idStr.trimmed().toUShort(&ok);
            if (ok && id > 0) {
                groundEquivalentGroup_.append(id);
            } else {
                warnings.append(QString("Invalid ground equivalent ID: %1").arg(idStr));
            }
        }
    }

    qDebug() << "GroundBrush::load: Loaded ground brush with ID" << getCurrentGroundItemId()
             << "hasOptionalBorder:" << hasOptionalBorder_
             << "equivalent group size:" << groundEquivalentGroup_.size();

    return true;
}
