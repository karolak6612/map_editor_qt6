#ifndef SELECTIONTRANSFORMATIONENGINE_H
#define SELECTIONTRANSFORMATIONENGINE_H

#include <QObject>
#include <QVariantMap>
#include <QSet>
#include <QMap>
#include <QList>
#include <QPointF>
#include <QRect>
#include <QMatrix>
#include <QTransform>
#include <QUndoCommand>

#include "MapPos.h"

// Forward declarations
class Map;
class Tile;
class Item;
class Selection;

/**
 * @brief Task 95: Selection Transformation Engine for complex item and tile transformations
 * 
 * Complete transformation system for selections with:
 * - Move operations with cut/paste logic and merging
 * - Rotation with proper item orientation handling
 * - Flip operations with item type updates
 * - Item property preservation and transformation
 * - Tile data modification with undo/redo support
 * - Performance optimization for large selections
 * - wxwidgets compatibility for transformation behavior
 */

/**
 * @brief Transformation operation types
 */
enum class TransformationType {
    MOVE,
    ROTATE,
    FLIP_HORIZONTAL,
    FLIP_VERTICAL,
    SCALE,
    SHEAR,
    CUSTOM
};

/**
 * @brief Transformation parameters container
 */
struct TransformationParameters {
    TransformationType type;
    QVariantMap parameters;
    MapPos pivot;
    bool preserveProperties;
    bool mergeWithExisting;
    bool autoRotateItems;
    
    TransformationParameters(TransformationType t = TransformationType::MOVE)
        : type(t), preserveProperties(true), mergeWithExisting(false), autoRotateItems(true) {}
};

/**
 * @brief Item transformation result
 */
struct ItemTransformationResult {
    Item* originalItem;
    Item* transformedItem;
    MapPos originalPosition;
    MapPos newPosition;
    bool wasRotated;
    bool wasFlipped;
    int newOrientation;
    quint16 newItemType;
    bool successful;
    QString errorMessage;
    
    ItemTransformationResult() : originalItem(nullptr), transformedItem(nullptr),
                               wasRotated(false), wasFlipped(false), newOrientation(0),
                               newItemType(0), successful(false) {}
};

/**
 * @brief Tile transformation result
 */
struct TileTransformationResult {
    Tile* originalTile;
    Tile* transformedTile;
    MapPos originalPosition;
    MapPos newPosition;
    QList<ItemTransformationResult> itemResults;
    bool successful;
    QString errorMessage;
    
    TileTransformationResult() : originalTile(nullptr), transformedTile(nullptr), successful(false) {}
};

/**
 * @brief Complete transformation result
 */
struct TransformationResult {
    QList<TileTransformationResult> tileResults;
    QSet<MapPos> affectedPositions;
    QSet<MapPos> originalPositions;
    QSet<MapPos> newPositions;
    int totalTiles;
    int totalItems;
    int successfulTiles;
    int successfulItems;
    bool overallSuccess;
    QStringList errors;
    qint64 executionTime;
    
    TransformationResult() : totalTiles(0), totalItems(0), successfulTiles(0),
                           successfulItems(0), overallSuccess(false), executionTime(0) {}
};

/**
 * @brief Main Selection Transformation Engine
 */
class SelectionTransformationEngine : public QObject
{
    Q_OBJECT

public:
    explicit SelectionTransformationEngine(QObject* parent = nullptr);
    ~SelectionTransformationEngine() override;

    // Component setup
    void setMap(Map* map);
    void setSelection(Selection* selection);
    Map* getMap() const { return map_; }
    Selection* getSelection() const { return selection_; }

    // Main transformation interface
    TransformationResult executeTransformation(const TransformationParameters& params);
    TransformationResult executeTransformation(TransformationType type, const QVariantMap& parameters);
    bool canExecuteTransformation(const TransformationParameters& params) const;
    void cancelTransformation();

    // Move operations
    TransformationResult moveSelection(const QPointF& offset);
    TransformationResult moveSelectionToPosition(const MapPos& targetPosition);
    TransformationResult moveSelectionByTiles(int deltaX, int deltaY, int deltaZ = 0);

    // Rotation operations
    TransformationResult rotateSelection(double degrees, const MapPos& pivot);
    TransformationResult rotateSelectionAroundCenter(double degrees);
    TransformationResult rotateSelectionClockwise();
    TransformationResult rotateSelectionCounterClockwise();

    // Flip operations
    TransformationResult flipSelectionHorizontal(const MapPos& pivot);
    TransformationResult flipSelectionVertical(const MapPos& pivot);
    TransformationResult flipSelectionHorizontalAroundCenter();
    TransformationResult flipSelectionVerticalAroundCenter();

    // Scale operations
    TransformationResult scaleSelection(double scaleX, double scaleY, const MapPos& pivot);
    TransformationResult scaleSelectionUniform(double scale, const MapPos& pivot);

    // Advanced operations
    TransformationResult duplicateSelection(const QPointF& offset);
    TransformationResult mirrorSelection(const MapPos& axis, bool horizontal);
    TransformationResult shearSelection(double shearX, double shearY, const MapPos& pivot);

    // Transformation settings
    void setPreserveItemProperties(bool preserve);
    bool isPreserveItemProperties() const { return preserveItemProperties_; }
    void setMergeWithExisting(bool merge);
    bool isMergeWithExisting() const { return mergeWithExisting_; }
    void setAutoRotateItems(bool autoRotate);
    bool isAutoRotateItems() const { return autoRotateItems_; }
    void setValidateTransformations(bool validate);
    bool isValidateTransformations() const { return validateTransformations_; }

