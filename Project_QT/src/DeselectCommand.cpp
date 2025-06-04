#include "DeselectCommand.h"
#include "Map.h"
#include <QDebug>

DeselectCommand::DeselectCommand(Map* map, const QSet<MapPos>& positions)
    : map_(map), positionsToDeselect_(positions) {
    if (map_) {
        // Store originally selected positions for undo
        originallySelectedPositions_ = map_->getSelectedPositions();
    }
}

DeselectCommand::~DeselectCommand() {
    // No cleanup needed
}

void DeselectCommand::execute() {
    if (!map_) {
        qWarning() << "DeselectCommand::execute: Map pointer is null";
        return;
    }

    // Deselect the specified positions
    for (const MapPos& pos : positionsToDeselect_) {
        map_->deselectPosition(pos);
    }

    qDebug() << "DeselectCommand: Deselected" << positionsToDeselect_.size() << "positions";
}

void DeselectCommand::undo() {
    if (!map_) {
        qWarning() << "DeselectCommand::undo: Map pointer is null";
        return;
    }

    // Restore original selection state
    map_->clearSelection();
    for (const MapPos& pos : originallySelectedPositions_) {
        map_->selectPosition(pos);
    }

    qDebug() << "DeselectCommand: Restored selection of" << originallySelectedPositions_.size() << "positions";
}
