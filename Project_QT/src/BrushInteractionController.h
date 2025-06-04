// BrushInteractionController.h - Comprehensive brush interaction system for Task 34
// This class manages the command system and map updates for brush interactions

#ifndef BRUSHINTERACTIONCONTROLLER_H
#define BRUSHINTERACTIONCONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QList>
#include <QUndoCommand>
#include <QUndoStack>
#include <QMap>
#include <QVariant>

class Brush;
class Map;
class MapView;
class BrushManager;
class QMouseEvent;

// Command classes for undo/redo system (Task 34 requirement)
class BrushDrawCommand : public QUndoCommand {
public:
    BrushDrawCommand(Brush* brush, Map* map, const QList<QPointF>& tiles, 
                     const QString& description = "Brush Draw", QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    Brush* brush_;
    Map* map_;
    QList<QPointF> affectedTiles_;
    QList<QVariant> originalTileData_;
    QList<QVariant> newTileData_;
};

class BrushEraseCommand : public QUndoCommand {
public:
    BrushEraseCommand(Brush* brush, Map* map, const QList<QPointF>& tiles,
                      const QString& description = "Brush Erase", QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    Brush* brush_;
    Map* map_;
    QList<QPointF> affectedTiles_;
    QList<QVariant> originalTileData_;
};

class BrushBatchCommand : public QUndoCommand {
public:
    BrushBatchCommand(const QString& description = "Brush Batch Operation", QUndoCommand* parent = nullptr);
    
    void addDrawCommand(const QList<QPointF>& tiles, Brush* brush, Map* map);
    void addEraseCommand(const QList<QPointF>& tiles, Brush* brush, Map* map);
    
    void undo() override;
    void redo() override;
    
private:
    QList<QUndoCommand*> subCommands_;
};

// Main brush interaction controller
class BrushInteractionController : public QObject {
    Q_OBJECT
    
public:
    explicit BrushInteractionController(MapView* mapView, BrushManager* brushManager, 
                                      Map* map, QUndoStack* undoStack, QObject* parent = nullptr);
    ~BrushInteractionController();
    
    // Brush action execution (Task 34 requirement)
    QUndoCommand* executeBrushAction(Brush* brush, const QPointF& mapPos, 
                                   QMouseEvent* event, bool isErase = false);
    QUndoCommand* executeBrushArea(Brush* brush, const QList<QPointF>& tiles, 
                                 QMouseEvent* event, bool isErase = false);
    QUndoCommand* executeBrushDrag(Brush* brush, const QPointF& startPos, 
                                 const QPointF& endPos, QMouseEvent* event);
    
    // Batch operations for multi-tile actions
    void startBatchOperation(const QString& description = "Brush Operation");
    void addToBatch(const QList<QPointF>& tiles, Brush* brush, bool isErase = false);
    QUndoCommand* finishBatchOperation();
    void cancelBatchOperation();
    
    // Map and selection update signaling (Task 34 requirement)
    void signalMapUpdate(const QRectF& area);
    void signalTileUpdate(const QPointF& tilePos);
    void signalSelectionUpdate();
    
    // Visual feedback management
    void updateVisualFeedback(const QList<QPointF>& previewTiles);
    void clearVisualFeedback();
    
    // Brush context management
    void setBrushContext(const QString& key, const QVariant& value);
    QVariant getBrushContext(const QString& key) const;
    void clearBrushContext();
    
    // State queries
    bool isBatchOperationActive() const { return currentBatchCommand_ != nullptr; }
    int getPendingCommandCount() const { return pendingCommands_.size(); }
    
signals:
    // Map update signals (Task 34 requirement)
    void mapAreaChanged(const QRectF& area);
    void tileChanged(const QPointF& tilePos);
    void selectionChanged();
    
    // Visual feedback signals
    void brushPreviewChanged(const QList<QPointF>& tiles);
    void brushPreviewCleared();
    
    // Operation signals
    void batchOperationStarted(const QString& description);
    void batchOperationFinished();
    void batchOperationCancelled();
    
public slots:
    // Brush manager integration
    void onCurrentBrushChanged(Brush* newBrush, Brush* previousBrush);
    void onBrushPropertiesChanged(Brush* brush);
    
    // Map integration
    void onMapChanged();
    void onMapTileChanged(const QPointF& tilePos);
    
private slots:
    void onUndoStackIndexChanged(int index);
    
private:
    // Helper methods
    QList<QPointF> validateTilePositions(const QList<QPointF>& tiles) const;
    bool canApplyBrushToTile(Brush* brush, const QPointF& tilePos) const;
    QVariant captureTileData(const QPointF& tilePos) const;
    void restoreTileData(const QPointF& tilePos, const QVariant& data);
    
    // Command creation helpers
    QUndoCommand* createDrawCommand(Brush* brush, const QList<QPointF>& tiles);
    QUndoCommand* createEraseCommand(Brush* brush, const QList<QPointF>& tiles);
    
    // Visual update helpers
    void updateMapDisplay(const QRectF& area);
    void updateTileDisplay(const QPointF& tilePos);
    
    // Member variables
    MapView* mapView_;                    // Non-owning pointer
    BrushManager* brushManager_;          // Non-owning pointer
    Map* map_;                           // Non-owning pointer
    QUndoStack* undoStack_;              // Non-owning pointer
    
    // Command batching
    BrushBatchCommand* currentBatchCommand_;
    QList<QUndoCommand*> pendingCommands_;
    QString currentBatchDescription_;
    
    // Brush context
    QVariantMap brushContext_;
    
    // Visual feedback state
    QList<QPointF> currentPreviewTiles_;
    bool visualFeedbackActive_;
    
    // Performance optimization
    QRectF accumulatedUpdateArea_;
    bool updateAreaDirty_;
};

#endif // BRUSHINTERACTIONCONTROLLER_H
