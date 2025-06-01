#ifndef SPAWNEDITORDIALOG_H
#define SPAWNEDITORDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QToolButton>
#include <QButtonGroup>
#include <QSplitter>
#include <QProgressBar>
#include <QSlider>
#include <QScrollArea>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QTimer>
#include <QUndoStack>
#include <QUndoCommand>

#include "Spawn.h"
#include "CreatureManager.h"
#include "Map.h"
#include "MapView.h"

/**
 * @brief Task 87: Spawn Editor Dialog
 * 
 * Comprehensive spawn editing dialog with:
 * - Complete spawn configuration (radius, interval, max creatures)
 * - Creature type selection and management
 * - Visual preview of spawn area
 * - Validation and error checking
 * - Undo/redo support
 * - 1:1 compatibility with wxwidgets spawn editor
 */

class SpawnEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpawnEditorDialog(Spawn* spawn, Map* map, QUndoStack* undoStack, QWidget* parent = nullptr);
    ~SpawnEditorDialog() override;

    // Dialog management
    Spawn* getSpawn() const { return spawn_; }
    bool hasChanges() const { return hasChanges_; }
    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return readOnly_; }

public slots:
    void accept() override;
    void reject() override;
    void apply();
    void reset();
    void validateSpawn();

private slots:
    // Basic properties slots
    void onPositionChanged();
    void onRadiusChanged();
    void onIntervalChanged();
    void onMaxCreaturesChanged();
    
    // Creature management slots
    void onAddCreature();
    void onRemoveCreature();
    void onCreatureSelectionChanged();
    void onCreatureDoubleClicked(QListWidgetItem* item);
    void onAvailableCreatureDoubleClicked(QListWidgetItem* item);
    
    // Preview and validation slots
    void onPreviewSpawn();
    void onCenterOnSpawn();
    void onHighlightSpawn();
    void onValidationTimer();
    
    // Utility slots
    void onSpawnChanged();
    void onMapChanged();
    void onUndoRedoChanged();

signals:
    void spawnModified(Spawn* spawn);
    void requestMapHighlight(const QList<MapPos>& positions);
    void requestMapCenter(const MapPos& position);

private:
    void setupUI();
    void setupBasicPropertiesTab();
    void setupCreatureManagementTab();
    void setupPreviewTab();
    void setupValidationTab();
    void setupButtonBox();
    
    void connectSignals();
    void loadSpawnData();
    void saveSpawnData();
    void updateUI();
    void updateValidation();
    void updatePreview();
    void updateStatistics();
    
    // Validation helpers
    bool validateBasicProperties();
    bool validateCreatures();
    bool validateSpawnArea();
    void showValidationResults();
    
    // UI update helpers
    void updateCreatureList();
    void updateAvailableCreaturesList();
    void updatePositionDisplay();
    void updateStatisticsDisplay();
    void updatePreviewArea();
    
    // Data management
    void markAsChanged();
    void createUndoCommand(const QString& description);
    
    // Helper methods
    QList<MapPos> getSpawnAreaPositions() const;
    bool isValidCreatureName(const QString& name) const;
    CreatureManager* getCreatureManager() const;

private:
    // Core data
    Spawn* spawn_;
    Spawn* originalSpawn_; // For reset functionality
    Map* map_;
    QUndoStack* undoStack_;
    
    // State management
    bool hasChanges_;
    bool readOnly_;
    bool isLoading_;
    
    // UI components
    QTabWidget* tabWidget_;
    
    // Basic Properties Tab
    QWidget* basicPropertiesTab_;
    QSpinBox* positionXSpinBox_;
    QSpinBox* positionYSpinBox_;
    QSpinBox* positionZSpinBox_;
    QSpinBox* radiusSpinBox_;
    QSpinBox* intervalSpinBox_;
    QSpinBox* maxCreaturesSpinBox_;
    QLabel* positionLabel_;
    QLabel* areaLabel_;
    
    // Creature Management Tab
    QWidget* creatureManagementTab_;
    QListWidget* assignedCreaturesList_;
    QListWidget* availableCreaturesList_;
    QPushButton* addCreatureButton_;
    QPushButton* removeCreatureButton_;
    QLineEdit* creatureFilterEdit_;
    QLabel* creatureCountLabel_;
    
    // Preview Tab
    QWidget* previewTab_;
    QTextEdit* previewDisplay_;
    QPushButton* previewButton_;
    QPushButton* centerButton_;
    QPushButton* highlightButton_;
    QLabel* previewStatusLabel_;
    
    // Validation Tab
    QWidget* validationTab_;
    QTextEdit* validationResults_;
    QProgressBar* validationProgress_;
    QPushButton* validateButton_;
    
    // Statistics display
    QLabel* statisticsLabel_;
    QTextEdit* statisticsDisplay_;
    
    // Button box
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QPushButton* applyButton_;
    QPushButton* resetButton_;
    
    // Timers and utilities
    QTimer* validationTimer_;
    QTimer* previewTimer_;
};

/**
 * @brief Creature Selection Dialog
 */
class CreatureSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreatureSelectionDialog(CreatureManager* creatureManager, QWidget* parent = nullptr);
    ~CreatureSelectionDialog() override = default;

    // Selection management
    QString getSelectedCreatureName() const;
    void setSelectedCreatureName(const QString& name);
    QStringList getSelectedCreatureNames() const;
    void setSelectedCreatureNames(const QStringList& names);
    
    // Multi-selection support
    void setMultiSelectionEnabled(bool enabled);
    bool isMultiSelectionEnabled() const;

private slots:
    void onCreatureSelectionChanged();
    void onFilterTextChanged();
    void onCreatureDoubleClicked(QListWidgetItem* item);

private:
    void setupUI();
    void populateCreatureList();
    void filterCreatureList();
    void connectSignals();

private:
    CreatureManager* creatureManager_;
    bool multiSelectionEnabled_;
    
    QVBoxLayout* layout_;
    QLineEdit* filterEdit_;
    QListWidget* creatureList_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
};

/**
 * @brief Spawn Area Preview Widget
 */
class SpawnAreaPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpawnAreaPreviewWidget(QWidget* parent = nullptr);
    ~SpawnAreaPreviewWidget() override = default;

    // Spawn configuration
    void setSpawn(const Spawn* spawn);
    void setMap(Map* map);
    
    // Visual settings
    void setShowGrid(bool show);
    bool isShowGrid() const { return showGrid_; }
    
    void setShowCreatures(bool show);
    bool isShowCreatures() const { return showCreatures_; }
    
    void setHighlightSpawnArea(bool highlight);
    bool isHighlightSpawnArea() const { return highlightSpawnArea_; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

signals:
    void positionClicked(const MapPos& position);
    void areaChanged();

private:
    void drawSpawnArea(QPainter* painter);
    void drawGrid(QPainter* painter);
    void drawCreatures(QPainter* painter);
    void drawSpawnRadius(QPainter* painter);
    
    MapPos screenToMap(const QPoint& screenPos) const;
    QPoint mapToScreen(const MapPos& mapPos) const;
    
    void updateViewport();

private:
    const Spawn* spawn_;
    Map* map_;
    
    // Visual settings
    bool showGrid_;
    bool showCreatures_;
    bool highlightSpawnArea_;
    
    // View settings
    QPointF viewCenter_;
    double zoomFactor_;
    int tileSize_;
    
    // Interaction state
    bool dragging_;
    QPoint lastMousePos_;
};

#endif // SPAWNEDITORDIALOG_H
