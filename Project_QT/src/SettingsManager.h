#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QByteArray>
#include <QCoreApplication>

// Settings keys constants (complete migration from wxWidgets Config enum)
namespace SettingsKeys {
    // General settings
    const QString ALWAYS_MAKE_BACKUP = "General/AlwaysMakeBackup";
    const QString CREATE_MAP_ON_STARTUP = "General/CreateMapOnStartup";
    const QString UPDATE_CHECK_ON_STARTUP = "General/UpdateCheckOnStartup";
    const QString ONLY_ONE_INSTANCE = "General/OnlyOneInstance";
    const QString UNDO_SIZE = "General/UndoSize";
    const QString UNDO_MEM_SIZE = "General/UndoMemSize";
    const QString WORKER_THREADS = "General/WorkerThreads";
    const QString REPLACE_SIZE = "General/ReplaceSize";
    const QString LISTBOX_MAX_HEIGHT = "General/ListboxMaxHeight";
    const QString RECENT_FILES_MAX = "General/RecentFilesMax";

    // Automagic settings
    const QString USE_AUTOMAGIC = "Editor/UseAutomagic";
    const QString BORDER_IS_GROUND = "Editor/BorderIsGround";
    const QString SAME_GROUND_TYPE_BORDER = "Editor/SameGroundTypeBorder";
    const QString WALLS_REPEL_BORDERS = "Editor/WallsRepelBorders";
    const QString LAYER_CARPETS = "Editor/LayerCarpets";
    const QString BORDERIZE_DELETE = "Editor/BorderizeDelete";
    const QString CUSTOM_BORDER_ENABLED = "Editor/CustomBorderEnabled";
    const QString CUSTOM_BORDER_ID = "Editor/CustomBorderId";
    const QString ERASER_LEAVE_UNIQUE = "Editor/EraserLeaveUnique";
    const QString DOODAD_BRUSH_ERASE_LIKE = "Editor/DoodadBrushEraseLike";
    const QString WARN_FOR_DESTRUCTIVE_ACTIONS = "Editor/WarnForDestructiveActions";

    // UI settings
    const QString WINDOW_GEOMETRY = "UI/WindowGeometry";
    const QString WINDOW_STATE = "UI/WindowState";
    const QString DOCK_LAYOUT = "UI/DockLayout";
    const QString USE_LARGE_CONTAINER_ICONS = "UI/UseLargeContainerIcons";
    const QString USE_LARGE_CHOOSE_ITEM_ICONS = "UI/UseLargeChooseItemIcons";
    const QString USE_LARGE_TERRAIN_TOOLBAR = "UI/UseLargeTerrainToolbar";
    const QString USE_LARGE_COLLECTION_TOOLBAR = "UI/UseLargeCollectionToolbar";
    const QString USE_LARGE_DOODAD_SIZEBAR = "UI/UseLargeDoodadSizebar";
    const QString USE_LARGE_ITEM_SIZEBAR = "UI/UseLargeItemSizebar";
    const QString USE_LARGE_HOUSE_SIZEBAR = "UI/UseLargeHouseSizebar";
    const QString USE_LARGE_RAW_SIZEBAR = "UI/UseLargeRawSizebar";
    const QString USE_GUI_SELECTION_SHADOW = "UI/UseGuiSelectionShadow";
    const QString PALETTE_COL_COUNT = "UI/PaletteColCount";
    const QString PALETTE_TERRAIN_STYLE = "UI/PaletteTerrainStyle";
    const QString PALETTE_COLLECTION_STYLE = "UI/PaletteCollectionStyle";
    const QString PALETTE_DOODAD_STYLE = "UI/PaletteDoodadStyle";
    const QString PALETTE_ITEM_STYLE = "UI/PaletteItemStyle";
    const QString PALETTE_RAW_STYLE = "UI/PaletteRawStyle";
    const QString PALETTE_HOUSE_STYLE = "UI/PaletteHouseStyle";
    const QString PALETTE_WAYPOINT_STYLE = "UI/PaletteWaypointStyle";
    const QString PALETTE_CREATURE_STYLE = "UI/PaletteCreatureStyle";
    const QString WELCOME_DIALOG = "UI/WelcomeDialog";
    const QString TOOLBAR_STANDARD_VISIBLE = "UI/ToolbarStandardVisible";
    const QString TOOLBAR_BRUSHES_VISIBLE = "UI/ToolbarBrushesVisible";
    const QString TOOLBAR_POSITION_VISIBLE = "UI/ToolbarPositionVisible";
    const QString TOOLBAR_SIZES_VISIBLE = "UI/ToolbarSizesVisible";
    const QString TOOLBAR_INDICATORS_VISIBLE = "UI/ToolbarIndicatorsVisible";

