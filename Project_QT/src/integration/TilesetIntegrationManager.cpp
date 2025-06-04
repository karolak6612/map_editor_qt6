#include "TilesetIntegrationManager.h"
#include "../TilesetManager.h"
#include "../TilesetBehaviorManager.h"
#include "../ItemManager.h"
#include "../BrushManager.h"
#include "../ui/MainPalette.h"
#include "../ui/BrushPalettePanel.h"
#include "../ui/TilesetEditorDialog.h"
#include "../Tileset.h"
#include <QDebug>
#include <QTimer>
#include <QMutexLocker>
#include <QApplication>
#include <QThread>
#include <QDateTime>
#include <QQueue>
#include <QHash>
#include <QList>
#include <QString>
#include <QVariantMap>

// Static constants for integration management (matching wxwidgets)
const int TilesetIntegrationManager::DEFAULT_UPDATE_INTERVAL = 100; // 100ms
const int TilesetIntegrationManager::DEFAULT_BATCH_SIZE = 50;
const int TilesetIntegrationManager::DEFAULT_THROTTLE_DELAY = 50; // 50ms
const int TilesetIntegrationManager::MAX_EVENT_QUEUE_SIZE = 1000;

// TilesetIntegrationEvent Implementation
TilesetIntegrationEvent::TilesetIntegrationEvent()
    : type(TilesetEventType::UNKNOWN)
    , timestamp(QDateTime::currentMSecsSinceEpoch())
{
}

TilesetIntegrationEvent::TilesetIntegrationEvent(TilesetEventType eventType, const QString& tileset, const QString& category, const QString& brush, quint16 item)
    : type(eventType)
    , tilesetName(tileset)
    , categoryName(category)
    , brushName(brush)
    , itemId(item)
    , timestamp(QDateTime::currentMSecsSinceEpoch())
{
}

bool TilesetIntegrationEvent::isValid() const {
    return type != TilesetEventType::UNKNOWN && !tilesetName.isEmpty();
}

QString TilesetIntegrationEvent::toString() const {
    return QString("TilesetEvent[type=%1, tileset=%2, category=%3, brush=%4, item=%5]")
           .arg(static_cast<int>(type))
           .arg(tilesetName)
           .arg(categoryName)
           .arg(brushName)
           .arg(itemId);
}

// TilesetIntegrationManager Implementation
TilesetIntegrationManager::TilesetIntegrationManager(QObject* parent)
    : QObject(parent)
    , tilesetManager_(nullptr)
    , behaviorManager_(nullptr)
    , itemManager_(nullptr)
    , brushManager_(nullptr)
    , mainPalette_(nullptr)
    , brushPalettePanel_(nullptr)
    , tilesetEditorDialog_(nullptr)
    , integrationEnabled_(true)
    , paletteSyncEnabled_(true)
    , behaviorSyncEnabled_(true)
    , editorSyncEnabled_(true)
    , updateInterval_(DEFAULT_UPDATE_INTERVAL)
    , batchSize_(DEFAULT_BATCH_SIZE)
    , throttleDelay_(DEFAULT_THROTTLE_DELAY)
    , syncInProgress_(false)
{
    // Initialize timers
    updateTimer_ = new QTimer(this);
    updateTimer_->setInterval(updateInterval_);
    updateTimer_->setSingleShot(false);
    connect(updateTimer_, &QTimer::timeout, this, &TilesetIntegrationManager::onUpdateTimer);
    
    throttleTimer_ = new QTimer(this);
    throttleTimer_->setInterval(throttleDelay_);
    throttleTimer_->setSingleShot(true);
    connect(throttleTimer_, &QTimer::timeout, this, &TilesetIntegrationManager::onThrottleTimer);
    
    // Start update timer
    updateTimer_->start();
    
    qDebug() << "TilesetIntegrationManager: Initialized with update interval" << updateInterval_ << "ms";
}

TilesetIntegrationManager::~TilesetIntegrationManager() {
    // Stop timers
    if (updateTimer_) {
        updateTimer_->stop();
    }
    if (throttleTimer_) {
        throttleTimer_->stop();
    }
    
    // Clear event queue
    QMutexLocker locker(&eventMutex_);
    eventQueue_.clear();
}

