#include "Town.h"

Town::Town(quint32 id, const QString& name, const MapPos& templePosition)
    : m_id(id), m_name(name), m_templePosition(templePosition) {
}

quint32 Town::getId() const {
    return m_id;
}

void Town::setId(quint32 id) {
    m_id = id;
}

QString Town::getName() const {
    return m_name;
}

void Town::setName(const QString& name) {
    m_name = name;
}

MapPos Town::getTemplePosition() const {
    return m_templePosition;
}

void Town::setTemplePosition(const MapPos& position) {
    m_templePosition = position;
}
