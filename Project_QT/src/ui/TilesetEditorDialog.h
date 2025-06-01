#ifndef TILESETEDITOR_DIALOG_H
#define TILESETEDITOR_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QListWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QScrollArea>
#include <QSplitter>
#include <QProgressBar>
#include <QSlider>
#include <QButtonGroup>
#include <QRadioButton>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QVariantMap>
#include <QStringList>

#include "TilesetManager.h"
#include "ItemManager.h"
#include "BrushManager.h"

// Forward declarations
class Map;
class Tile;
class Item;
class Brush;
class MainPalette;

/**
 * @brief Task 91: Advanced Tileset Definition/Editing UI
 * 
 * Complete TilesetEditorDialog for advanced tileset management:
 * - Create, edit, and delete tileset categories
 * - Manage item and brush assignments within tilesets
 * - Configure tileset properties and behaviors
 * - Handle special tileset types (palettedoor, etc.)
 * - Integration with TilesetManager and MainPalette
 * - Support for tileset import/export
 * - Advanced filtering and search capabilities
 * - Visual tileset preview and organization
 * - 1:1 compatibility with wxwidgets TilesetWindows functionality
 */

/**
 * @brief Tileset Behavior Configuration
 */
struct TilesetBehavior {
    enum Type {
        NORMAL,           // Standard tileset behavior
        DOOR_TILESET,     // Special door tileset (palettedoor)
        WALL_TILESET,     // Wall tileset with auto-connection
        GROUND_TILESET,   // Ground tileset with border handling
        DECORATION_TILESET, // Decoration items
        CREATURE_TILESET  // Creature spawns
    };
    
    Type type;
    QVariantMap properties;
    bool autoConnect;
    bool updateBorders;
    bool specialDrawing;
    
    TilesetBehavior(Type t = NORMAL) 
        : type(t), autoConnect(false), updateBorders(false), specialDrawing(false) {}
};

/**
 * @brief Tileset Property Configuration
 */
struct TilesetProperties {
    QString name;
    QString description;
    QString author;
    QString version;
    QDateTime created;
    QDateTime modified;
    TilesetBehavior behavior;
    QColor displayColor;
    bool readOnly;
    bool visible;
    int priority;
    
    TilesetProperties() 
        : created(QDateTime::currentDateTime())
        , modified(QDateTime::currentDateTime())
        , displayColor(Qt::white)
        , readOnly(false)
        , visible(true)
        , priority(0) {}
};

/**
 * @brief Main Tileset Editor Dialog
 */
class TilesetEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TilesetEditorDialog(QWidget* parent = nullptr);
    ~TilesetEditorDialog() override;

    // Component setup
    void setTilesetManager(TilesetManager* manager);
    void setItemManager(ItemManager* manager);
    void setBrushManager(BrushManager* manager);
    void setMainPalette(MainPalette* palette);

    // Tileset management
    void setCurrentTileset(const QString& tilesetName);
    QString getCurrentTileset() const { return currentTileset_; }
    void refreshTilesets();
    void refreshCategories();
    void refreshItems();

    // Category management
    void setCurrentCategory(const QString& categoryName);
    QString getCurrentCategory() const { return currentCategory_; }
    void setCurrentCategoryType(TilesetCategoryType type);
    TilesetCategoryType getCurrentCategoryType() const { return currentCategoryType_; }

    // Special tileset handling
    void configureDoorTileset(const QString& tilesetName);
    void configureWallTileset(const QString& tilesetName);
    void configureGroundTileset(const QString& tilesetName);
    bool isDoorTileset(const QString& tilesetName) const;
    bool isWallTileset(const QString& tilesetName) const;
    bool isGroundTileset(const QString& tilesetName) const;

public slots:
    void accept() override;
    void reject() override;

signals:
    void tilesetCreated(const QString& tilesetName);
    void tilesetModified(const QString& tilesetName);
    void tilesetDeleted(const QString& tilesetName);
    void categoryCreated(const QString& tilesetName, const QString& categoryName);
    void categoryModified(const QString& tilesetName, const QString& categoryName);
    void categoryDeleted(const QString& tilesetName, const QString& categoryName);
    void itemAssigned(const QString& tilesetName, const QString& categoryName, quint16 itemId);
    void itemUnassigned(const QString& tilesetName, const QString& categoryName, quint16 itemId);
    void paletteUpdateRequested();