// Component registration (matching wxwidgets integration patterns)
void TilesetIntegrationManager::setTilesetManager(TilesetManager* manager) {
    if (tilesetManager_ == manager) return;
    
    // Disconnect old manager
    if (tilesetManager_) {
        disconnect(tilesetManager_, nullptr, this, nullptr);
    }
    
    tilesetManager_ = manager;
    
    // Connect new manager signals (matching wxwidgets tileset events)
    if (tilesetManager_) {
        connect(tilesetManager_, &TilesetManager::tilesetAdded, this, [this](const QString& name) {
            queueEvent(TilesetIntegrationEvent(TilesetEventType::TILESET_ADDED, name));
        });
        
        connect(tilesetManager_, &TilesetManager::tilesetRemoved, this, [this](const QString& name) {
            queueEvent(TilesetIntegrationEvent(TilesetEventType::TILESET_REMOVED, name));
        });
        
        connect(tilesetManager_, &TilesetManager::categoryAdded, this, [this](const QString& tileset, const QString& category) {
            queueEvent(TilesetIntegrationEvent(TilesetEventType::CATEGORY_ADDED, tileset, category));
        });
        
        connect(tilesetManager_, &TilesetManager::categoryRemoved, this, [this](const QString& tileset, const QString& category) {
            queueEvent(TilesetIntegrationEvent(TilesetEventType::CATEGORY_REMOVED, tileset, category));
        });
        
        connect(tilesetManager_, &TilesetManager::categoryChanged, this, [this](const QString& tileset, const QString& category) {
            queueEvent(TilesetIntegrationEvent(TilesetEventType::CATEGORY_CHANGED, tileset, category));
        });
        
        qDebug() << "TilesetIntegrationManager: Connected to TilesetManager";
    }
}

void TilesetIntegrationManager::setTilesetBehaviorManager(TilesetBehaviorManager* manager) {
    if (behaviorManager_ == manager) return;
    
    // Disconnect old manager
    if (behaviorManager_) {
        disconnect(behaviorManager_, nullptr, this, nullptr);
    }
    
    behaviorManager_ = manager;
    
    // Connect new manager signals
    if (behaviorManager_) {
        // Connect behavior change signals if available
        qDebug() << "TilesetIntegrationManager: Connected to TilesetBehaviorManager";
    }
}

void TilesetIntegrationManager::setItemManager(ItemManager* manager) {
    if (itemManager_ == manager) return;
    
    // Disconnect old manager
    if (itemManager_) {
        disconnect(itemManager_, nullptr, this, nullptr);
    }
    
    itemManager_ = manager;
    
    // Connect new manager signals
    if (itemManager_) {
        // Connect item change signals if available
        qDebug() << "TilesetIntegrationManager: Connected to ItemManager";
    }
}

void TilesetIntegrationManager::setBrushManager(BrushManager* manager) {
    if (brushManager_ == manager) return;
    
    // Disconnect old manager
    if (brushManager_) {
        disconnect(brushManager_, nullptr, this, nullptr);
    }
    
    brushManager_ = manager;
    
    // Connect new manager signals (matching wxwidgets brush events)
    if (brushManager_) {
        // Connect brush assignment/unassignment signals if available
        qDebug() << "TilesetIntegrationManager: Connected to BrushManager";
    }
}

void TilesetIntegrationManager::setMainPalette(MainPalette* palette) {
    if (mainPalette_ == palette) return;
    
    // Disconnect old palette
    if (mainPalette_) {
        disconnect(mainPalette_, nullptr, this, nullptr);
    }
    
    mainPalette_ = palette;
    
    // Connect new palette signals
    if (mainPalette_) {
        // Connect palette refresh signals if available
        qDebug() << "TilesetIntegrationManager: Connected to MainPalette";
    }
}

void TilesetIntegrationManager::setBrushPalettePanel(BrushPalettePanel* panel) {
    if (brushPalettePanel_ == panel) return;
    
    // Disconnect old panel
    if (brushPalettePanel_) {
        disconnect(brushPalettePanel_, nullptr, this, nullptr);
    }
    
    brushPalettePanel_ = panel;
    
    // Connect new panel signals
    if (brushPalettePanel_) {
        // Connect brush palette refresh signals if available
        qDebug() << "TilesetIntegrationManager: Connected to BrushPalettePanel";
    }
}

