#include "SetTileColorCommand.h"
#include "Map.h"   // Include actual Map header
#include "Tile.h"  // Include actual Tile header
#include <QDebug>  // For logging

// Assume Map has: Tile* getTile(const QPointF& pos);
// Assume Tile has: void setDebugColor(const QColor& color); QColor getDebugColor() const;

SetTileColorCommand::SetTileColorCommand(Map* map, const QPointF& tilePos, const QColor& newColor, QUndoCommand* parent)
    : QUndoCommand(parent), map_(map), tilePos_(tilePos), newColor_(newColor), firstRedo_(true) {
    // setText will be set by the brush or handler
}

SetTileColorCommand::~SetTileColorCommand() {}

void SetTileColorCommand::undo() {
    if (!map_) return;
    Tile* tile = map_->getTile(tilePos_); // Assuming getTile uses QPointF
    if (tile) {
        tile->setDebugColor(oldColor_);
        // map_->updateTileVisual(tilePos_); // Notify map/view of change
        qDebug() << "SetTileColorCommand: Undone on tile" << tilePos_ << "to color" << oldColor_;
    } else {
        qWarning() << "SetTileColorCommand: Could not find tile at" << tilePos_ << "during undo.";
    }
}

void SetTileColorCommand::redo() {
    if (!map_) return;
    Tile* tile = map_->getTile(tilePos_);
    if (tile) {
        if (firstRedo_) {
            oldColor_ = tile->getDebugColor(); // Capture original color on first redo
            firstRedo_ = false;
        }
        tile->setDebugColor(newColor_);
        // map_->updateTileVisual(tilePos_); // Notify map/view of change
        qDebug() << "SetTileColorCommand: Redone on tile" << tilePos_ << "to color" << newColor_;
        if (text().isEmpty()) { // Set default text if not already set
             setText(QObject::tr("Set Tile Color to %1 at (%2,%3)")
                        .arg(newColor_.name())
                        .arg(tilePos_.x())
                        .arg(tilePos_.y()));
        }
    } else {
        qWarning() << "SetTileColorCommand: Could not find tile at" << tilePos_ << "during redo.";
    }
}
