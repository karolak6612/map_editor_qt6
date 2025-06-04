#include "ModifySpawnCommand.h"
#include "Spawn.h"
#include <QDebug>

ModifySpawnCommand::ModifySpawnCommand(Spawn* spawn, const SpawnData& newData, QUndoCommand* parent)
    : QUndoCommand(parent),
      spawn_(spawn),
      newData_(newData),
      firstRedo_(true)
{
    if (!spawn_) {
        qWarning() << "ModifySpawnCommand: Spawn pointer is null";
        return;
    }

    // Generate command text
    setText(QObject::tr("Modify Spawn"));
}

void ModifySpawnCommand::redo()
{
    if (!spawn_) {
        qWarning() << "ModifySpawnCommand::redo(): Spawn pointer is null";
        return;
    }

    // Task 019: Capture old data only on first redo to avoid overwriting original state
    if (firstRedo_) {
        oldData_ = captureSpawnData(spawn_);
        firstRedo_ = false;
        qDebug() << "ModifySpawnCommand: Captured old spawn data - radius:" << oldData_.radius
                 << "interval:" << oldData_.interval << "maxCreatures:" << oldData_.maxCreatures
                 << "creatures:" << oldData_.creatureNames;
    }

    // Apply the new data
    applySpawnData(spawn_, newData_);
    qDebug() << "ModifySpawnCommand: Applied new spawn data - radius:" << newData_.radius
             << "interval:" << newData_.interval << "maxCreatures:" << newData_.maxCreatures
             << "creatures:" << newData_.creatureNames;
}

void ModifySpawnCommand::undo()
{
    if (!spawn_) {
        qWarning() << "ModifySpawnCommand::undo(): Spawn pointer is null";
        return;
    }

    // Restore the old data
    applySpawnData(spawn_, oldData_);
    qDebug() << "ModifySpawnCommand: Restored old spawn data - radius:" << oldData_.radius
             << "interval:" << oldData_.interval << "maxCreatures:" << oldData_.maxCreatures
             << "creatures:" << oldData_.creatureNames;
}

ModifySpawnCommand::SpawnData ModifySpawnCommand::captureSpawnData(Spawn* spawn) const
{
    if (!spawn) {
        return SpawnData();
    }

    return SpawnData(
        spawn->radius(),
        spawn->interval(),
        spawn->maxCreatures(),
        spawn->creatureNames()
    );
}

void ModifySpawnCommand::applySpawnData(Spawn* spawn, const SpawnData& data) const
{
    if (!spawn) {
        return;
    }

    spawn->setRadius(data.radius);
    spawn->setInterval(data.interval);
    spawn->setMaxCreatures(data.maxCreatures);
    spawn->setCreatureNames(data.creatureNames);
}
