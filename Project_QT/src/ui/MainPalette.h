#ifndef MAINPALETTE_H
#define MAINPALETTE_H

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QSplitter>
#include <QTimer>
#include "TilesetManager.h"
#include "BrushManager.h"
#include "ItemManager.h"
#include "BrushSizePanel.h"  // Include the proper BrushSizePanel class

// Forward declarations
class Map;
class Brush;
class TilesetCategory;
class ItemButton;
class BrushToolPanel;

// Task 70: Main palette widget for tileset-based item/brush selection
class MainPalette : public QWidget {
    Q_OBJECT

public:
    explicit MainPalette(QWidget* parent = nullptr);
    ~MainPalette() override;

    // Integration methods
    void setTilesetManager(TilesetManager* tilesetManager);
    void setBrushManager(BrushManager* brushManager);
    void setItemManager(ItemManager* itemManager);
    void setMap(Map* map);
    
    // Palette management
    void initializePalettes();
    void reloadPalettes();
    void invalidateContents();
    void loadCurrentContents();
    
    // Selection methods
    Brush* getSelectedBrush() const;
    int getSelectedBrushSize() const;
    TilesetCategoryType getSelectedPaletteType() const;
    bool selectBrush(const Brush* brush, TilesetCategoryType preferredType = TilesetCategoryType::Unknown);
    
    // Page selection
    void selectPage(TilesetCategoryType type);
    void selectPage(int index);
    
    // Action ID support (from wxwidgets)
    void setActionIdEnabled(bool enabled);
    bool isActionIdEnabled() const;
    void setActionId(quint16 actionId);
    quint16 getActionId() const;

signals:
    void brushSelected(Brush* brush);
    void brushSizeChanged(int size);
    void paletteChanged(TilesetCategoryType type);
    void actionIdChanged(quint16 actionId, bool enabled);

public slots:
    void onMapChanged(Map* map);
    void onTilesetChanged();
    void onBrushSizeUpdate(int size);
    void onSettingsChanged();

private slots:
    void onTabChanged(int index);
    void onActionIdToggled(bool enabled);
    void onActionIdValueChanged(int value);
    void onRefreshTimer();

private:
    // UI setup methods
    void setupUI();
    void setupActionIdControls();
    void setupTabWidget();
    void connectSignals();
    
    // Palette creation methods
    QWidget* createTerrainPalette();
    QWidget* createDoodadPalette();
    QWidget* createItemPalette();
    QWidget* createCollectionPalette();
    QWidget* createCreaturePalette();
    QWidget* createHousePalette();
    QWidget* createWaypointPalette();
    QWidget* createRawPalette();
    
    // Helper methods
    void populatePaletteFromCategory(QWidget* palette, TilesetCategoryType categoryType);
    void updatePaletteVisibility();
    void updateButtonStates();
    TilesetCategoryType indexToTilesetType(int index) const;
    int tilesetTypeToIndex(TilesetCategoryType type) const;
    
    // Main UI components
    QVBoxLayout* mainLayout_;
    QHBoxLayout* actionIdLayout_;
    QTabWidget* tabWidget_;
    
    // Action ID controls (from wxwidgets)
    QSpinBox* actionIdSpinBox_;
    QCheckBox* actionIdCheckBox_;
    quint16 actionId_;
    bool actionIdEnabled_;
    
    // Palette tabs
    QWidget* terrainPalette_;
    QWidget* doodadPalette_;
    QWidget* itemPalette_;
    QWidget* collectionPalette_;
    QWidget* creaturePalette_;
    QWidget* housePalette_;
    QWidget* waypointPalette_;
    QWidget* rawPalette_;
    
    // Integration
    TilesetManager* tilesetManager_;
    BrushManager* brushManager_;
    ItemManager* itemManager_;
    Map* map_;
    
    // State management
    Brush* selectedBrush_;
    int selectedBrushSize_;
    TilesetCategoryType currentPaletteType_;
    bool needsRefresh_;
    
