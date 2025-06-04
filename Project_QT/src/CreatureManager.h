#ifndef CREATUREMANAGER_H
#define CREATUREMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QVariant>
#include "CreatureProperties.h"

class CreatureManager : public QObject {
    Q_OBJECT

public:
    // Singleton pattern
    static CreatureManager& getInstance();
    static void destroyInstance();

    ~CreatureManager();

    // Core loading and access methods
    bool loadCreaturesFromXml(const QString& filePath, QString& error, QStringList& warnings, bool standard = true);
    bool loadCreaturesFromXml(const QString& filePath, bool standard = true);
    bool importXMLFromOT(const QString& filePath, QString& error, QStringList& warnings);
    bool importXMLFromOT(const QString& filePath);
    bool saveToXML(const QString& filePath) const;

    CreatureProperties getCreatureProperties(int id) const;
    CreatureProperties getCreatureProperties(const QString& name) const;
    bool hasCreature(int id) const;
    bool hasCreature(const QString& name) const;

    // Enhanced creature management
    CreatureProperties* addMissingCreatureType(const QString& name, bool isNpc);
    CreatureProperties* addCreatureType(const QString& name, bool isNpc, const Outfit& outfit);
    bool hasMissing() const;

    // Collection methods
    QList<CreatureProperties> getAllCreatures() const;
    QList<CreatureProperties> getCreaturesByType(bool isNpc) const;
    QStringList getCreatureNames() const;
    int getCreatureCount() const;

    // Utility methods
    void clear();
    bool isLoaded() const;

signals:
    void creaturesLoaded();
    void creaturesCleared();
    void creatureAdded(const QString& name, int id);
    void creatureRemoved(const QString& name, int id);

private:
    // Private constructor for singleton
    explicit CreatureManager(QObject* parent = nullptr);
    CreatureManager(const CreatureManager&) = delete;
    CreatureManager& operator=(const CreatureManager&) = delete;

    // Data storage
    QMap<int, CreatureProperties> creaturePropertiesMap_;
    QMap<QString, int> creatureNameToIdMap_;
    bool loaded_ = false;

    // Singleton instance
    static CreatureManager* s_instance;
};

#endif // CREATUREMANAGER_H
