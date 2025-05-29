#ifndef GRAPHICMANAGER_H
#define GRAPHICMANAGER_H

#include <QImage>
#include <QPixmap> // Added for QPixmap
#include <QString>
#include <QMap>
#include <QVector>
#include <QSharedPointer> 
#include <QStringList> 

class Sprite;
class GameSprite;
class EditorSprite;
class QFile; 

struct ClientVersion { 
    QString versionString;
};

struct SpriteLight { 
    uint8_t intensity = 0;
    uint8_t color = 0; 
};

class GraphicManager {
public:
    GraphicManager();
    ~GraphicManager();

    void clear(); 
    void cleanSoftwareSprites(); 

    QSharedPointer<Sprite> getSprite(int id); 
    virtual QSharedPointer<GameSprite> getGameSprite(int id); 
    QSharedPointer<EditorSprite> getEditorSprite(int editor_sprite_id); 

    QPixmap getSpritePixmap(int id);
    QPixmap getGameSpritePixmap(int id);
    QPixmap getEditorSpritePixmap(int editor_sprite_id);

    bool loadEditorSprites(); 
    bool loadSpriteAssets(const QString& dataPath, QString& error, QStringList& warnings);
    
    QImage loadImageFromData(const QByteArray& data, const char* format = "PNG");
    bool loadGameSpriteData(QSharedPointer<GameSprite> gameSprite, const QByteArray& sprite_sheet_data );

    void setClientVersion(const ClientVersion& version);
    const ClientVersion* getClientVersion() const;

private:
    QMap<int, QSharedPointer<Sprite>> m_sprite_cache; 
    QMap<int, QSharedPointer<GameSprite>> m_item_sprite_cache;
    QMap<int, QSharedPointer<GameSprite>> m_creature_sprite_cache;
    
    ClientVersion m_client_version; 
    QString m_metadata_file_path;
    QString m_sprites_file_path;
    uint16_t m_item_count = 0;
    uint16_t m_creature_count = 0;
};

#endif // GRAPHICMANAGER_H