void TilesetIntegrationManager::setTilesetEditorDialog(TilesetEditorDialog* dialog) {
    if (tilesetEditorDialog_ == dialog) return;
    
    // Disconnect old dialog
    if (tilesetEditorDialog_) {
        disconnect(tilesetEditorDialog_, nullptr, this, nullptr);
    }
    
    tilesetEditorDialog_ = dialog;
    
    // Connect new dialog signals
    if (tilesetEditorDialog_) {
        // Connect editor change signals if available
        qDebug() << "TilesetIntegrationManager: Connected to TilesetEditorDialog";
    }
}

// Integration control
void TilesetIntegrationManager::enableIntegration(bool enabled) {
    if (integrationEnabled_ == enabled) return;
    
    integrationEnabled_ = enabled;
    
    if (enabled) {
        updateTimer_->start();
        qDebug() << "TilesetIntegrationManager: Integration enabled";
    } else {
        updateTimer_->stop();
        throttleTimer_->stop();
        qDebug() << "TilesetIntegrationManager: Integration disabled";
    }
}

void TilesetIntegrationManager::enablePaletteSync(bool enabled) {
    paletteSyncEnabled_ = enabled;
    qDebug() << "TilesetIntegrationManager: Palette sync" << (enabled ? "enabled" : "disabled");
}

void TilesetIntegrationManager::enableBehaviorSync(bool enabled) {
    behaviorSyncEnabled_ = enabled;
    qDebug() << "TilesetIntegrationManager: Behavior sync" << (enabled ? "enabled" : "disabled");
}

void TilesetIntegrationManager::enableEditorSync(bool enabled) {
    editorSyncEnabled_ = enabled;
    qDebug() << "TilesetIntegrationManager: Editor sync" << (enabled ? "enabled" : "disabled");
}

// Update control
void TilesetIntegrationManager::setUpdateInterval(int milliseconds) {
    updateInterval_ = qMax(10, milliseconds); // Minimum 10ms
    updateTimer_->setInterval(updateInterval_);
    qDebug() << "TilesetIntegrationManager: Update interval set to" << updateInterval_ << "ms";
}

void TilesetIntegrationManager::setBatchSize(int maxEvents) {
    batchSize_ = qMax(1, maxEvents); // Minimum 1 event
    qDebug() << "TilesetIntegrationManager: Batch size set to" << batchSize_;
}

void TilesetIntegrationManager::setThrottleDelay(int milliseconds) {
    throttleDelay_ = qMax(0, milliseconds);
    throttleTimer_->setInterval(throttleDelay_);
    qDebug() << "TilesetIntegrationManager: Throttle delay set to" << throttleDelay_ << "ms";
}

// Manual synchronization (matching wxwidgets tileset refresh patterns)
void TilesetIntegrationManager::syncMainPalette() {
    if (!integrationEnabled_ || !paletteSyncEnabled_ || !mainPalette_) {
        return;
    }

    qDebug() << "TilesetIntegrationManager: Syncing MainPalette";

    // Refresh main palette display
    refreshMainPaletteDisplay();

    emit paletteUpdateRequired();
}

void TilesetIntegrationManager::syncBrushPalette() {
    if (!integrationEnabled_ || !paletteSyncEnabled_ || !brushPalettePanel_) {
        return;
    }

    qDebug() << "TilesetIntegrationManager: Syncing BrushPalette";

    // Refresh brush palette display
    refreshBrushPaletteDisplay();
}

void TilesetIntegrationManager::syncTilesetEditor() {
    if (!integrationEnabled_ || !editorSyncEnabled_ || !tilesetEditorDialog_) {
        return;
    }

    qDebug() << "TilesetIntegrationManager: Syncing TilesetEditor";

    // Refresh tileset editor display
    refreshTilesetEditorDisplay();

    emit editorUpdateRequired();
}

void TilesetIntegrationManager::syncBehaviors() {
    if (!integrationEnabled_ || !behaviorSyncEnabled_ || !behaviorManager_) {
        return;
    }

    qDebug() << "TilesetIntegrationManager: Syncing Behaviors";

    // Refresh behavior synchronization
    // This would update any behavior-related UI components
}

