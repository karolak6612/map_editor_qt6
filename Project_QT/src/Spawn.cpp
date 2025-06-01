#include "Spawn.h"
#include <QDebug>
#include <QtMath> // For potential debugging, not strictly required by task but good practice

Spawn::Spawn(QObject* parent)
    : QObject(parent),
      position_(MapPos(0,0,0)), // Default position
      radius_(3),
      interval_(10000),
      maxCreatures_(3),
      selected_(false) {
    // Default creatureNames_ is already an empty QStringList
}

Spawn::Spawn(const MapPos& position, int radius, const QStringList& creatureNames, int interval, int count, QObject* parent)
    : QObject(parent),
      position_(position),
      radius_(radius),
      creatureNames_(creatureNames),
      interval_(interval),
      maxCreatures_(count),
      selected_(false) {
}

Spawn::~Spawn() {
}

MapPos Spawn::position() const {
    return position_;
}

void Spawn::setPosition(const MapPos& position) {
    // Assuming MapPos has operator!= or we compare members
    if (position_.x != position.x || position_.y != position.y || position_.z != position.z) {
        position_ = position;
        emit spawnChanged();
    }
}

int Spawn::radius() const {
    return radius_;
}

void Spawn::setRadius(int radius) {
    if (radius_ != radius) {
        radius_ = radius;
        emit spawnChanged();
    }
}

QStringList Spawn::creatureNames() const {
    return creatureNames_;
}

void Spawn::setCreatureNames(const QStringList& names) {
    if (creatureNames_ != names) {
        creatureNames_ = names;
        emit spawnChanged();
    }
}

void Spawn::addCreatureName(const QString& name) {
    if (!name.isEmpty() && !creatureNames_.contains(name)) {
        creatureNames_.append(name);
        // Sort for consistency, optional
        // creatureNames_.sort(); 
        emit spawnChanged();
    }
}

void Spawn::removeCreatureName(const QString& name) {
    if (creatureNames_.removeAll(name) > 0) { // removeAll returns number of items removed
        emit spawnChanged();
    }
}

int Spawn::interval() const {
    return interval_;
}

void Spawn::setInterval(int interval) {
    if (interval_ != interval) {
        interval_ = interval;
        emit spawnChanged();
    }
}

int Spawn::maxCreatures() const {
    return maxCreatures_;
}

void Spawn::setMaxCreatures(int count) {
    if (maxCreatures_ != count) {
        maxCreatures_ = count;
        emit spawnChanged();
    }
}

bool Spawn::isSelected() const {
    return selected_;
}

void Spawn::setSelected(bool selected) {
    if (selected_ != selected) {
        selected_ = selected;
        emit spawnChanged();
    }
}

quint32 Spawn::memsize() const {
    quint32 size = sizeof(Spawn);

    // Add size of creature names list
    for (const QString& name : creatureNames_) {
        size += name.size() * sizeof(QChar);
    }

    return size;
}

// Additional methods (wxWidgets compatibility)
Spawn* Spawn::deepCopy() const {
    Spawn* copy = new Spawn();
    copy->position_ = position_;
    copy->radius_ = radius_;
    copy->creatureNames_ = creatureNames_;
    copy->interval_ = interval_;
    copy->maxCreatures_ = maxCreatures_;
    copy->selected_ = selected_;
    return copy;
}

bool Spawn::canDraw() const {
    // A spawn can be drawn if it has at least one creature name and valid position
    return !creatureNames_.isEmpty() && isValid();
}

// Spawn area management
bool Spawn::isInSpawnArea(const MapPos& pos) const {
    int dx = pos.x - position_.x;
    int dy = pos.y - position_.y;
    int dz = pos.z - position_.z;

    // Check if position is within the spawn radius (using Manhattan distance for simplicity)
    // In wxWidgets, this might use Euclidean distance, but Manhattan is more common for tile-based games
    return (qAbs(dx) <= radius_ && qAbs(dy) <= radius_ && dz == 0);
}

QList<MapPos> Spawn::getSpawnArea() const {
    QList<MapPos> area;

    for (int x = position_.x - radius_; x <= position_.x + radius_; ++x) {
        for (int y = position_.y - radius_; y <= position_.y + radius_; ++y) {
            MapPos pos(x, y, position_.z);
            if (isInSpawnArea(pos)) {
                area.append(pos);
            }
        }
    }

    return area;
}

int Spawn::getSpawnAreaSize() const {
    // Calculate the number of tiles in the spawn area
    // For a square area: (2 * radius + 1)^2
    int sideLength = 2 * radius_ + 1;
    return sideLength * sideLength;
}

// Validation
bool Spawn::isValid() const {
    // A spawn is valid if:
    // 1. It has at least one creature name
    // 2. Radius is positive
    // 3. Interval is positive
    // 4. Max creatures is positive
    return !creatureNames_.isEmpty() &&
           radius_ > 0 &&
           interval_ > 0 &&
           maxCreatures_ > 0;
}

QString Spawn::getValidationError() const {
    if (creatureNames_.isEmpty()) {
        return "Spawn must have at least one creature name";
    }
    if (radius_ <= 0) {
        return "Spawn radius must be positive";
    }
    if (interval_ <= 0) {
        return "Spawn interval must be positive";
    }
    if (maxCreatures_ <= 0) {
        return "Max creatures must be positive";
    }
    return QString(); // No error
}
