#ifndef TOWN_H
#define TOWN_H

#include <QString>
// MapPos is defined in Map.h, so including Map.h
#include "Map.h" // For MapPos

class Town {
public:
    Town(quint32 id = 0, const QString& name = QString(), const MapPos& templePosition = MapPos());

    quint32 getId() const;
    void setId(quint32 id);

    QString getName() const;
    void setName(const QString& name);

    MapPos getTemplePosition() const;
    void setTemplePosition(const MapPos& position);

private:
    quint32 m_id;
    QString m_name;
    MapPos m_templePosition;
};

#endif // TOWN_H