void TilesetIntegrationManager::syncAll() {
    if (!integrationEnabled_) {
        return;
    }

    emit syncStarted();

    syncMainPalette();
    syncBrushPalette();
    syncTilesetEditor();
    syncBehaviors();

    emit syncCompleted();
}

void TilesetIntegrationManager::forceUpdate() {
    // Clear event queue and force immediate sync
    QMutexLocker locker(&eventMutex_);
    eventQueue_.clear();
    locker.unlock();

    syncAll();
}

// Event processing (matching wxwidgets event handling)
void TilesetIntegrationManager::queueEvent(const TilesetIntegrationEvent& event) {
    if (!integrationEnabled_ || !event.isValid()) {
        return;
    }

    QMutexLocker locker(&eventMutex_);

    // Check queue size limit
    if (eventQueue_.size() >= MAX_EVENT_QUEUE_SIZE) {
        qWarning() << "TilesetIntegrationManager: Event queue full, dropping oldest events";
        // Remove oldest events
        while (eventQueue_.size() >= MAX_EVENT_QUEUE_SIZE * 0.8) {
            eventQueue_.dequeue();
        }
    }

    eventQueue_.enqueue(event);

    // Start throttle timer if not already running
    if (throttleDelay_ > 0 && !throttleTimer_->isActive()) {
        throttleTimer_->start();
    }
}

void TilesetIntegrationManager::processEvent(const TilesetIntegrationEvent& event) {
    if (!integrationEnabled_ || syncInProgress_) {
        return;
    }

    switch (event.type) {
        case TilesetEventType::TILESET_ADDED:
            syncTilesetAdded(event.tilesetName);
            break;
        case TilesetEventType::TILESET_REMOVED:
            syncTilesetRemoved(event.tilesetName);
            break;
        case TilesetEventType::CATEGORY_ADDED:
            syncCategoryAdded(event.tilesetName, event.categoryName);
            break;
        case TilesetEventType::CATEGORY_REMOVED:
            syncCategoryRemoved(event.tilesetName, event.categoryName);
            break;
        case TilesetEventType::CATEGORY_CHANGED:
            syncCategoryChanged(event.tilesetName, event.categoryName);
            break;
        case TilesetEventType::ITEM_ADDED:
            syncItemAdded(event.tilesetName, event.categoryName, event.itemId);
            break;
        case TilesetEventType::ITEM_REMOVED:
            syncItemRemoved(event.tilesetName, event.categoryName, event.itemId);
            break;
        case TilesetEventType::BRUSH_ASSIGNED:
            syncBrushAssignment(event.tilesetName, event.categoryName, event.brushName);
            break;
        case TilesetEventType::BRUSH_UNASSIGNED:
            syncBrushUnassignment(event.tilesetName, event.categoryName, event.brushName);
            break;
        case TilesetEventType::BEHAVIOR_CHANGED:
            syncBehaviorChange(event.tilesetName);
            break;
        case TilesetEventType::PALETTE_REFRESH:
            syncPaletteRefresh();
            break;
        default:
            qWarning() << "TilesetIntegrationManager: Unknown event type:" << static_cast<int>(event.type);
            break;
    }
}

void TilesetIntegrationManager::processBatchedEvents(const QList<TilesetIntegrationEvent>& events) {
    if (!integrationEnabled_ || events.isEmpty()) {
        return;
    }

    syncInProgress_ = true;

    // Group events by type for efficient processing
    QHash<TilesetEventType, QList<TilesetIntegrationEvent>> eventGroups;
    for (const TilesetIntegrationEvent& event : events) {
        eventGroups[event.type].append(event);
    }

    // Process events in priority order
    QList<TilesetEventType> processingOrder = {
        TilesetEventType::TILESET_ADDED,
        TilesetEventType::CATEGORY_ADDED,
        TilesetEventType::ITEM_ADDED,
        TilesetEventType::BRUSH_ASSIGNED,
        TilesetEventType::CATEGORY_CHANGED,
        TilesetEventType::BEHAVIOR_CHANGED,
        TilesetEventType::BRUSH_UNASSIGNED,
        TilesetEventType::ITEM_REMOVED,
        TilesetEventType::CATEGORY_REMOVED,
        TilesetEventType::TILESET_REMOVED,
        TilesetEventType::PALETTE_REFRESH
    };

    for (TilesetEventType eventType : processingOrder) {
        if (eventGroups.contains(eventType)) {
            for (const TilesetIntegrationEvent& event : eventGroups[eventType]) {
                processEvent(event);
            }
        }
    }

    syncInProgress_ = false;
}

