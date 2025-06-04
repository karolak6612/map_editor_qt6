#include "SettingsManager.h"
#include <QApplication>
#include <QDebug>

SettingsManager* SettingsManager::instance_ = nullptr;

SettingsManager* SettingsManager::getInstance() {
    if (!instance_) {
        // Parent to QApplication for automatic cleanup
        QObject* appParent = QCoreApplication::instance();
        instance_ = new SettingsManager(appParent);

        // Store reference in QApplication for easy access
        if (appParent) {
            appParent->setProperty("SettingsManager", QVariant::fromValue(instance_));
        }
    }
    return instance_;
}

void SettingsManager::destroyInstance() {
    if (instance_) {
        // Remove from QApplication properties
        QObject* appParent = QCoreApplication::instance();
        if (appParent) {
            appParent->setProperty("SettingsManager", QVariant());
        }

        delete instance_;
        instance_ = nullptr;
    }
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent) {
    // Set application properties for QSettings
    QApplication::setOrganizationName("IdlersMapEditor");
    QApplication::setApplicationName("MapEditor");
    
    settings_ = new QSettings(this);
    initializeSettings();
}

SettingsManager::~SettingsManager() {
    saveSettings();
    // Clear static instance pointer when destroyed
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

void SettingsManager::initializeSettings() {
    // Load settings or set defaults if first run
    if (!settings_->contains(SettingsKeys::USE_AUTOMAGIC)) {
        setDefaults();
    }
    loadSettings();
}

void SettingsManager::setDefaults() {
    qDebug() << "SettingsManager: Setting default values";
    
    // Automagic defaults (matching wxWidgets)
    setBool(SettingsKeys::USE_AUTOMAGIC, true);
    setBool(SettingsKeys::BORDER_IS_GROUND, false);
    setBool(SettingsKeys::SAME_GROUND_TYPE_BORDER, false);
    setBool(SettingsKeys::WALLS_REPEL_BORDERS, false);
    setBool(SettingsKeys::LAYER_CARPETS, false);
    setBool(SettingsKeys::BORDERIZE_DELETE, false);
    setBool(SettingsKeys::CUSTOM_BORDER_ENABLED, false);
    setInt(SettingsKeys::CUSTOM_BORDER_ID, 1);
    
    // Editor defaults
    setInt(SettingsKeys::SELECTION_TYPE, 0); // SELECT_CURRENT_FLOOR
    setBool(SettingsKeys::COMPENSATED_SELECT, true);
    setFloat(SettingsKeys::SCROLL_SPEED, 3.5f);
    setFloat(SettingsKeys::ZOOM_SPEED, 1.4f);
    
    // Graphics defaults
    setBool(SettingsKeys::TEXTURE_MANAGEMENT, true);
    setBool(SettingsKeys::HIDE_ITEMS_WHEN_ZOOMED, true);
    setInt(SettingsKeys::CURSOR_RED, 0);
    setInt(SettingsKeys::CURSOR_GREEN, 166);
    setInt(SettingsKeys::CURSOR_BLUE, 0);
    setInt(SettingsKeys::CURSOR_ALPHA, 128);
}

// Basic getters
bool SettingsManager::getBool(const QString& key, bool defaultValue) const {
    return settings_->value(key, defaultValue).toBool();
}

int SettingsManager::getInt(const QString& key, int defaultValue) const {
    return settings_->value(key, defaultValue).toInt();
}

float SettingsManager::getFloat(const QString& key, float defaultValue) const {
    return settings_->value(key, defaultValue).toFloat();
}

QString SettingsManager::getString(const QString& key, const QString& defaultValue) const {
    return settings_->value(key, defaultValue).toString();
}

// Basic setters
void SettingsManager::setBool(const QString& key, bool value) {
    settings_->setValue(key, value);
    emit settingChanged(key, value);
}

void SettingsManager::setInt(const QString& key, int value) {
    settings_->setValue(key, value);
    emit settingChanged(key, value);
}

void SettingsManager::setFloat(const QString& key, float value) {
    settings_->setValue(key, value);
    emit settingChanged(key, value);
}

void SettingsManager::setString(const QString& key, const QString& value) {
    settings_->setValue(key, value);
    emit settingChanged(key, value);
}

// Convenience methods for automagic settings
bool SettingsManager::isAutomagicEnabled() const {
    return getBool(SettingsKeys::USE_AUTOMAGIC, true);
}

bool SettingsManager::isSameGroundTypeBorderEnabled() const {
    return getBool(SettingsKeys::SAME_GROUND_TYPE_BORDER, false);
}

bool SettingsManager::isWallsRepelBordersEnabled() const {
    return getBool(SettingsKeys::WALLS_REPEL_BORDERS, false);
}

bool SettingsManager::isLayerCarpetsEnabled() const {
    return getBool(SettingsKeys::LAYER_CARPETS, false);
}

bool SettingsManager::isBorderizeDeleteEnabled() const {
    return getBool(SettingsKeys::BORDERIZE_DELETE, false);
}

bool SettingsManager::isCustomBorderEnabled() const {
    return getBool(SettingsKeys::CUSTOM_BORDER_ENABLED, false);
}

int SettingsManager::getCustomBorderId() const {
    return getInt(SettingsKeys::CUSTOM_BORDER_ID, 1);
}

void SettingsManager::setAutomagicEnabled(bool enabled) {
    setBool(SettingsKeys::USE_AUTOMAGIC, enabled);
    setBool(SettingsKeys::BORDER_IS_GROUND, enabled); // wxWidgets compatibility
    emitAutomagicChanged();
}

void SettingsManager::setSameGroundTypeBorderEnabled(bool enabled) {
    setBool(SettingsKeys::SAME_GROUND_TYPE_BORDER, enabled);
    emitAutomagicChanged();
}

void SettingsManager::setWallsRepelBordersEnabled(bool enabled) {
    setBool(SettingsKeys::WALLS_REPEL_BORDERS, enabled);
    emitAutomagicChanged();
}

void SettingsManager::setLayerCarpetsEnabled(bool enabled) {
    setBool(SettingsKeys::LAYER_CARPETS, enabled);
    emitAutomagicChanged();
}

void SettingsManager::setBorderizeDeleteEnabled(bool enabled) {
    setBool(SettingsKeys::BORDERIZE_DELETE, enabled);
    emitAutomagicChanged();
}

void SettingsManager::setCustomBorderEnabled(bool enabled) {
    setBool(SettingsKeys::CUSTOM_BORDER_ENABLED, enabled);
    emitAutomagicChanged();
}

void SettingsManager::setCustomBorderId(int id) {
    setInt(SettingsKeys::CUSTOM_BORDER_ID, id);
    emitAutomagicChanged();
}

void SettingsManager::loadSettings() {
    // Settings are automatically loaded by QSettings
    qDebug() << "SettingsManager: Settings loaded from" << settings_->fileName();
}

void SettingsManager::saveSettings() {
    settings_->sync();
    qDebug() << "SettingsManager: Settings saved to" << settings_->fileName();
}

void SettingsManager::sync() {
    settings_->sync();
}

void SettingsManager::emitAutomagicChanged() {
    emit automagicSettingsChanged();
}


