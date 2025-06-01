#include "TileLockActions.h"
#include "Map.h"
#include "MapView.h"
#include "Selection.h"
#include "commands/TileLockCommands.h"
#include <QMenu>
#include <QToolBar>
#include <QUndoStack>
#include <QKeySequence>
#include <QIcon>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QShortcut>
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>

// =============================================================================
// TileLockActions Implementation
// =============================================================================

TileLockActions::TileLockActions(Map* map, MapView* mapView, QUndoStack* undoStack, QObject* parent)
    : QObject(parent)
    , map_(map)
    , mapView_(mapView)
    , undoStack_(undoStack)
    , tileLockManager_(nullptr)
    , lockTileAction_(nullptr)
    , unlockTileAction_(nullptr)
    , toggleLockAction_(nullptr)
    , lockSelectionAction_(nullptr)
    , unlockSelectionAction_(nullptr)
    , lockAllAction_(nullptr)
    , unlockAllAction_(nullptr)
    , lockAreaAction_(nullptr)
    , unlockAreaAction_(nullptr)
    , showLockedTilesAction_(nullptr)
    , lockStatisticsAction_(nullptr)
    , lockActionGroup_(nullptr)
{
    Q_ASSERT(map_);
    Q_ASSERT(mapView_);
    Q_ASSERT(undoStack_);
    
    createActions();
    setupActionProperties();
    setupActionConnections();
}

void TileLockActions::createActions()
{
    createLockActions();
    createSelectionActions();
    createAreaActions();
    createViewActions();
    createUtilityActions();
}

void TileLockActions::createLockActions()
{
    // Single tile lock/unlock actions
    lockTileAction_ = new QAction(QIcon(":/icons/lock_tile.png"), tr("&Lock Tile"), this);
    lockTileAction_->setShortcut(QKeySequence("Ctrl+L"));
    lockTileAction_->setStatusTip(tr("Lock the selected tile to prevent editing"));
    lockTileAction_->setToolTip(tr("Lock Tile (Ctrl+L)"));
    
    unlockTileAction_ = new QAction(QIcon(":/icons/unlock_tile.png"), tr("&Unlock Tile"), this);
    unlockTileAction_->setShortcut(QKeySequence("Ctrl+U"));
    unlockTileAction_->setStatusTip(tr("Unlock the selected tile to allow editing"));
    unlockTileAction_->setToolTip(tr("Unlock Tile (Ctrl+U)"));
    
    toggleLockAction_ = new QAction(QIcon(":/icons/toggle_lock.png"), tr("&Toggle Lock"), this);
    toggleLockAction_->setShortcut(QKeySequence("Ctrl+T"));
    toggleLockAction_->setStatusTip(tr("Toggle the lock state of the selected tile"));
    toggleLockAction_->setToolTip(tr("Toggle Lock (Ctrl+T)"));
}

void TileLockActions::createSelectionActions()
{
    // Selection-based lock/unlock actions
    lockSelectionAction_ = new QAction(QIcon(":/icons/lock_selection.png"), tr("Lock &Selection"), this);
    lockSelectionAction_->setShortcut(QKeySequence("Ctrl+Shift+L"));
    lockSelectionAction_->setStatusTip(tr("Lock all tiles in the current selection"));
    lockSelectionAction_->setToolTip(tr("Lock Selection (Ctrl+Shift+L)"));
    
    unlockSelectionAction_ = new QAction(QIcon(":/icons/unlock_selection.png"), tr("Unlock S&election"), this);
    unlockSelectionAction_->setShortcut(QKeySequence("Ctrl+Shift+U"));
    unlockSelectionAction_->setStatusTip(tr("Unlock all tiles in the current selection"));
    unlockSelectionAction_->setToolTip(tr("Unlock Selection (Ctrl+Shift+U)"));
}

