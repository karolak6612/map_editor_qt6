#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H

#include <QObject>
#include <QSet>
#include <QCoreApplication>
#include <QVariant>
#include "ClipboardData.h"
#include "Map.h"
#include "PasteCommand.h"

// Forward declarations
class QUndoStack;

/**
 * ClipboardManager - Global clipboard management for copy/cut/paste operations
 * This class provides a centralized interface for clipboard operations
 * and integrates with the undo/redo system
 */
class ClipboardManager : public QObject {
    Q_OBJECT

public:
    explicit ClipboardManager(QObject* parent = nullptr);
    ~ClipboardManager();

    // Singleton access
    static ClipboardManager* instance();
    static void setInstance(ClipboardManager* manager);

    // Clipboard operations
    bool copySelection(const QSet<MapPos>& selection, const Map& map);
    bool cutSelection(const QSet<MapPos>& selection, Map& map, QUndoStack* undoStack);
    bool paste(Map& map, const MapPos& targetPosition, QUndoStack* undoStack, 
               PasteCommand::PasteMode mode = PasteCommand::REPLACE_MODE);
    
    // System clipboard integration
    bool copyToSystemClipboard();
    bool pasteFromSystemClipboard();
    
    // Clipboard state
    bool hasClipboardData() const;
    bool isCutOperation() const;
    void clearClipboard();
    
    // Clipboard data access
    const ClipboardData& getClipboardData() const;
    int getClipboardTileCount() const;
    int getClipboardItemCount() const;
    
    // Settings
    void setAutoSystemClipboard(bool enabled);
    bool isAutoSystemClipboardEnabled() const;

signals:
    void clipboardChanged();
    void clipboardCleared();
    void operationCompleted(const QString& operation, int tileCount);
    void operationFailed(const QString& operation, const QString& error);

private:
    static ClipboardManager* instance_;
    
    ClipboardData clipboardData_;
    bool autoSystemClipboard_;
    
    // Helper methods
    void notifyClipboardChanged();
    QString getOperationStatusMessage(const QString& operation, int tileCount) const;
};

#endif // CLIPBOARDMANAGER_H
