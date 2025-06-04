#ifndef MODIFYSPAWNCOMMAND_H
#define MODIFYSPAWNCOMMAND_H

#include <QUndoCommand>
#include <QStringList>

class Spawn;

/**
 * @brief Command for modifying spawn properties with undo/redo support
 * 
 * This command allows dialogs to modify spawn properties in an undoable way.
 * It stores the old and new spawn data and can restore them during undo operations.
 * 
 * Task 019: Undo integration for spawn property modifications
 */
class ModifySpawnCommand : public QUndoCommand
{
public:
    /**
     * @brief Spawn data structure for storing spawn properties
     */
    struct SpawnData {
        int radius = 3;
        int interval = 60;
        int maxCreatures = 1;
        QStringList creatureNames;
        
        SpawnData() = default;
        SpawnData(int r, int i, int m, const QStringList& names)
            : radius(r), interval(i), maxCreatures(m), creatureNames(names) {}
    };

    /**
     * @brief Construct a new ModifySpawnCommand
     * @param spawn The spawn to modify
     * @param newData The new spawn data
     * @param parent Parent command for command grouping
     */
    ModifySpawnCommand(Spawn* spawn, const SpawnData& newData, QUndoCommand* parent = nullptr);

    /**
     * @brief Apply the spawn modification
     */
    void redo() override;

    /**
     * @brief Revert the spawn modification
     */
    void undo() override;

private:
    Spawn* spawn_;              ///< The spawn being modified
    SpawnData oldData_;         ///< The original spawn data
    SpawnData newData_;         ///< The new spawn data
    bool firstRedo_;            ///< Flag to capture old data only once
    
    /**
     * @brief Capture current spawn data
     * @param spawn The spawn to capture data from
     * @return SpawnData structure with current values
     */
    SpawnData captureSpawnData(Spawn* spawn) const;
    
    /**
     * @brief Apply spawn data to a spawn object
     * @param spawn The spawn to apply data to
     * @param data The data to apply
     */
    void applySpawnData(Spawn* spawn, const SpawnData& data) const;
};

#endif // MODIFYSPAWNCOMMAND_H