// Specific synchronization methods (matching wxwidgets tileset operations)
void TilesetIntegrationManager::syncTilesetAdded(const QString& tilesetName) {
    qDebug() << "TilesetIntegrationManager: Syncing tileset added:" << tilesetName;

    if (paletteSyncEnabled_) {
        updateMainPaletteTileset(tilesetName);
        updateBrushPaletteTileset(tilesetName);
    }

    if (editorSyncEnabled_) {
        updateTilesetEditorTileset(tilesetName);
    }
}

void TilesetIntegrationManager::syncTilesetRemoved(const QString& tilesetName) {
    qDebug() << "TilesetIntegrationManager: Syncing tileset removed:" << tilesetName;

    if (paletteSyncEnabled_) {
        refreshMainPaletteDisplay();
        refreshBrushPaletteDisplay();
    }

    if (editorSyncEnabled_) {
        refreshTilesetEditorDisplay();
    }
}

void TilesetIntegrationManager::syncCategoryAdded(const QString& tilesetName, const QString& categoryName) {
    qDebug() << "TilesetIntegrationManager: Syncing category added:" << tilesetName << "/" << categoryName;

    if (paletteSyncEnabled_) {
        updateMainPaletteCategory(tilesetName, categoryName);
        updateBrushPaletteCategory(tilesetName, categoryName);
    }

    if (editorSyncEnabled_) {
        updateTilesetEditorCategory(tilesetName, categoryName);
    }
}

void TilesetIntegrationManager::syncCategoryRemoved(const QString& tilesetName, const QString& categoryName) {
    qDebug() << "TilesetIntegrationManager: Syncing category removed:" << tilesetName << "/" << categoryName;

    if (paletteSyncEnabled_) {
        refreshMainPaletteDisplay();
        refreshBrushPaletteDisplay();
    }

    if (editorSyncEnabled_) {
        refreshTilesetEditorDisplay();
    }
}

void TilesetIntegrationManager::syncCategoryChanged(const QString& tilesetName, const QString& categoryName) {
    qDebug() << "TilesetIntegrationManager: Syncing category changed:" << tilesetName << "/" << categoryName;

    if (paletteSyncEnabled_) {
        updateMainPaletteCategory(tilesetName, categoryName);
        updateBrushPaletteCategory(tilesetName, categoryName);
    }

    if (editorSyncEnabled_) {
        updateTilesetEditorCategory(tilesetName, categoryName);
    }
}

void TilesetIntegrationManager::syncItemAdded(const QString& tilesetName, const QString& categoryName, quint16 itemId) {
    qDebug() << "TilesetIntegrationManager: Syncing item added:" << tilesetName << "/" << categoryName << "/" << itemId;

    if (paletteSyncEnabled_) {
        updateMainPaletteItem(itemId);
    }
}

void TilesetIntegrationManager::syncItemRemoved(const QString& tilesetName, const QString& categoryName, quint16 itemId) {
    qDebug() << "TilesetIntegrationManager: Syncing item removed:" << tilesetName << "/" << categoryName << "/" << itemId;

    if (paletteSyncEnabled_) {
        updateMainPaletteCategory(tilesetName, categoryName);
    }
}

void TilesetIntegrationManager::syncBrushAssignment(const QString& tilesetName, const QString& categoryName, const QString& brushName) {
    qDebug() << "TilesetIntegrationManager: Syncing brush assigned:" << tilesetName << "/" << categoryName << "/" << brushName;

    if (paletteSyncEnabled_) {
        updateBrushPaletteBrush(brushName);
    }
}

void TilesetIntegrationManager::syncBrushUnassignment(const QString& tilesetName, const QString& categoryName, const QString& brushName) {
    qDebug() << "TilesetIntegrationManager: Syncing brush unassigned:" << tilesetName << "/" << categoryName << "/" << brushName;

    if (paletteSyncEnabled_) {
        refreshBrushPaletteDisplay();
    }
}

