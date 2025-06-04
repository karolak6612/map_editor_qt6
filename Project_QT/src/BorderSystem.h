#ifndef BORDERSYSTEM_H
#define BORDERSYSTEM_H

#include <QObject>
#include <QRect>
#include <QPoint>
#include <QList>
#include <QVector3D>
#include <QCoreApplication>
#include <QVariant>

// Forward declarations
#include "Map.h"
class Tile;
class SettingsManager;

// Task 52: Border type enum (from AutoBorder.h)
enum class BorderType;

/**
 * BorderSystem - Placeholder class for automagic border functionality
 * This class will eventually contain the logic for automatic border placement,
 * ground type detection, and border management that was present in the wxWidgets version.
 */
class BorderSystem : public QObject {
    Q_OBJECT

public:
    static BorderSystem* getInstance();
    static void destroyInstance();

    // Task 52: Enhanced border functionality with Qt rendering integration
    void applyAutomagicBorders(Map* map, Tile* tile);
    void applyCustomBorders(Map* map, Tile* tile, int borderId);
    void removeBorders(Map* map, Tile* tile);
    void reborderizeTile(Map* map, Tile* tile);

    // Task 52: Area-based border processing for efficiency
    void processBorderArea(Map* map, const QRect& area);
    void processBorderArea(Map* map, const QList<QPoint>& tilePositions);
    void processBorderUpdates(Map* map, const QList<QVector3D>& affectedTiles);

    // Task 52: Neighbor analysis for border determination
    QList<Tile*> getNeighborTiles(Map* map, Tile* centerTile) const;
    QList<Tile*> getNeighborTiles(Map* map, int x, int y, int z) const;
    bool analyzeGroundCompatibility(Tile* tile1, Tile* tile2) const;
    BorderType determineBorderType(Map* map, Tile* tile, int neighborX, int neighborY) const;

    // Border validation and checking
    bool shouldApplyBorders(Map* map, Tile* tile) const;
    bool isValidBorderPosition(Map* map, int x, int y, int z) const;
    
    // Settings integration
    void updateFromSettings();
    bool isAutomagicEnabled() const;
    bool isSameGroundTypeBorderEnabled() const;
    bool isWallsRepelBordersEnabled() const;
    bool isLayerCarpetsEnabled() const;
    bool isBorderizeDeleteEnabled() const;
    bool isCustomBorderEnabled() const;
    int getCustomBorderId() const;

public slots:
    void onAutomagicSettingsChanged();
    void onTileChanged(Map* map, Tile* tile);
    void onSelectionBorderize();

signals:
    // Task 52: Enhanced signals for Qt rendering integration
    void borderingStarted();
    void borderingFinished();
    void borderingProgress(int percentage);

    // Task 52: Visual update signals for MapView integration
    void tilesNeedVisualUpdate(const QList<QVector3D>& tileCoordinates);
    void borderItemsChanged(Map* map, const QList<QVector3D>& affectedTiles);
    void borderProcessingComplete(Map* map, const QRect& processedArea);

private:
    explicit BorderSystem(QObject* parent = nullptr);
    ~BorderSystem();

    static BorderSystem* instance_;
    SettingsManager* settingsManager_;
    
    // Internal state
    bool automagicEnabled_;
    bool sameGroundTypeBorder_;
    bool wallsRepelBorders_;
    bool layerCarpets_;
    bool borderizeDelete_;
    bool customBorderEnabled_;
    int customBorderId_;
    
    void initializeFromSettings();
    void logBorderAction(const QString& action, Map* map, Tile* tile);

    // Task 52: Helper method for border item application
    void applyBorderItem(Map* map, Tile* tile, BorderType borderType);
};

#endif // BORDERSYSTEM_H
