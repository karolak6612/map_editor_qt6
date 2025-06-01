#include "BrushManager.h"
#include "Brush.h" // For Brush::name()
#include <QDebug> // Optional: for logging
#include <QVariantMap>
#include <QTimer> // Task 81: Auto-save timer
#include <QStandardPaths> // Task 81: Default directories
#include <QDir> // Task 81: Directory operations
#include "BrushPersistence.h" // Task 81: Brush persistence system

// Include all brush types for factory creation
#include "GroundBrush.h"
#include "DoorBrush.h"
#include "PixelBrush.h"
#include "CarpetBrush.h"
#include "TableBrush.h"
#include "CreatureBrush.h"
#include "SpawnBrush.h"
#include "WallBrush.h"
#include "WallDecorationBrush.h"
#include "EraserBrush.h"
#include "FlagBrush.h"
// Note: Other brush headers will be included as they are implemented

BrushManager::BrushManager(QObject *parent)
    : QObject(parent),
      currentBrush_(nullptr),
      currentBrushShared_(nullptr),
      currentActionId_(0),
      actionIdEnabled_(false),
      selectedItem_(nullptr),
      currentDrawingMode_("None"),
      currentDrawingModeDescription_("No drawing mode selected"),
      // Task 81: Initialize brush persistence system
      brushPersistence_(nullptr),
      autoSaveTimer_(nullptr),
      autoSaveEnabled_(true),
      autoSaveInterval_(5) { // 5 minutes default

    qDebug() << "BrushManager initialized with enhanced functionality.";

    // Initialize default parameters for each brush type
    initializeDefaultParameters();

    // Register brush factories
    registerBrushFactories();

    // Task 81: Initialize brush persistence system
    initializeBrushPersistence();
}

BrushManager::~BrushManager() {
    qDebug() << "BrushManager destroyed. Cleaning up" << brushes_.size() << "raw brushes and"
             << sharedBrushes_.size() << "shared brushes.";

    // Disconnect signals from all brushes before deletion
    for (auto it = brushes_.begin(); it != brushes_.end(); ++it) {
        disconnectBrushSignals(it.value());
    }

    // Clean up raw pointer brushes
    qDeleteAll(brushes_);
    brushes_.clear();

    // Shared brushes will be automatically cleaned up by shared_ptr
    sharedBrushes_.clear();

    currentBrush_ = nullptr;
    currentBrushShared_.reset();

    // Task 81: Clean up brush persistence system
    if (autoSaveTimer_) {
        autoSaveTimer_->stop();
        delete autoSaveTimer_;
        autoSaveTimer_ = nullptr;
    }

    delete brushPersistence_;
    brushPersistence_ = nullptr;
}

void BrushManager::addBrush(Brush* brush) {
    if (!brush || brush->name().isEmpty()) {
        qWarning() << "BrushManager: Attempted to add null or unnamed brush.";
        return;
    }

    if (brushes_.contains(brush->name())) {
        Brush* oldBrush = brushes_.value(brush->name());
        if (oldBrush != brush) {
            // qWarning() << "BrushManager: Replacing existing brush with name:" << brush->name();
            delete oldBrush; // Delete the old brush object
        } else {
            // Same brush instance, do nothing
            return;
        }
    }
    brushes_.insert(brush->name(), brush);
}

void BrushManager::removeBrush(const QString& name) {
    if (!brushes_.contains(name)) {
        // qWarning() << "BrushManager: Attempted to remove non-existent brush:" << name;
        return;
    }

    Brush* brushToRemove = brushes_.value(name);

    if (currentBrush_ == brushToRemove) {
        setCurrentBrush(static_cast<Brush*>(nullptr)); // Explicitly cast nullptr if required by compiler/strictness, or just pass nullptr
    }

    brushes_.remove(name);
    delete brushToRemove; // Delete the brush object itself
    brushToRemove = nullptr; // Good practice
}

Brush* BrushManager::getBrush(const QString& name) const {
    return brushes_.value(name, nullptr);
}

QMap<QString, Brush*> BrushManager::getBrushes() const {
    return brushes_; // Returns a copy
}

void BrushManager::setCurrentBrush(const QString& name) {
    Brush* brush = getBrush(name);
    // If name is not found, getBrush returns nullptr, which is handled by the overloaded setCurrentBrush
    setCurrentBrush(brush);
}

