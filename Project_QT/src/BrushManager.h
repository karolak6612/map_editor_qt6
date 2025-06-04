#ifndef BRUSHMANAGER_H
#define BRUSHMANAGER_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QString>
#include <QList>
#include <QVariant>
#include <memory>
#include "Brush.h"
#include "ItemManager.h" // For ItemTypes_t

// Forward declarations
class Item;
class Tile;
class Map;
class BrushPersistence; // Task 81: Brush persistence system

class BrushManager : public QObject {
    Q_OBJECT

public:
    explicit BrushManager(QObject *parent = nullptr);
    ~BrushManager();

    // Enhanced brush creation factory method (Task 36 requirement - Smart Pointers)
    Brush* createBrush(Brush::Type type, const QVariantMap& parameters = QVariantMap());
    std::shared_ptr<Brush> createBrushShared(Brush::Type type, const QVariantMap& parameters = QVariantMap());
    std::unique_ptr<Brush> createBrushUnique(Brush::Type type, const QVariantMap& parameters = QVariantMap());

    // Brush management (existing functionality)
    void addBrush(Brush* brush);
    void removeBrush(const QString& name);
    Brush* getBrush(const QString& name) const;
    QMap<QString, Brush*> getBrushes() const;

    // Enhanced brush management with smart pointers
    void addBrushShared(std::shared_ptr<Brush> brush);
    void removeBrushShared(const QString& name);
    std::shared_ptr<Brush> getBrushShared(const QString& name) const;
    QList<std::shared_ptr<Brush>> getAllBrushesShared() const;

    // Active brush management
    void setCurrentBrush(const QString& name);
    void setCurrentBrush(Brush* brush);
    void setCurrentBrushShared(std::shared_ptr<Brush> brush);
    Brush* getCurrentBrush() const;
    std::shared_ptr<Brush> getCurrentBrushShared() const;

    // Brush state update methods (Task 33 requirement)
    void updateFloodFillTargetItems(const QList<Item*>& targetItems);
    void updateCurrentBrushTargetType(const ItemTypes_t& targetType);
    void setLastSelectedTilesForPaste(const QList<Tile*>& tilesToDrawOn);
    void updateBrushSize(int newSize);
    void updateBrushShape(Brush::BrushShape newShape);
    void updateBrushProperties(const QVariantMap& properties);

    // Brush context and state management
    void setBrushContext(const QString& contextKey, const QVariant& contextValue);
    QVariant getBrushContext(const QString& contextKey) const;
    void clearBrushContext();

    // Brush validation and capabilities
    bool canCreateBrush(Brush::Type type) const;
    QStringList getAvailableBrushTypes() const;
    QStringList getBrushesOfType(Brush::Type type) const;

    // Task 77: Action ID management
    void setActionId(quint16 actionId);
    quint16 getActionId() const { return currentActionId_; }
    void setActionIdEnabled(bool enabled);
    bool isActionIdEnabled() const { return actionIdEnabled_; }

    // Task 77: Selected item management
    void setSelectedItem(Item* item);
    Item* getSelectedItem() const { return selectedItem_; }
    QString getSelectedItemInfo() const;

    // Task 77: Drawing mode management
    void setDrawingMode(const QString& modeName, const QString& description = QString());
    QString getCurrentDrawingMode() const { return currentDrawingMode_; }
    QString getCurrentDrawingModeDescription() const { return currentDrawingModeDescription_; }

    // Task 81: Brush persistence and loading/saving
    BrushPersistence* getBrushPersistence() const { return brushPersistence_; }
    bool saveBrushes(const QString& filePath, const QString& format = QString());
    bool loadBrushes(const QString& filePath, const QString& format = QString());
    bool saveBrushCollection(const QString& filePath, const QString& collectionName, const QStringList& brushNames);
    bool loadBrushCollection(const QString& filePath);
    bool exportBrush(const QString& filePath, const QString& brushName, const QString& format = QString());
    bool importBrush(const QString& filePath, const QString& format = QString());

    // Task 81: User-defined brush management
    bool createUserDefinedBrush(const QString& name, Brush::Type type, const QVariantMap& properties);
    bool modifyUserDefinedBrush(const QString& name, const QVariantMap& properties);
    bool deleteUserDefinedBrush(const QString& name);
    QStringList getUserDefinedBrushes() const;
    bool isUserDefinedBrush(const QString& name) const;

    // Task 81: Brush state management
    void markBrushAsModified(const QString& name);
    bool isBrushModified(const QString& name) const;
    QStringList getModifiedBrushes() const;
    void clearModifiedFlags();