void TilesetIntegrationManager::syncBehaviorChange(const QString& tilesetName) {
    qDebug() << "TilesetIntegrationManager: Syncing behavior change:" << tilesetName;

    if (behaviorSyncEnabled_) {
        // Update behavior-related UI components
    }
}

void TilesetIntegrationManager::syncPaletteRefresh() {
    qDebug() << "TilesetIntegrationManager: Syncing palette refresh";

    if (paletteSyncEnabled_) {
        refreshMainPaletteDisplay();
        refreshBrushPaletteDisplay();
    }
}

// UI integration methods (matching wxwidgets palette integration)
void TilesetIntegrationManager::updateMainPaletteTileset(const QString& tilesetName) {
    if (!mainPalette_ || !tilesetManager_) return;

    // Update main palette with new tileset
    // This would call mainPalette_->addTileset(tilesetName) or similar
    qDebug() << "TilesetIntegrationManager: Updating MainPalette tileset:" << tilesetName;
}

void TilesetIntegrationManager::updateMainPaletteCategory(const QString& tilesetName, const QString& categoryName) {
    if (!mainPalette_ || !tilesetManager_) return;

    // Update main palette category
    // This would call mainPalette_->updateCategory(tilesetName, categoryName) or similar
    qDebug() << "TilesetIntegrationManager: Updating MainPalette category:" << tilesetName << "/" << categoryName;
}

void TilesetIntegrationManager::updateMainPaletteItem(quint16 itemId) {
    if (!mainPalette_ || !itemManager_) return;

    // Update main palette item
    // This would call mainPalette_->updateItem(itemId) or similar
    qDebug() << "TilesetIntegrationManager: Updating MainPalette item:" << itemId;
}

void TilesetIntegrationManager::refreshMainPaletteDisplay() {
    if (!mainPalette_) return;

    // Refresh main palette display
    // This would call mainPalette_->refresh() or similar
    qDebug() << "TilesetIntegrationManager: Refreshing MainPalette display";
}

void TilesetIntegrationManager::updateBrushPaletteTileset(const QString& tilesetName) {
    if (!brushPalettePanel_ || !tilesetManager_) return;

    // Update brush palette with new tileset
    qDebug() << "TilesetIntegrationManager: Updating BrushPalette tileset:" << tilesetName;
}

void TilesetIntegrationManager::updateBrushPaletteCategory(const QString& tilesetName, const QString& categoryName) {
    if (!brushPalettePanel_ || !tilesetManager_) return;

    // Update brush palette category
    qDebug() << "TilesetIntegrationManager: Updating BrushPalette category:" << tilesetName << "/" << categoryName;
}

void TilesetIntegrationManager::updateBrushPaletteBrush(const QString& brushName) {
    if (!brushPalettePanel_ || !brushManager_) return;

    // Update brush palette brush
    qDebug() << "TilesetIntegrationManager: Updating BrushPalette brush:" << brushName;
}

void TilesetIntegrationManager::refreshBrushPaletteDisplay() {
    if (!brushPalettePanel_) return;

    // Refresh brush palette display
    qDebug() << "TilesetIntegrationManager: Refreshing BrushPalette display";
}

void TilesetIntegrationManager::updateTilesetEditorTileset(const QString& tilesetName) {
    if (!tilesetEditorDialog_ || !tilesetManager_) return;

    // Update tileset editor with new tileset
    qDebug() << "TilesetIntegrationManager: Updating TilesetEditor tileset:" << tilesetName;
}

void TilesetIntegrationManager::updateTilesetEditorCategory(const QString& tilesetName, const QString& categoryName) {
    if (!tilesetEditorDialog_ || !tilesetManager_) return;

    // Update tileset editor category
    qDebug() << "TilesetIntegrationManager: Updating TilesetEditor category:" << tilesetName << "/" << categoryName;
}

void TilesetIntegrationManager::refreshTilesetEditorDisplay() {
    if (!tilesetEditorDialog_) return;

    // Refresh tileset editor display
    qDebug() << "TilesetIntegrationManager: Refreshing TilesetEditor display";
}

