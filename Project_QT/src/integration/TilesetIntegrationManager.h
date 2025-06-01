#ifndef TILESETINTEGRATIONMANAGER_H
#define TILESETINTEGRATIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QVariantMap>
#include <QStringList>
#include <QElapsedTimer>
#include <QQueue>

#include "TilesetManager.h"
#include "managers/TilesetBehaviorManager.h"

// Forward declarations
class MainPalette;
class BrushPalettePanel;
class TilesetEditorDialog;
class ItemManager;
class BrushManager;
class Map;
class MapView;

/**
 * @brief Task 91: Tileset Integration Manager
 * 
 * Complete integration system for tileset management with UI components:
 * - MainPalette synchronization with tileset changes
 * - BrushPalettePanel integration for tileset display
 * - TilesetEditorDialog integration for advanced editing
 * - Real-time tileset updates and notifications
 * - Performance optimization for large tilesets
 * - Event batching and throttling for smooth operation
 * - 1:1 compatibility with wxwidgets tileset integration
 */

/**
 * @brief Tileset Integration Event for batched updates
 */
struct TilesetIntegrationEvent {
    enum Type {
        TILESET_ADDED,
        TILESET_REMOVED,
        TILESET_MODIFIED,
        CATEGORY_ADDED,
        CATEGORY_REMOVED,
        CATEGORY_MODIFIED,
        ITEM_ASSIGNED,
        ITEM_UNASSIGNED,
        BRUSH_ASSIGNED,
        BRUSH_UNASSIGNED,
        BEHAVIOR_CHANGED,
        PALETTE_REFRESH_REQUESTED
    };
    
    Type type;
    QString tilesetName;
    QString categoryName;
    QVariantMap data;
    qint64 timestamp;
    int priority;
    
    TilesetIntegrationEvent(Type t = TILESET_MODIFIED, const QString& tileset = QString(), 
                           const QString& category = QString(), int p = 0)
        : type(t), tilesetName(tileset), categoryName(category), priority(p) {
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    
    bool operator<(const TilesetIntegrationEvent& other) const {
        if (priority != other.priority) return priority < other.priority;
        return timestamp < other.timestamp;
    }
};

/**
 * @brief Main Tileset Integration Manager
 */
class TilesetIntegrationManager : public QObject
{
    Q_OBJECT

public:
    explicit TilesetIntegrationManager(QObject* parent = nullptr);
    ~TilesetIntegrationManager() override;

    // Component registration
    void setTilesetManager(TilesetManager* manager);
    void setTilesetBehaviorManager(TilesetBehaviorManager* manager);
    void setItemManager(ItemManager* manager);
    void setBrushManager(BrushManager* manager);
    void setMainPalette(MainPalette* palette);
    void setBrushPalettePanel(BrushPalettePanel* panel);
    void setTilesetEditorDialog(TilesetEditorDialog* dialog);

    // Integration control
    void enableIntegration(bool enabled);
    bool isIntegrationEnabled() const { return integrationEnabled_; }
    
    void enablePaletteSync(bool enabled);
    bool isPaletteSyncEnabled() const { return paletteSyncEnabled_; }
    
    void enableBehaviorSync(bool enabled);
    bool isBehaviorSyncEnabled() const { return behaviorSyncEnabled_; }
    
    void enableEditorSync(bool enabled);
    bool isEditorSyncEnabled() const { return editorSyncEnabled_; }

    // Update control
    void setUpdateInterval(int milliseconds);
    int getUpdateInterval() const { return updateInterval_; }
    
    void setBatchSize(int maxEvents);
    int getBatchSize() const { return batchSize_; }
    
    void setThrottleDelay(int milliseconds);
    int getThrottleDelay() const { return throttleDelay_; }

    // Manual synchronization
    void syncMainPalette();
    void syncBrushPalette();
    void syncTilesetEditor();
    void syncBehaviors();
    void syncAll();
    void forceUpdate();

