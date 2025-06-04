#ifndef PLACECREATURECOMMAND_H
#define PLACECREATURECOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QString>

// Forward declarations
class Map;
class Creature;

/**
 * @brief Command for placing creatures on the map
 * 
 * This command handles placing creatures on tiles in an undoable manner.
 * It creates the creature instance and manages its lifecycle.
 */
class PlaceCreatureCommand : public QUndoCommand
{
public:
    PlaceCreatureCommand(Map* map,
                        const QPointF& tilePos,
                        quint16 creatureId,
                        const QString& creatureName = QString(),
                        QUndoCommand* parent = nullptr);
    ~PlaceCreatureCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    quint16 creatureId_;
    QString creatureName_;
    Creature* creatureInstance_;
    bool tilePreviouslyExisted_;
};

#endif // PLACECREATURECOMMAND_H