void TileLockActions::createAreaActions()
{
    // Area-based lock/unlock actions
    lockAreaAction_ = new QAction(QIcon(":/icons/lock_area.png"), tr("Lock &Area"), this);
    lockAreaAction_->setShortcut(QKeySequence("Ctrl+Alt+L"));
    lockAreaAction_->setStatusTip(tr("Lock all tiles in a specified area"));
    lockAreaAction_->setToolTip(tr("Lock Area (Ctrl+Alt+L)"));
    
    unlockAreaAction_ = new QAction(QIcon(":/icons/unlock_area.png"), tr("Unlock A&rea"), this);
    unlockAreaAction_->setShortcut(QKeySequence("Ctrl+Alt+U"));
    unlockAreaAction_->setStatusTip(tr("Unlock all tiles in a specified area"));
    unlockAreaAction_->setToolTip(tr("Unlock Area (Ctrl+Alt+U)"));
    
    // Global lock/unlock actions
    lockAllAction_ = new QAction(QIcon(":/icons/lock_all.png"), tr("Lock &All Tiles"), this);
    lockAllAction_->setShortcut(QKeySequence("Ctrl+Shift+Alt+L"));
    lockAllAction_->setStatusTip(tr("Lock all tiles in the map"));
    lockAllAction_->setToolTip(tr("Lock All Tiles (Ctrl+Shift+Alt+L)"));
    
    unlockAllAction_ = new QAction(QIcon(":/icons/unlock_all.png"), tr("Unlock A&ll Tiles"), this);
    unlockAllAction_->setShortcut(QKeySequence("Ctrl+Shift+Alt+U"));
    unlockAllAction_->setStatusTip(tr("Unlock all tiles in the map"));
    unlockAllAction_->setToolTip(tr("Unlock All Tiles (Ctrl+Shift+Alt+U)"));
}

void TileLockActions::createViewActions()
{
    // View-related actions
    showLockedTilesAction_ = new QAction(QIcon(":/icons/show_locked.png"), tr("Show &Locked Tiles"), this);
    showLockedTilesAction_->setCheckable(true);
    showLockedTilesAction_->setChecked(true);
    showLockedTilesAction_->setShortcut(QKeySequence("Ctrl+Shift+V"));
    showLockedTilesAction_->setStatusTip(tr("Show visual indication of locked tiles"));
    showLockedTilesAction_->setToolTip(tr("Show Locked Tiles (Ctrl+Shift+V)"));
}

void TileLockActions::createUtilityActions()
{
    // Utility actions
    lockStatisticsAction_ = new QAction(QIcon(":/icons/lock_stats.png"), tr("Lock &Statistics"), this);
    lockStatisticsAction_->setShortcut(QKeySequence("Ctrl+Shift+S"));
    lockStatisticsAction_->setStatusTip(tr("Show tile lock statistics"));
    lockStatisticsAction_->setToolTip(tr("Lock Statistics (Ctrl+Shift+S)"));
}

void TileLockActions::setupActionProperties()
{
    // Create action group for mutually exclusive actions if needed
    lockActionGroup_ = new QActionGroup(this);
    // Add actions to group if they should be mutually exclusive
    
    // Set initial states
    updateActionStates();
}

void TileLockActions::setupActionConnections()
{
    // Connect actions to slots
    connect(lockTileAction_, &QAction::triggered, this, &TileLockActions::onLockTile);
    connect(unlockTileAction_, &QAction::triggered, this, &TileLockActions::onUnlockTile);
    connect(toggleLockAction_, &QAction::triggered, this, &TileLockActions::onToggleLock);
    connect(lockSelectionAction_, &QAction::triggered, this, &TileLockActions::onLockSelection);
    connect(unlockSelectionAction_, &QAction::triggered, this, &TileLockActions::onUnlockSelection);
    connect(lockAllAction_, &QAction::triggered, this, &TileLockActions::onLockAll);
    connect(unlockAllAction_, &QAction::triggered, this, &TileLockActions::onUnlockAll);
    connect(lockAreaAction_, &QAction::triggered, this, &TileLockActions::onLockArea);
    connect(unlockAreaAction_, &QAction::triggered, this, &TileLockActions::onUnlockArea);
    connect(showLockedTilesAction_, &QAction::triggered, this, &TileLockActions::onToggleShowLockedTiles);
    connect(lockStatisticsAction_, &QAction::triggered, this, &TileLockActions::onShowLockStatistics);
    
    // Connect to map and view signals
    if (mapView_) {
        // Connect to selection changes, cursor position changes, etc.
        // connect(mapView_, &MapView::selectionChanged, this, &TileLockActions::onSelectionChanged);
        // connect(mapView_, &MapView::cursorPositionChanged, this, &TileLockActions::updateActionStates);
    }
    
    if (map_) {
        // Connect to map changes
        // connect(map_, &Map::mapChanged, this, &TileLockActions::onMapChanged);
    }
}