void BrushManager::setCurrentBrush(Brush* brush) {
    if (currentBrush_ != brush) {
        Brush* previousBrush = currentBrush_;
        currentBrush_ = brush;

        // Update shared pointer if applicable
        if (brush) {
            // Try to find shared version
            for (auto it = sharedBrushes_.begin(); it != sharedBrushes_.end(); ++it) {
                if (it.value().get() == brush) {
                    currentBrushShared_ = it.value();
                    break;
                }
            }
        } else {
            currentBrushShared_.reset();
        }

        // Emit enhanced signal with both new and previous brush
        emit currentBrushChanged(currentBrush_, previousBrush);
    }
}

Brush* BrushManager::getCurrentBrush() const {
    return currentBrush_;
}

// Enhanced brush creation factory method (Task 33 requirement)
Brush* BrushManager::createBrush(Brush::Type type, const QVariantMap& parameters) {
    Brush* brush = createBrushInternal(type, parameters);
    if (brush) {
        // Add to raw pointer storage
        addBrush(brush);

        // Connect signals
        connectBrushSignals(brush);

        // Emit creation signal
        emit brushCreated(brush);

        qDebug() << "BrushManager: Created brush of type" << static_cast<int>(type) << "with name" << brush->name();
    }
    return brush;
}

std::shared_ptr<Brush> BrushManager::createBrushShared(Brush::Type type, const QVariantMap& parameters) {
    Brush* rawBrush = createBrushInternal(type, parameters);
    if (!rawBrush) {
        return nullptr;
    }

    // Create shared pointer
    std::shared_ptr<Brush> sharedBrush(rawBrush);

    // Add to shared storage
    addBrushShared(sharedBrush);

    // Connect signals
    connectBrushSignals(rawBrush);

    // Emit creation signal
    emit brushCreated(rawBrush);

    qDebug() << "BrushManager: Created shared brush of type" << static_cast<int>(type) << "with name" << rawBrush->name();

    return sharedBrush;
}

std::unique_ptr<Brush> BrushManager::createBrushUnique(Brush::Type type, const QVariantMap& parameters) {
    Brush* rawBrush = createBrushInternal(type, parameters);
    if (!rawBrush) {
        return nullptr;
    }

    // Create unique pointer (no storage in manager - caller owns it)
    std::unique_ptr<Brush> uniqueBrush(rawBrush);

    // Connect signals (brush will be disconnected when destroyed)
    connectBrushSignals(rawBrush);

    // Emit creation signal
    emit brushCreated(rawBrush);

    qDebug() << "BrushManager: Created unique brush of type" << static_cast<int>(type) << "with name" << rawBrush->name();

    return uniqueBrush;
}

// Internal brush creation method
Brush* BrushManager::createBrushInternal(Brush::Type type, const QVariantMap& parameters) {
    if (!isValidBrushType(type)) {
        qWarning() << "BrushManager::createBrushInternal: Invalid brush type" << static_cast<int>(type);
        return nullptr;
    }

    // Merge with default parameters
    QVariantMap mergedParams = defaultBrushParameters_.value(type);
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        mergedParams[it.key()] = it.value();
    }

    Brush* brush = nullptr;

    // Factory creation based on type
    switch (type) {
        case Brush::Type::Ground: {
            quint16 groundId = mergedParams.value("groundId", 100).toUInt();
            brush = new GroundBrush(groundId, this);
            break;
        }
        case Brush::Type::Door: {
            DoorType doorType = static_cast<DoorType>(mergedParams.value("doorType", static_cast<int>(DoorType::Normal)).toInt());
            brush = new DoorBrush(doorType, this);
            break;
        }
        case Brush::Type::Pixel: {
            QColor color = mergedParams.value("color", QColor(Qt::black)).value<QColor>();
            brush = new PixelBrush(color, this);
            break;
        }
        // Task 53: Implemented brush types
        case Brush::Type::Wall:
            brush = new WallBrush(this);
            break;
        case Brush::Type::WallDecoration:
            brush = new WallDecorationBrush(this);
            break;
        case Brush::Type::Table:
            brush = new TableBrush(this);
            break;
        case Brush::Type::Carpet:
            brush = new CarpetBrush(this);
            break;
        case Brush::Type::Creature:
            brush = new CreatureBrush(this);
            break;
        case Brush::Type::Spawn:
            brush = new SpawnBrush(this);
            break;
        case Brush::Type::Flag:
            brush = new FlagBrush(this);
            break;
        case Brush::Type::Eraser:
            brush = new EraserBrush(this);
            break;

        // TODO: Add remaining brush types as they are implemented
        case Brush::Type::Raw:
        case Brush::Type::Doodad:
        case Brush::Type::Terrain:
        case Brush::Type::OptionalBorder:
        case Brush::Type::House:
        case Brush::Type::HouseExit:
        case Brush::Type::Waypoint:
        default:
            qWarning() << "BrushManager::createBrushInternal: Brush type" << static_cast<int>(type) << "not yet implemented";
            return nullptr;
    }

    return brush;
}

