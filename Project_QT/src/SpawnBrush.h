#ifndef SPAWNBRUSH_H
#define SPAWNBRUSH_H

#include "Brush.h"
#include "Spawn.h"
#include "Map.h"
#include "MapView.h"
#include "CreatureManager.h"

// Forward declarations
class SpawnEditorDialog;
class QGraphicsItem;

/**
 * @brief Task 87: Enhanced SpawnBrush with full configuration and visual representation
 *
 * Complete spawn brush implementation:
 * - Full spawn configuration via dialog
 * - Visual representation of spawn areas and radius
 * - Creature type selection and management
 * - Proper tile integration and undo/redo support
 * - 1:1 compatibility with wxwidgets spawn brush
 */

class SpawnBrush : public Brush {
    Q_OBJECT

public:
    explicit SpawnBrush(QObject *parent = nullptr);

    // Brush type identification
    Type type() const override;
    QString name() const override;
    bool isSpawn() const override;
    SpawnBrush* asSpawn() override;
    const SpawnBrush* asSpawn() const override;

    // Brush properties
    int getBrushSize() const override;
    BrushShape getBrushShape() const override;
    bool canDrag() const override;
    bool canSmear() const override;
    bool oneSizeFitsAll() const override;
    int getLookID() const override;

    // Core brush action interface
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Mouse event handlers with proper signatures
    QUndoCommand* mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                  Map* map, QUndoStack* undoStack,
                                  bool shiftPressed, bool ctrlPressed, bool altPressed,
                                  QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                 Map* map, QUndoStack* undoStack,
                                 bool shiftPressed, bool ctrlPressed, bool altPressed,
                                 QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                    Map* map, QUndoStack* undoStack,
                                    bool shiftPressed, bool ctrlPressed, bool altPressed,
                                    QUndoCommand* parentCommand = nullptr) override;
    void cancel() override;

    // Task 87: Enhanced SpawnBrush specific methods
    void setDefaultSpawnRadius(int radius);
    int getDefaultSpawnRadius() const;

    void setDefaultSpawnInterval(int interval);
    int getDefaultSpawnInterval() const;

    void setDefaultMaxCreatures(int maxCreatures);
    int getDefaultMaxCreatures() const;

    void setDefaultCreatureNames(const QStringList& creatureNames);
    QStringList getDefaultCreatureNames() const;
    void addDefaultCreatureName(const QString& name);
    void removeDefaultCreatureName(const QString& name);

    // Task 87: Spawn configuration dialog
    bool showSpawnConfigDialog(QWidget* parent = nullptr);
    void setSpawnTemplate(const Spawn* spawnTemplate);
    Spawn* getSpawnTemplate() const;

    // Task 87: Visual representation
    void setShowSpawnRadius(bool show);
    bool isShowSpawnRadius() const;

    void setSpawnRadiusColor(const QColor& color);
    QColor getSpawnRadiusColor() const;

    // Task 87: Validation and utility
    bool isValidSpawnConfiguration() const;
    QString getValidationError() const;
    bool canPlaceSpawn(Map* map, const MapPos& position) const;

    // Task 53: Core spawn placement functionality
    void draw(Map* map, Tile* tile, void* parameter = nullptr);
    void undraw(Map* map, Tile* tile);

    // Task 87: Enhanced placement with full configuration
    Spawn* createSpawn(const MapPos& position) const;
    bool placeSpawn(Map* map, Tile* tile, Spawn* spawn = nullptr);
    bool removeSpawn(Map* map, Tile* tile);

    // Task 87: Visual feedback
    void drawSpawnRadius(QPainter* painter, const MapPos& center, int radius, const QColor& color) const;
    QGraphicsItem* createSpawnVisualization(const Spawn* spawn) const;

signals:
    // Task 87: Enhanced signals
    void spawnConfigurationChanged();
    void spawnRadiusChanged(int radius);
    void spawnVisualizationUpdated();

private:
    // Brush properties
    int brushSize_ = 1;
    BrushShape brushShape_ = BrushShape::Square;

    // Task 87: Default spawn configuration
    int defaultRadius_ = 3;
    int defaultInterval_ = 10000; // 10 seconds
    int defaultMaxCreatures_ = 3;
    QStringList defaultCreatureNames_;

    // Task 87: Visual settings
    bool showSpawnRadius_ = true;
    QColor spawnRadiusColor_ = QColor(255, 0, 0, 100);

    // Task 87: Spawn template for configuration
    Spawn* spawnTemplate_ = nullptr;

    // Task 87: Dialog management
    mutable SpawnEditorDialog* configDialog_ = nullptr;

    // Helper methods
    void initializeDefaults();
    void updateSpawnTemplate();
    CreatureManager* getCreatureManager() const;
};

#endif // SPAWNBRUSH_H
