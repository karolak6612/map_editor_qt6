#ifndef TILEPROPERTYDIALOG_H
#define TILEPROPERTYDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QColorDialog>
#include <QUndoStack>
#include <QTimer>
#include <QVariantMap>
#include <QStringList>

#include "MapPos.h"

// Forward declarations
class Tile;
class Map;
class TilePropertyEditor;

/**
 * @brief Task 93: Comprehensive Tile Property Dialog
 * 
 * Complete tile property editing dialog with full functionality:
 * - All tile properties editing (flags, house ID, zone IDs, minimap color)
 * - Real-time data binding and validation
 * - Undo/redo support with command pattern
 * - Map updates and automatic refresh
 * - Batch editing for multiple tiles
 * - Property templates and presets
 * - Advanced filtering and search
 * - Import/export functionality
 * - 1:1 compatibility with wxwidgets tile property editing
 */

/**
 * @brief Main Tile Property Dialog
 */
class TilePropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TilePropertyDialog(QWidget* parent = nullptr);
    ~TilePropertyDialog() override;

    // Component setup
    void setMap(Map* map);
    void setUndoStack(QUndoStack* undoStack);
    Map* getMap() const { return map_; }
    QUndoStack* getUndoStack() const { return undoStack_; }

    // Tile editing
    void editTile(Tile* tile);
    void editTiles(const QList<Tile*>& tiles);
    void clearTile();
    Tile* getCurrentTile() const { return currentTile_; }
    QList<Tile*> getCurrentTiles() const { return currentTiles_; }

    // Dialog control
    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return readOnly_; }
    void enableAutoApply(bool enabled);
    bool isAutoApplyEnabled() const { return autoApply_; }
    void enableBatchMode(bool enabled);
    bool isBatchModeEnabled() const { return batchMode_; }

    // Property templates
    void saveTemplate(const QString& name);
    void loadTemplate(const QString& name);
    void deleteTemplate(const QString& name);
    QStringList getTemplateNames() const;

public slots:
    void onApply();
    void onReset();
    void onRefresh();
    void onAutoApplyToggled(bool enabled);
    void onBatchModeToggled(bool enabled);

    // Property change handlers
    void onMapFlagChanged();
    void onStateFlagChanged();
    void onHouseIdChanged();
    void onZoneIdChanged();
    void onMinimapColorChanged();

    // Template management
    void onSaveTemplate();
    void onLoadTemplate();
    void onDeleteTemplate();

signals:
    void tilePropertyChanged(Tile* tile, const QString& property, const QVariant& oldValue, const QVariant& newValue);
    void tilePropertiesApplied(const QList<Tile*>& tiles);
    void dialogClosed();

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void onPropertyChanged();
    void onValidationTimer();
    void onColorButtonClicked();
    void onAddZoneId();
    void onRemoveZoneId();
    void onZoneIdSelectionChanged();

private:
    void setupUI();
    void setupBasicPropertiesTab();
    void setupFlagsTab();
    void setupAdvancedTab();
    void setupTemplatesTab();
    void setupButtonBox();

    // Data binding
    void bindControls();
    void unbindControls();
    void updateControlsFromTile();
    void updateControlsFromTiles();
    void updateTileFromControls();
    void updateTilesFromControls();

    // Validation
    bool validateAllProperties();
    bool validateProperty(const QString& property, const QVariant& value);
    void showValidationError(const QString& property, const QString& error);
    void clearValidationErrors();

    // Change tracking
    void trackPropertyChange(const QString& property, const QVariant& oldValue, const QVariant& newValue);
    bool hasUnsavedChanges() const;
    void markAsModified(bool modified = true);
    void resetToOriginalValues();

    // Command creation
    void applyChanges();
    void createAndExecuteCommand();
    QUndoCommand* createPropertyCommand(const QString& property, const QVariant& oldValue, const QVariant& newValue);
    QUndoCommand* createBatchCommand();

    // UI updates
    void updateUI();
    void updateButtonStates();
    void updateTabStates();
    void updateBatchModeUI();
    void updateReadOnlyState();

    // Template management
    void updateTemplatesList();
    QVariantMap getCurrentProperties() const;
    void applyProperties(const QVariantMap& properties);

    // Helper methods
    QString formatPropertyValue(const QString& property, const QVariant& value) const;
    QColor getMinimapColor(quint8 colorIndex) const;
    quint8 getMinimapColorIndex(const QColor& color) const;

