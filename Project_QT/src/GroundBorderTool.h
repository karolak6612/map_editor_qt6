#ifndef GROUNDBORDERTOOL_H
#define GROUNDBORDERTOOL_H

#include "Brush.h"
#include "BorderSystem.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "QUndoCommand"
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QList>
#include <QMap>
#include <QVariant>

// Forward declarations
class GroundBrush;
class SettingsManager;

/**
 * @brief Task 98: Ground Border Tool - Specialized tool for fine-grained ground border control
 * 
 * This tool provides specialized functionality for placing and adjusting ground borders,
 * separate from the general automagic border application. It offers:
 * - Manual border placement between different ground types
 * - Border fixing and adjustment for existing ground
 * - Specialized border rules and overrides
 * - Integration with BorderSystem and GroundBrush
 * - Complete wxwidgets compatibility
 */

/**
 * @brief Ground border operation mode
 */
enum class GroundBorderMode {
    AUTO_FIX,           // Automatically fix borders on existing ground
    MANUAL_PLACE,       // Manually place borders between ground types
    TRANSITION_BORDER,  // Place specific transition borders
    BORDER_OVERRIDE,    // Override existing borders with new ones
    BORDER_REMOVE,      // Remove specific borders
    BORDER_VALIDATE     // Validate and report border issues
};

/**
 * @brief Ground border tool configuration
 */
struct GroundBorderToolConfig {
    GroundBorderMode mode;
    bool respectWalls;
    bool layerCarpets;
    bool overrideExisting;
    bool validatePlacement;
    int customBorderId;
    QList<quint16> allowedGroundTypes;
    QList<quint16> excludedGroundTypes;
    
    GroundBorderToolConfig()
        : mode(GroundBorderMode::AUTO_FIX)
        , respectWalls(true)
        , layerCarpets(true)
        , overrideExisting(false)
        , validatePlacement(true)
        , customBorderId(0) {}
};

/**
 * @brief Border placement result
 */
struct BorderPlacementResult {
    bool success;
    QString errorMessage;
    QList<QPoint> affectedTiles;
    QList<quint16> placedBorderIds;
    int totalBordersPlaced;
    int totalBordersRemoved;
    
    BorderPlacementResult()
        : success(false)
        , totalBordersPlaced(0)
        , totalBordersRemoved(0) {}
};

/**
 * @brief Ground Border Tool Brush - Specialized brush for ground border operations
 */
class GroundBorderToolBrush : public Brush
{
    Q_OBJECT

public:
    explicit GroundBorderToolBrush(QObject* parent = nullptr);
    ~GroundBorderToolBrush() override = default;

    // Brush interface implementation
    Type type() const override { return Type::GroundBorderTool; }
    QString name() const override { return "Ground Border Tool"; }
    // Note: getDescription() is not part of base Brush interface - removed override
    QString getDescription() const { return "Specialized tool for fine-grained ground border control"; }
    