    // Tileset operations
    void refreshTilesetDisplay(const QString& tilesetName);
    void refreshCategoryDisplay(const QString& tilesetName, const QString& categoryName);
    void refreshItemDisplay(quint16 itemId);
    void refreshBrushDisplay(const QString& brushName);

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    bool isSyncInProgress() const;
    int getPendingEventCount() const;

public slots:
    // TilesetManager event handlers
    void onTilesetAdded(const QString& tilesetName);
    void onTilesetRemoved(const QString& tilesetName);
    void onTilesetModified(const QString& tilesetName);
    void onCategoryAdded(const QString& tilesetName, const QString& categoryName);
    void onCategoryRemoved(const QString& tilesetName, const QString& categoryName);
    void onCategoryModified(const QString& tilesetName, const QString& categoryName);
    void onItemAssigned(const QString& tilesetName, const QString& categoryName, quint16 itemId);
    void onItemUnassigned(const QString& tilesetName, const QString& categoryName, quint16 itemId);
    void onBrushAssigned(const QString& tilesetName, const QString& categoryName, const QString& brushName);
    void onBrushUnassigned(const QString& tilesetName, const QString& categoryName, const QString& brushName);
    
    // TilesetBehaviorManager event handlers
    void onBehaviorRegistered(const QString& tilesetName, TilesetBehavior::Type type);
    void onBehaviorUnregistered(const QString& tilesetName);
    void onBehaviorApplied(const QString& tilesetName, const QPoint& position, quint16 itemId);
    
    // TilesetEditorDialog event handlers
    void onEditorTilesetCreated(const QString& tilesetName);
    void onEditorTilesetModified(const QString& tilesetName);
    void onEditorTilesetDeleted(const QString& tilesetName);
    void onEditorCategoryCreated(const QString& tilesetName, const QString& categoryName);
    void onEditorCategoryModified(const QString& tilesetName, const QString& categoryName);
    void onEditorCategoryDeleted(const QString& tilesetName, const QString& categoryName);
    void onEditorPaletteUpdateRequested();
    
    // Update processing
    void processUpdates();
    void processBatch();

signals:
    void syncStarted();
    void syncCompleted();
    void syncError(const QString& error);
    void paletteUpdateRequired();
    void editorUpdateRequired();
    void statisticsUpdated(const QVariantMap& stats);

private slots:
    void onUpdateTimer();
    void onThrottleTimer();

private:
    // Event processing
    void queueEvent(const TilesetIntegrationEvent& event);
    void processEvent(const TilesetIntegrationEvent& event);
    void processBatchedEvents(const QList<TilesetIntegrationEvent>& events);
    
    // Specific sync operations
    void syncTilesetAddition(const QString& tilesetName);
    void syncTilesetRemoval(const QString& tilesetName);
    void syncTilesetModification(const QString& tilesetName);
    void syncCategoryAddition(const QString& tilesetName, const QString& categoryName);
    void syncCategoryRemoval(const QString& tilesetName, const QString& categoryName);
    void syncCategoryModification(const QString& tilesetName, const QString& categoryName);
    void syncItemAssignment(const QString& tilesetName, const QString& categoryName, quint16 itemId);
    void syncItemUnassignment(const QString& tilesetName, const QString& categoryName, quint16 itemId);
    void syncBrushAssignment(const QString& tilesetName, const QString& categoryName, const QString& brushName);
    void syncBrushUnassignment(const QString& tilesetName, const QString& categoryName, const QString& brushName);
    void syncBehaviorChange(const QString& tilesetName);
    void syncPaletteRefresh();
    
    // MainPalette integration
    void updateMainPaletteTileset(const QString& tilesetName);
    void updateMainPaletteCategory(const QString& tilesetName, const QString& categoryName);
    void updateMainPaletteItem(quint16 itemId);
    void refreshMainPaletteDisplay();
    
    // BrushPalettePanel integration
    void updateBrushPaletteTileset(const QString& tilesetName);
    void updateBrushPaletteCategory(const QString& tilesetName, const QString& categoryName);
    void updateBrushPaletteBrush(const QString& brushName);
    void refreshBrushPaletteDisplay();
    
    // TilesetEditorDialog integration
    void updateEditorTilesetList();
    void updateEditorCategoryList(const QString& tilesetName);
    void updateEditorItemList(const QString& tilesetName, const QString& categoryName);
    void updateEditorBrushList(const QString& tilesetName, const QString& categoryName);
    void refreshEditorDisplay();
    
    // Optimization
    void optimizeEventQueue();
    void mergeEvents(QList<TilesetIntegrationEvent>& events);
    bool canMergeEvents(const TilesetIntegrationEvent& a, const TilesetIntegrationEvent& b) const;
    TilesetIntegrationEvent mergeEvents(const TilesetIntegrationEvent& a, const TilesetIntegrationEvent& b) const;
    
    // Throttling and batching
    bool shouldThrottle(const TilesetIntegrationEvent& event) const;
    void startThrottleTimer();
    void stopThrottleTimer();
    
