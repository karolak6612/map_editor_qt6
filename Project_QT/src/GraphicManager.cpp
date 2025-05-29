#include "GraphicManager.h"
#include "Sprite.h"
#include "GameSprite.h"
#include "EditorSprite.h"
#include <QFile>
#include <QBuffer>
#include <QDebug> 
#include <QStringList> 
#include <QPixmap> // Added for QPixmap

enum EditorSpriteId {
    EDITOR_SPRITE_SELECTION_MARKER = -1,
    EDITOR_SPRITE_BRUSH_CD_1x1 = -2,
};

GraphicManager::GraphicManager() {
}

GraphicManager::~GraphicManager() {
    clear();
}

void GraphicManager::clear() {
    m_sprite_cache.clear();
    m_item_sprite_cache.clear();
    m_creature_sprite_cache.clear();
    m_item_count = 0;
    m_creature_count = 0;
}

void GraphicManager::cleanSoftwareSprites() {
    qDebug() << "GraphicManager::cleanSoftwareSprites() called - currently a no-op or consider cache clearing.";
}

QSharedPointer<Sprite> GraphicManager::getSprite(int id) {
    if (m_sprite_cache.contains(id)) {
        return m_sprite_cache.value(id);
    }
    return QSharedPointer<Sprite>(); 
}

QSharedPointer<GameSprite> GraphicManager::getGameSprite(int id) {
    if (id >= 0 && id < m_item_count) { 
         if (m_item_sprite_cache.contains(id)) {
            return m_item_sprite_cache.value(id);
        }
    } else { 
        int creature_id_offset = id - m_item_count; 
         if (m_creature_sprite_cache.contains(creature_id_offset)) {
            return m_creature_sprite_cache.value(creature_id_offset);
        }
    }
    return QSharedPointer<GameSprite>();
}

QSharedPointer<EditorSprite> GraphicManager::getEditorSprite(int editor_sprite_id) {
    if (m_sprite_cache.contains(editor_sprite_id)) {
        QSharedPointer<Sprite> sprite = m_sprite_cache.value(editor_sprite_id);
        return qSharedPointerCast<EditorSprite>(sprite);
    }
    return QSharedPointer<EditorSprite>();
}

QPixmap GraphicManager::getSpritePixmap(int id) {
    QSharedPointer<Sprite> sprite = getSprite(id);
    if (sprite) {
        return sprite->getPixmap();
    }
    return QPixmap(); 
}

QPixmap GraphicManager::getGameSpritePixmap(int id) {
    QSharedPointer<GameSprite> gameSprite = getGameSprite(id);
    if (gameSprite) {
        return gameSprite->getPixmap();
    }
    return QPixmap();
}

QPixmap GraphicManager::getEditorSpritePixmap(int editor_sprite_id) {
    QSharedPointer<EditorSprite> editorSprite = getEditorSprite(editor_sprite_id);
    if (editorSprite) {
        return editorSprite->getPixmap();
    }
    return QPixmap();
}

bool GraphicManager::loadEditorSprites() {
    qDebug() << "GraphicManager::loadEditorSprites() - Placeholder. Implement loading from resources.";
    return true; 
}

bool GraphicManager::loadSpriteAssets(const QString& dataPath, QString& error, QStringList& warnings) {
    qDebug() << "GraphicManager::loadSpriteAssets() - Placeholder for complex asset loading pipeline.";
    error = "Asset loading not yet implemented.";
    (void)dataPath; 
    (void)warnings; 
    return false; 
}

QImage GraphicManager::loadImageFromData(const QByteArray& data, const char* format) {
    QImage image;
    if (image.loadFromData(data, format)) {
        return image;
    }
    qWarning() << "GraphicManager::loadImageFromData - Failed to load image from data. Format:" << (format ? format : "autodetect");
    return QImage(); 
}

bool GraphicManager::loadGameSpriteData(QSharedPointer<GameSprite> gameSprite, const QByteArray& sprite_data) {
    if (!gameSprite) {
        qWarning() << "GraphicManager::loadGameSpriteData - Null GameSprite provided.";
        return false;
    }
    QImage whole_sheet;
    if (whole_sheet.loadFromData(sprite_data)) {
        if(gameSprite) { 
             gameSprite->setImage(whole_sheet);
        }
        return true;
    }
    qWarning() << "GraphicManager::loadGameSpriteData - Failed to load image from provided data.";
    return false;
}

void GraphicManager::setClientVersion(const ClientVersion& version) {
    m_client_version = version;
}

const ClientVersion* GraphicManager::getClientVersion() const {
    return &m_client_version;
}