    // Editor settings
    const QString SELECTION_TYPE = "Editor/SelectionType";
    const QString COMPENSATED_SELECT = "Editor/CompensatedSelect";
    const QString SCROLL_SPEED = "Editor/ScrollSpeed";
    const QString ZOOM_SPEED = "Editor/ZoomSpeed";
    const QString SWITCH_MOUSEBUTTONS = "Editor/SwitchMouseButtons";
    const QString DOUBLECLICK_PROPERTIES = "Editor/DoubleclickProperties";
    const QString INVERSED_SCROLL = "Editor/InversedScroll";
    const QString HARD_REFRESH_RATE = "Editor/HardRefreshRate";
    const QString DEFAULT_SPAWNTIME = "Editor/DefaultSpawntime";
    const QString MERGE_MOVE = "Editor/MergeMove";
    const QString MERGE_PASTE = "Editor/MergePaste";
    const QString HOUSE_BRUSH_REMOVE_ITEMS = "Editor/HouseBrushRemoveItems";
    const QString AUTO_ASSIGN_DOORID = "Editor/AutoAssignDoorid";
    const QString ERASER_LEAVE_UNIQUE_ITEMS = "Editor/EraserLeaveUniqueItems";
    const QString AUTO_CREATE_SPAWN = "Editor/AutoCreateSpawn";
    const QString ALLOW_MULTIPLE_ORDERITEMS = "Editor/AllowMultipleOrderitems";
    const QString HOUSE_BRUSH_REMOVE_ITEMS_BOOL = "Editor/HouseBrushRemoveItemsBool";
    const QString RAW_LIKE_SIMONE_BRUSH = "Editor/RawLikeSimoneBrush";

    // Graphics settings
    const QString TEXTURE_MANAGEMENT = "Graphics/TextureManagement";
    const QString TEXTURE_CLEAN_PULSE = "Graphics/TextureCleanPulse";
    const QString TEXTURE_LONGEVITY = "Graphics/TextureLongevity";
    const QString TEXTURE_CLEAN_THRESHOLD = "Graphics/TextureCleanThreshold";
    const QString SOFTWARE_CLEAN_THRESHOLD = "Graphics/SoftwareCleanThreshold";
    const QString SOFTWARE_CLEAN_SIZE = "Graphics/SoftwareCleanSize";
    const QString ICON_BACKGROUND = "Graphics/IconBackground";
    const QString HIDE_ITEMS_WHEN_ZOOMED = "Graphics/HideItemsWhenZoomed";
    const QString SCREENSHOT_DIRECTORY = "Graphics/ScreenshotDirectory";
    const QString SCREENSHOT_FORMAT = "Graphics/ScreenshotFormat";
    const QString USE_MEMCACHED_SPRITES = "Graphics/UseMemcachedSprites";
    const QString MINIMAP_UPDATE_DELAY = "Graphics/MinimapUpdateDelay";
    const QString MINIMAP_VIEW_BOX = "Graphics/MinimapViewBox";
    const QString MINIMAP_EXPORT_DIR = "Graphics/MinimapExportDir";
    const QString TILESET_EXPORT_DIR = "Graphics/TilesetExportDir";
    const QString CURSOR_RED = "Graphics/CursorRed";
    const QString CURSOR_GREEN = "Graphics/CursorGreen";
    const QString CURSOR_BLUE = "Graphics/CursorBlue";
    const QString CURSOR_ALPHA = "Graphics/CursorAlpha";
    const QString CURSOR_ALT_RED = "Graphics/CursorAltRed";
    const QString CURSOR_ALT_GREEN = "Graphics/CursorAltGreen";
    const QString CURSOR_ALT_BLUE = "Graphics/CursorAltBlue";
    const QString CURSOR_ALT_ALPHA = "Graphics/CursorAltAlpha";

    // Client version settings
    const QString CHECK_SIGNATURES = "Client/CheckSignatures";
    const QString USE_OTGZ = "Client/UseOtgz";
    const QString CLIENT_VERSION_ID = "Client/VersionId";
    const QString DATA_DIRECTORY = "Client/DataDirectory";
    const QString CLIENT_PATH = "Client/ClientPath";
    const QString CLIENT_VERSION_OVERRIDE = "Client/VersionOverride";