// Slots
void TilesetIntegrationManager::onUpdateTimer() {
    if (!integrationEnabled_ || syncInProgress_) {
        return;
    }

    QMutexLocker locker(&eventMutex_);

    if (eventQueue_.isEmpty()) {
        return;
    }

    // Process events in batches
    QList<TilesetIntegrationEvent> batch;
    int processedCount = 0;

    while (!eventQueue_.isEmpty() && processedCount < batchSize_) {
        batch.append(eventQueue_.dequeue());
        processedCount++;
    }

    locker.unlock();

    if (!batch.isEmpty()) {
        processBatchedEvents(batch);
    }
}

void TilesetIntegrationManager::onThrottleTimer() {
    // Throttle timer expired, allow processing
    // This helps prevent excessive updates during rapid changes
}

// TilesetPaletteSynchronizer Implementation
TilesetPaletteSynchronizer::TilesetPaletteSynchronizer(QObject* parent)
    : QObject(parent)
    , tilesetManager_(nullptr)
    , mainPalette_(nullptr)
    , syncEnabled_(true)
    , refreshMode_(0)
{
}

void TilesetPaletteSynchronizer::setTilesetManager(TilesetManager* manager) {
    if (tilesetManager_ == manager) return;

    // Disconnect old manager
    if (tilesetManager_) {
        disconnect(tilesetManager_, nullptr, this, nullptr);
    }

    tilesetManager_ = manager;

    // Connect new manager signals
    if (tilesetManager_) {
        connect(tilesetManager_, &TilesetManager::tilesetAdded, this, &TilesetPaletteSynchronizer::onTilesetChanged);
        connect(tilesetManager_, &TilesetManager::categoryChanged, this, &TilesetPaletteSynchronizer::onCategoryChanged);
    }
}

void TilesetPaletteSynchronizer::setMainPalette(MainPalette* palette) {
    mainPalette_ = palette;
}

void TilesetPaletteSynchronizer::enableSync(bool enabled) {
    syncEnabled_ = enabled;
}

void TilesetPaletteSynchronizer::setRefreshMode(int mode) {
    refreshMode_ = mode;
}

void TilesetPaletteSynchronizer::syncTilesetToCategory(const QString& tilesetName, const QString& categoryName) {
    if (!syncEnabled_ || !tilesetManager_ || !mainPalette_) return;

    qDebug() << "TilesetPaletteSynchronizer: Syncing tileset to category:" << tilesetName << "/" << categoryName;

    // Get tileset and update palette category
    Tileset* tileset = tilesetManager_->getTileset(tilesetName);
    if (tileset) {
        // This would update the main palette with tileset data
        emit categoryRefreshed(categoryName);
    }
}

void TilesetPaletteSynchronizer::syncCategoryToItems(const QString& tilesetName, const QString& categoryName) {
    if (!syncEnabled_ || !tilesetManager_ || !mainPalette_) return;

    qDebug() << "TilesetPaletteSynchronizer: Syncing category to items:" << tilesetName << "/" << categoryName;

    // Get category items and update palette
    // This would iterate through category items and update the palette
}

void TilesetPaletteSynchronizer::syncItemToPalette(quint16 itemId) {
    if (!syncEnabled_ || !mainPalette_) return;

    qDebug() << "TilesetPaletteSynchronizer: Syncing item to palette:" << itemId;

    // Update specific item in palette
    emit itemRefreshed(itemId);
}

void TilesetPaletteSynchronizer::refreshPaletteCategory(const QString& categoryName) {
    if (!syncEnabled_ || !mainPalette_) return;

    qDebug() << "TilesetPaletteSynchronizer: Refreshing palette category:" << categoryName;

    // Refresh specific category in palette
    emit categoryRefreshed(categoryName);
}

void TilesetPaletteSynchronizer::onTilesetChanged(const QString& tilesetName) {
    if (!syncEnabled_) return;

    qDebug() << "TilesetPaletteSynchronizer: Tileset changed:" << tilesetName;

    // Handle tileset change
    refreshAllCategories();
}

void TilesetPaletteSynchronizer::onCategoryChanged(const QString& tilesetName, const QString& categoryName) {
    if (!syncEnabled_) return;

    qDebug() << "TilesetPaletteSynchronizer: Category changed:" << tilesetName << "/" << categoryName;

    // Handle category change
    refreshPaletteCategory(categoryName);
}

void TilesetPaletteSynchronizer::onItemChanged(quint16 itemId) {
    if (!syncEnabled_) return;

    qDebug() << "TilesetPaletteSynchronizer: Item changed:" << itemId;

    // Handle item change
    syncItemToPalette(itemId);
}

