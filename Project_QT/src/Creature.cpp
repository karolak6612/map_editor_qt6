#include "Creature.h"
#include "Brush.h" // For Brush* type
#include <QPainter> // For draw method
#include <QColor>   // For draw method placeholder
#include <QDebug>
// DrawingOptions.h is included via Creature.h

Creature::Creature(const QString& name, QObject *parent) : QObject(parent),
    name_(name),
    lookType_(0),
    lookHead_(0),
    lookBody_(0),
    lookLegs_(0),
    lookFeet_(0),
    lookAddons_(0),
    lookMount_(0),
    lookMountHead_(0),
    lookMountBody_(0),
    lookMountLegs_(0),
    lookMountFeet_(0),
    speed_(220),      // Default speed
    health_(100),     // Default current health
    maxHealth_(100),  // Default max health
    lightLevel_(0),
    lightColor_(0),
    skull_(0),
    shield_(0),
    emblem_(0),
    icon_(0),
    corpseId_(0),
    direction_(Direction::South), // Default direction
    spawnTime_(0),    // Default to 0, instance specific
    isNpc_(false),
    isSelected_(false),
    saved_(false),
    brush_(nullptr)   // Not owned
    // currentSprite_ = nullptr; // Future use
{
    // Initialization of members to default values is done in the member initializer list
}

Creature::~Creature() {
    // No owned resources to delete in this basic version 
    // (brush_ is not owned, currentSprite_ is for later and would also likely not be owned here)
}

// Name
QString Creature::name() const { return name_; }
void Creature::setName(const QString& name) {
    if (name_ != name) {
        name_ = name;
        emit creatureChanged();
    }
}

// Outfit Details
int Creature::lookType() const { return lookType_; }
void Creature::setLookType(int type) {
    if (lookType_ != type) {
        lookType_ = type;
        emit creatureChanged();
    }
}

int Creature::lookHead() const { return lookHead_; }
void Creature::setLookHead(int head) {
    if (lookHead_ != head) {
        lookHead_ = head;
        emit creatureChanged();
    }
}

int Creature::lookBody() const { return lookBody_; }
void Creature::setLookBody(int body) {
    if (lookBody_ != body) {
        lookBody_ = body;
        emit creatureChanged();
    }
}

int Creature::lookLegs() const { return lookLegs_; }
void Creature::setLookLegs(int legs) {
    if (lookLegs_ != legs) {
        lookLegs_ = legs;
        emit creatureChanged();
    }
}

int Creature::lookFeet() const { return lookFeet_; }
void Creature::setLookFeet(int feet) {
    if (lookFeet_ != feet) {
        lookFeet_ = feet;
        emit creatureChanged();
    }
}

int Creature::lookAddons() const { return lookAddons_; }
void Creature::setLookAddons(int addons) {
    if (lookAddons_ != addons) {
        lookAddons_ = addons;
        emit creatureChanged();
    }
}

int Creature::lookMount() const { return lookMount_; }
void Creature::setLookMount(int mountId) {
    if (lookMount_ != mountId) {
        lookMount_ = mountId;
        emit creatureChanged();
    }
}

int Creature::lookMountHead() const { return lookMountHead_; }
void Creature::setLookMountHead(int head) {
    if (lookMountHead_ != head) {
        lookMountHead_ = head;
        emit creatureChanged();
    }
}

int Creature::lookMountBody() const { return lookMountBody_; }
void Creature::setLookMountBody(int body) {
    if (lookMountBody_ != body) {
        lookMountBody_ = body;
        emit creatureChanged();
    }
}

int Creature::lookMountLegs() const { return lookMountLegs_; }
void Creature::setLookMountLegs(int legs) {
    if (lookMountLegs_ != legs) {
        lookMountLegs_ = legs;
        emit creatureChanged();
    }
}

int Creature::lookMountFeet() const { return lookMountFeet_; }
void Creature::setLookMountFeet(int feet) {
    if (lookMountFeet_ != feet) {
        lookMountFeet_ = feet;
        emit creatureChanged();
    }
}

// Properties
int Creature::speed() const { return speed_; }
void Creature::setSpeed(int speed) {
    if (speed_ != speed) {
        speed_ = speed;
        emit creatureChanged();
    }
}

int Creature::health() const { return health_; }
void Creature::setHealth(int health) {
    if (health_ != health) {
        health_ = health;
        emit creatureChanged();
    }
}

int Creature::maxHealth() const { return maxHealth_; }
void Creature::setMaxHealth(int maxHealth) {
    if (maxHealth_ != maxHealth) {
        maxHealth_ = maxHealth;
        emit creatureChanged();
    }
}

quint8 Creature::lightLevel() const { return lightLevel_; }
void Creature::setLightLevel(quint8 level) {
    if (lightLevel_ != level) {
        lightLevel_ = level;
        emit creatureChanged();
    }
}

