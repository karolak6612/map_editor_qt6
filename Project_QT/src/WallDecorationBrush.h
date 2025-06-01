#ifndef WALLDECORATIONBRUSH_H
#define WALLDECORATIONBRUSH_H

#include "WallBrush.h" // Base class

// Forward declarations (Map, QUndoCommand, etc. are in Brush.h/TerrainBrush.h/WallBrush.h's scope for virtual methods)
// QMouseEvent is included by Brush.h indirectly.

class WallDecorationBrush : public WallBrush {
    Q_OBJECT
public:
    explicit WallDecorationBrush(QObject* parent = nullptr);
    ~WallDecorationBrush() override;

    // --- Overrides from Brush Interface ---
    Type type() const override;
    // Name can be set via TerrainBrush::setSpecificName in constructor to "Wall Decoration Brush"

    // This method is intended to override a virtual method in Brush.h (to be added in Step 4 of main plan)
    bool isWallDecoration() const override;

    // Core action methods
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Mouse event methods (must match signature in Brush.h)
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

    // Inherits getBrushSize, getBrushShape, canSmear, needBorders, canDrag, getLookID from WallBrush/TerrainBrush.
    // These can be overridden if WallDecorationBrush behaves differently.
    // For example, canSmear() might be true if decorations can be "painted".
    // getBrushSize() for decorations is typically 0.

    // --- WallDecorationBrush specific methods ---
    void setCurrentDecorationItemId(quint16 itemId);
    quint16 getCurrentDecorationItemId() const;

    // Direct migration from wxwidgets - uses same XML loading as WallBrush (Task 40 requirement)
    // WallDecorationBrush uses the exact same wall_items structure and loading algorithm as WallBrush
    // The difference is in the draw() method which places decorations based on existing wall alignment
};

#endif // WALLDECORATIONBRUSH_H