private slots:
    // Tileset management slots
    void onCreateTileset();
    void onEditTileset();
    void onDeleteTileset();
    void onDuplicateTileset();
    void onRenameTileset();
    void onTilesetSelectionChanged();
    void onTilesetDoubleClicked();
    
    // Category management slots
    void onCreateCategory();
    void onEditCategory();
    void onDeleteCategory();
    void onDuplicateCategory();
    void onRenameCategory();
    void onCategorySelectionChanged();
    void onCategoryDoubleClicked();
    void onCategoryTypeChanged();
    
    // Item management slots
    void onAddItem();
    void onRemoveItem();
    void onMoveItemUp();
    void onMoveItemDown();
    void onItemSelectionChanged();
    void onItemDoubleClicked();
    void onAvailableItemDoubleClicked();
    
    // Brush management slots
    void onAddBrush();
    void onRemoveBrush();
    void onBrushSelectionChanged();
    void onBrushDoubleClicked();
    
    // Filtering and search slots
    void onFilterChanged();
    void onSearchTextChanged(const QString& text);
    void onShowOnlyUnassignedChanged(bool show);
    void onItemTypeFilterChanged();
    
    // Properties slots
    void onTilesetPropertiesChanged();
    void onCategoryPropertiesChanged();
    void onBehaviorTypeChanged();
    void onBehaviorPropertiesChanged();
    void onDisplayColorChanged();
    
    // Import/Export slots
    void onImportTileset();
    void onExportTileset();
    void onImportCategory();
    void onExportCategory();
    void onImportFromFile();
    void onExportToFile();
    
    // Preview slots
    void onPreviewModeChanged();
    void onPreviewSizeChanged();
    void onPreviewBackgroundChanged();
    void onRefreshPreview();
    
    // Validation slots
    void onValidateTileset();
    void onValidateCategory();
    void onValidateAll();

private:
    // UI setup methods
    void setupUI();
    void setupTilesetTab();
    void setupCategoryTab();
    void setupItemsTab();
    void setupBrushesTab();
    void setupPropertiesTab();
    void setupBehaviorTab();
    void setupPreviewTab();
    void setupButtonBox();
    void connectSignals();
    
    // Data management methods
    void loadTilesetData();
    void saveTilesetData();
    void loadCategoryData();
    void saveCategoryData();
    void updateTilesetList();
    void updateCategoryList();
    void updateItemLists();
    void updateBrushLists();
    void updatePreview();
    void updateButtonStates();
    void validateInput();
    
    // Tileset operations
    void createNewTileset(const QString& name, const TilesetProperties& properties);
    void deleteTileset(const QString& name);
    void renameTileset(const QString& oldName, const QString& newName);
    void duplicateTileset(const QString& sourceName, const QString& newName);
    bool importTilesetFromFile(const QString& filePath);
    bool exportTilesetToFile(const QString& filePath, const QString& tilesetName);
    
    // Category operations
    void createNewCategory(const QString& tilesetName, const QString& categoryName, TilesetCategoryType type);
    void deleteCategory(const QString& tilesetName, const QString& categoryName);
    void renameCategory(const QString& tilesetName, const QString& oldName, const QString& newName);
    void duplicateCategory(const QString& sourceTileset, const QString& sourceCategory, 
                          const QString& targetTileset, const QString& targetCategory);
    
    // Item operations
    void addItemToCategory(quint16 itemId);
    void removeItemFromCategory(quint16 itemId);
    void moveItemInCategory(int fromIndex, int toIndex);
    QList<quint16> getFilteredItems() const;
    QList<quint16> searchItems(const QString& searchText) const;
    QList<quint16> getUnassignedItems() const;
    
    // Brush operations
    void addBrushToCategory(Brush* brush);
    void removeBrushFromCategory(Brush* brush);
    QList<Brush*> getFilteredBrushes() const;
    QList<Brush*> getUnassignedBrushes() const;
    
    // Special behavior handling
    void configureTilesetBehavior(const QString& tilesetName, const TilesetBehavior& behavior);
    TilesetBehavior getTilesetBehavior(const QString& tilesetName) const;
    void applyDoorTilesetBehavior(const QString& tilesetName);
    void applyWallTilesetBehavior(const QString& tilesetName);
    void applyGroundTilesetBehavior(const QString& tilesetName);
    
    // Validation methods
    bool validateTilesetName(const QString& name) const;
    bool validateCategoryName(const QString& name) const;
    QStringList validateTilesetData(const QString& tilesetName) const;
    QStringList validateCategoryData(const QString& tilesetName, const QString& categoryName) const;
    
    // Utility methods
    void markAsModified();
    void resetModifiedFlag();
    bool hasUnsavedChanges() const;
    QString generateUniqueItemName(const QString& baseName) const;
    QString generateUniqueCategoryName(const QString& baseName) const;

