#ifndef QT_OPTIONALBORDERBRUSH_H
#define QT_OPTIONALBORDERBRUSH_H

#include "Brush.h" // Base class

// Forward declarations from Brush.h are assumed to cover Map, Tile, QPointF, etc.
// class Map;
// class Tile;
// class QUndoCommand;
// class QMouseEvent;
// class MapView;
// class QUndoStack;


class OptionalBorderBrush : public Brush {
    Q_OBJECT

public:
    explicit OptionalBorderBrush(QObject *parent = nullptr);
    ~OptionalBorderBrush() override;

    // --- Overrides from Brush base class ---
    Type type() const override;

    QString name() const override;
    int getLookID() const override; // For UI representation
    bool canDrag() const override; // Optional borders are usually toggled per tile

    // Core action methods (implementing Brush pure virtuals)
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Mouse event methods (implementing Brush pure virtuals)
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

    // Brush geometry (implementing Brush pure virtuals)
    int getBrushSize() const override;
    BrushShape getBrushShape() const override;

    // Type identification
    bool isOptionalBorder() const override;
    OptionalBorderBrush* asOptionalBorder() override;
    const OptionalBorderBrush* asOptionalBorder() const override;

private:
    // OptionalBorderBrush primarily modifies tile state (TileStateFlag::OptionalBorder).
    // Name can be hardcoded. LookID might not be relevant or could be a generic icon.
    // For simplicity, these are not made configurable via XML in this step.
    // QString m_name; // Name will be set in constructor
    // int m_look_id;  // LookID can be set in constructor or default from Brush
};

#endif // QT_OPTIONALBORDERBRUSH_H