    // Paths settings
    const QString WORKING_DIRECTORY = "Paths/WorkingDirectory";
    const QString EXTENSIONS_DIRECTORY = "Paths/ExtensionsDirectory";
    const QString INDIRECTORY_INSTALLATION = "Paths/IndirectoryInstallation";

    // LOD (Level of Detail) settings
    const QString LOD_ENABLED = "LOD/Enabled";
    const QString LOD_THRESHOLD_VERY_CLOSE = "LOD/ThresholdVeryClose";
    const QString LOD_THRESHOLD_CLOSE = "LOD/ThresholdClose";
    const QString LOD_THRESHOLD_FAR = "LOD/ThresholdFar";
    const QString LOD_THRESHOLD_VERY_FAR = "LOD/ThresholdVeryFar";

    // Hotkey settings
    const QString HOTKEY_PREFIX = "Hotkeys/";

    // Recent files
    const QString RECENT_FILES = "RecentFiles/Files";
    const QString RECENT_DIRECTORIES = "RecentFiles/Directories";
}

class SettingsManager : public QObject {
    Q_OBJECT

public:
    static SettingsManager* getInstance();
    static void destroyInstance();

    // Basic getters
    bool getBool(const QString& key, bool defaultValue = false) const;
    int getInt(const QString& key, int defaultValue = 0) const;
    float getFloat(const QString& key, float defaultValue = 0.0f) const;
    QString getString(const QString& key, const QString& defaultValue = QString()) const;
    QStringList getStringList(const QString& key, const QStringList& defaultValue = QStringList()) const;
    QByteArray getByteArray(const QString& key, const QByteArray& defaultValue = QByteArray()) const;
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    // Basic setters
    void setBool(const QString& key, bool value);
    void setInt(const QString& key, int value);
    void setFloat(const QString& key, float value);
    void setString(const QString& key, const QString& value);
    void setStringList(const QString& key, const QStringList& value);
    void setByteArray(const QString& key, const QByteArray& value);
    void setValue(const QString& key, const QVariant& value);

    // General settings
    bool getAlwaysMakeBackup() const;
    bool getCreateMapOnStartup() const;
    bool getUpdateCheckOnStartup() const;
    bool getOnlyOneInstance() const;
    int getUndoSize() const;
    int getUndoMemSize() const;
    int getWorkerThreads() const;
    int getReplaceSize() const;
    int getListboxMaxHeight() const;
    int getRecentFilesMax() const;

    void setAlwaysMakeBackup(bool enabled);
    void setCreateMapOnStartup(bool enabled);
    void setUpdateCheckOnStartup(bool enabled);
    void setOnlyOneInstance(bool enabled);
    void setUndoSize(int size);
    void setUndoMemSize(int size);
    void setWorkerThreads(int threads);
    void setReplaceSize(int size);
    void setListboxMaxHeight(int height);
    void setRecentFilesMax(int max);

    // Automagic settings
    bool isAutomagicEnabled() const;
    bool isBorderIsGroundEnabled() const;
    bool isSameGroundTypeBorderEnabled() const;
    bool isWallsRepelBordersEnabled() const;
    bool isLayerCarpetsEnabled() const;
    bool isBorderizeDeleteEnabled() const;
    bool isCustomBorderEnabled() const;
    int getCustomBorderId() const;
    bool isEraserLeaveUniqueEnabled() const;
    bool isDoodadBrushEraseLikeEnabled() const;
    bool isWarnForDestructiveActionsEnabled() const;

    void setAutomagicEnabled(bool enabled);
    void setBorderIsGroundEnabled(bool enabled);
    void setSameGroundTypeBorderEnabled(bool enabled);
    void setWallsRepelBordersEnabled(bool enabled);
    void setLayerCarpetsEnabled(bool enabled);
    void setBorderizeDeleteEnabled(bool enabled);
    void setCustomBorderEnabled(bool enabled);
    void setCustomBorderId(int id);
    void setEraserLeaveUniqueEnabled(bool enabled);
    void setDoodadBrushEraseLikeEnabled(bool enabled);
    void setWarnForDestructiveActionsEnabled(bool enabled);