    // Item type handling
    void registerRotatableItemType(quint16 itemType, const QList<quint16>& rotatedVersions);
    void registerFlippableItemType(quint16 itemType, quint16 horizontalFlip, quint16 verticalFlip);
    bool isItemTypeRotatable(quint16 itemType) const;
    bool isItemTypeFlippable(quint16 itemType) const;
    quint16 getRotatedItemType(quint16 itemType, int rotationSteps) const;
    quint16 getFlippedItemType(quint16 itemType, bool horizontal) const;

    // Validation and analysis
    bool validateTransformation(const TransformationParameters& params) const;
    QStringList getTransformationErrors(const TransformationParameters& params) const;
    QSet<MapPos> calculateAffectedPositions(const TransformationParameters& params) const;
    QRect calculateTransformationBounds(const TransformationParameters& params) const;
    MapPos calculateTransformationCenter() const;

    // Performance and optimization
    void setOptimizationEnabled(bool enabled);
    bool isOptimizationEnabled() const { return optimizationEnabled_; }
    void setMaxTransformationSize(int maxTiles);
    int getMaxTransformationSize() const { return maxTransformationSize_; }
    void setBatchProcessing(bool enabled);
    bool isBatchProcessing() const { return batchProcessing_; }

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    TransformationResult getLastResult() const { return lastResult_; }

public slots:
    void onSelectionChanged();
    void onMapChanged();

signals:
    void transformationStarted(const TransformationParameters& params);
    void transformationProgress(int current, int total);
    void transformationCompleted(const TransformationResult& result);
    void transformationCancelled();
    void transformationError(const QString& error);

private:
    // Core transformation implementation
    TransformationResult executeMove(const TransformationParameters& params);
    TransformationResult executeRotation(const TransformationParameters& params);
    TransformationResult executeFlip(const TransformationParameters& params);
    TransformationResult executeScale(const TransformationParameters& params);
    TransformationResult executeShear(const TransformationParameters& params);

    // Tile transformation
    TileTransformationResult transformTile(Tile* tile, const MapPos& originalPos, const MapPos& newPos, const TransformationParameters& params);
    Tile* createTransformedTile(Tile* originalTile, const TransformationParameters& params);
    void copyTileProperties(Tile* source, Tile* destination);
    bool mergeTileWithExisting(Tile* newTile, Tile* existingTile);

    // Item transformation
    ItemTransformationResult transformItem(Item* item, const TransformationParameters& params);
    Item* createTransformedItem(Item* originalItem, const TransformationParameters& params);
    void rotateItem(Item* item, double degrees);
    void flipItem(Item* item, bool horizontal, bool vertical);
    void scaleItem(Item* item, double scaleX, double scaleY);
    void updateItemOrientation(Item* item, int newOrientation);
    void updateItemType(Item* item, quint16 newType);

    // Position calculation
    MapPos transformPosition(const MapPos& position, const TransformationParameters& params);
    QPointF transformPoint(const QPointF& point, const QTransform& transform);
    QTransform createTransformationMatrix(const TransformationParameters& params);
    QTransform createRotationMatrix(double degrees, const MapPos& pivot);
    QTransform createFlipMatrix(bool horizontal, bool vertical, const MapPos& pivot);
    QTransform createScaleMatrix(double scaleX, double scaleY, const MapPos& pivot);

    // Cut/paste logic for move operations
    void cutSelectionFromMap(Map* map, const QSet<MapPos>& positions);
    void pasteSelectionToMap(Map* map, const QSet<MapPos>& positions, const QMap<MapPos, Tile*>& tileData);
    QMap<MapPos, Tile*> extractTileData(Map* map, const QSet<MapPos>& positions);
    void clearPositions(Map* map, const QSet<MapPos>& positions);

    // Validation helpers
    bool isValidPosition(const MapPos& position) const;
    bool isPositionInBounds(const MapPos& position) const;
    bool wouldOverwriteImportantData(const MapPos& position) const;
    bool hasCircularDependency(const TransformationParameters& params) const;

    // Optimization
    void optimizeTransformation(TransformationParameters& params);
    void batchTransformItems(const QList<Item*>& items, const TransformationParameters& params);
    void precomputeTransformations(const TransformationParameters& params);
    void cacheTransformationResults();

    // Error handling
    void handleTransformationError(const QString& error);
    void logTransformationWarning(const QString& warning);
    void addTransformationError(TransformationResult& result, const QString& error);

    // Statistics tracking
    void updateStatistics(const TransformationResult& result);
    void trackTransformationTime(qint64 duration);
    void incrementTransformationCounter(TransformationType type);

private:
    // Core components
    Map* map_;
    Selection* selection_;

    // Transformation settings
    bool preserveItemProperties_;
    bool mergeWithExisting_;
    bool autoRotateItems_;
    bool validateTransformations_;
    bool optimizationEnabled_;
    bool batchProcessing_;
    int maxTransformationSize_;

    // Item type mappings
    QMap<quint16, QList<quint16>> rotatableItemTypes_;
    QMap<quint16, QPair<quint16, quint16>> flippableItemTypes_;

    // State tracking
    TransformationResult lastResult_;
    bool transformationInProgress_;
    bool transformationCancelled_;

    // Performance tracking
    mutable QVariantMap statistics_;
    QMap<TransformationType, int> transformationCounts_;
    qint64 totalTransformationTime_;
    int totalTransformations_;

    // Caching
    QMap<QString, QTransform> transformationMatrixCache_;
    QMap<MapPos, MapPos> positionTransformCache_;
    QMap<quint16, quint16> itemTypeTransformCache_;

    // Constants
    static const int DEFAULT_MAX_TRANSFORMATION_SIZE = 5000;
    static const double ROTATION_EPSILON = 0.001;
    static const double SCALE_EPSILON = 0.001;
};

#endif // SELECTIONTRANSFORMATIONENGINE_H
