#ifndef REMOVECREATURECOMMAND_H
#define REMOVECREATURECOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QString>

// Forward declarations
class Map;
class Creature;

/**
 * @brief Command for removing creatures from the map
 * 
 * This command handles removing creatures from tiles in an undoable manner.
 * It stores the creature data to allow restoration.
 */
class RemoveCreatureCommand : public QUndoCommand
{
public:
    RemoveCreatureCommand(Map* map,
                         const QPointF& tilePos,
                         Creature* creature = nullptr,
                         QUndoCommand* parent = nullptr);
    ~RemoveCreatureCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    Creature* creatureInstance_;
    quint16 creatureId_;
    QString creatureName_;
    bool creatureWasRemoved_;
    bool tileBecameEmpty_;
};

#endif // REMOVECREATURECOMMAND_H
