#ifndef WAYPOINTBRUSH_H
#define WAYPOINTBRUSH_H

#include "Brush.h"
#include <QString>
#include <QColor>

// Forward declarations
class Waypoint;
class Map;
class Tile;

class WaypointBrush : public Brush {
    Q_OBJECT

public:
    explicit WaypointBrush(QObject* parent = nullptr);
    ~WaypointBrush() override;

    // Brush interface implementation
    Type type() const override;
    QString name() const override;
    void setName(const QString& newName) override;
    int getLookID() const override;

    // Type identification
    bool isWaypoint() const override;
    WaypointBrush* asWaypoint() override;
    const WaypointBrush* asWaypoint() const override;

    // Core brush actions
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Waypoint-specific properties
    QString getWaypointName() const;
    void setWaypointName(const QString& name);
    QString getWaypointType() const;
    void setWaypointType(const QString& type);
    QString getWaypointScript() const;
    void setWaypointScript(const QString& script);
    int getWaypointRadius() const;
    void setWaypointRadius(int radius);
    QColor getWaypointColor() const;
    void setWaypointColor(const QColor& color);
    QString getWaypointIcon() const;
    void setWaypointIcon(const QString& icon);

    // Brush configuration
    bool getAutoGenerateName() const;
    void setAutoGenerateName(bool autoGenerate);
    bool getMarkWithItem() const;
    void setMarkWithItem(bool mark);
    quint16 getMarkerItemId() const;
    void setMarkerItemId(quint16 itemId);
    bool getReplaceExisting() const;
    void setReplaceExisting(bool replace);

    // Drawing options
    bool canDrag() const override;
    bool canSmear() const override;
    bool oneSizeFitsAll() const override;

    // Brush loading
    bool load(const QDomElement& element, QStringList& warnings) override;

signals:
    void waypointPropertiesChanged();

private:
    QString name_;
    QString waypointName_;
    QString waypointType_;
    QString waypointScript_;
    int waypointRadius_;
    QColor waypointColor_;
    QString waypointIcon_;
    bool autoGenerateName_;
    bool markWithItem_;
    quint16 markerItemId_;
    bool replaceExisting_;

    // Helper methods
    QString generateWaypointName(Map* map, const QPointF& tilePos) const;
    void addWaypointMarker(Tile* tile) const;
    void removeWaypointMarker(Tile* tile) const;
    bool isValidWaypointPosition(Map* map, const QPointF& tilePos) const;
    Waypoint* createWaypoint(Map* map, const QPointF& tilePos) const;
};

// Undo command for waypoint brush operations
class WaypointBrushCommand : public QUndoCommand {
public:
    WaypointBrushCommand(Map* map, const QPointF& tilePos, const QString& waypointName, 
                        const QString& waypointType, const QString& waypointScript,
                        int radius, const QColor& color, const QString& icon,
                        bool isAdding, QUndoCommand* parent = nullptr);
    ~WaypointBrushCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    QString waypointName_;
    QString waypointType_;
    QString waypointScript_;
    int waypointRadius_;
    QColor waypointColor_;
    QString waypointIcon_;
    bool isAdding_;
    bool hadWaypoint_;
    QString previousWaypointName_;
    bool hadMarkerItem_;
    quint16 markerItemId_;
    
    void executeOperation(bool adding);
    void storeCurrentState();
    void restorePreviousState();
};

#endif // WAYPOINTBRUSH_H
