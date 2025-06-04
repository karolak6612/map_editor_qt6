#include "SelectionBrush.h"
#include "Map.h"
#include "Selection.h"
#include "SelectionCommands.h"
#include "ClipboardManager.h"
#include "SettingsManager.h"
#include <QUndoStack>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>

SelectionBrush::SelectionBrush(QObject* parent)
    : Brush(parent)
    , selectionMode_(RECTANGLE_SELECT)
    , mouseState_(IDLE)
    , isDragging_(false)
    , isMoving_(false)
    , currentModifiers_(Qt::NoModifier)
    , selection_(nullptr)
    , clipboardManager_(nullptr)
    , undoStack_(nullptr)
    , currentMap_(nullptr)
    , showRubberBand_(false)
    , showMovePreview_(false)
    , autoSelectAfterPaste_(true)
    , confirmDestructiveOperations_(true)
    , enableAutomagicBordering_(true)
{
    // Load settings
    SettingsManager* settings = SettingsManager::getInstance();
    if (settings) {
        autoSelectAfterPaste_ = settings->getBool("selection/auto_select_after_paste", true);
        confirmDestructiveOperations_ = settings->getBool("selection/confirm_destructive", true);
        enableAutomagicBordering_ = settings->getBool("automagic/enabled", true);
    }
}

SelectionBrush::~SelectionBrush() = default;

QString SelectionBrush::getName() const {
    return tr("Selection");
}

QString SelectionBrush::getDescription() const {
    return tr("Select, copy, cut, paste and transform map areas");
}

QIcon SelectionBrush::getIcon() const {
    return QIcon(":/icons/selection.png");
}

Brush::BrushType SelectionBrush::getType() const {
    return BrushType::SELECTION_BRUSH;
}

bool SelectionBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext)
    return map != nullptr;
}

QUndoCommand* SelectionBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext)
    Q_UNUSED(parentCommand)
    
    if (!map || !selection_) {
        return nullptr;
    }
    
    currentMap_ = map;
    MapPos mapPos = screenToMapPosition(tilePos);
    
    // Handle selection based on current mode
    switch (selectionMode_) {
        case SINGLE_SELECT:
            selectSingle(map, mapPos, currentModifiers_ & Qt::ControlModifier);
            break;
            
        case RECTANGLE_SELECT:
            // Rectangle selection is handled by mouse events
            break;
            
        case POLYGON_SELECT:
            // Polygon selection is handled by mouse events
            // Points are collected and selection is finalized on double-click or right-click
            break;

        case LASSO_SELECT:
            // Lasso selection is handled by mouse events
            // Path is traced and selection is finalized on mouse release
            break;
    }
    
    return nullptr; // Selection operations are handled by commands
}

QUndoCommand* SelectionBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(map)
    Q_UNUSED(tilePos)
    Q_UNUSED(drawingContext)
    Q_UNUSED(parentCommand)
    
    // Right-click operations could be handled here
    return nullptr;
}

void SelectionBrush::setSelectionMode(SelectionMode mode) {
    if (selectionMode_ != mode) {
        selectionMode_ = mode;
        emit selectionModeChanged(mode);
        updateCursor();
    }
}

SelectionBrush::SelectionMode SelectionBrush::getSelectionMode() const {
    return selectionMode_;
}

void SelectionBrush::selectSingle(Map* map, const MapPos& position, bool addToSelection) {
    if (!map || !selection_ || !undoStack_) {
        return;
    }
    
    SelectSingleCommand* command = new SelectSingleCommand(map, selection_, position, nullptr, addToSelection);
    executeSelectionCommand(command, "Select Single");
}

void SelectionBrush::selectRectangle(Map* map, const MapPos& startPos, const MapPos& endPos, bool addToSelection) {
    if (!map || !selection_ || !undoStack_) {
        return;
    }
    
    SelectRectangleCommand* command = new SelectRectangleCommand(map, selection_, startPos, endPos, addToSelection);
    executeSelectionCommand(command, "Select Rectangle");
}

void SelectionBrush::selectPolygon(Map* map, const QList<MapPos>& polygonPoints, bool addToSelection) {
    if (!map || !selection_ || !undoStack_ || polygonPoints.size() < 3) {
        return;
    }

    SelectPolygonCommand* command = new SelectPolygonCommand(map, selection_, polygonPoints, addToSelection);
    executeSelectionCommand(command, "Select Polygon");
}

void SelectionBrush::selectLasso(Map* map, const QList<MapPos>& lassoPath, bool addToSelection) {
    if (!map || !selection_ || !undoStack_ || lassoPath.size() < 2) {
        return;
    }

    SelectLassoCommand* command = new SelectLassoCommand(map, selection_, lassoPath, addToSelection);
    executeSelectionCommand(command, "Select Lasso");
}