private:
    // Core components
    Map* map_;
    QUndoStack* undoStack_;
    Tile* currentTile_;
    QList<Tile*> currentTiles_;

    // UI components
    QTabWidget* tabWidget_;
    
    // Basic Properties Tab
    QWidget* basicTab_;
    QLineEdit* positionLineEdit_;
    QSpinBox* houseIdSpinBox_;
    QListWidget* zoneIdsList_;
    QLineEdit* zoneIdLineEdit_;
    QPushButton* addZoneIdButton_;
    QPushButton* removeZoneIdButton_;
    QLineEdit* minimapColorLineEdit_;
    QPushButton* minimapColorButton_;
    QLabel* minimapColorPreview_;

    // Flags Tab
    QWidget* flagsTab_;
    QGroupBox* mapFlagsGroup_;
    QGroupBox* stateFlagsGroup_;
    
    // Map flags checkboxes
    QCheckBox* protectionZoneCheckBox_;
    QCheckBox* noPvpZoneCheckBox_;
    QCheckBox* pvpZoneCheckBox_;
    QCheckBox* refreshCheckBox_;
    QCheckBox* zoneBrushCheckBox_;
    
    // State flags checkboxes
    QCheckBox* uniqueCheckBox_;
    QCheckBox* optionalBorderCheckBox_;
    QCheckBox* hasTableCheckBox_;
    QCheckBox* hasCarpetCheckBox_;
    
    // Raw flag editors
    QLineEdit* mapFlagsLineEdit_;
    QLineEdit* stateFlagsLineEdit_;

    // Advanced Tab
    QWidget* advancedTab_;
    QTextEdit* debugInfoText_;
    QTableWidget* propertiesTable_;
    QCheckBox* showAdvancedCheckBox_;

    // Templates Tab
    QWidget* templatesTab_;
    QListWidget* templatesList_;
    QLineEdit* templateNameLineEdit_;
    QPushButton* saveTemplateButton_;
    QPushButton* loadTemplateButton_;
    QPushButton* deleteTemplateButton_;

    // Button box
    QWidget* buttonBox_;
    QPushButton* applyButton_;
    QPushButton* resetButton_;
    QPushButton* refreshButton_;
    QPushButton* closeButton_;
    QCheckBox* autoApplyCheckBox_;
    QCheckBox* batchModeCheckBox_;
    QLabel* modifiedLabel_;

    // State management
    bool readOnly_;
    bool autoApply_;
    bool batchMode_;
    bool hasUnsavedChanges_;
    QVariantMap originalValues_;
    QVariantMap pendingChanges_;

    // Validation
    QTimer* validationTimer_;
    QStringList validationErrors_;

    // Color management
    QColor currentMinimapColor_;
    QColorDialog* colorDialog_;

    // Templates
    QVariantMap templates_;
    QString templatesFilePath_;

    // Constants
    static const int VALIDATION_DELAY = 500; // milliseconds
};

/**
 * @brief Tile Property Batch Editor for multiple tiles
 */
class TilePropertyBatchEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TilePropertyBatchEditor(QWidget* parent = nullptr);
    ~TilePropertyBatchEditor() override = default;

    // Component setup
    void setMap(Map* map);
    void setUndoStack(QUndoStack* undoStack);

    // Batch editing
    void editTiles(const QList<Tile*>& tiles);
    void clearTiles();
    QList<Tile*> getTiles() const { return tiles_; }

    // Property operations
    void setPropertyForAll(const QString& property, const QVariant& value);
    void addFlagToAll(const QString& flagType, quint32 flag);
    void removeFlagFromAll(const QString& flagType, quint32 flag);
    void addZoneIdToAll(quint16 zoneId);
    void removeZoneIdFromAll(quint16 zoneId);

public slots:
    void onApplyToAll();
    void onResetAll();

signals:
    void batchOperationCompleted(const QList<Tile*>& tiles);
    void batchOperationFailed(const QString& error);

private:
    void setupUI();
    void updateUI();
    bool validateBatchOperation(const QString& property, const QVariant& value);
    QUndoCommand* createBatchCommand(const QString& property, const QVariant& value);

private:
    Map* map_;
    QUndoStack* undoStack_;
    QList<Tile*> tiles_;

    // UI components
    QComboBox* propertyComboBox_;
    QLineEdit* valueLineEdit_;
    QPushButton* applyToAllButton_;
    QPushButton* resetAllButton_;
    QLabel* tilesCountLabel_;
    QProgressBar* progressBar_;
};

#endif // TILEPROPERTYDIALOG_H