    // UI settings
    QByteArray getWindowGeometry() const;
    QByteArray getWindowState() const;
    QByteArray getDockLayout() const;
    bool getUseLargeContainerIcons() const;
    bool getUseLargeChooseItemIcons() const;
    bool getUseLargeTerrainToolbar() const;
    bool getUseLargeCollectionToolbar() const;
    bool getUseLargeDoodadSizebar() const;
    bool getUseLargeItemSizebar() const;
    bool getUseLargeHouseSizebar() const;
    bool getUseLargeRawSizebar() const;
    bool getUseGuiSelectionShadow() const;
    int getPaletteColCount() const;
    QString getPaletteTerrainStyle() const;
    QString getPaletteCollectionStyle() const;
    QString getPaletteDoodadStyle() const;
    QString getPaletteItemStyle() const;
    QString getPaletteRawStyle() const;
    QString getPaletteHouseStyle() const;
    QString getPaletteWaypointStyle() const;
    QString getPaletteCreatureStyle() const;
    bool getWelcomeDialog() const;
    bool getToolbarStandardVisible() const;
    bool getToolbarBrushesVisible() const;
    bool getToolbarPositionVisible() const;
    bool getToolbarSizesVisible() const;
    bool getToolbarIndicatorsVisible() const;

    void setWindowGeometry(const QByteArray& geometry);
    void setWindowState(const QByteArray& state);
    void setDockLayout(const QByteArray& layout);
    void setUseLargeContainerIcons(bool enabled);
    void setUseLargeChooseItemIcons(bool enabled);
    void setUseLargeTerrainToolbar(bool enabled);
    void setUseLargeCollectionToolbar(bool enabled);
    void setUseLargeDoodadSizebar(bool enabled);
    void setUseLargeItemSizebar(bool enabled);
    void setUseLargeHouseSizebar(bool enabled);
    void setUseLargeRawSizebar(bool enabled);
    void setUseGuiSelectionShadow(bool enabled);
    void setPaletteColCount(int count);
    void setPaletteTerrainStyle(const QString& style);
    void setPaletteCollectionStyle(const QString& style);
    void setPaletteDoodadStyle(const QString& style);
    void setPaletteItemStyle(const QString& style);
    void setPaletteRawStyle(const QString& style);
    void setPaletteHouseStyle(const QString& style);
    void setPaletteWaypointStyle(const QString& style);
    void setPaletteCreatureStyle(const QString& style);
    void setWelcomeDialog(bool enabled);
    void setToolbarStandardVisible(bool visible);
    void setToolbarBrushesVisible(bool visible);
    void setToolbarPositionVisible(bool visible);
    void setToolbarSizesVisible(bool visible);
    void setToolbarIndicatorsVisible(bool visible);

    // Settings management
    void loadSettings();
    void saveSettings();
    void setDefaults();
    void resetToDefaults();
    void importSettings(const QString& filePath);
    void exportSettings(const QString& filePath);

    // Editor settings
    int getSelectionType() const;
    bool getCompensatedSelect() const;
    float getScrollSpeed() const;
    float getZoomSpeed() const;
    bool getSwitchMouseButtons() const;
    bool getDoubleclickProperties() const;
    bool getInversedScroll() const;
    int getHardRefreshRate() const;
    int getDefaultSpawntime() const;
    bool getMergeMove() const;
    bool getMergePaste() const;
    bool getHouseBrushRemoveItems() const;
    bool getAutoAssignDoorid() const;
    bool getEraserLeaveUniqueItems() const;
    bool getAutoCreateSpawn() const;
    bool getAllowMultipleOrderitems() const;
    bool getHouseBrushRemoveItemsBool() const;
    bool getRawLikeSimoneBrush() const;

    void setSelectionType(int type);
    void setCompensatedSelect(bool enabled);
    void setScrollSpeed(float speed);
    void setZoomSpeed(float speed);
    void setSwitchMouseButtons(bool enabled);
    void setDoubleclickProperties(bool enabled);
    void setInversedScroll(bool enabled);
    void setHardRefreshRate(int rate);
    void setDefaultSpawntime(int time);
    void setMergeMove(bool enabled);
    void setMergePaste(bool enabled);
    void setHouseBrushRemoveItems(bool enabled);
    void setAutoAssignDoorid(bool enabled);
    void setEraserLeaveUniqueItems(bool enabled);
    void setAutoCreateSpawn(bool enabled);
    void setAllowMultipleOrderitems(bool enabled);
    void setHouseBrushRemoveItemsBool(bool enabled);
    void setRawLikeSimoneBrush(bool enabled);

