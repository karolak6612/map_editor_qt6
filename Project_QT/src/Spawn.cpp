#include "Spawn.h"
#include <QDebug> // For potential debugging, not strictly required by task but good practice

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
