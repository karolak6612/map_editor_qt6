#ifndef CREATURE_H
#define CREATURE_H

#include <QObject>
#include <QString>
#include <QtGlobal> // For quint8, quint16

// Forward declarations
class GameSprite; // For future use: GameSprite* currentSprite_;
class Brush;    // For associated brush

// Direction enum (can be moved to a common types file later if widely used)
enum class Direction : quint8 {
    North = 0,
    East  = 1,
    South = 2,
    West  = 3
    // NorthEast, SouthEast, SouthWest, NorthWest can be added if needed for diagonal
};
// Q_ENUM(Direction) // If you want to make it known to Qt's meta-object system (requires QObject context or global registration)

#include <QRectF> // For draw method targetRect
#include "DrawingOptions.h" // For draw method options
#include "Outfit.h" // For Outfit struct integration

// Forward declarations
class QPainter;

class Creature : public QObject {
    Q_OBJECT

public:
    explicit Creature(const QString& name = QString(), QObject *parent = nullptr);
    ~Creature() override;

    // Name
    QString name() const;
    void setName(const QString& name);

    // Outfit Details
    int lookType() const;
    void setLookType(int type);

    int lookHead() const;
    void setLookHead(int head);

    int lookBody() const;
    void setLookBody(int body);

    int lookLegs() const;
    void setLookLegs(int legs);

    int lookFeet() const;
    void setLookFeet(int feet);

    int lookAddons() const;
    void setLookAddons(int addons);
    
    int lookMount() const; // Mount outfit/sprite ID
    void setLookMount(int mountId);

    int lookMountHead() const;
    void setLookMountHead(int head);

    int lookMountBody() const;
    void setLookMountBody(int body);

    int lookMountLegs() const;
    void setLookMountLegs(int legs);

    int lookMountFeet() const;
    void setLookMountFeet(int feet);

    // Properties
    int speed() const;
    void setSpeed(int speed);

    int health() const; // Current health, might not be relevant for editor definition
    void setHealth(int health);

    int maxHealth() const; // Max health, part of definition
    void setMaxHealth(int maxHealth);

    quint8 lightLevel() const;
    void setLightLevel(quint8 level);

    quint8 lightColor() const;
    void setLightColor(quint8 color);

    quint8 skull() const; // PK skull, etc.
    void setSkull(quint8 skull);

    quint8 shield() const; // Shield GFX
    void setShield(quint8 shield);

    quint8 emblem() const; // Guild emblem, etc.
    void setEmblem(quint8 emblem);
    
    quint8 icon() const; // Icon displayed in some UIs (e.g. party)
    void setIcon(quint8 icon);

    quint16 corpseId() const; // Server ID of corpse item
    void setCorpseId(quint16 id);

    Direction direction() const;
    void setDirection(Direction direction);
    
    int spawnTime() const; // Instance-specific spawn time (original had this)
    void setSpawnTime(int time);

    bool isNpc() const;
    void setIsNpc(bool isNpc); // Distinguishes from monster

    bool isSelected() const; // Editor-specific state
    void setSelected(bool selected);

    Brush* getBrush() const;      // Associated brush for placing this creature
    void setBrush(Brush* brush); // Not owned by Creature instance, set by CreatureManager

    // Save/reset functionality for editor state management
    bool isSaved() const;
    void save();
    void reset();

    // Outfit integration
    Outfit getOutfit() const;
    void setOutfit(const Outfit& outfit);

    // Static direction conversion methods (wxWidgets compatibility)
    static QString directionToString(Direction dir);
    static Direction stringToDirection(const QString& dirStr);
    static quint16 directionToId(Direction dir);
    static Direction idToDirection(quint16 id);

    virtual void draw(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const;
    Creature* deepCopy() const;
    quint32 memsize() const; // Memory footprint calculation

signals:
    void creatureChanged(); // Emitted when any property changes that might require UI/data update

private:
    QString name_;
    
    // Outfit
    int lookType_ = 0;
    int lookHead_ = 0;
    int lookBody_ = 0;
    int lookLegs_ = 0;
    int lookFeet_ = 0;
    int lookAddons_ = 0;
    int lookMount_ = 0;
    int lookMountHead_ = 0;
    int lookMountBody_ = 0;
    int lookMountLegs_ = 0;
    int lookMountFeet_ = 0;

    // Stats & Other Properties
    int speed_ = 220; // Default from plan (typical player speed)
    int health_ = 100; // Current health (might be mostly for runtime state)
    int maxHealth_ = 100; // Max health (part of definition)
    quint8 lightLevel_ = 0;
    quint8 lightColor_ = 0;
    quint8 skull_ = 0;
    quint8 shield_ = 0;
    quint8 emblem_ = 0;
    quint8 icon_ = 0;
    quint16 corpseId_ = 0; // Server ID of the corpse item
    Direction direction_ = Direction::South; // Default direction
    
    // Instance-specific data (not part of creature type definition)
    int spawnTime_ = 0; 

    // Type/State flags
    bool isNpc_ = false;
    bool isSelected_ = false; // Editor specific selection state
    bool saved_ = false; // Editor save state

    Brush* brush_ = nullptr; // Associated brush, not owned by creature instance. Set by CreatureManager.
    // GameSprite* currentSprite_ = nullptr; // For future sprite integration (Task 27)
};

#endif // CREATURE_H
