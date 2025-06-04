#ifndef SPAWN_H
#define SPAWN_H

#include <QObject>
#include <QStringList>
#include "MapPos.h" // Direct include for MapPos typedef

class Spawn : public QObject {
    Q_OBJECT

public:
    explicit Spawn(QObject* parent = nullptr);
    Spawn(const MapPos& position, int radius, const QStringList& creatureNames, int interval, int count, QObject* parent = nullptr);
    ~Spawn() override;

    MapPos position() const;
    void setPosition(const MapPos& position);

    int radius() const; // In map units (tiles)
    void setRadius(int radius);

    QStringList creatureNames() const;
    void setCreatureNames(const QStringList& names);
    void addCreatureName(const QString& name);
    void removeCreatureName(const QString& name);

    int interval() const; // In milliseconds
    void setInterval(int interval);

    int maxCreatures() const; // Max number of creatures this spawn point can have active
    void setMaxCreatures(int count);

    bool isSelected() const;
    void setSelected(bool selected);

    quint32 memsize() const; // Memory footprint calculation

    // Additional methods (wxWidgets compatibility)
    Spawn* deepCopy() const;
    bool canDraw() const;

    // Spawn area management
    bool isInSpawnArea(const MapPos& pos) const;
    QList<MapPos> getSpawnArea() const;
    int getSpawnAreaSize() const;

    // Validation
    bool isValid() const;
    QString getValidationError() const;

signals:
    void spawnChanged();

private:
    MapPos position_;
    int radius_ = 3;
    QStringList creatureNames_;
    int interval_ = 10000; // 10 seconds
    int maxCreatures_ = 3;
    bool selected_ = false;
};

#endif // SPAWN_H
