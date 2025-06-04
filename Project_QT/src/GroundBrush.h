#ifndef GROUNDBRUSH_H
#define GROUNDBRUSH_H

#include "TerrainBrush.h" // Changed from Brush.h

// Forward declarations (if any specific to GroundBrush are needed)
// Map, QUndoCommand, QMouseEvent, MapView are likely included via TerrainBrush.h -> Brush.h

class GroundBrush : public TerrainBrush { // Changed base class
    Q_OBJECT 

public:
    explicit GroundBrush(QObject *parent = nullptr);
    explicit GroundBrush(quint16 groundItemId, QObject *parent = nullptr); // Task 38 requirement
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

    // Task 016: Override getLookID to use currentGroundItemId_ as primary storage
    int getLookID() const override;

    // Optional border support (Task 38 requirement)
    virtual bool hasOptionalBorder() const;
    virtual bool useSoloOptionalBorder() const;
    virtual bool isReRandomizable() const;

    // Border system hooks (Task 38 requirement - placeholders for Task 52)
    void requestBorderUpdate(Map* map, const QPointF& tilePos) const;
    bool checkGroundEquivalent(Map* map, const QPointF& tilePos, quint16 groundItemId) const;

    // Ground placement validation (Task 38 requirement)
    bool canPlaceGroundAt(Map* map, const QPointF& tilePos, quint16 groundItemId) const;

    // XML loading support (structure for Task 81)
    virtual bool load(const QDomElement& element, QStringList& warnings) override;

private:
    // Core ground brush properties (Task 38 requirement)
    quint16 currentGroundItemId_ = 0;
    bool hasOptionalBorder_ = false;
    bool useSoloOptionalBorder_ = false;
    bool isReRandomizable_ = false;

    // Ground equivalent groups for placement validation
    QVector<quint16> groundEquivalentGroup_;
};

#endif // GROUNDBRUSH_H