// Brush state update methods (Task 33 requirement)
void BrushManager::updateFloodFillTargetItems(const QList<Item*>& targetItems) {
    setBrushContext("floodFillTargetItems", QVariant::fromValue(targetItems));

    if (currentBrush_) {
        // Update current brush if it supports flood fill
        updateCurrentBrushProperties({{"targetItems", QVariant::fromValue(targetItems)}});
    }

    qDebug() << "BrushManager: Updated flood fill target items, count:" << targetItems.size();
}

void BrushManager::updateCurrentBrushTargetType(const ItemType& targetType) {
    setBrushContext("targetType", QVariant::fromValue(targetType));

    if (currentBrush_) {
        updateCurrentBrushProperties({{"targetType", QVariant::fromValue(targetType)}});
    }

    qDebug() << "BrushManager: Updated current brush target type";
}

void BrushManager::setLastSelectedTilesForPaste(const QList<Tile*>& tilesToDrawOn) {
    setBrushContext("selectedTilesForPaste", QVariant::fromValue(tilesToDrawOn));

    if (currentBrush_) {
        updateCurrentBrushProperties({{"selectedTiles", QVariant::fromValue(tilesToDrawOn)}});
    }

    qDebug() << "BrushManager: Set selected tiles for paste, count:" << tilesToDrawOn.size();
}

void BrushManager::updateBrushSize(int newSize) {
    if (newSize <= 0) {
        qWarning() << "BrushManager::updateBrushSize: Invalid size" << newSize;
        return;
    }

    setBrushContext("brushSize", newSize);

    if (currentBrush_) {
        updateCurrentBrushProperties({{"size", newSize}});
    }

    emit brushSizeChanged(newSize);
    qDebug() << "BrushManager: Updated brush size to" << newSize;
}

void BrushManager::updateBrushShape(Brush::BrushShape newShape) {
    setBrushContext("brushShape", static_cast<int>(newShape));

    if (currentBrush_) {
        updateCurrentBrushProperties({{"shape", static_cast<int>(newShape)}});
    }

    emit brushShapeChanged(newShape);
    qDebug() << "BrushManager: Updated brush shape to" << static_cast<int>(newShape);
}

void BrushManager::updateBrushProperties(const QVariantMap& properties) {
    // Update global context
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        setBrushContext(it.key(), it.value());
    }

    if (currentBrush_) {
        updateCurrentBrushProperties(properties);
    }

    qDebug() << "BrushManager: Updated brush properties, count:" << properties.size();
}

// Brush context management
void BrushManager::setBrushContext(const QString& contextKey, const QVariant& contextValue) {
    brushContext_[contextKey] = contextValue;
    emit brushContextChanged(contextKey, contextValue);
}

QVariant BrushManager::getBrushContext(const QString& contextKey) const {
    return brushContext_.value(contextKey);
}

void BrushManager::clearBrushContext() {
    brushContext_.clear();
    qDebug() << "BrushManager: Cleared brush context";
}

// Enhanced brush management with smart pointers
void BrushManager::addBrushShared(std::shared_ptr<Brush> brush) {
    if (!brush || brush->name().isEmpty()) {
        qWarning() << "BrushManager: Attempted to add null or unnamed shared brush.";
        return;
    }

    QString name = brush->name();
    if (sharedBrushes_.contains(name)) {
        qWarning() << "BrushManager: Replacing existing shared brush with name:" << name;
    }

    sharedBrushes_.insert(name, brush);
    connectBrushSignals(brush.get());
}