void TileLockActions::setupMenus(QMenu* editMenu, QMenu* viewMenu)
{
    if (editMenu) {
        editMenu->addSeparator();
        editMenu->addAction(lockTileAction_);
        editMenu->addAction(unlockTileAction_);
        editMenu->addAction(toggleLockAction_);
        editMenu->addSeparator();
        editMenu->addAction(lockSelectionAction_);
        editMenu->addAction(unlockSelectionAction_);
        editMenu->addSeparator();
        editMenu->addAction(lockAreaAction_);
        editMenu->addAction(unlockAreaAction_);
        editMenu->addSeparator();
        editMenu->addAction(lockAllAction_);
        editMenu->addAction(unlockAllAction_);
    }
    
    if (viewMenu) {
        viewMenu->addSeparator();
        viewMenu->addAction(showLockedTilesAction_);
        viewMenu->addAction(lockStatisticsAction_);
    }
}

void TileLockActions::setupToolBar(QToolBar* toolBar)
{
    if (toolBar) {
        toolBar->addSeparator();
        toolBar->addAction(lockTileAction_);
        toolBar->addAction(unlockTileAction_);
        toolBar->addAction(toggleLockAction_);
        toolBar->addSeparator();
        toolBar->addAction(lockSelectionAction_);
        toolBar->addAction(unlockSelectionAction_);
        toolBar->addSeparator();
        toolBar->addAction(showLockedTilesAction_);
        toolBar->addAction(lockStatisticsAction_);
    }
}

void TileLockActions::setupContextMenu(QMenu* contextMenu)
{
    if (contextMenu) {
        contextMenu->addSeparator();
        contextMenu->addAction(lockTileAction_);
        contextMenu->addAction(unlockTileAction_);
        contextMenu->addAction(toggleLockAction_);
        contextMenu->addSeparator();
        contextMenu->addAction(lockSelectionAction_);
        contextMenu->addAction(unlockSelectionAction_);
    }
}

void TileLockActions::updateActionStates()
{
    updateLockActionStates();
    updateSelectionActionStates();
    updateAreaActionStates();
    updateViewActionStates();
}

void TileLockActions::updateActionStates(const MapPos& position)
{
    Q_UNUSED(position)
    updateActionStates();
}

void TileLockActions::updateActionStates(const QList<MapPos>& positions)
{
    Q_UNUSED(positions)
    updateActionStates();
}

void TileLockActions::updateLockActionStates()
{
    MapPos currentPos = getCurrentTilePosition();
    bool hasValidTile = map_ && map_->getTile(currentPos.x, currentPos.y, currentPos.z) != nullptr;
    bool isTileLocked = false;
    
    if (hasValidTile && tileLockManager_) {
        isTileLocked = tileLockManager_->isTileLocked(currentPos);
    }
    
    lockTileAction_->setEnabled(hasValidTile && !isTileLocked);
    unlockTileAction_->setEnabled(hasValidTile && isTileLocked);
    toggleLockAction_->setEnabled(hasValidTile);
}

void TileLockActions::updateSelectionActionStates()
{
    bool hasSelection = hasValidSelection();
    bool hasLockedInSelection = hasLockedTilesInSelection();
    bool hasUnlockedInSelection = hasUnlockedTilesInSelection();
    
    lockSelectionAction_->setEnabled(hasSelection && hasUnlockedInSelection);
    unlockSelectionAction_->setEnabled(hasSelection && hasLockedInSelection);
}

