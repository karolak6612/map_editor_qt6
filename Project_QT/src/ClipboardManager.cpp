#include "ClipboardManager.h"
#include "PasteCommand.h"
#include <QUndoStack>
#include <QDebug>

// Static instance
ClipboardManager* ClipboardManager::instance_ = nullptr;

ClipboardManager::ClipboardManager(QObject* parent)
    : QObject(parent), autoSystemClipboard_(true) {
    // Initialize clipboard manager
}

ClipboardManager::~ClipboardManager() {
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

// Singleton access
ClipboardManager* ClipboardManager::instance() {
    return instance_;
}

void ClipboardManager::setInstance(ClipboardManager* manager) {
    instance_ = manager;
}

// Clipboard operations
bool ClipboardManager::copySelection(const QSet<MapPos>& selection, const Map& map) {
    if (selection.isEmpty()) {
        emit operationFailed("Copy", "No tiles selected");
        return false;
    }
    
    try {
        clipboardData_.populateFromSelectionCopy(selection, map);
        
        if (autoSystemClipboard_) {
            copyToSystemClipboard();
        }
        
        notifyClipboardChanged();
        
        QString message = getOperationStatusMessage("Copy", clipboardData_.getTileCount());
        emit operationCompleted("Copy", clipboardData_.getTileCount());
        qDebug() << message;
        
        return true;
    } catch (const std::exception& e) {
        QString error = QString("Failed to copy selection: %1").arg(e.what());
        emit operationFailed("Copy", error);
        qWarning() << error;
        return false;
    }
}

bool ClipboardManager::cutSelection(const QSet<MapPos>& selection, Map& map, QUndoStack* undoStack) {
    if (selection.isEmpty()) {
        emit operationFailed("Cut", "No tiles selected");
        return false;
    }
    
    if (!undoStack) {
        emit operationFailed("Cut", "No undo stack available");
        return false;
    }
    
    try {
        // Create and execute cut command
        CutCommand* cutCommand = new CutCommand(&map, selection, clipboardData_);
        undoStack->push(cutCommand);
        
        if (autoSystemClipboard_) {
            copyToSystemClipboard();
        }
        
        notifyClipboardChanged();
        
        QString message = getOperationStatusMessage("Cut", clipboardData_.getTileCount());
        emit operationCompleted("Cut", clipboardData_.getTileCount());
        qDebug() << message;
        
        return true;
    } catch (const std::exception& e) {
        QString error = QString("Failed to cut selection: %1").arg(e.what());
        emit operationFailed("Cut", error);
        qWarning() << error;
        return false;
    }
}

bool ClipboardManager::paste(Map& map, const MapPos& targetPosition, QUndoStack* undoStack, 
                             PasteCommand::PasteMode mode) {
    if (clipboardData_.isEmpty()) {
        emit operationFailed("Paste", "Clipboard is empty");
        return false;
    }
    
    if (!undoStack) {
        emit operationFailed("Paste", "No undo stack available");
        return false;
    }
    
    try {
        // Create and execute paste command
        PasteCommand* pasteCommand = new PasteCommand(&map, clipboardData_, targetPosition, mode);
        undoStack->push(pasteCommand);
        
        QString modeStr = (mode == PasteCommand::MERGE_MODE) ? "Merge" : "Replace";
        QString message = getOperationStatusMessage(QString("Paste (%1)").arg(modeStr), clipboardData_.getTileCount());
        emit operationCompleted("Paste", clipboardData_.getTileCount());
        qDebug() << message;
        
        return true;
    } catch (const std::exception& e) {
        QString error = QString("Failed to paste: %1").arg(e.what());
        emit operationFailed("Paste", error);
        qWarning() << error;
        return false;
    }
}

// System clipboard integration
bool ClipboardManager::copyToSystemClipboard() {
    try {
        clipboardData_.copyToSystemClipboard();
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Failed to copy to system clipboard:" << e.what();
        return false;
    }
}

bool ClipboardManager::pasteFromSystemClipboard() {
    try {
        bool success = clipboardData_.pasteFromSystemClipboard();
        if (success) {
            notifyClipboardChanged();
        }
        return success;
    } catch (const std::exception& e) {
        qWarning() << "Failed to paste from system clipboard:" << e.what();
        return false;
    }
}

// Clipboard state
bool ClipboardManager::hasClipboardData() const {
    return !clipboardData_.isEmpty();
}

bool ClipboardManager::isCutOperation() const {
    return clipboardData_.isCutOperation();
}

void ClipboardManager::clearClipboard() {
    clipboardData_.clear();
    emit clipboardCleared();
    qDebug() << "Clipboard cleared";
}

// Clipboard data access
const ClipboardData& ClipboardManager::getClipboardData() const {
    return clipboardData_;
}

int ClipboardManager::getClipboardTileCount() const {
    return clipboardData_.getTileCount();
}

int ClipboardManager::getClipboardItemCount() const {
    return clipboardData_.getItemCount();
}

// Settings
void ClipboardManager::setAutoSystemClipboard(bool enabled) {
    autoSystemClipboard_ = enabled;
}

bool ClipboardManager::isAutoSystemClipboardEnabled() const {
    return autoSystemClipboard_;
}

// Helper methods
void ClipboardManager::notifyClipboardChanged() {
    emit clipboardChanged();
}

QString ClipboardManager::getOperationStatusMessage(const QString& operation, int tileCount) const {
    QString itemInfo = QString("%1 tiles").arg(tileCount);
    int itemCount = clipboardData_.getItemCount();
    if (itemCount > 0) {
        itemInfo += QString(", %1 items").arg(itemCount);
    }
    
    return QString("%1 completed: %2").arg(operation, itemInfo);
}

#include "ClipboardManager.moc"
