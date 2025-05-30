#ifndef CREATUREMANAGER_H
#define CREATUREMANAGER_H

#include <QMap>
#include <QString>
#include "CreatureProperties.h"

class CreatureManager {
public:
    CreatureManager();
    ~CreatureManager();

    bool loadCreaturesFromXml(const QString& filePath);
    CreatureProperties getCreatureProperties(int id) const;
    CreatureProperties getCreatureProperties(const QString& name) const;
    bool hasCreature(int id) const;
    bool hasCreature(const QString& name) const;

private:
    QMap<int, CreatureProperties> creaturePropertiesMap_;
    QMap<QString, int> creatureNameToIdMap_;
};

#endif // CREATUREMANAGER_H
