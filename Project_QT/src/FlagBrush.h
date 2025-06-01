#ifndef FLAGBRUSH_H
#define FLAGBRUSH_H

#include "Brush.h"
#include "Tile.h"

// Forward declarations
class Map;
class QUndoCommand;

class FlagBrush : public Brush {
    Q_OBJECT

public:
    // Flag type constants (matching wxwidgets TILESTATE values)
    enum FlagType : quint32 {
        ProtectionZone = 0x0001,  // TILESTATE_PROTECTIONZONE
        NoPVP = 0x0004,           // TILESTATE_NOPVP
        NoLogout = 0x0008,        // TILESTATE_NOLOGOUT
        PVPZone = 0x0010,         // TILESTATE_PVPZONE
        ZoneBrush = 0x0040        // TILESTATE_ZONE_BRUSH
    };

    explicit FlagBrush(quint32 flag, QObject *parent = nullptr);
    ~FlagBrush() override;

    // Brush type identification
    Type type() const override;
    QString name() const override;
    void setName(const QString& newName) override;
    bool isFlag() const override;
    FlagBrush* asFlag() override;
    const FlagBrush* asFlag() const override;

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

    // FlagBrush specific methods
    quint32 getFlag() const;
    void setFlag(quint32 flag);
    void setZoneId(quint16 zoneId);
    quint16 getZoneId() const;

    // Flag type helpers
    bool isProtectionZone() const;
    bool isNoPVP() const;
    bool isNoLogout() const;
    bool isPVPZone() const;
    bool isZoneBrush() const;

    // Static factory methods for common flag brushes
    static FlagBrush* createProtectionZoneBrush(QObject* parent = nullptr);
    static FlagBrush* createNoPVPBrush(QObject* parent = nullptr);
    static FlagBrush* createNoLogoutBrush(QObject* parent = nullptr);
    static FlagBrush* createPVPZoneBrush(QObject* parent = nullptr);
    static FlagBrush* createZoneBrush(QObject* parent = nullptr);

    // Brush loading
    bool load(const QDomElement& element, QStringList& warnings) override;

signals:
    void flagChanged(quint32 flag);
    void zoneIdChanged(quint16 zoneId);

private:
    QString name_;
    quint32 flag_;
    quint16 zoneId_;
    int brushSize_;
    BrushShape brushShape_;

    // Helper methods
    void applyFlagToTile(Tile* tile) const;
    void removeFlagFromTile(Tile* tile) const;
    bool tileHasGround(Tile* tile) const;
    QString getFlagName(quint32 flag) const;
    int getFlagLookID(quint32 flag) const;
};

// Undo command for flag brush operations
class FlagBrushCommand : public QUndoCommand {
public:
    FlagBrushCommand(Map* map, const QPointF& tilePos, quint32 flag, quint16 zoneId,
                    bool isAdding, QUndoCommand* parent = nullptr);
    ~FlagBrushCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    quint32 flag_;
    quint16 zoneId_;
    bool isAdding_;

    // State preservation
    Tile::TileMapFlags previousMapFlags_;
    QVector<quint16> previousZoneIds_;
    bool hadTile_;

    void executeOperation(bool adding);
    void storeCurrentState();
    void restorePreviousState();
};

#endif // FLAGBRUSH_H
