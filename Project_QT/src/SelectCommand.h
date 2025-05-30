#ifndef SELECT_COMMAND_H
#define SELECT_COMMAND_H

#include "Map.h" // For MapPos
#include <QSet>

// Forward declare Map if the command needs to interact with it.
class Map; 

class SelectCommand {
public:
    SelectCommand(Map* map, const QSet<MapPos>& positions);
    ~SelectCommand();

    void execute();
    void undo();

private:
    Map* map_; // The map context
    QSet<MapPos> positionsToSelect_;
    QSet<MapPos> previouslySelectedPositions_; // For undo
};

#endif // SELECT_COMMAND_H