    // Task 81: Brush collection management
    QStringList getAvailableCollections(const QString& directory = QString()) const;
    bool createBrushCollection(const QString& name, const QStringList& brushNames, const QString& description = QString());
    bool deleteBrushCollection(const QString& name);

    // Task 81: Auto-save and backup functionality
    void setAutoSaveEnabled(bool enabled);
    bool isAutoSaveEnabled() const;
    void setAutoSaveInterval(int minutes);
    int getAutoSaveInterval() const;
    void performAutoSave();

    // Task 81: Brush validation and dependencies
    QStringList validateBrushDependencies(const QString& brushName) const;
    QStringList getMissingDependencies() const;
    bool resolveDependencies(const QStringList& dependencyPaths);

signals:
    // Enhanced signal system (Task 33 requirement)
    void currentBrushChanged(Brush* newBrush, Brush* previousBrush);
    void brushCreated(Brush* brush);
    void brushPropertiesChanged(Brush* brush);
    void brushSizeChanged(int newSize);
    void brushShapeChanged(Brush::BrushShape newShape);
    void brushContextChanged(const QString& contextKey, const QVariant& value);

    // Task 77: Action ID and UI synchronization signals
    void actionIdChanged(quint16 actionId, bool enabled);
    void selectedItemChanged(Item* item, const QString& itemInfo);
    void drawingModeChanged(const QString& modeName, const QString& description);

    // Task 81: Brush persistence signals
    void brushSaved(const QString& brushName, const QString& filePath);
    void brushLoaded(const QString& brushName, const QString& filePath);
    void brushCollectionSaved(const QString& collectionName, const QString& filePath);
    void brushCollectionLoaded(const QString& collectionName, const QString& filePath);
    void userDefinedBrushCreated(const QString& brushName);
    void userDefinedBrushModified(const QString& brushName);
    void userDefinedBrushDeleted(const QString& brushName);
    void brushModificationStateChanged(const QString& brushName, bool isModified);
    void autoSavePerformed(int brushCount, const QString& filePath);
    void dependencyResolutionRequired(const QStringList& missingDependencies);
    void persistenceError(const QString& operation, const QString& error);

private slots:
    // Internal slots for brush change notifications
    void onBrushPropertiesChanged();

    // Task 81: Brush persistence slots
    void onAutoSaveTimer();
    void onBrushPersistenceError(const QString& error, const QString& details);

private:
    // Brush storage (dual system for compatibility)
    QMap<QString, Brush*> brushes_;                           // Raw pointer storage (existing)
    QHash<QString, std::shared_ptr<Brush>> sharedBrushes_;    // Smart pointer storage (new)

    // Current brush tracking
    Brush* currentBrush_;
    std::shared_ptr<Brush> currentBrushShared_;

    // Brush context and state
    QVariantMap brushContext_;                                // Global brush context
    QHash<Brush::Type, QVariantMap> defaultBrushParameters_;  // Default parameters per type

    // Task 77: Action ID and UI synchronization state
    quint16 currentActionId_;                                 // Current action ID for placed items
    bool actionIdEnabled_;                                    // Whether action ID is enabled
    Item* selectedItem_;                                      // Currently selected item for drawing
    QString currentDrawingMode_;                              // Current drawing mode name
    QString currentDrawingModeDescription_;                   // Current drawing mode description

    // Task 81: Brush persistence system
    BrushPersistence* brushPersistence_;                      // Brush persistence manager
    QTimer* autoSaveTimer_;                                   // Auto-save timer
    bool autoSaveEnabled_;                                    // Auto-save enabled flag
    int autoSaveInterval_;                                    // Auto-save interval in minutes
    QString defaultBrushDirectory_;                           // Default directory for brush files
    QString defaultCollectionDirectory_;                      // Default directory for collections
    QStringList userDefinedBrushNames_;                       // List of user-defined brush names
    QStringList modifiedBrushNames_;                          // List of modified brush names
    QMap<QString, QString> brushFilePaths_;                   // Brush name -> file path mapping

    // Internal helper methods
    void connectBrushSignals(Brush* brush);
    void disconnectBrushSignals(Brush* brush);
    Brush* createBrushInternal(Brush::Type type, const QVariantMap& parameters);
    void initializeDefaultParameters();
    void updateCurrentBrushProperties(const QVariantMap& properties);
    QString generateBrushName(Brush::Type type, const QVariantMap& parameters) const;

    // Brush factory registration
    void registerBrushFactories();
    bool isValidBrushType(Brush::Type type) const;

    // Task 81: Brush persistence initialization
    void initializeBrushPersistence();
};

#endif // BRUSHMANAGER_H