    QUndoCommand* applyBrush(Map* map, const QPointF& position, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;  // FIXED: Added missing drawingContext parameter
    QUndoCommand* applyBrushArea(Map* map, const QRect& area, QUndoCommand* parentCommand = nullptr);  // FIXED: Removed override - not in base class
    QUndoCommand* applyBrushSelection(Map* map, const QList<QPoint>& selection, QUndoCommand* parentCommand = nullptr);  // FIXED: Removed override - not in base class

    // Ground border tool specific methods
    void setConfiguration(const GroundBorderToolConfig& config);
    GroundBorderToolConfig getConfiguration() const { return config_; }
    
    void setMode(GroundBorderMode mode);
    GroundBorderMode getMode() const { return config_.mode; }
    
    void setCustomBorderId(int borderId);
    int getCustomBorderId() const { return config_.customBorderId; }
    
    void setAllowedGroundTypes(const QList<quint16>& groundTypes);
    QList<quint16> getAllowedGroundTypes() const { return config_.allowedGroundTypes; }
    
    void setExcludedGroundTypes(const QList<quint16>& groundTypes);
    QList<quint16> getExcludedGroundTypes() const { return config_.excludedGroundTypes; }

    // Border analysis and validation
    BorderPlacementResult analyzeBorderNeeds(Map* map, const QPoint& position);
    BorderPlacementResult analyzeBorderNeeds(Map* map, const QRect& area);
    BorderPlacementResult analyzeBorderNeeds(Map* map, const QList<QPoint>& selection);
    
    bool canPlaceBorderAt(Map* map, const QPoint& position, quint16 borderId) const;
    bool shouldPlaceBorderBetween(Map* map, const QPoint& pos1, const QPoint& pos2) const;
    QList<quint16> getSuggestedBorderIds(Map* map, const QPoint& position) const;

    // Border operations
    BorderPlacementResult placeBorderAt(Map* map, const QPoint& position, quint16 borderId);
    BorderPlacementResult removeBorderAt(Map* map, const QPoint& position, quint16 borderId = 0);
    BorderPlacementResult fixBordersAt(Map* map, const QPoint& position);
    BorderPlacementResult validateBordersAt(Map* map, const QPoint& position);

    // Integration with existing systems
    void setBorderSystem(BorderSystem* borderSystem);
    BorderSystem* getBorderSystem() const { return borderSystem_; }
    
    void setGroundBrush(GroundBrush* groundBrush);
    GroundBrush* getGroundBrush() const { return groundBrush_; }

    // Settings integration
    void loadFromSettings(SettingsManager* settingsManager);
    void saveToSettings(SettingsManager* settingsManager);
    void updateFromSettings();

public slots:
    void onBorderSystemChanged();
    void onGroundBrushChanged();
    void onSettingsChanged();

signals:
    void borderPlaced(const QPoint& position, quint16 borderId);
    void borderRemoved(const QPoint& position, quint16 borderId);
    void borderOperationCompleted(const BorderPlacementResult& result);
    void configurationChanged(const GroundBorderToolConfig& config);

private:
    // Border placement implementation
    BorderPlacementResult performAutoFix(Map* map, const QPoint& position);
    BorderPlacementResult performManualPlace(Map* map, const QPoint& position);
    BorderPlacementResult performTransitionBorder(Map* map, const QPoint& position);
    BorderPlacementResult performBorderOverride(Map* map, const QPoint& position);
    BorderPlacementResult performBorderRemove(Map* map, const QPoint& position);
    BorderPlacementResult performBorderValidate(Map* map, const QPoint& position);

    // Border analysis helpers
    bool isGroundTile(Map* map, const QPoint& position) const;
    quint16 getGroundTypeAt(Map* map, const QPoint& position) const;
    QList<quint16> getBorderItemsAt(Map* map, const QPoint& position) const;
    bool hasBorderBetween(Map* map, const QPoint& pos1, const QPoint& pos2) const;
    
    // Ground type compatibility
    bool areGroundTypesCompatible(quint16 groundType1, quint16 groundType2) const;
    bool needsBorderBetween(quint16 groundType1, quint16 groundType2) const;
    quint16 getBorderIdForTransition(quint16 groundType1, quint16 groundType2) const;
    
    // Border validation
    bool isValidBorderPlacement(Map* map, const QPoint& position, quint16 borderId) const;
    bool respectsWallConstraints(Map* map, const QPoint& position, quint16 borderId) const;
    bool respectsLayerConstraints(Map* map, const QPoint& position, quint16 borderId) const;
    
    // Utility methods
    QList<QPoint> getNeighborPositions(const QPoint& position) const;
    QList<QPoint> getAffectedPositions(Map* map, const QPoint& position) const;
    void logBorderOperation(const QString& operation, const QPoint& position, quint16 borderId);

private:
    GroundBorderToolConfig config_;
    BorderSystem* borderSystem_;
    GroundBrush* groundBrush_;
    SettingsManager* settingsManager_;
    
    // Border type mappings
    QMap<QPair<quint16, quint16>, quint16> transitionBorderMap_;
    QMap<quint16, QList<quint16>> compatibleGroundTypes_;
    QMap<quint16, QList<quint16>> borderVariations_;
    
    // Performance optimization
    mutable QMap<QPoint, BorderPlacementResult> analysisCache_;
    mutable QMap<QPair<quint16, quint16>, bool> compatibilityCache_;
};

/**
 * @brief Undo command for ground border tool operations
 */
class GroundBorderToolCommand : public QUndoCommand
{
public:
    GroundBorderToolCommand(Map* map, const QPoint& position, const GroundBorderToolConfig& config,
                           const BorderPlacementResult& result, QUndoCommand* parent = nullptr);
    ~GroundBorderToolCommand() override = default;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPoint position_;
    GroundBorderToolConfig config_;
    BorderPlacementResult result_;
    QMap<QPoint, QList<quint16>> originalBorderItems_;
    QMap<QPoint, QList<quint16>> newBorderItems_;
    bool executed_;
};

/**
 * @brief Ground Border Tool Manager - Manages ground border tool instances and operations
 */
class GroundBorderToolManager : public QObject
{
    Q_OBJECT

public:
    explicit GroundBorderToolManager(QObject* parent = nullptr);
    ~GroundBorderToolManager() override = default;

    // Tool management
    GroundBorderToolBrush* createGroundBorderTool(const GroundBorderToolConfig& config = GroundBorderToolConfig());
    void destroyGroundBorderTool(GroundBorderToolBrush* tool);
    QList<GroundBorderToolBrush*> getActiveTools() const { return activeTools_; }

    // Global operations
    BorderPlacementResult fixAllBorders(Map* map, const QRect& area);
    BorderPlacementResult validateAllBorders(Map* map, const QRect& area);
    BorderPlacementResult removeAllBorders(Map* map, const QRect& area);

    // Settings and configuration
    void setDefaultConfiguration(const GroundBorderToolConfig& config);
    GroundBorderToolConfig getDefaultConfiguration() const { return defaultConfig_; }
    
    void setBorderSystem(BorderSystem* borderSystem);
    BorderSystem* getBorderSystem() const { return borderSystem_; }

public slots:
    void onBorderSystemChanged();
    void onSettingsChanged();

signals:
    void toolCreated(GroundBorderToolBrush* tool);
    void toolDestroyed(GroundBorderToolBrush* tool);
    void globalOperationCompleted(const BorderPlacementResult& result);

private:
    QList<GroundBorderToolBrush*> activeTools_;
    GroundBorderToolConfig defaultConfig_;
    BorderSystem* borderSystem_;
};

#endif // GROUNDBORDERTOOL_H