    // Graphics settings
    bool getTextureManagement() const;
    int getTextureCleanPulse() const;
    int getTextureLongevity() const;
    int getTextureCleanThreshold() const;
    int getSoftwareCleanThreshold() const;
    int getSoftwareCleanSize() const;
    int getIconBackground() const;
    bool getHideItemsWhenZoomed() const;
    QString getScreenshotDirectory() const;
    QString getScreenshotFormat() const;
    bool getUseMemcachedSprites() const;
    int getMinimapUpdateDelay() const;
    bool getMinimapViewBox() const;
    QString getMinimapExportDir() const;
    QString getTilesetExportDir() const;
    int getCursorRed() const;
    int getCursorGreen() const;
    int getCursorBlue() const;
    int getCursorAlpha() const;
    int getCursorAltRed() const;
    int getCursorAltGreen() const;
    int getCursorAltBlue() const;
    int getCursorAltAlpha() const;

    void setTextureManagement(bool enabled);
    void setTextureCleanPulse(int pulse);
    void setTextureLongevity(int longevity);
    void setTextureCleanThreshold(int threshold);
    void setSoftwareCleanThreshold(int threshold);
    void setSoftwareCleanSize(int size);
    void setIconBackground(int background);
    void setHideItemsWhenZoomed(bool enabled);
    void setScreenshotDirectory(const QString& directory);
    void setScreenshotFormat(const QString& format);
    void setUseMemcachedSprites(bool enabled);
    void setMinimapUpdateDelay(int delay);
    void setMinimapViewBox(bool enabled);
    void setMinimapExportDir(const QString& directory);
    void setTilesetExportDir(const QString& directory);
    void setCursorRed(int red);
    void setCursorGreen(int green);
    void setCursorBlue(int blue);
    void setCursorAlpha(int alpha);
    void setCursorAltRed(int red);
    void setCursorAltGreen(int green);
    void setCursorAltBlue(int blue);
    void setCursorAltAlpha(int alpha);

    // Client version settings
    bool getCheckSignatures() const;
    bool getUseOtgz() const;
    int getClientVersionId() const;
    QString getDataDirectory() const;
    QString getClientPath() const;
    bool getClientVersionOverride() const;

    void setCheckSignatures(bool enabled);
    void setUseOtgz(bool enabled);
    void setClientVersionId(int id);
    void setDataDirectory(const QString& directory);
    void setClientPath(const QString& path);
    void setClientVersionOverride(bool enabled);

    // Paths settings
    QString getWorkingDirectory() const;
    QString getExtensionsDirectory() const;
    bool getIndirectoryInstallation() const;

    void setWorkingDirectory(const QString& directory);
    void setExtensionsDirectory(const QString& directory);
    void setIndirectoryInstallation(bool enabled);

    // LOD settings
    bool getLODEnabled() const;
    float getLODThresholdVeryClose() const;
    float getLODThresholdClose() const;
    float getLODThresholdFar() const;
    float getLODThresholdVeryFar() const;

    void setLODEnabled(bool enabled);
    void setLODThresholdVeryClose(float threshold);
    void setLODThresholdClose(float threshold);
    void setLODThresholdFar(float threshold);
    void setLODThresholdVeryFar(float threshold);

    // Hotkey settings
    QString getHotkey(const QString& action) const;
    QMap<QString, QString> getAllHotkeys() const;
    void setHotkey(const QString& action, const QString& keySequence);
    void clearHotkey(const QString& action);
    void resetHotkeysToDefaults();

    // Recent files
    QStringList getRecentFiles() const;
    QStringList getRecentDirectories() const;
    void addRecentFile(const QString& filePath);
    void addRecentDirectory(const QString& directoryPath);
    void clearRecentFiles();
    void clearRecentDirectories();

    // Sync methods
    void sync();

    // Application lifecycle
    void saveOnExit();
    void loadOnStartup();
    void applySettingsToApplication();

signals:
    void settingChanged(const QString& key, const QVariant& value);
    void automagicSettingsChanged();
    void uiSettingsChanged();
    void editorSettingsChanged();
    void graphicsSettingsChanged();
    void clientVersionSettingsChanged();
    void lodSettingsChanged();
    void hotkeySettingsChanged();
    void recentFilesChanged();

private:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager();
    
    static SettingsManager* instance_;
    QSettings* settings_;
    
    void initializeSettings();
    void emitAutomagicChanged();
};

#endif // SETTINGSMANAGER_H