void BrushManager::removeBrushShared(const QString& name) {
    if (!sharedBrushes_.contains(name)) {
        return;
    }

    auto brush = sharedBrushes_.value(name);

    if (currentBrushShared_ == brush) {
        currentBrushShared_.reset();
        currentBrush_ = nullptr;
    }

    disconnectBrushSignals(brush.get());
    sharedBrushes_.remove(name);
}

std::shared_ptr<Brush> BrushManager::getBrushShared(const QString& name) const {
    return sharedBrushes_.value(name, nullptr);
}

QList<std::shared_ptr<Brush>> BrushManager::getAllBrushesShared() const {
    return sharedBrushes_.values();
}

void BrushManager::setCurrentBrushShared(std::shared_ptr<Brush> brush) {
    if (currentBrushShared_ != brush) {
        Brush* previousBrush = currentBrush_;
        currentBrushShared_ = brush;
        currentBrush_ = brush ? brush.get() : nullptr;

        emit currentBrushChanged(currentBrush_, previousBrush);
    }
}

std::shared_ptr<Brush> BrushManager::getCurrentBrushShared() const {
    return currentBrushShared_;
}

// Brush validation and capabilities
bool BrushManager::canCreateBrush(Brush::Type type) const {
    return isValidBrushType(type);
}

QStringList BrushManager::getAvailableBrushTypes() const {
    QStringList types;
    types << "Ground" << "Door" << "Pixel";
    // TODO: Add other implemented types
    return types;
}

QStringList BrushManager::getBrushesOfType(Brush::Type type) const {
    QStringList brushNames;

    // Check raw brushes
    for (auto it = brushes_.begin(); it != brushes_.end(); ++it) {
        if (it.value()->type() == type) {
            brushNames.append(it.key());
        }
    }

    // Check shared brushes
    for (auto it = sharedBrushes_.begin(); it != sharedBrushes_.end(); ++it) {
        if (it.value()->type() == type && !brushNames.contains(it.key())) {
            brushNames.append(it.key());
        }
    }

    return brushNames;
}

// Task 77: Action ID management implementation
void BrushManager::setActionId(quint16 actionId) {
    if (currentActionId_ != actionId) {
        currentActionId_ = actionId;
        emit actionIdChanged(currentActionId_, actionIdEnabled_);
        qDebug() << "BrushManager: Action ID changed to" << actionId;
    }
}

void BrushManager::setActionIdEnabled(bool enabled) {
    if (actionIdEnabled_ != enabled) {
        actionIdEnabled_ = enabled;
        emit actionIdChanged(currentActionId_, actionIdEnabled_);
        qDebug() << "BrushManager: Action ID enabled state changed to" << enabled;
    }
}

// Task 77: Selected item management implementation
void BrushManager::setSelectedItem(Item* item) {
    if (selectedItem_ != item) {
        selectedItem_ = item;
        QString itemInfo = getSelectedItemInfo();
        emit selectedItemChanged(selectedItem_, itemInfo);
        qDebug() << "BrushManager: Selected item changed to" << itemInfo;
    }
}

QString BrushManager::getSelectedItemInfo() const {
    if (!selectedItem_) {
        return "No item selected";
    }

    QString info = QString("Item: %1 (ID: %2)")
                   .arg(selectedItem_->getName())
                   .arg(selectedItem_->getID());

    if (selectedItem_->getActionID() > 0) {
        info += QString(" [AID: %1]").arg(selectedItem_->getActionID());
    }

    if (selectedItem_->getUniqueID() > 0) {
        info += QString(" [UID: %1]").arg(selectedItem_->getUniqueID());
    }

    return info;
}

// Task 77: Drawing mode management implementation
void BrushManager::setDrawingMode(const QString& modeName, const QString& description) {
    if (currentDrawingMode_ != modeName || currentDrawingModeDescription_ != description) {
        currentDrawingMode_ = modeName;
        currentDrawingModeDescription_ = description.isEmpty() ? modeName : description;
        emit drawingModeChanged(currentDrawingMode_, currentDrawingModeDescription_);
        qDebug() << "BrushManager: Drawing mode changed to" << modeName << "-" << currentDrawingModeDescription_;
    }
}

#include "BrushManager.moc"

// Include helper methods implementation
#include "BrushManagerHelpers.cpp"
