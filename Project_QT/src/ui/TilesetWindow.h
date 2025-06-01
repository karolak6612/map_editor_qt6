#ifndef TILESETWINDOW_H
#define TILESETWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
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

// Forward declarations
class TilesetManager;
class ItemManager;
class Map;
class Tile;
class Item;

/**
 * @brief Advanced tileset editor window for managing tileset definitions
 * 
 * Task 78: Complete migration of wxwidgets TilesetWindow with enhanced features:
 * - Tileset creation and editing with comprehensive properties
 * - Item assignment and management within tilesets
 * - Tileset category organization and hierarchy
 * - Item filtering and search capabilities
 * - Tileset import/export functionality
 * - Visual tileset preview and editing
 * - Integration with TilesetManager and ItemManager
 */
class TilesetWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TilesetWindow(QWidget* parent = nullptr);
    explicit TilesetWindow(QWidget* parent, const Map* map, const Tile* tile, Item* item);
    ~TilesetWindow() override;

    // Tileset management
    void setCurrentTileset(const QString& tilesetName);
    QString getCurrentTileset() const;
    void refreshTilesets();
    void refreshItems();

public slots:
    void accept() override;
    void reject() override;

private slots:
    // Tileset management slots
    void onTilesetChanged(const QString& tileset);
    void onPaletteChanged(const QString& palette);
    void onCreateTileset();
    void onDeleteTileset();
    void onRenameTileset();
    void onDuplicateTileset();
    void onImportTileset();
    void onExportTileset();
    
    // Item management slots
    void onAddItem();
    void onRemoveItem();
    void onMoveItemUp();
    void onMoveItemDown();
    void onItemSelectionChanged();
    void onItemDoubleClicked();
    void onFilterChanged();
    void onSearchTextChanged(const QString& text);
    
    // Properties slots
    void onTilesetNameChanged(const QString& name);
    void onTilesetDescriptionChanged();
    void onTilesetCategoryChanged(const QString& category);
    void onTilesetVisibilityChanged(bool visible);
    void onTilesetPriorityChanged(int priority);
    void onTilesetColorChanged();
    
    // Preview slots
    void onPreviewModeChanged(int mode);
    void onPreviewSizeChanged(int size);
    void onPreviewBackgroundChanged();
    void onRefreshPreview();

private:
    // UI setup methods
    void setupUI();
    void setupTilesetTab();
    void setupItemsTab();
    void setupPropertiesTab();
    void setupPreviewTab();
    void setupButtonBox();
    void connectSignals();
    
    // Helper methods
    void loadTilesetData();
    void saveTilesetData();
    void updateItemList();
    void updatePreview();
    void updateButtonStates();
    void validateInput();
    bool hasUnsavedChanges() const;
    void markAsModified();
    void resetModifiedFlag();
    
    // Tileset operations
    void createNewTileset(const QString& name, const QString& category);
    void deleteTileset(const QString& name);
    void renameTileset(const QString& oldName, const QString& newName);
    void duplicateTileset(const QString& sourceName, const QString& newName);
    bool importTilesetFromFile(const QString& filePath);
    bool exportTilesetToFile(const QString& filePath);
    
    // Item operations
    void addItemToTileset(quint16 itemId);
    void removeItemFromTileset(quint16 itemId);
    void moveItemInTileset(int fromIndex, int toIndex);
    QList<quint16> getFilteredItems() const;
    QList<quint16> searchItems(const QString& searchText) const;
    
    // UI components
    QTabWidget* tabWidget_;
    QVBoxLayout* mainLayout_;
    QHBoxLayout* buttonLayout_;
    
    // Buttons
    QPushButton* createButton_;
    QPushButton* deleteButton_;
    QPushButton* renameButton_;
    QPushButton* duplicateButton_;
    QPushButton* importButton_;
    QPushButton* exportButton_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QPushButton* applyButton_;
    
    // Tileset tab controls
    QComboBox* paletteComboBox_;
    QComboBox* tilesetComboBox_;
    QListWidget* tilesetListWidget_;
    QLineEdit* tilesetNameLineEdit_;
    QTextEdit* tilesetDescriptionTextEdit_;
    QComboBox* tilesetCategoryComboBox_;
    QCheckBox* tilesetVisibleCheckBox_;
    QSpinBox* tilesetPrioritySpinBox_;
    QPushButton* tilesetColorButton_;
    
    // Items tab controls
    QSplitter* itemsSplitter_;
    QListWidget* availableItemsListWidget_;
    QListWidget* tilesetItemsListWidget_;
    QPushButton* addItemButton_;
    QPushButton* removeItemButton_;
    QPushButton* moveUpButton_;
    QPushButton* moveDownButton_;
    QLineEdit* searchLineEdit_;
    QComboBox* filterComboBox_;
    QCheckBox* showOnlyUnassignedCheckBox_;
    
    // Properties tab controls
    QFormLayout* propertiesLayout_;
    QLabel* itemCountLabel_;
    QLabel* tilesetSizeLabel_;
    QLabel* createdDateLabel_;
    QLabel* modifiedDateLabel_;
    QLabel* authorLabel_;
    QLineEdit* authorLineEdit_;
    QTextEdit* notesTextEdit_;
    QCheckBox* readOnlyCheckBox_;
    
    // Preview tab controls
    QScrollArea* previewScrollArea_;
    QLabel* previewLabel_;
    QComboBox* previewModeComboBox_;
    QSlider* previewSizeSlider_;
    QPushButton* previewBackgroundButton_;
    QPushButton* refreshPreviewButton_;
    QLabel* previewInfoLabel_;
    
    // Data management
    TilesetManager* tilesetManager_;
    ItemManager* itemManager_;
    const Map* map_;
    const Tile* tile_;
    Item* item_;
    QString currentTileset_;
    QString currentPalette_;
    bool modified_;
    bool loading_;
    
    // Preview data
    QPixmap previewPixmap_;
    QColor previewBackgroundColor_;
    int previewSize_;
    int previewMode_;
};

#endif // TILESETWINDOW_H
