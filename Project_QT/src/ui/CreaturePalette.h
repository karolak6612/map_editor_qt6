#ifndef CREATUREPALETTE_H
#define CREATUREPALETTE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QToolButton>
#include <QButtonGroup>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QScrollArea>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QTimer>
#include <QPixmap>
#include <QIcon>

#include "CreatureManager.h"
#include "CreatureBrush.h"
#include "SpawnBrush.h"
#include "BrushManager.h"
#include "GameSprite.h"
#include "SpriteManager.h"

/**
 * @brief Task 87: Creature Palette for creature and spawn selection
 * 
 * Complete creature palette implementation:
 * - List available creature types from CreatureManager
 * - Visual creature sprites with outfit/looktype support
 * - Creature filtering and searching
 * - CreatureBrush and SpawnBrush configuration
 * - Integration with main palette system
 * - 1:1 compatibility with wxwidgets creature palette
 */

class CreaturePalette : public QWidget
{
    Q_OBJECT

public:
    explicit CreaturePalette(QWidget* parent = nullptr);
    ~CreaturePalette() override;

    // Palette management
    void setCreatureManager(CreatureManager* creatureManager);
    CreatureManager* getCreatureManager() const { return creatureManager_; }
    
    void setBrushManager(BrushManager* brushManager);
    BrushManager* getBrushManager() const { return brushManager_; }
    
    void setSpriteManager(SpriteManager* spriteManager);
    SpriteManager* getSpriteManager() const { return spriteManager_; }

    // Brush access
    CreatureBrush* getCreatureBrush() const { return creatureBrush_; }
    SpawnBrush* getSpawnBrush() const { return spawnBrush_; }

    // Selection management
    void selectCreature(quint16 creatureId);
    void selectCreature(const QString& creatureName);
    quint16 getSelectedCreatureId() const { return selectedCreatureId_; }
    QString getSelectedCreatureName() const;
    const CreatureProperties* getSelectedCreature() const;

    // Filter and search
    void setFilter(const QString& filter);
    QString getFilter() const { return filterText_; }
    void setShowNpcs(bool show);
    bool isShowNpcs() const { return showNpcs_; }
    void setShowMonsters(bool show);
    bool isShowMonsters() const { return showMonsters_; }

    // Visual settings
    void setIconSize(int size);
    int getIconSize() const { return iconSize_; }
    void setShowCreatureNames(bool show);
    bool isShowCreatureNames() const { return showCreatureNames_; }

public slots:
    void refreshCreatureList();
    void updateCreatureSprites();
    void onCreatureManagerChanged();
    void onBrushChanged();

private slots:
    // Creature selection slots
    void onCreatureItemClicked(QListWidgetItem* item);
    void onCreatureItemDoubleClicked(QListWidgetItem* item);
    void onCreatureSelectionChanged();
    
    // Filter and search slots
    void onFilterTextChanged();
    void onFilterTypeChanged();
    void onShowNpcsChanged(bool show);
    void onShowMonstersChanged(bool show);
    
    // Brush configuration slots
    void onCreatureBrushSelected();
    void onSpawnBrushSelected();
    void onConfigureCreatureBrush();
    void onConfigureSpawnBrush();
    
    // Visual settings slots
    void onIconSizeChanged(int size);
    void onShowNamesChanged(bool show);
    void onRefreshSprites();

signals:
    void creatureSelected(quint16 creatureId);
    void creatureBrushActivated(CreatureBrush* brush);
    void spawnBrushActivated(SpawnBrush* brush);
    void brushConfigurationChanged();

private:
    void setupUI();
    void setupCreatureList();
    void setupFilterControls();
    void setupBrushControls();
    void setupVisualControls();
    void connectSignals();
    
    void populateCreatureList();
    void filterCreatureList();
    void updateCreatureItem(QListWidgetItem* item, const CreatureProperties& creature);
    QPixmap getCreaturePixmap(const CreatureProperties& creature, int size) const;
    
    // Helper methods
    bool matchesFilter(const CreatureProperties& creature) const;
    void selectCreatureItem(QListWidgetItem* item);
    void activateCreatureBrush();
    void activateSpawnBrush();

private:
    // Core components
    CreatureManager* creatureManager_;
    BrushManager* brushManager_;
    SpriteManager* spriteManager_;
    CreatureBrush* creatureBrush_;
    SpawnBrush* spawnBrush_;
    
