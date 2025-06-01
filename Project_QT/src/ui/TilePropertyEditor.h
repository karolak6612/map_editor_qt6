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

    // Task 49: Enhanced tile property display
    void displayTileProperties(Tile* tile);
    void clearProperties();

    // Task 49: Property access methods
    bool hasValidTile() const { return currentTile_ != nullptr; }
    Tile* getCurrentTile() const { return currentTile_; }

public slots:
    // Task 49: Placeholder slots for future editing functionality
    void onRefreshProperties();
    void onTileSelectionChanged();

signals:
    // Task 49: Signals for future integration
    void tilePropertyChanged(Tile* tile, const QString& property, const QVariant& value);
    void refreshRequested();

private:
    void setupUI();
    void setupBasicPropertiesGroup();
    void setupFlagsGroup();
    void setupItemsGroup();
    void setupAdvancedGroup();

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
    QTextEdit* m_debugInfoText;
    QPushButton* m_refreshButton;

    // Task 49: Current tile reference
    Tile* currentTile_;
};

#endif // TILEPROPERTYEDITOR_H