void TileLockActions::updateAreaActionStates()
{
    bool hasMap = map_ != nullptr;
    
    lockAreaAction_->setEnabled(hasMap);
    unlockAreaAction_->setEnabled(hasMap);
    lockAllAction_->setEnabled(hasMap);
    unlockAllAction_->setEnabled(hasMap);
}

void TileLockActions::updateViewActionStates()
{
    bool hasMap = map_ != nullptr;
    
    showLockedTilesAction_->setEnabled(hasMap);
    lockStatisticsAction_->setEnabled(hasMap);
}

void TileLockActions::setTileLockManager(TileLockManager* manager)
{
    if (tileLockManager_) {
        disconnect(tileLockManager_, nullptr, this, nullptr);
    }
    
    tileLockManager_ = manager;
    
    if (tileLockManager_) {
        connect(tileLockManager_, &TileLockManager::lockStateChanged,
                this, &TileLockActions::onTileLockStateChanged);
        connect(tileLockManager_, &TileLockManager::statisticsChanged,
                this, &TileLockActions::updateActionStates);
    }
    
    updateActionStates();
}

void TileLockActions::setShowLockedTiles(bool show)
{
    if (showLockedTilesAction_) {
        showLockedTilesAction_->setChecked(show);
        emit showLockedTilesChanged(show);
    }
}

bool TileLockActions::isShowLockedTiles() const
{
    return showLockedTilesAction_ ? showLockedTilesAction_->isChecked() : false;
}

// Action handlers
void TileLockActions::onLockTile()
{
    MapPos pos = getCurrentTilePosition();
    if (map_ && map_->getTile(pos.x, pos.y, pos.z)) {
        auto* command = new LockTileCommand(map_, pos, true);
        undoStack_->push(command);
        emit lockTileRequested(pos);
    }
}

void TileLockActions::onUnlockTile()
{
    MapPos pos = getCurrentTilePosition();
    if (map_ && map_->getTile(pos.x, pos.y, pos.z)) {
        auto* command = new LockTileCommand(map_, pos, false);
        undoStack_->push(command);
        emit unlockTileRequested(pos);
    }
}

void TileLockActions::onToggleLock()
{
    MapPos pos = getCurrentTilePosition();
    if (map_ && map_->getTile(pos.x, pos.y, pos.z)) {
        auto* command = new ToggleTileLockCommand(map_, {pos});
        undoStack_->push(command);
        emit toggleLockRequested(pos);
    }
}

void TileLockActions::onLockSelection()
{
    QList<MapPos> positions = getSelectedTilePositions();
    if (!positions.isEmpty()) {
        auto* command = new LockTilesCommand(map_, positions, true);
        undoStack_->push(command);
        emit lockSelectionRequested();
    }
}

void TileLockActions::onUnlockSelection()
{
    QList<MapPos> positions = getSelectedTilePositions();
    if (!positions.isEmpty()) {
        auto* command = new LockTilesCommand(map_, positions, false);
        undoStack_->push(command);
        emit unlockSelectionRequested();
    }
}

void TileLockActions::onLockAll()
{
    if (map_) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            qobject_cast<QWidget*>(parent()),
            tr("Lock All Tiles"),
            tr("Are you sure you want to lock all tiles in the map?\nThis action can be undone."),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            // Create command to lock all tiles
            QList<MapPos> allPositions;
            for (int z = 0; z < map_->getDepth(); ++z) {
                for (int y = 0; y < map_->getHeight(); ++y) {
                    for (int x = 0; x < map_->getWidth(); ++x) {
                        if (map_->getTile(x, y, z)) {
                            allPositions.append(MapPos(x, y, z));
                        }
                    }
                }
            }
            
            if (!allPositions.isEmpty()) {
                auto* command = new LockTilesCommand(map_, allPositions, true);
                undoStack_->push(command);
                emit lockAllRequested();
            }
        }
    }
}