void TilesetPaletteSynchronizer::refreshAllCategories() {
    if (!syncEnabled_ || !mainPalette_) return;

    qDebug() << "TilesetPaletteSynchronizer: Refreshing all categories";

    // Refresh entire palette
    emit paletteRefreshed();
}

void TilesetPaletteSynchronizer::updatePaletteCategory(const QString& categoryName, const QList<quint16>& itemIds) {
    if (!syncEnabled_ || !mainPalette_) return;

    qDebug() << "TilesetPaletteSynchronizer: Updating palette category:" << categoryName << "with" << itemIds.size() << "items";

    // Update palette category with item list
    // This would call mainPalette_->updateCategory(categoryName, itemIds) or similar
}

void TilesetPaletteSynchronizer::updatePaletteItem(quint16 itemId, const QString& categoryName) {
    if (!syncEnabled_ || !mainPalette_) return;

    qDebug() << "TilesetPaletteSynchronizer: Updating palette item:" << itemId << "in category:" << categoryName;

    // Update specific item in palette
    // This would call mainPalette_->updateItem(itemId, categoryName) or similar
}

// TilesetEditorSynchronizer Implementation
TilesetEditorSynchronizer::TilesetEditorSynchronizer(QObject* parent)
    : QObject(parent)
    , tilesetManager_(nullptr)
    , tilesetEditorDialog_(nullptr)
    , syncEnabled_(true)
{
}

void TilesetEditorSynchronizer::setTilesetManager(TilesetManager* manager) {
    if (tilesetManager_ == manager) return;

    // Disconnect old manager
    if (tilesetManager_) {
        disconnect(tilesetManager_, nullptr, this, nullptr);
    }

    tilesetManager_ = manager;

    // Connect new manager signals
    if (tilesetManager_) {
        connect(tilesetManager_, &TilesetManager::tilesetAdded, this, &TilesetEditorSynchronizer::onTilesetManagerChanged);
        connect(tilesetManager_, &TilesetManager::tilesetRemoved, this, &TilesetEditorSynchronizer::onTilesetManagerChanged);
        connect(tilesetManager_, &TilesetManager::categoryChanged, this, &TilesetEditorSynchronizer::onTilesetManagerChanged);
    }
}

void TilesetEditorSynchronizer::setTilesetEditorDialog(TilesetEditorDialog* dialog) {
    if (tilesetEditorDialog_ == dialog) return;

    // Disconnect old dialog
    if (tilesetEditorDialog_) {
        disconnect(tilesetEditorDialog_, nullptr, this, nullptr);
    }

    tilesetEditorDialog_ = dialog;

    // Connect new dialog signals
    if (tilesetEditorDialog_) {
        // Connect editor change signals if available
        // This would depend on the TilesetEditorDialog implementation
    }
}

void TilesetEditorSynchronizer::enableSync(bool enabled) {
    syncEnabled_ = enabled;
}

void TilesetEditorSynchronizer::onTilesetManagerChanged() {
    if (!syncEnabled_) return;

    qDebug() << "TilesetEditorSynchronizer: TilesetManager changed";

    // Sync manager changes to editor
    syncManagerToEditor();
}

void TilesetEditorSynchronizer::onEditorDialogChanged() {
    if (!syncEnabled_) return;

    qDebug() << "TilesetEditorSynchronizer: EditorDialog changed";

    // Sync editor changes to manager
    syncEditorToManager();
}

void TilesetEditorSynchronizer::syncEditorToManager() {
    if (!syncEnabled_ || !tilesetManager_ || !tilesetEditorDialog_) return;

    qDebug() << "TilesetEditorSynchronizer: Syncing editor to manager";

    // Sync editor state to tileset manager
    // This would read editor state and update the tileset manager

    emit managerSynced();
}

void TilesetEditorSynchronizer::syncManagerToEditor() {
    if (!syncEnabled_ || !tilesetManager_ || !tilesetEditorDialog_) return;

    qDebug() << "TilesetEditorSynchronizer: Syncing manager to editor";

    // Sync tileset manager state to editor
    // This would read manager state and update the editor dialog

    emit editorSynced();
}
