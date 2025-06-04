#ifndef TILEPROPERTYEDITOR_H
#define TILEPROPERTYEDITOR_H

#include <QWidget>

// Forward declarations
class QLabel;
class QLineEdit;
class QCheckBox;
class QFormLayout;
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QSpinBox;
class QComboBox;
class QTextEdit;
class QListWidget;
class QTabWidget;
class Tile; // For future use with displayTileProperties

class TilePropertyEditor : public QWidget {
    Q_OBJECT
public:
    explicit TilePropertyEditor(QWidget *parent = nullptr);
    ~TilePropertyEditor() override;

    // Task 93: Enhanced tile property editing with full functionality
    void displayTileProperties(Tile* tile);
    void clearProperties();
    void setMap(Map* map);
    void setUndoStack(QUndoStack* undoStack);

    // Task 93: Property access methods
    bool hasValidTile() const { return currentTile_ != nullptr; }
    Tile* getCurrentTile() const { return currentTile_; }
    Map* getMap() const { return map_; }

    // Task 93: Editing control
    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return readOnly_; }
    void enableAutoApply(bool enabled);
    bool isAutoApplyEnabled() const { return autoApply_; }

public slots:
    // Task 93: Enhanced editing functionality
    void onRefreshProperties();
    void onTileSelectionChanged();
    void onApplyChanges();
    void onResetChanges();
    void onPropertyChanged();

    // Task 93: Individual property change handlers
    void onMapFlagChanged();
    void onStateFlagChanged();
    void onHouseIdChanged();
    void onZoneIdChanged();
    void onMinimapColorChanged();
    void onMinimapColorButtonClicked();

signals:
    // Task 93: Enhanced signals for map integration
    void tilePropertyChanged(Tile* tile, const QString& property, const QVariant& oldValue, const QVariant& newValue);
    void tilePropertiesApplied(Tile* tile);
    void refreshRequested();
    void undoCommandCreated(QUndoCommand* command);

private:
    void setupUI();
    void setupBasicPropertiesGroup();
    void setupFlagsGroup();
    void setupItemsGroup();
    void setupAdvancedGroup();
    void setupEditingControls();

    // Task 93: Data binding and validation
    void bindPropertyControls();
    void unbindPropertyControls();
    void updateControlsFromTile();
    void updateTileFromControls();
    bool validatePropertyChanges();
    void resetControlsToOriginalValues();

    // Task 93: Property change tracking
    void trackPropertyChange(const QString& property, const QVariant& oldValue, const QVariant& newValue);
    bool hasUnsavedChanges() const;
    void markAsModified(bool modified = true);

    // Task 93: Command creation
    QUndoCommand* createPropertyCommand(const QString& property, const QVariant& oldValue, const QVariant& newValue);
    void applyPropertyCommand(QUndoCommand* command);

    // Task 93: UI update helpers
    void updateMapFlagControls();
    void updateStateFlagControls();
    void updateMinimapColorDisplay();
    void updateEditingControlsState();

    // Task 93: Validation helpers
    bool isValidHouseId(quint32 houseId) const;
    bool isValidZoneId(quint16 zoneId) const;
    bool isValidMinimapColor(quint8 color) const;

    // Task 49: UI organization
    QTabWidget* m_tabWidget;

    // Basic Properties Tab
    QWidget* m_basicTab;
    QLineEdit* m_positionLineEdit;
    QLineEdit* m_houseIdLineEdit;
    QLineEdit* m_zoneIdsLineEdit;
    QLineEdit* m_itemCountLineEdit;
    QLineEdit* m_creatureCountLineEdit;
    QLineEdit* m_memoryUsageLineEdit;
    QCheckBox* m_isEmptyCheckBox;
    QCheckBox* m_isModifiedCheckBox;
    QCheckBox* m_isSelectedCheckBox;
    QCheckBox* m_isBlockingCheckBox;

    // Flags Tab
    QWidget* m_flagsTab;
    QGroupBox* m_mapFlagsGroup;
    QCheckBox* m_pzCheckBox;
    QCheckBox* m_noPvpCheckBox;
    QCheckBox* m_noLogoutCheckBox;
    QCheckBox* m_pvpZoneCheckBox;
    QCheckBox* m_refreshCheckBox;
    QCheckBox* m_zoneBrushCheckBox;

    QGroupBox* m_stateFlagsGroup;
    QCheckBox* m_uniqueCheckBox;
    QCheckBox* m_optionalBorderCheckBox;
    QCheckBox* m_hasTableCheckBox;
    QCheckBox* m_hasCarpetCheckBox;

    QLineEdit* m_mapFlagsLineEdit;
    QLineEdit* m_stateFlagsLineEdit;

    // Items Tab
    QWidget* m_itemsTab;
    QListWidget* m_itemsList;
    QLineEdit* m_groundItemLineEdit;
    QLineEdit* m_topItemLineEdit;
    QLineEdit* m_topSelectableItemLineEdit;
    QLineEdit* m_creatureLineEdit;
    QLineEdit* m_spawnLineEdit;

    // Advanced Tab
    QWidget* m_advancedTab;
    QLineEdit* m_minimapColorLineEdit;
    QPushButton* m_minimapColorButton;
    QTextEdit* m_debugInfoText;
    QPushButton* m_refreshButton;

    // Task 93: Editing controls
    QWidget* m_editingControlsWidget;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    QCheckBox* m_autoApplyCheckBox;
    QLabel* m_modifiedLabel;

    // Task 93: Enhanced property controls
    QSpinBox* m_houseIdSpinBox;
    QLineEdit* m_zoneIdLineEdit;
    QPushButton* m_addZoneIdButton;
    QPushButton* m_removeZoneIdButton;
    QListWidget* m_zoneIdsList;

    // Task 93: Core components
    Tile* currentTile_;
    Map* map_;
    QUndoStack* undoStack_;

    // Task 93: State management
    bool readOnly_;
    bool autoApply_;
    bool hasUnsavedChanges_;
    QVariantMap originalValues_;
    QVariantMap pendingChanges_;

    // Task 93: Color management
    QColor currentMinimapColor_;
};

#endif // TILEPROPERTYEDITOR_H
