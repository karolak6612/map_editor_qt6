#ifndef TILELOCKACTIONS_H
#define TILELOCKACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QToolBar>
#include <QKeySequence>
#include <QIcon>
#include <QList>
#include "../MapPos.h"

// Forward declarations
class Map;
class MapView;
class Selection;
class TileLockManager;
class QUndoStack;

/**
 * @brief Tile Lock Actions for Task 85
 * 
 * Complete implementation of UI actions for tile locking:
 * - Menu actions for lock/unlock operations
 * - Toolbar buttons with icons and shortcuts
 * - Context menu integration
 * - Keyboard shortcuts and accelerators
 * - Visual feedback and status updates
 * - Integration with undo/redo system
 */

/**
 * @brief Main tile lock actions manager
 */
class TileLockActions : public QObject
{
    Q_OBJECT

public:
    explicit TileLockActions(Map* map, MapView* mapView, QUndoStack* undoStack, QObject* parent = nullptr);
    ~TileLockActions() override = default;

    // Action creation and setup
    void createActions();
    void setupMenus(QMenu* editMenu, QMenu* viewMenu);
    void setupToolBar(QToolBar* toolBar);
    void setupContextMenu(QMenu* contextMenu);
    void setupKeyboardShortcuts();
    
    // Action access
    QAction* getLockTileAction() const { return lockTileAction_; }
    QAction* getUnlockTileAction() const { return unlockTileAction_; }
    QAction* getToggleLockAction() const { return toggleLockAction_; }
    QAction* getLockSelectionAction() const { return lockSelectionAction_; }
    QAction* getUnlockSelectionAction() const { return unlockSelectionAction_; }
    QAction* getLockAllAction() const { return lockAllAction_; }
    QAction* getUnlockAllAction() const { return unlockAllAction_; }
    QAction* getShowLockedTilesAction() const { return showLockedTilesAction_; }
    QAction* getLockStatisticsAction() const { return lockStatisticsAction_; }
    
    // State management
    void updateActionStates();
    void updateActionStates(const MapPos& position);
    void updateActionStates(const QList<MapPos>& positions);
    
    // Configuration
    void setTileLockManager(TileLockManager* manager);
    TileLockManager* getTileLockManager() const { return tileLockManager_; }
    
    // Visual feedback
    void setShowLockedTiles(bool show);
    bool isShowLockedTiles() const;

signals:
    void lockTileRequested(const MapPos& position);
    void unlockTileRequested(const MapPos& position);
    void toggleLockRequested(const MapPos& position);
    void lockSelectionRequested();
    void unlockSelectionRequested();
    void lockAllRequested();
    void unlockAllRequested();
    void lockAreaRequested(const MapPos& topLeft, const MapPos& bottomRight);
    void unlockAreaRequested(const MapPos& topLeft, const MapPos& bottomRight);
    void showLockedTilesChanged(bool show);
    void lockStatisticsRequested();

public slots:
    // Action handlers
    void onLockTile();
    void onUnlockTile();
    void onToggleLock();
    void onLockSelection();
    void onUnlockSelection();
    void onLockAll();
    void onUnlockAll();
    void onLockArea();
    void onUnlockArea();
    void onToggleShowLockedTiles();
    void onShowLockStatistics();
    
    // Context menu handlers
    void onContextMenuRequested(const QPoint& position);
    void onLockTileAt(const MapPos& position);
    void onUnlockTileAt(const MapPos& position);
    void onToggleLockAt(const MapPos& position);
    
    // State update handlers
    void onSelectionChanged();
    void onMapChanged();
    void onTileLockStateChanged(const MapPos& position, bool isLocked);

private:
    // Core components
    Map* map_;
    MapView* mapView_;
    QUndoStack* undoStack_;
    TileLockManager* tileLockManager_;
    
    // Actions
    QAction* lockTileAction_;
    QAction* unlockTileAction_;
    QAction* toggleLockAction_;
    QAction* lockSelectionAction_;
    QAction* unlockSelectionAction_;
    QAction* lockAllAction_;
    QAction* unlockAllAction_;
    QAction* lockAreaAction_;
    QAction* unlockAreaAction_;
    QAction* showLockedTilesAction_;
    QAction* lockStatisticsAction_;
    
    // Action groups
    QActionGroup* lockActionGroup_;
    
    // Helper methods
    void createLockActions();
    void createSelectionActions();
    void createAreaActions();
    void createViewActions();
    void createUtilityActions();
    
    void setupActionProperties();
    void setupActionConnections();
    
    MapPos getCurrentTilePosition() const;
    QList<MapPos> getSelectedTilePositions() const;
    bool hasValidSelection() const;
    bool hasLockedTilesInSelection() const;
    bool hasUnlockedTilesInSelection() const;
    