quint8 Creature::lightColor() const { return lightColor_; }
void Creature::setLightColor(quint8 color) {
    if (lightColor_ != color) {
        lightColor_ = color;
        emit creatureChanged();
    }
}

quint8 Creature::skull() const { return skull_; }
void Creature::setSkull(quint8 skull) {
    if (skull_ != skull) {
        skull_ = skull;
        emit creatureChanged();
    }
}

quint8 Creature::shield() const { return shield_; }
void Creature::setShield(quint8 shield) {
    if (shield_ != shield) {
        shield_ = shield;
        emit creatureChanged();
    }
}

quint8 Creature::emblem() const { return emblem_; }
void Creature::setEmblem(quint8 emblem) {
    if (emblem_ != emblem) {
        emblem_ = emblem;
        emit creatureChanged();
    }
}

quint8 Creature::icon() const { return icon_; }
void Creature::setIcon(quint8 icon) {
    if (icon_ != icon) {
        icon_ = icon;
        emit creatureChanged();
    }
}

quint16 Creature::corpseId() const { return corpseId_; }
void Creature::setCorpseId(quint16 id) {
    if (corpseId_ != id) {
        corpseId_ = id;
        emit creatureChanged();
    }
}

Direction Creature::direction() const { return direction_; }
void Creature::setDirection(Direction direction) {
    if (direction_ != direction) {
        direction_ = direction;
        emit creatureChanged();
    }
}

int Creature::spawnTime() const { return spawnTime_; }
void Creature::setSpawnTime(int time) {
    // This is instance specific, might not need creatureChanged() if it's not part of "type" definition
    if (spawnTime_ != time) {
        spawnTime_ = time;
        // emit creatureChanged(); // Or a more specific signal if this change is frequent and non-defining
    }
}

bool Creature::isNpc() const { return isNpc_; }
void Creature::setIsNpc(bool isNpc) {
    if (isNpc_ != isNpc) {
        isNpc_ = isNpc;
        emit creatureChanged();
    }
}

bool Creature::isSelected() const { return isSelected_; }
void Creature::setSelected(bool selected) {
    if (isSelected_ != selected) {
        isSelected_ = selected;
        // Selection is often a view-specific state, might not need a full creatureChanged()
        // or could have its own selectionChanged() signal.
        // For now, assume it's a property that might trigger general updates.
        emit creatureChanged(); 
    }
}

Brush* Creature::getBrush() const { return brush_; }
void Creature::setBrush(Brush* brush) {
    // Not emitting creatureChanged() here as brush is external metadata,
    // not an intrinsic property of the creature type or instance data.
    // This is typically set by a CreatureManager when loading definitions.
    brush_ = brush;
}

// --- Save/Reset Functionality ---

bool Creature::isSaved() const {
    return saved_;
}

void Creature::save() {
    saved_ = true;
}

void Creature::reset() {
    saved_ = false;
}

// --- Outfit Integration ---

Outfit Creature::getOutfit() const {
    Outfit outfit;
    outfit.lookType = lookType_;
    outfit.lookHead = lookHead_;
    outfit.lookBody = lookBody_;
    outfit.lookLegs = lookLegs_;
    outfit.lookFeet = lookFeet_;
    outfit.lookAddon = lookAddons_;
    outfit.lookMount = lookMount_;
    outfit.lookMountHead = lookMountHead_;
    outfit.lookMountBody = lookMountBody_;
    outfit.lookMountLegs = lookMountLegs_;
    outfit.lookMountFeet = lookMountFeet_;
    return outfit;
}

void Creature::setOutfit(const Outfit& outfit) {
    bool changed = false;

    if (lookType_ != outfit.lookType) {
        lookType_ = outfit.lookType;
        changed = true;
    }
    if (lookHead_ != outfit.lookHead) {
        lookHead_ = outfit.lookHead;
        changed = true;
    }
    if (lookBody_ != outfit.lookBody) {
        lookBody_ = outfit.lookBody;
        changed = true;
    }
    if (lookLegs_ != outfit.lookLegs) {
        lookLegs_ = outfit.lookLegs;
        changed = true;
    }
    if (lookFeet_ != outfit.lookFeet) {
        lookFeet_ = outfit.lookFeet;
        changed = true;
    }
    if (lookAddons_ != outfit.lookAddon) {
        lookAddons_ = outfit.lookAddon;
        changed = true;
    }
    if (lookMount_ != outfit.lookMount) {
        lookMount_ = outfit.lookMount;
        changed = true;
    }
    if (lookMountHead_ != outfit.lookMountHead) {
        lookMountHead_ = outfit.lookMountHead;
        changed = true;
    }
    if (lookMountBody_ != outfit.lookMountBody) {
        lookMountBody_ = outfit.lookMountBody;
        changed = true;
    }
    if (lookMountLegs_ != outfit.lookMountLegs) {
        lookMountLegs_ = outfit.lookMountLegs;
        changed = true;
    }
    if (lookMountFeet_ != outfit.lookMountFeet) {
        lookMountFeet_ = outfit.lookMountFeet;
        changed = true;
    }

    if (changed) {
        emit creatureChanged();
    }
}

