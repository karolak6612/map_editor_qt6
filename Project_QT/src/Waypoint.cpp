#include "Waypoint.h"
#include <QDebug> // For potential debugging

Waypoint::Waypoint(QObject* parent)
    : QObject(parent),
      name_(""),
      position_(MapPos(0,0,0)),
      type_("generic"),
      scriptOrText_(""),
      radius_(1),
      color_(Qt::blue),
      iconType_("generic"),
      selected_(false) {
}

Waypoint::Waypoint(const QString& name, const MapPos& position, const QString& type, const QString& scriptOrText, QObject* parent)
    : QObject(parent),
      name_(name),
      position_(position),
      type_(type.isEmpty() ? "generic" : type), // Default to "generic" if type is empty
      scriptOrText_(scriptOrText),
      radius_(1),
      color_(Qt::blue),
      iconType_("generic"),
      selected_(false) {
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

// Additional attributes (extended from wxWidgets)
int Waypoint::radius() const {
    return radius_;
}

void Waypoint::setRadius(int radius) {
    if (radius_ != radius) {
        radius_ = radius;
        emit waypointChanged();
    }
}

QColor Waypoint::color() const {
    return color_;
}

void Waypoint::setColor(const QColor& color) {
    if (color_ != color) {
        color_ = color;
        emit waypointChanged();
    }
}

QString Waypoint::iconType() const {
    return iconType_;
}

void Waypoint::setIconType(const QString& iconType) {
    if (iconType_ != iconType) {
        iconType_ = iconType;
        emit waypointChanged();
    }
}

bool Waypoint::isSelected() const {
    return selected_;
}

void Waypoint::setSelected(bool selected) {
    if (selected_ != selected) {
        selected_ = selected;
        emit waypointChanged();
    }
}

// Utility methods
Waypoint* Waypoint::deepCopy() const {
    Waypoint* copy = new Waypoint();
    copy->name_ = name_;
    copy->position_ = position_;
    copy->type_ = type_;
    copy->scriptOrText_ = scriptOrText_;
    copy->radius_ = radius_;
    copy->color_ = color_;
    copy->iconType_ = iconType_;
    copy->selected_ = selected_;
    return copy;
}

bool Waypoint::isValid() const {
    // A waypoint is valid if it has a non-empty name
    return !name_.trimmed().isEmpty();
}

QString Waypoint::getValidationError() const {
    if (name_.trimmed().isEmpty()) {
        return "Waypoint name cannot be empty";
    }
    return QString(); // No error
}

quint32 Waypoint::memsize() const {
    quint32 size = sizeof(Waypoint);
    size += name_.size() * sizeof(QChar);
    size += type_.size() * sizeof(QChar);
    size += scriptOrText_.size() * sizeof(QChar);
    size += iconType_.size() * sizeof(QChar);
    return size;
}

// Case-insensitive name comparison (wxWidgets compatibility)
QString Waypoint::normalizedName() const {
    return normalizeName(name_);
}

QString Waypoint::normalizeName(const QString& name) {
    return name.toLower().trimmed();
}