    // Auto-refresh timer
    QTimer* refreshTimer_;
};

// Task 70: Tileset category panel for displaying items/brushes
class TilesetCategoryPanel : public QWidget {
    Q_OBJECT

public:
    explicit TilesetCategoryPanel(TilesetCategoryType categoryType, QWidget* parent = nullptr);
    ~TilesetCategoryPanel() override;

    // Setup and population
    void setTilesetManager(TilesetManager* tilesetManager);
    void setBrushManager(BrushManager* brushManager);
    void setItemManager(ItemManager* itemManager);
    void populateFromCategory(TilesetCategory* category);
    void clearContents();
    void refreshContents();
    
    // Selection methods
    Brush* getSelectedBrush() const;
    bool selectBrush(const Brush* brush);
    void selectFirstBrush();
    
    // Display options
    void setDisplayMode(int mode); // 0=List, 1=Grid, 2=Large Icons
    void setShowItemIds(bool show);
    void setIconSize(int size);

signals:
    void brushSelected(Brush* brush);
    void selectionChanged();

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem* item);
    void onDisplayModeChanged();

private:
    // UI setup
    void setupUI();
    void setupToolbar();
    void setupItemList();
    void connectSignals();
    
    // Item management
    void addBrushToList(Brush* brush);
    void addItemToList(quint16 itemId);
    void addCreatureToList(const QString& creatureName);
    QListWidgetItem* createBrushItem(Brush* brush);
    QListWidgetItem* createItemItem(quint16 itemId);
    QListWidgetItem* createCreatureItem(const QString& creatureName);
    
    // Helper methods
    QPixmap getBrushIcon(Brush* brush, int size = 32);
    QPixmap getItemIcon(quint16 itemId, int size = 32);
    QPixmap getCreatureIcon(const QString& creatureName, int size = 32);
    QString getBrushTooltip(Brush* brush);
    QString getItemTooltip(quint16 itemId);
    QString getCreatureTooltip(const QString& creatureName);
    
    // UI components
    QVBoxLayout* mainLayout_;
    QToolBar* toolbar_;
    QScrollArea* scrollArea_;
    QListWidget* itemList_;
    
    // Toolbar controls
    QAction* listModeAction_;
    QAction* gridModeAction_;
    QAction* largeModeAction_;
    QActionGroup* displayModeGroup_;
    QAction* showIdsAction_;
    QComboBox* iconSizeCombo_;
    
    // Data
    TilesetCategoryType categoryType_;
    TilesetManager* tilesetManager_;
    BrushManager* brushManager_;
    ItemManager* itemManager_;
    TilesetCategory* currentCategory_;
    
    // Display settings
    int displayMode_; // 0=List, 1=Grid, 2=Large Icons
    bool showItemIds_;
    int iconSize_;
    
    // Selection state
    Brush* selectedBrush_;
    QListWidgetItem* selectedItem_;
};

// BrushSizePanel is now included from BrushSizePanel.h

// Task 70: Tool panel for brush tool selection
class BrushToolPanel : public QWidget {
    Q_OBJECT

public:
    explicit BrushToolPanel(QWidget* parent = nullptr);
    ~BrushToolPanel() override;

    // Tool selection
    void setSelectedTool(int tool);
    int getSelectedTool() const;
    
    // Toolbar integration
    void setToolbarIconSize(bool large);

signals:
    void toolSelected(int tool);

private slots:
    void onToolChanged();

private:
    void setupUI();
    void connectSignals();
    
    QHBoxLayout* layout_;
    QButtonGroup* toolGroup_;
    QPushButton* paintButton_;
    QPushButton* fillButton_;
    QPushButton* replaceButton_;
    QPushButton* selectButton_;
    
    int selectedTool_;
    bool largeIcons_;
};

#endif // MAINPALETTE_H
