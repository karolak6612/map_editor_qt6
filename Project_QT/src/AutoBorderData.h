#ifndef AUTOBORDERDATA_H
#define AUTOBORDERDATA_H

#include "BrushCommon.h" // For BorderEdgeType
#include <QMap>
#include <QtGlobal>     // For quint16, quint32

class AutoBorderData {
public:
    explicit AutoBorderData(quint32 definitionId = 0);
    ~AutoBorderData();

    // Getters
    quint32 definitionId() const;
    quint16 borderGroupId() const;
    bool definesGroundEquivalent() const;
    quint16 getEdgeItemId(BorderEdgeType edge) const; // Returns 0 if edge type not found or no item set

    // Setters
    void setDefinitionId(quint32 id); // Should ideally only be set at creation from XML ID
    void setBorderGroupId(quint16 groupId);
    void setDefinesGroundEquivalent(bool defines);
    void setEdgeItemId(BorderEdgeType edge, quint16 itemId); // itemId = 0 means clear for that edge

    void clearEdgeItemIds(); // Clears all stored edge item IDs

private:
    quint32 definitionId_;      // Original ID of this border definition (e.g., from XML)
    quint16 borderGroupId_ = 0; // Group this border belongs to, initialized
    bool definesGroundEquivalent_ = false; // True if this border also implies a ground type, initialized

    // Stores item IDs for each of the 13 edge types.
    // An entry missing for an edge means no specific item for that edge.
    // An entry with itemId 0 can also mean "no item" for that edge.
    QMap<BorderEdgeType, quint16> edgeItemIds_;
};

#endif // AUTOBORDERDATA_H
