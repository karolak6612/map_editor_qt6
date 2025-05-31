#ifndef GROUNDBRUSH_H
#define GROUNDBRUSH_H

#include "TerrainBrush.h" // Changed from Brush.h

// Forward declarations (if any specific to GroundBrush are needed)
// Map, QUndoCommand, QMouseEvent, MapView are likely included via TerrainBrush.h -> Brush.h

class GroundBrush : public TerrainBrush { // Changed base class
    Q_OBJECT 

public:
    explicit GroundBrush(QObject *parent = nullptr);
    ~GroundBrush() override;

    // --- Overrides from Brush/TerrainBrush ---
    // name() is inherited from TerrainBrush, which uses specificName_.
    // GroundBrush constructor will call setSpecificName("Ground Brush").
    // If a different behavior for name() is needed, then override it.
    // For now, assume TerrainBrush::name() is sufficient.
    Type type() const override;     // Must implement: return Brush::Type::Ground

    // Core action methods (MUST implement as they are pure in TerrainBrush or Brush)
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Brush geometry (MUST implement as they are pure in Brush)
    int getBrushSize() const override;
    BrushShape getBrushShape() const override;

    // Mouse event methods (ensure signature matches Brush.h)
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

    // Convenience method (already existed, good to keep)
    bool isGround() const override;

    // --- Methods for managing the ground item ID ---
    void setCurrentGroundItemId(quint16 itemId);
    quint16 getCurrentGroundItemId() const;

    // Optional border support
    virtual bool hasOptionalBorder() const;

private:
    quint16 currentGroundItemId_ = 0;
    // Add a known default if possible, e.g. common grass ID, otherwise 0 indicates "not set".
    // bool m_supportsOptionalBorder = false; // Member for hasOptionalBorder - can be added later
};

#endif // GROUNDBRUSH_H