    void updateLockActionStates();
    void updateSelectionActionStates();
    void updateAreaActionStates();
    void updateViewActionStates();
};

/**
 * @brief Tile lock context menu provider
 */
class TileLockContextMenu : public QObject
{
    Q_OBJECT

public:
    explicit TileLockContextMenu(TileLockActions* actions, QObject* parent = nullptr);
    ~TileLockContextMenu() override = default;

    // Context menu creation
    QMenu* createContextMenu(const MapPos& position, QWidget* parent = nullptr);
    QMenu* createSelectionContextMenu(const QList<MapPos>& positions, QWidget* parent = nullptr);
    QMenu* createAreaContextMenu(const MapPos& topLeft, const MapPos& bottomRight, QWidget* parent = nullptr);
    
    // Menu population
    void populateContextMenu(QMenu* menu, const MapPos& position);
    void populateSelectionContextMenu(QMenu* menu, const QList<MapPos>& positions);
    void populateAreaContextMenu(QMenu* menu, const MapPos& topLeft, const MapPos& bottomRight);

signals:
    void lockTileRequested(const MapPos& position);
    void unlockTileRequested(const MapPos& position);
    void toggleLockRequested(const MapPos& position);
    void lockSelectionRequested(const QList<MapPos>& positions);
    void unlockSelectionRequested(const QList<MapPos>& positions);
    void lockAreaRequested(const MapPos& topLeft, const MapPos& bottomRight);
    void unlockAreaRequested(const MapPos& topLeft, const MapPos& bottomRight);

private slots:
    void onLockTileTriggered();
    void onUnlockTileTriggered();
    void onToggleLockTriggered();
    void onLockSelectionTriggered();
    void onUnlockSelectionTriggered();
    void onLockAreaTriggered();
    void onUnlockAreaTriggered();

private:
    TileLockActions* actions_;
    MapPos currentPosition_;
    QList<MapPos> currentSelection_;
    MapPos currentAreaTopLeft_;
    MapPos currentAreaBottomRight_;
    
    void setupMenuActions(QMenu* menu);
    bool isTileLocked(const MapPos& position) const;
    int getLockedTileCount(const QList<MapPos>& positions) const;
    int getUnlockedTileCount(const QList<MapPos>& positions) const;
};

/**
 * @brief Tile lock status widget for status bar
 */
class TileLockStatusWidget : public QObject
{
    Q_OBJECT

public:
    explicit TileLockStatusWidget(TileLockManager* manager, QObject* parent = nullptr);
    ~TileLockStatusWidget() override = default;

    // Widget creation
    QWidget* createStatusWidget(QWidget* parent = nullptr);
    
    // Status updates
    void updateStatus();
    void updateStatus(const QString& message);

signals:
    void statusClicked();
    void statisticsRequested();

public slots:
    void onLockStateChanged();
    void onStatisticsChanged();
    void onTileLocked(const MapPos& position, const QString& reason);
    void onTileUnlocked(const MapPos& position, const QString& reason);

private slots:
    void onStatusWidgetClicked();

private:
    TileLockManager* tileLockManager_;
    QWidget* statusWidget_;
    QLabel* statusLabel_;
    QPushButton* statisticsButton_;
    
    void setupStatusWidget();
    void updateStatusText();
    QString formatLockStatistics() const;
};

/**
 * @brief Tile lock keyboard handler
 */
class TileLockKeyboardHandler : public QObject
{
    Q_OBJECT

public:
    explicit TileLockKeyboardHandler(TileLockActions* actions, QObject* parent = nullptr);
    ~TileLockKeyboardHandler() override = default;

    // Keyboard handling
    bool handleKeyPress(QKeyEvent* event);
    bool handleKeyRelease(QKeyEvent* event);
    
    // Shortcut management
    void registerShortcuts(QWidget* widget);
    void unregisterShortcuts(QWidget* widget);
    
    // Configuration
    void setShortcutsEnabled(bool enabled);
    bool areShortcutsEnabled() const;

signals:
    void lockShortcutActivated();
    void unlockShortcutActivated();
    void toggleLockShortcutActivated();
    void lockSelectionShortcutActivated();
    void unlockSelectionShortcutActivated();
    void lockAllShortcutActivated();
    void unlockAllShortcutActivated();

private slots:
    void onShortcutActivated();

private:
    TileLockActions* actions_;
    bool shortcutsEnabled_;
    
    QList<QShortcut*> shortcuts_;
    
    void createShortcuts();
    void setupShortcutConnections();
    QShortcut* createShortcut(const QKeySequence& key, QWidget* parent, const char* slot);
};

#endif // TILELOCKACTIONS_H