private:
    // Core components
    TilesetManager* tilesetManager_;
    ItemManager* itemManager_;
    BrushManager* brushManager_;
    MainPalette* mainPalette_;
    
    // Current state
    QString currentTileset_;
    QString currentCategory_;
    TilesetCategoryType currentCategoryType_;
    bool modified_;
    bool loading_;
    
    // UI components
    QTabWidget* tabWidget_;
    QVBoxLayout* mainLayout_;
    QHBoxLayout* buttonLayout_;
    
    // Tileset tab controls
    QListWidget* tilesetListWidget_;
    QPushButton* createTilesetButton_;
    QPushButton* editTilesetButton_;
    QPushButton* deleteTilesetButton_;
    QPushButton* duplicateTilesetButton_;
    QPushButton* renameTilesetButton_;
    
    // Category tab controls
    QTreeWidget* categoryTreeWidget_;
    QComboBox* categoryTypeComboBox_;
    QPushButton* createCategoryButton_;
    QPushButton* editCategoryButton_;
    QPushButton* deleteCategoryButton_;
    QPushButton* duplicateCategoryButton_;
    QPushButton* renameCategoryButton_;
    
    // Items tab controls
    QSplitter* itemsSplitter_;
    QListWidget* availableItemsListWidget_;
    QListWidget* categoryItemsListWidget_;
    QPushButton* addItemButton_;
    QPushButton* removeItemButton_;
    QPushButton* moveItemUpButton_;
    QPushButton* moveItemDownButton_;
    QLineEdit* searchLineEdit_;
    QComboBox* itemFilterComboBox_;
    QCheckBox* showOnlyUnassignedCheckBox_;
    
    // Brushes tab controls
    QSplitter* brushesSplitter_;
    QListWidget* availableBrushesListWidget_;
    QListWidget* categoryBrushesListWidget_;
    QPushButton* addBrushButton_;
    QPushButton* removeBrushButton_;
    
    // Properties tab controls
    QFormLayout* propertiesLayout_;
    QLineEdit* tilesetNameLineEdit_;
    QTextEdit* tilesetDescriptionTextEdit_;
    QLineEdit* tilesetAuthorLineEdit_;
    QLineEdit* tilesetVersionLineEdit_;
    QLabel* tilesetCreatedLabel_;
    QLabel* tilesetModifiedLabel_;
    QCheckBox* tilesetReadOnlyCheckBox_;
    QCheckBox* tilesetVisibleCheckBox_;
    QSpinBox* tilesetPrioritySpinBox_;
    QPushButton* displayColorButton_;
    
    // Behavior tab controls
    QComboBox* behaviorTypeComboBox_;
    QCheckBox* autoConnectCheckBox_;
    QCheckBox* updateBordersCheckBox_;
    QCheckBox* specialDrawingCheckBox_;
    QTableWidget* behaviorPropertiesTable_;
    
    // Preview tab controls
    QScrollArea* previewScrollArea_;
    QLabel* previewLabel_;
    QComboBox* previewModeComboBox_;
    QSlider* previewSizeSlider_;
    QPushButton* previewBackgroundButton_;
    QPushButton* refreshPreviewButton_;
    QLabel* previewInfoLabel_;
    
    // Button box
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QPushButton* applyButton_;
    QPushButton* importButton_;
    QPushButton* exportButton_;
    QPushButton* validateButton_;
    
    // Data storage
    QMap<QString, TilesetProperties> tilesetProperties_;
    QMap<QString, TilesetBehavior> tilesetBehaviors_;
    QPixmap previewPixmap_;
    QColor previewBackgroundColor_;
    int previewSize_;
    int previewMode_;
    
    // Performance tracking
    QElapsedTimer operationTimer_;
    QMutex dataMutex_;
};

#endif // TILESETEDITOR_DIALOG_H
