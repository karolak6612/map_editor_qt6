#ifndef DESELECT_COMMAND_H
#define DESELECT_COMMAND_H

#include "Map.h" // For MapPos
#include <QSet>

class Map;

class DeselectCommand {
public:
    DeselectCommand(Map* map, const QSet<MapPos>& positions); // Or DeselectCommand(Map* map) to deselect all
    ~DeselectCommand();

    void execute();
    void undo();

private:
    Map* map_;
    QSet<MapPos> positionsToDeselect_;
    QSet<MapPos> originallySelectedPositions_; // For undo
};

#endif // DESELECT_COMMAND_H
