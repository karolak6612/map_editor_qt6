#ifndef SETITEMATTRIBUTECOMMAND_H
#define SETITEMATTRIBUTECOMMAND_H

#include <QUndoCommand>
#include <QVariant>
#include <QString>

class Item;

/**
 * @brief Command for setting item attributes with undo/redo support
 * 
 * This command allows property editors to modify item attributes in an undoable way.
 * It stores the old and new values and can restore them during undo operations.
 * 
 * Task 019: Generic command for property editor undo integration
 */
class SetItemAttributeCommand : public QUndoCommand
{
public:
    /**
     * @brief Construct a new SetItemAttributeCommand
     * @param item The item to modify
     * @param attributeKey The attribute key to set
     * @param newValue The new value to set
     * @param parent Parent command for command grouping
     */
    SetItemAttributeCommand(Item* item, const QString& attributeKey, 
                           const QVariant& newValue, QUndoCommand* parent = nullptr);

    /**
     * @brief Apply the attribute change
     */
    void redo() override;

    /**
     * @brief Revert the attribute change
     */
    void undo() override;

    /**
     * @brief Get the command ID for merging
     * @return Command ID
     */
    int id() const override;

    /**
     * @brief Check if this command can be merged with another
     * @param other The other command to check
     * @return True if commands can be merged
     */
    bool mergeWith(const QUndoCommand* other) override;

private:
    Item* item_;                    ///< The item being modified
    QString attributeKey_;          ///< The attribute key
    QVariant oldValue_;            ///< The original value
    QVariant newValue_;            ///< The new value
    bool firstRedo_;               ///< Flag to capture old value only once
    
    static const int COMMAND_ID = 1001; ///< Command ID for merging
};

#endif // SETITEMATTRIBUTECOMMAND_H
