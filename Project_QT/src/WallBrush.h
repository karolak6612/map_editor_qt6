#ifndef WALLBRUSH_H
#define WALLBRUSH_H

#include "TerrainBrush.h"
#include "BrushCommon.h" // For WallAlignment, DoorTypeQt
#include <QMap>
#include <QVector>
#include <QString>

// Forward declarations (Map, QUndoCommand, etc. are in Brush.h/TerrainBrush.h's scope for virtual methods)

// Helper structs for configuration (can be defined here or in BrushCommon.h if widely used)
struct WallItemConfig {
    quint16 itemId = 0;
    int chance = 100;
};

struct WallDoorConfig {
    DoorTypeQt doorType = DoorTypeQt::Undefined;
    quint16 itemId = 0;
    bool isLocked = false;
};


class WallBrush : public TerrainBrush {
    Q_OBJECT
public:
    explicit WallBrush(QObject* parent = nullptr);
    ~WallBrush() override;

    // --- Overrides from Brush/TerrainBrush ---
    // Name can be set via TerrainBrush::setSpecificName in constructor
    Type type() const override;
    bool isWall() const override; // Convenience, already in Brush.h (will return true)

    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    int getBrushSize() const override;
    BrushShape getBrushShape() const override;

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

    bool canSmear() const override; // wxWallBrush returned false

    // --- WallBrush specific methods ---
    void setCurrentWallItemId(quint16 itemId);
    quint16 getCurrentWallItemId() const;

    // Wall alignment and connection system (Task 39 requirement)
    enum class WallAlignment {
        Undefined = 0,
        Horizontal = 1,
        Vertical = 2,
        Corner = 3,
        TJunction = 4,
        Cross = 5
    };

    // Door handling (Task 39 requirement)
    enum class DoorType {
        None = 0,
        Normal = 1,
        Locked = 2,
        Quest = 3,
        Magic = 4,
        Level = 5,
        Key = 6
    };

    // Auto-connection stubs (Task 39 requirement - placeholders for full implementation)
    void requestWallUpdate(Map* map, const QPointF& tilePos) const;
    WallAlignment calculateWallAlignment(Map* map, const QPointF& tilePos) const;
    quint16 getWallItemForAlignment(WallAlignment alignment) const;

    // Door information handling (Task 39 requirement)
    bool hasWall(quint16 itemId) const;
    DoorType getDoorTypeFromID(quint16 itemId) const;
    bool isDoorItem(quint16 itemId) const;
    void setDoorType(DoorType doorType);
    DoorType getDoorType() const;

    // Wall selection/deselection stubs (Task 39 requirement)
    void selectWall(Map* map, const QPointF& tilePos);
    void deselectWall(Map* map, const QPointF& tilePos);

    // XML loading support (structure for Task 81)
    virtual bool load(const QDomElement& element, QStringList& warnings) override;

private:
    // Initialization helper (Task 39 requirement)
    void initializeDefaultWallMappings();
    // Core wall brush properties (Task 39 requirement)
    quint16 currentWallItemId_ = 0; // Primary item ID to place for this wall type
    DoorType currentDoorType_ = DoorType::None; // Current door type for this wall
    WallAlignment currentAlignment_ = WallAlignment::Undefined; // Current wall alignment

    // Wall item mapping for different alignments (Task 39 requirement)
    QMap<WallAlignment, quint16> wallItemsByAlignment_;

    // Door item ranges for identification (Task 39 requirement)
    QVector<QPair<quint16, quint16>> doorItemRanges_; // Start and end IDs for door items

    // Wall item ranges for identification (Task 39 requirement)
    QVector<QPair<quint16, quint16>> wallItemRanges_; // Start and end IDs for wall items
};

#endif // WALLBRUSH_H
