#ifndef LIVECURSOR_H
#define LIVECURSOR_H

#include <QColor>
#include <QMetaType>

// Forward declaration for Position
struct Position;

/**
 * @brief Represents a cursor position for collaborative editing
 * 
 * This structure holds information about a user's cursor position in the map editor,
 * including their unique ID, cursor color, and current position. This is used for
 * real-time collaborative editing to show where other users are working.
 */
struct LiveCursor {
    quint32 id;         ///< Unique client/cursor ID
    QColor color;       ///< Cursor display color
    Position pos;       ///< Current cursor position in the map

    /**
     * @brief Default constructor
     */
    LiveCursor();

    /**
     * @brief Constructor with parameters
     * @param id_ Client ID
     * @param color_ Cursor color
     * @param pos_ Cursor position
     */
    LiveCursor(quint32 id_, const QColor& color_, const Position& pos_);

    /**
     * @brief Copy constructor
     */
    LiveCursor(const LiveCursor& other) = default;

    /**
     * @brief Assignment operator
     */
    LiveCursor& operator=(const LiveCursor& other) = default;

    /**
     * @brief Equality comparison
     */
    bool operator==(const LiveCursor& other) const;

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const LiveCursor& other) const;
};

// Register the type with Qt's meta-object system for use in signals/slots
Q_DECLARE_METATYPE(LiveCursor)

#endif // LIVECURSOR_H