// --- Static Direction Conversion Methods ---

QString Creature::directionToString(Direction dir) {
    switch (dir) {
        case Direction::North:
            return QStringLiteral("North");
        case Direction::East:
            return QStringLiteral("East");
        case Direction::South:
            return QStringLiteral("South");
        case Direction::West:
            return QStringLiteral("West");
        default:
            return QStringLiteral("Unknown");
    }
}

Direction Creature::stringToDirection(const QString& dirStr) {
    QString lowerStr = dirStr.toLower();
    if (lowerStr == "north") {
        return Direction::North;
    } else if (lowerStr == "east") {
        return Direction::East;
    } else if (lowerStr == "south") {
        return Direction::South;
    } else if (lowerStr == "west") {
        return Direction::West;
    }
    return Direction::South; // Default
}

quint16 Creature::directionToId(Direction dir) {
    return static_cast<quint16>(dir);
}

Direction Creature::idToDirection(quint16 id) {
    switch (id) {
        case 0:
            return Direction::North;
        case 1:
            return Direction::East;
        case 2:
            return Direction::South;
        case 3:
            return Direction::West;
        default:
            return Direction::South; // Default
    }
}

Creature* Creature::deepCopy() const {
    // Parent will be set by caller (e.g., Tile or when placing on map)
    Creature* newCreature = new Creature(this->name_); 
    
    newCreature->lookType_ = this->lookType_;
    newCreature->lookHead_ = this->lookHead_;
    newCreature->lookBody_ = this->lookBody_;
    newCreature->lookLegs_ = this->lookLegs_;
    newCreature->lookFeet_ = this->lookFeet_;
    newCreature->lookAddons_ = this->lookAddons_;
    newCreature->lookMount_ = this->lookMount_;
    newCreature->lookMountHead_ = this->lookMountHead_;
    newCreature->lookMountBody_ = this->lookMountBody_;
    newCreature->lookMountLegs_ = this->lookMountLegs_;
    newCreature->lookMountFeet_ = this->lookMountFeet_;
    newCreature->speed_ = this->speed_;
    newCreature->health_ = this->health_; // Copy current health for an instance copy
    newCreature->maxHealth_ = this->maxHealth_;
    newCreature->lightLevel_ = this->lightLevel_;
    newCreature->lightColor_ = this->lightColor_;
    newCreature->skull_ = this->skull_;
    newCreature->shield_ = this->shield_;
    newCreature->emblem_ = this->emblem_;
    newCreature->icon_ = this->icon_;
    newCreature->corpseId_ = this->corpseId_;
    newCreature->direction_ = this->direction_;
    newCreature->spawnTime_ = this->spawnTime_; // Copy instance-specific spawn time
    newCreature->isNpc_ = this->isNpc_;
    newCreature->isSelected_ = this->isSelected_; // Copy selection state
    newCreature->saved_ = this->saved_; // Copy saved state
    newCreature->brush_ = this->brush_; // Copy brush pointer (not owned, just a reference)
    // newCreature->currentSprite_ = nullptr; // Sprites are not deeply copied here, handled by sprite manager

    return newCreature;
}

void Creature::draw(QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) const {
    if (!painter) return;

    // Placeholder: Draw a semi-transparent reddish rectangle for a creature
    QColor creatureColor = Qt::red;
    // Vary color based on lookType_
    if (lookType_ != 0) { 
        creatureColor.setHsv(((lookType_ * 45) % 360), 255, 210); // Adjusted multiplier for more variance
    }

    painter->fillRect(targetRect, QColor(creatureColor.red(), creatureColor.green(), creatureColor.blue(), 128)); // Semi-transparent
    painter->drawRect(targetRect); // Draw an outline

    // Optional: Use options to decide whether to draw name, health bar, etc.
    // if (options.showCreatureNames && !name_.isEmpty()) { 
    //     painter->save();
    //     painter->setPen(Qt::white); // Example: white text for names
    //     // Basic text centering (approximate)
    //     painter->drawText(targetRect, Qt::AlignCenter, name_);
    //     painter->restore();
    // }
    
    // qDebug() << "Creature::draw() called for:" << name_ << "at" << targetRect;
}

quint32 Creature::memsize() const {
    quint32 size = sizeof(Creature);

    // Add size of string member
    size += name_.size() * sizeof(QChar);

    return size;
}
