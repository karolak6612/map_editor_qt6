#include "ItemPropertyEditorBase.h"

ItemPropertyEditorBase::ItemPropertyEditorBase(QWidget *parent)
    : QWidget(parent), m_editingObject(nullptr) {
    // Constructor body can be empty for this abstract base, or setup common UI elements if any.
}

ItemPropertyEditorBase::~ItemPropertyEditorBase() {
    // Destructor body.
    // m_editingObject is not owned by the editor base by default, so not deleted here.
    // Subclasses might take ownership or handle it differently if needed.
}

bool ItemPropertyEditorBase::hasPendingChanges() const {
    // Default implementation: assume no pending changes.
    // Subclasses should override this if they track changes.
    return false;
}