    // Statistics tracking
    void updateStatistics();
    void trackEvent(const TilesetIntegrationEvent& event);
    void trackSyncTime(qint64 duration);

private:
    // Core components
    TilesetManager* tilesetManager_;
    TilesetBehaviorManager* behaviorManager_;
    ItemManager* itemManager_;
    BrushManager* brushManager_;
    MainPalette* mainPalette_;
    BrushPalettePanel* brushPalettePanel_;
    TilesetEditorDialog* tilesetEditorDialog_;
    
    // Integration settings
    bool integrationEnabled_;
    bool paletteSyncEnabled_;
    bool behaviorSyncEnabled_;
    bool editorSyncEnabled_;
    
    // Update control
    int updateInterval_;
    int batchSize_;
    int throttleDelay_;
    
    // Event processing
    QMutex eventMutex_;
    QQueue<TilesetIntegrationEvent> eventQueue_;
    QTimer* updateTimer_;
    QTimer* throttleTimer_;
    bool syncInProgress_;
    
    // Performance tracking
    mutable QMutex statsMutex_;
    QVariantMap statistics_;
    QElapsedTimer syncTimer_;
    int eventsProcessed_;
    int batchesProcessed_;
    qint64 totalSyncTime_;
    
    // Throttling state
    QElapsedTimer lastSyncTime_;
    QMap<TilesetIntegrationEvent::Type, qint64> lastEventTime_;
    
    // Constants
    static const int DEFAULT_UPDATE_INTERVAL = 100; // milliseconds
    static const int DEFAULT_BATCH_SIZE = 10;
    static const int DEFAULT_THROTTLE_DELAY = 50; // milliseconds
    static const int MAX_QUEUE_SIZE = 1000;
};

/**
 * @brief Tileset Palette Synchronizer for MainPalette integration
 */
class TilesetPaletteSynchronizer : public QObject
{
    Q_OBJECT

public:
    explicit TilesetPaletteSynchronizer(QObject* parent = nullptr);
    ~TilesetPaletteSynchronizer() override = default;

    // Component setup
    void setTilesetManager(TilesetManager* manager);
    void setMainPalette(MainPalette* palette);

    // Synchronization control
    void enableSync(bool enabled);
    bool isSyncEnabled() const { return syncEnabled_; }
    
    void setRefreshMode(int mode);
    int getRefreshMode() const { return refreshMode_; }

    // Manual synchronization
    void syncTilesetToCategory(const QString& tilesetName, const QString& categoryName);
    void syncCategoryToItems(const QString& tilesetName, const QString& categoryName);
    void syncItemToPalette(quint16 itemId);
    void refreshPaletteCategory(const QString& categoryName);

public slots:
    void onTilesetChanged(const QString& tilesetName);
    void onCategoryChanged(const QString& tilesetName, const QString& categoryName);
    void onItemChanged(quint16 itemId);
    void refreshAllCategories();

signals:
    void categoryRefreshed(const QString& categoryName);
    void itemRefreshed(quint16 itemId);
    void paletteRefreshed();

private:
    void updatePaletteCategory(const QString& categoryName, const QList<quint16>& itemIds);
    void updatePaletteItem(quint16 itemId, const QString& categoryName);

private:
    TilesetManager* tilesetManager_;
    MainPalette* mainPalette_;
    bool syncEnabled_;
    int refreshMode_;
};

/**
 * @brief Tileset Editor Synchronizer for TilesetEditorDialog integration
 */
class TilesetEditorSynchronizer : public QObject
{
    Q_OBJECT

public:
    explicit TilesetEditorSynchronizer(QObject* parent = nullptr);
    ~TilesetEditorSynchronizer() override = default;

    // Component setup
    void setTilesetManager(TilesetManager* manager);
    void setTilesetEditorDialog(TilesetEditorDialog* dialog);

    // Synchronization control
    void enableSync(bool enabled);
    bool isSyncEnabled() const { return syncEnabled_; }

public slots:
    void onTilesetManagerChanged();
    void onEditorDialogChanged();
    void syncEditorToManager();
    void syncManagerToEditor();

signals:
    void editorSynced();
    void managerSynced();

private:
    TilesetManager* tilesetManager_;
    TilesetEditorDialog* tilesetEditorDialog_;
    bool syncEnabled_;
};

#endif // TILESETINTEGRATIONMANAGER_H