void SelectionBrush::clearSelection() {
    if (selection_) {
        selection_->clear();
        emit selectionChanged();
    }
}

void SelectionBrush::copySelection() {
    if (!currentMap_ || !selection_ || !clipboardManager_ || !undoStack_) {
        qWarning() << "SelectionBrush::copySelection: Missing required components";
        return;
    }
    
    if (!hasSelection()) {
        qWarning() << "SelectionBrush::copySelection: No selection to copy";
        return;
    }
    
    CopySelectionCommand* command = new CopySelectionCommand(currentMap_, selection_, clipboardManager_);
    executeSelectionCommand(command, "Copy");
}

void SelectionBrush::cutSelection() {
    if (!currentMap_ || !selection_ || !clipboardManager_ || !undoStack_) {
        qWarning() << "SelectionBrush::cutSelection: Missing required components";
        return;
    }
    
    if (!hasSelection()) {
        qWarning() << "SelectionBrush::cutSelection: No selection to cut";
        return;
    }
    
    // Confirm destructive operation if enabled
    if (confirmDestructiveOperations_) {
        int result = QMessageBox::question(nullptr, tr("Cut Selection"),
                                          tr("Are you sure you want to cut the selected area?\n"
                                             "This will remove the selected items from the map."),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
        if (result != QMessageBox::Yes) {
            return;
        }
    }
    
    CutSelectionCommand* command = new CutSelectionCommand(currentMap_, selection_, clipboardManager_);
    executeSelectionCommand(command, "Cut");
}

void SelectionBrush::pasteSelection(Map* map, const MapPos& targetPosition) {
    if (!map || !selection_ || !clipboardManager_ || !undoStack_) {
        qWarning() << "SelectionBrush::pasteSelection: Missing required components";
        return;
    }
    
    if (!clipboardManager_->hasClipboardData()) {
        qWarning() << "SelectionBrush::pasteSelection: No clipboard data to paste";
        return;
    }
    
    currentMap_ = map;
    
    PasteSelectionCommand* command = new PasteSelectionCommand(map, selection_, clipboardManager_, 
                                                              targetPosition, PasteSelectionCommand::REPLACE_MODE);
    executeSelectionCommand(command, "Paste");
}

void SelectionBrush::deleteSelection() {
    if (!currentMap_ || !selection_ || !undoStack_) {
        qWarning() << "SelectionBrush::deleteSelection: Missing required components";
        return;
    }
    
    if (!hasSelection()) {
        qWarning() << "SelectionBrush::deleteSelection: No selection to delete";
        return;
    }
    
    // Confirm destructive operation if enabled
    if (confirmDestructiveOperations_) {
        int result = QMessageBox::question(nullptr, tr("Delete Selection"),
                                          tr("Are you sure you want to delete the selected area?\n"
                                             "This action cannot be undone except through the undo system."),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
        if (result != QMessageBox::Yes) {
            return;
        }
    }
    
    DeleteSelectionCommand* command = new DeleteSelectionCommand(currentMap_, selection_);
    executeSelectionCommand(command, "Delete");
}

void SelectionBrush::moveSelection(const QPointF& offset) {
    if (!currentMap_ || !selection_ || !undoStack_) {
        return;
    }
    
    if (!hasSelection()) {
        return;
    }
    
    QVariant parameters = offset;
    SelectionTransformCommand* command = new SelectionTransformCommand(currentMap_, selection_, 
                                                                      SelectionTransformCommand::MOVE_TRANSFORM, 
                                                                      parameters);
    executeSelectionCommand(command, "Move");
}

void SelectionBrush::rotateSelection(int degrees) {
    if (!currentMap_ || !selection_ || !undoStack_) {
        return;
    }
    
    if (!hasSelection()) {
        return;
    }
    
    QVariant parameters = degrees;
    SelectionTransformCommand* command = new SelectionTransformCommand(currentMap_, selection_, 
                                                                      SelectionTransformCommand::ROTATE_TRANSFORM, 
                                                                      parameters);
    executeSelectionCommand(command, QString("Rotate %1°").arg(degrees));
}

void SelectionBrush::flipSelectionHorizontal() {
    if (!currentMap_ || !selection_ || !undoStack_) {
        return;
    }
    
    if (!hasSelection()) {
        return;
    }
    
    SelectionTransformCommand* command = new SelectionTransformCommand(currentMap_, selection_, 
                                                                      SelectionTransformCommand::FLIP_HORIZONTAL_TRANSFORM, 
                                                                      QVariant());
    executeSelectionCommand(command, "Flip Horizontal");
}

void SelectionBrush::flipSelectionVertical() {
    if (!currentMap_ || !selection_ || !undoStack_) {
        return;
    }
    
    if (!hasSelection()) {
        return;
    }
    
    SelectionTransformCommand* command = new SelectionTransformCommand(currentMap_, selection_, 
                                                                      SelectionTransformCommand::FLIP_VERTICAL_TRANSFORM, 
                                                                      QVariant());
    executeSelectionCommand(command, "Flip Vertical");
}

void SelectionBrush::handleMousePress(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers) {
    if (!map || !selection_) {
        return;
    }
    
    currentMap_ = map;
    currentModifiers_ = modifiers;
    lastMousePosition_ = position;
    
    MapPos mapPos = screenToMapPosition(position);
    
    if (button == Qt::LeftButton) {
        if (isPositionInSelection(mapPos) && !(modifiers & Qt::ControlModifier)) {
            // Start moving selection
            startMoving(map, position, modifiers);
        } else {
            // Start new selection
            startDragging(map, position, modifiers);
        }
    } else if (button == Qt::RightButton) {
        // Context menu or clear selection
        if (!(modifiers & Qt::ControlModifier)) {
            clearSelection();
        }
    }
}

void SelectionBrush::handleMouseMove(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers) {
    if (!map) {
        return;
    }
    
    currentModifiers_ = modifiers;
    
    if (mouseState_ == DRAGGING) {
        updateDragging(map, position, modifiers);
    } else if (mouseState_ == MOVING_SELECTION) {
        updateMoving(map, position, modifiers);
    }
    
    lastMousePosition_ = position;
}

void SelectionBrush::handleMouseRelease(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers) {
    if (!map || button != Qt::LeftButton) {
        return;
    }
    
    currentModifiers_ = modifiers;
    
    if (mouseState_ == DRAGGING) {
        finishDragging(map, position, modifiers);
    } else if (mouseState_ == MOVING_SELECTION) {
        finishMoving(map, position, modifiers);
    }
    
    setState(IDLE);
}

void SelectionBrush::handleMouseDoubleClick(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers) {
    Q_UNUSED(map)
    Q_UNUSED(position)
    Q_UNUSED(button)
    Q_UNUSED(modifiers)
    
    // Future: Could implement smart selection (e.g., select all connected tiles of same type)
}

void SelectionBrush::handleKeyPress(QKeyEvent* event) {
    if (!event) {
        return;
    }
    
    // Handle keyboard shortcuts
    switch (event->key()) {
        case Qt::Key_Delete:
            deleteSelection();
            break;
            
        case Qt::Key_C:
            if (event->modifiers() & Qt::ControlModifier) {
                copySelection();
            }
            break;
            
        case Qt::Key_X:
            if (event->modifiers() & Qt::ControlModifier) {
                cutSelection();
            }
            break;
            
        case Qt::Key_V:
            if (event->modifiers() & Qt::ControlModifier && currentMap_) {
                // Paste at current mouse position or center of view
                MapPos pastePos = screenToMapPosition(lastMousePosition_);
                pasteSelection(currentMap_, pastePos);
            }
            break;
            
        case Qt::Key_A:
            if (event->modifiers() & Qt::ControlModifier) {
                // Select all (future implementation)
            }
            break;
            
        case Qt::Key_Escape:
            clearSelection();
            setState(IDLE);
            break;
    }
}

void SelectionBrush::handleKeyRelease(QKeyEvent* event) {
    Q_UNUSED(event)
    // Handle key release events if needed
}

bool SelectionBrush::hasSelection() const {
    return selection_ && !selection_->isEmpty();
}

bool SelectionBrush::isSelecting() const {
    return mouseState_ == DRAGGING;
}

bool SelectionBrush::isMoving() const {
    return mouseState_ == MOVING_SELECTION;
}

int SelectionBrush::getSelectedTileCount() const {
    return selection_ ? selection_->size() : 0;
}

int SelectionBrush::getSelectedItemCount() const {
    if (!selection_ || !currentMap_) {
        return 0;
    }
    
    int itemCount = 0;
    QSet<MapPos> selectedTiles = selection_->getSelectedTiles();
    
    for (const MapPos& pos : selectedTiles) {
        Tile* tile = currentMap_->getTile(pos.x, pos.y, pos.z);
        if (tile) {
            itemCount += tile->getItemCount();
            if (tile->getCreature()) itemCount++;
            if (tile->getSpawn()) itemCount++;
        }
    }
    
    return itemCount;
}

// Integration methods
void SelectionBrush::setSelection(Selection* selection) {
    if (selection_ != selection) {
        if (selection_) {
            disconnect(selection_, &Selection::selectionChanged, this, &SelectionBrush::onSelectionChanged);
        }

        selection_ = selection;

        if (selection_) {
            connect(selection_, &Selection::selectionChanged, this, &SelectionBrush::onSelectionChanged);
        }

        emit selectionChanged();
    }
}

Selection* SelectionBrush::getSelection() const {
    return selection_;
}

void SelectionBrush::setClipboardManager(ClipboardManager* clipboardManager) {
    clipboardManager_ = clipboardManager;
}

ClipboardManager* SelectionBrush::getClipboardManager() const {
    return clipboardManager_;
}

void SelectionBrush::setUndoStack(QUndoStack* undoStack) {
    undoStack_ = undoStack;
}

QUndoStack* SelectionBrush::getUndoStack() const {
    return undoStack_;
}

// Private slots
void SelectionBrush::onSelectionChanged() {
    emit selectionChanged();
    updateVisualFeedback();
}

// Private helper methods
void SelectionBrush::startDragging(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers) {
    Q_UNUSED(map)

    setState(DRAGGING);
    dragStartPosition_ = position;
    dragCurrentPosition_ = position;
    selectionStartPos_ = screenToMapPosition(position);

    emit selectionStarted();
}

void SelectionBrush::updateDragging(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers) {
    Q_UNUSED(map)
    Q_UNUSED(modifiers)

    dragCurrentPosition_ = position;
    showRubberBand_ = true;
    updateVisualFeedback();
}

void SelectionBrush::finishDragging(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers) {
    dragCurrentPosition_ = position;
    selectionEndPos_ = screenToMapPosition(position);

    // Create rectangle selection
    bool addToSelection = modifiers & Qt::ControlModifier;
    selectRectangle(map, selectionStartPos_, selectionEndPos_, addToSelection);

    showRubberBand_ = false;
    emit selectionFinished();
}

void SelectionBrush::startMoving(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers) {
    Q_UNUSED(map)
    Q_UNUSED(modifiers)

    setState(MOVING_SELECTION);
    dragStartPosition_ = position;
    dragCurrentPosition_ = position;
    moveOffset_ = QPointF(0, 0);
    showMovePreview_ = true;
}

void SelectionBrush::updateMoving(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers) {
    Q_UNUSED(map)
    Q_UNUSED(modifiers)

    dragCurrentPosition_ = position;
    moveOffset_ = position - dragStartPosition_;
    updateVisualFeedback();
}

void SelectionBrush::finishMoving(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers) {
    Q_UNUSED(modifiers)

    moveOffset_ = position - dragStartPosition_;

    // Apply move transformation if there's significant movement
    if (moveOffset_.manhattanLength() > 5.0) { // 5 pixel threshold
        moveSelection(moveOffset_);
    }

    showMovePreview_ = false;
    moveOffset_ = QPointF(0, 0);
}

bool SelectionBrush::isPositionInSelection(const MapPos& position) const {
    return selection_ && selection_->isSelected(position);
}

MapPos SelectionBrush::screenToMapPosition(const QPointF& screenPos) const {
    // Convert screen coordinates to map coordinates
    // This is a simplified implementation - actual conversion depends on view transformation
    return MapPos(static_cast<int>(screenPos.x()), static_cast<int>(screenPos.y()), 7); // Default to ground floor
}

QPointF SelectionBrush::mapToScreenPosition(const MapPos& mapPos) const {
    // Convert map coordinates to screen coordinates
    // This is a simplified implementation - actual conversion depends on view transformation
    return QPointF(mapPos.x, mapPos.y);
}

void SelectionBrush::executeCommand(QUndoCommand* command) {
    if (command && undoStack_) {
        undoStack_->push(command);
    } else {
        delete command;
    }
}

void SelectionBrush::executeSelectionCommand(QUndoCommand* command, const QString& operationName) {
    if (!command) {
        return;
    }

    executeCommand(command);

    // Emit operation completed signal with statistics
    // Note: This is a simplified implementation - actual statistics would come from the command
    int tileCount = getSelectedTileCount();
    int itemCount = getSelectedItemCount();

    emit operationCompleted(operationName, tileCount, itemCount);
}

void SelectionBrush::setState(MouseState newState) {
    if (mouseState_ != newState) {
        mouseState_ = newState;
        emit mouseStateChanged(newState);
        updateCursor();
    }
}

void SelectionBrush::updateCursor() {
    // Update cursor based on current state and mode
    // This would typically interact with the view to set appropriate cursor
    qDebug() << "SelectionBrush::updateCursor - State:" << mouseState_ << "Mode:" << selectionMode_;
}

void SelectionBrush::updateVisualFeedback() {
    // Update visual feedback for selection, rubber band, move preview, etc.
    // This would typically trigger view updates
    qDebug() << "SelectionBrush::updateVisualFeedback - RubberBand:" << showRubberBand_ << "MovePreview:" << showMovePreview_;
}
