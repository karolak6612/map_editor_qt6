#ifndef CREATUREBRUSH_H
#define CREATUREBRUSH_H

#include "Brush.h"
#include "Creature.h"
#include "CreatureManager.h"
#include "GameSprite.h"
#include "SpriteManager.h"
#include "Map.h"
#include "MapView.h"

// Forward declarations
class CreatureProperties;
class Outfit;

/**
 * @brief Task 87: Enhanced CreatureBrush with full sprite visualization and attribute handling
 *
 * Complete creature brush implementation:
 * - Creature type selection from CreatureManager
 * - Full sprite visualization with outfit/looktype support
 * - Attribute handling (name, health, direction, etc.)
 * - Proper tile integration and undo/redo support
 * - 1:1 compatibility with wxwidgets creature brush
 */

class CreatureBrush : public Brush {
    Q_OBJECT

public:
    explicit CreatureBrush(QObject *parent = nullptr);

    // Brush type identification
    Type type() const override;
    QString name() const override;
    bool isCreature() const override;
    CreatureBrush* asCreature() override;
    const CreatureBrush* asCreature() const override;

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

    // Task 87: Enhanced CreatureBrush specific methods
    void setCreatureId(quint16 creatureId);
    quint16 getCreatureId() const;

    void setCreatureName(const QString& name);
    QString getCreatureName() const;

    void setCreatureType(const CreatureProperties* creatureType);
    const CreatureProperties* getCreatureType() const;

    // Task 87: Sprite and outfit management
    GameSprite* getCreatureSprite() const;
    QPixmap getCreaturePixmap(int width = 32, int height = 32) const;
    void updateSpriteVisualization();

    // Task 87: Attribute management
    void setDefaultHealth(int health);
    int getDefaultHealth() const;

    void setDefaultDirection(Direction direction);
    Direction getDefaultDirection() const;

    void setDefaultOutfit(const Outfit& outfit);
    Outfit getDefaultOutfit() const;

    // Task 87: Validation and utility
    bool isValidCreatureType() const;
    QString getValidationError() const;
    bool canPlaceCreature(Map* map, const MapPos& position) const;

    // Task 53: Core creature placement functionality
    void draw(Map* map, Tile* tile, void* parameter = nullptr);
    void undraw(Map* map, Tile* tile);

    // Task 87: Enhanced placement with full attributes
    Creature* createCreature() const;
    bool placeCreature(Map* map, Tile* tile, Creature* creature = nullptr);
    bool removeCreature(Map* map, Tile* tile);

signals:
    // Task 87: Enhanced signals
    void creatureTypeChanged(quint16 creatureId);
    void creatureAttributesChanged();
    void spriteVisualizationUpdated();

private:
    // Core properties
    quint16 creatureId_ = 0;
    QString creatureName_;
    const CreatureProperties* creatureType_ = nullptr;

    // Brush properties
    int brushSize_ = 1;
    BrushShape brushShape_ = BrushShape::Square;

    // Task 87: Enhanced attributes
    int defaultHealth_ = 100;
    Direction defaultDirection_ = Direction::South;
    Outfit defaultOutfit_;

    // Task 87: Sprite management
    mutable GameSprite* cachedSprite_ = nullptr;
    mutable QPixmap cachedPixmap_;
    mutable bool spriteNeedsUpdate_ = true;

    // Helper methods
    void updateCachedSprite() const;
    void invalidateSprite();
    CreatureManager* getCreatureManager() const;
    SpriteManager* getSpriteManager() const;
};

#endif // CREATUREBRUSH_H