    // Selection state
    quint16 selectedCreatureId_;
    
    // Filter settings
    QString filterText_;
    bool showNpcs_;
    bool showMonsters_;
    
    // Visual settings
    int iconSize_;
    bool showCreatureNames_;
    
    // UI components
    QVBoxLayout* mainLayout_;
    
    // Filter controls
    QGroupBox* filterGroup_;
    QLineEdit* filterEdit_;
    QComboBox* filterTypeCombo_;
    QCheckBox* showNpcsCheckBox_;
    QCheckBox* showMonstersCheckBox_;
    QPushButton* clearFilterButton_;
    
    // Creature list
    QGroupBox* creatureListGroup_;
    QListWidget* creatureList_;
    QLabel* creatureCountLabel_;
    
    // Brush controls
    QGroupBox* brushGroup_;
    QPushButton* creatureBrushButton_;
    QPushButton* spawnBrushButton_;
    QPushButton* configureCreatureButton_;
    QPushButton* configureSpawnButton_;
    QButtonGroup* brushButtonGroup_;
    
    // Visual controls
    QGroupBox* visualGroup_;
    QSlider* iconSizeSlider_;
    QLabel* iconSizeLabel_;
    QCheckBox* showNamesCheckBox_;
    QPushButton* refreshSpritesButton_;
    
    // Status
    QLabel* statusLabel_;
};

/**
 * @brief Creature List Item Widget
 */
class CreatureListItem : public QListWidgetItem
{
public:
    explicit CreatureListItem(const CreatureProperties& creature, QListWidget* parent = nullptr);
    ~CreatureListItem() override = default;

    // Creature data
    quint16 getCreatureId() const { return creatureId_; }
    QString getCreatureName() const { return creatureName_; }
    const CreatureProperties& getCreature() const { return creature_; }

    // Visual representation
    void updatePixmap(const QPixmap& pixmap);
    void updateText(bool showName);
    void setIconSize(int size);

private:
    quint16 creatureId_;
    QString creatureName_;
    CreatureProperties creature_;
    QPixmap creaturePixmap_;
};

/**
 * @brief Creature Filter Widget
 */
class CreatureFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CreatureFilterWidget(QWidget* parent = nullptr);
    ~CreatureFilterWidget() override = default;

    // Filter settings
    void setFilterText(const QString& text);
    QString getFilterText() const;
    
    void setFilterType(int type);
    int getFilterType() const;
    
    void setShowNpcs(bool show);
    bool isShowNpcs() const;
    
    void setShowMonsters(bool show);
    bool isShowMonsters() const;

signals:
    void filterChanged();
    void filterTextChanged(const QString& text);
    void filterTypeChanged(int type);
    void showNpcsChanged(bool show);
    void showMonstersChanged(bool show);

private slots:
    void onFilterTextChanged();
    void onFilterTypeChanged();
    void onShowNpcsChanged();
    void onShowMonstersChanged();
    void onClearFilter();

private:
    void setupUI();
    void connectSignals();

private:
    QHBoxLayout* layout_;
    QLineEdit* filterEdit_;
    QComboBox* filterTypeCombo_;
    QCheckBox* showNpcsCheckBox_;
    QCheckBox* showMonstersCheckBox_;
    QPushButton* clearButton_;
};

/**
 * @brief Creature Brush Configuration Widget
 */
class CreatureBrushConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CreatureBrushConfigWidget(CreatureBrush* brush, QWidget* parent = nullptr);
    ~CreatureBrushConfigWidget() override = default;

    // Brush configuration
    void setBrush(CreatureBrush* brush);
    CreatureBrush* getBrush() const { return brush_; }
    
    void updateFromBrush();
    void applyToBrush();

signals:
    void configurationChanged();

private slots:
    void onHealthChanged();
    void onDirectionChanged();
    void onOutfitChanged();

private:
    void setupUI();
    void connectSignals();

private:
    CreatureBrush* brush_;
    
    QVBoxLayout* layout_;
    QSpinBox* healthSpinBox_;
    QComboBox* directionCombo_;
    // Outfit configuration widgets would go here
};

#endif // CREATUREPALETTE_H
