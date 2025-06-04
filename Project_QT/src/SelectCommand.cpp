#include "SelectCommand.h"
#include "Map.h"
#include <QDebug>

SelectCommand::SelectCommand(Map* map, const QSet<MapPos>& positions)
    : map_(map), positionsToSelect_(positions) {
    if (map_) {
        // Store previously selected positions for undo
        previouslySelectedPositions_ = map_->getSelectedPositions();
    }
}

SelectCommand::~SelectCommand() {
    // No cleanup needed
}

void SelectCommand::execute() {
    if (!map_) {
        qWarning() << "SelectCommand::execute: Map pointer is null";
        return;
    }

    // Select the specified positions
    for (const MapPos& pos : positionsToSelect_) {
        map_->selectPosition(pos);
    }

    qDebug() << "SelectCommand: Selected" << positionsToSelect_.size() << "positions";
}

void SelectCommand::undo() {
    if (!map_) {
        qWarning() << "SelectCommand::undo: Map pointer is null";
        return;
    }

    // Restore previous selection state
    map_->clearSelection();
    for (const MapPos& pos : previouslySelectedPositions_) {
        map_->selectPosition(pos);
    }

    qDebug() << "SelectCommand: Restored previous selection of" << previouslySelectedPositions_.size() << "positions";
}
