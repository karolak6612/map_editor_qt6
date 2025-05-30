#include "Waypoint.h"
#include <QDebug> // For potential debugging

Waypoint::Waypoint(QObject* parent)
    : QObject(parent),
      name_(""),
      position_(MapPos(0,0,0)),
      type_("generic"),
      scriptOrText_("") {
}

Waypoint::Waypoint(const QString& name, const MapPos& position, const QString& type, const QString& scriptOrText, QObject* parent)
    : QObject(parent),
      name_(name),
      position_(position),
      type_(type.isEmpty() ? "generic" : type), // Default to "generic" if type is empty
      scriptOrText_(scriptOrText) {
}

Waypoint::~Waypoint() {
}

QString Waypoint::name() const {
    return name_;
}

void Waypoint::setName(const QString& name) {
    if (name_ != name) {
        name_ = name;
        emit waypointChanged();
    }
}

MapPos Waypoint::position() const {
    return position_;
}

void Waypoint::setPosition(const MapPos& position) {
    // Assuming MapPos has operator!= or we compare members
    if (position_.x != position.x || position_.y != position.y || position_.z != position.z) {
        position_ = position;
        emit waypointChanged();
    }
}

QString Waypoint::type() const {
    return type_;
}

void Waypoint::setType(const QString& type) {
    if (type_ != type) {
        type_ = type;
        emit waypointChanged();
    }
}

QString Waypoint::scriptOrText() const {
    return scriptOrText_;
}

void Waypoint::setScriptOrText(const QString& scriptOrText) {
    if (scriptOrText_ != scriptOrText) {
        scriptOrText_ = scriptOrText;
        emit waypointChanged();
    }
}
