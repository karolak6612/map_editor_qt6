#include "SetItemAttributeCommand.h"
#include "Item.h"
#include <QDebug>

SetItemAttributeCommand::SetItemAttributeCommand(Item* item, const QString& attributeKey, 
                                               const QVariant& newValue, QUndoCommand* parent)
    : QUndoCommand(parent),
      item_(item),
      attributeKey_(attributeKey),
      newValue_(newValue),
      firstRedo_(true)
{
    if (!item_) {
        qWarning() << "SetItemAttributeCommand: Item pointer is null";
        return;
    }

    // Generate command text
    setText(QObject::tr("Set %1 to %2").arg(attributeKey_).arg(newValue_.toString()));
}

void SetItemAttributeCommand::redo()
{
    if (!item_) {
        qWarning() << "SetItemAttributeCommand::redo(): Item pointer is null";
        return;
    }

    // Task 019: Capture old value only on first redo to avoid overwriting original state
    if (firstRedo_) {
        oldValue_ = item_->getAttribute(attributeKey_);
        firstRedo_ = false;
        qDebug() << "SetItemAttributeCommand: Captured old value" << oldValue_ << "for attribute" << attributeKey_;
    }

    // Apply the new value
    item_->setAttribute(attributeKey_, newValue_);
    qDebug() << "SetItemAttributeCommand: Set attribute" << attributeKey_ << "to" << newValue_;
}

void SetItemAttributeCommand::undo()
{
    if (!item_) {
        qWarning() << "SetItemAttributeCommand::undo(): Item pointer is null";
        return;
    }

    // Restore the old value
    item_->setAttribute(attributeKey_, oldValue_);
    qDebug() << "SetItemAttributeCommand: Restored attribute" << attributeKey_ << "to" << oldValue_;
}

int SetItemAttributeCommand::id() const
{
    return COMMAND_ID;
}

bool SetItemAttributeCommand::mergeWith(const QUndoCommand* other)
{
    // Task 019: Allow merging of consecutive attribute changes to the same item/attribute
    const SetItemAttributeCommand* otherCmd = static_cast<const SetItemAttributeCommand*>(other);
    
    if (!otherCmd || otherCmd->item_ != item_ || otherCmd->attributeKey_ != attributeKey_) {
        return false;
    }

    // Merge by updating our new value to the other command's new value
    // Keep our old value (the original state)
    newValue_ = otherCmd->newValue_;
    setText(QObject::tr("Set %1 to %2").arg(attributeKey_).arg(newValue_.toString()));
    
    qDebug() << "SetItemAttributeCommand: Merged commands for attribute" << attributeKey_ 
             << "new value:" << newValue_;
    
    return true;
}
