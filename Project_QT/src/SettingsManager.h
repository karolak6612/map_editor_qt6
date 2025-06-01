#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>

// Settings keys constants (matching wxWidgets Config enum)
namespace SettingsKeys {
    // Automagic settings
    const QString USE_AUTOMAGIC = "Editor/UseAutomagic";
    const QString BORDER_IS_GROUND = "Editor/BorderIsGround";
    const QString SAME_GROUND_TYPE_BORDER = "Editor/SameGroundTypeBorder";
    const QString WALLS_REPEL_BORDERS = "Editor/WallsRepelBorders";
    const QString LAYER_CARPETS = "Editor/LayerCarpets";
    const QString BORDERIZE_DELETE = "Editor/BorderizeDelete";
    const QString CUSTOM_BORDER_ENABLED = "Editor/CustomBorderEnabled";
    const QString CUSTOM_BORDER_ID = "Editor/CustomBorderId";
    
    // UI settings
    const QString WINDOW_GEOMETRY = "UI/WindowGeometry";
    const QString WINDOW_STATE = "UI/WindowState";
    const QString DOCK_LAYOUT = "UI/DockLayout";
    
    // Editor settings
    const QString SELECTION_TYPE = "Editor/SelectionType";
    const QString COMPENSATED_SELECT = "Editor/CompensatedSelect";
    const QString SCROLL_SPEED = "Editor/ScrollSpeed";
    const QString ZOOM_SPEED = "Editor/ZoomSpeed";
    
    // Graphics settings
    const QString TEXTURE_MANAGEMENT = "Graphics/TextureManagement";
    const QString HIDE_ITEMS_WHEN_ZOOMED = "Graphics/HideItemsWhenZoomed";
    const QString CURSOR_RED = "Graphics/CursorRed";
    const QString CURSOR_GREEN = "Graphics/CursorGreen";
    const QString CURSOR_BLUE = "Graphics/CursorBlue";
    const QString CURSOR_ALPHA = "Graphics/CursorAlpha";
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
    
    // Basic setters
    void setBool(const QString& key, bool value);
    void setInt(const QString& key, int value);
    void setFloat(const QString& key, float value);
    void setString(const QString& key, const QString& value);
    
    // Convenience methods for automagic settings
    bool isAutomagicEnabled() const;
    bool isSameGroundTypeBorderEnabled() const;
    bool isWallsRepelBordersEnabled() const;
    bool isLayerCarpetsEnabled() const;
    bool isBorderizeDeleteEnabled() const;
    bool isCustomBorderEnabled() const;
    int getCustomBorderId() const;
    
    void setAutomagicEnabled(bool enabled);
    void setSameGroundTypeBorderEnabled(bool enabled);
    void setWallsRepelBordersEnabled(bool enabled);
    void setLayerCarpetsEnabled(bool enabled);
    void setBorderizeDeleteEnabled(bool enabled);
    void setCustomBorderEnabled(bool enabled);
    void setCustomBorderId(int id);
    
    // Settings management
    void loadSettings();
    void saveSettings();
    void setDefaults();
    
    // Sync methods
    void sync();

signals:
    void settingChanged(const QString& key, const QVariant& value);
    void automagicSettingsChanged();

private:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager();
    
    static SettingsManager* instance_;
    QSettings* settings_;
    
    void initializeSettings();
    void emitAutomagicChanged();
};

#endif // SETTINGSMANAGER_H