void TileLockActions::onUnlockAll()
{
    if (map_) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            qobject_cast<QWidget*>(parent()),
            tr("Unlock All Tiles"),
            tr("Are you sure you want to unlock all tiles in the map?\nThis action can be undone."),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            // Create command to unlock all tiles
            QList<MapPos> allPositions;
            for (int z = 0; z < map_->getDepth(); ++z) {
                for (int y = 0; y < map_->getHeight(); ++y) {
                    for (int x = 0; x < map_->getWidth(); ++x) {
                        if (map_->getTile(x, y, z)) {
                            allPositions.append(MapPos(x, y, z));
                        }
                    }
                }
            }
            
            if (!allPositions.isEmpty()) {
                auto* command = new LockTilesCommand(map_, allPositions, false);
                undoStack_->push(command);
                emit unlockAllRequested();
            }
        }
    }
}

void TileLockActions::onLockArea()
{
    // This would typically open a dialog to select an area
    // For now, we'll emit the signal for external handling
    emit lockAreaRequested(MapPos(0, 0, 0), MapPos(0, 0, 0));
}

void TileLockActions::onUnlockArea()
{
    // This would typically open a dialog to select an area
    // For now, we'll emit the signal for external handling
    emit unlockAreaRequested(MapPos(0, 0, 0), MapPos(0, 0, 0));
}

void TileLockActions::onToggleShowLockedTiles()
{
    bool show = showLockedTilesAction_->isChecked();
    emit showLockedTilesChanged(show);
}

void TileLockActions::onShowLockStatistics()
{
    emit lockStatisticsRequested();
}

// Context menu handlers
void TileLockActions::onContextMenuRequested(const QPoint& position)
{
    Q_UNUSED(position)
    // Handle context menu request
}

void TileLockActions::onLockTileAt(const MapPos& position)
{
    if (map_ && map_->getTile(position.x, position.y, position.z)) {
        auto* command = new LockTileCommand(map_, position, true);
        undoStack_->push(command);
        emit lockTileRequested(position);
    }
}

void TileLockActions::onUnlockTileAt(const MapPos& position)
{
    if (map_ && map_->getTile(position.x, position.y, position.z)) {
        auto* command = new LockTileCommand(map_, position, false);
        undoStack_->push(command);
        emit unlockTileRequested(position);
    }
}

void TileLockActions::onToggleLockAt(const MapPos& position)
{
    if (map_ && map_->getTile(position.x, position.y, position.z)) {
        auto* command = new ToggleTileLockCommand(map_, {position});
        undoStack_->push(command);
        emit toggleLockRequested(position);
    }
}

// State update handlers
void TileLockActions::onSelectionChanged()
{
    updateActionStates();
}

void TileLockActions::onMapChanged()
{
    updateActionStates();
}

void TileLockActions::onTileLockStateChanged(const MapPos& position, bool isLocked)
{
    Q_UNUSED(position)
    Q_UNUSED(isLocked)
    updateActionStates();
}

// Helper methods
MapPos TileLockActions::getCurrentTilePosition() const
{
    // This would get the current cursor position from MapView
    // For now, return a default position
    return MapPos(0, 0, 0);
}

QList<MapPos> TileLockActions::getSelectedTilePositions() const
{
    // This would get selected tile positions from Selection
    // For now, return an empty list
    return QList<MapPos>();
}

bool TileLockActions::hasValidSelection() const
{
    return !getSelectedTilePositions().isEmpty();
}

bool TileLockActions::hasLockedTilesInSelection() const
{
    if (!tileLockManager_) return false;

    QList<MapPos> positions = getSelectedTilePositions();
    for (const MapPos& pos : positions) {
        if (tileLockManager_->isTileLocked(pos)) {
            return true;
        }
    }
    return false;
}

bool TileLockActions::hasUnlockedTilesInSelection() const
{
    if (!tileLockManager_) return false;

    QList<MapPos> positions = getSelectedTilePositions();
    for (const MapPos& pos : positions) {
        if (!tileLockManager_->isTileLocked(pos)) {
            return true;
        }
    }
    return false;
}

#include "TileLockActions.moc"
