#ifndef SPAWN_H
#define SPAWN_H

#include <QObject> 
#include <QList>   // For list of creatures later
#include <QString> // For creature names

// Forward declaration
class Creature;

class Spawn : public QObject { 
    Q_OBJECT 
public:
    explicit Spawn(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Spawn() {}

    // Example methods
    // virtual int getRadius() const { return 1; }
    // virtual QList<QString> getCreatureNames() const { return {}; }
};

#endif // SPAWN_H
