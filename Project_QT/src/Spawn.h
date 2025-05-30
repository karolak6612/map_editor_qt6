#ifndef SPAWN_H
#define SPAWN_H

#include <QObject>
#include <QStringList>
#include "Map.h" // For MapPos

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
