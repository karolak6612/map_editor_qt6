#ifndef QT_ANIMATOR_H
#define QT_ANIMATOR_H

#include <QObject>
#include <QVector>
#include <QElapsedTimer> // For time-based updates
#include "Sprite.h" // For AnimationDirection enum

class Animator : public QObject {
    Q_OBJECT

public:
    struct FrameDuration {
        int min = 500; // Default duration, e.g., from ItemAnimationDuration
        int max = 500;

        FrameDuration(int minDuration = 500, int maxDuration = 500)
            : min(minDuration), max(maxDuration) {
            if (min > max) { // Ensure min <= max
                min = max;
            }
        }

        int getDuration() const {
            if (min == max) {
                return min;
            }
            // Qt's random generator is usually QRandomGenerator, but for simplicity here,
            // we'll just use min. A more complex random can be added if needed.
            // Or, the caller managing Animator can inject randomness if desired.
            return min; // Or: return QRandomGenerator::global()->bounded(min, max + 1);
        }
    };

    explicit Animator(QObject *parent = nullptr);
    Animator(int frameCount, int startFrame, int loopCount, bool isAsync, QObject *parent = nullptr);
    ~Animator() override;

    void setup(int frameCount, int startFrame, int loopCount, bool isAsync);

    void update(qint64 elapsedTimeMs); // Call this periodically to update the animation
    void reset();

    int getCurrentFrameIndex() const { return m_currentFrameIndex; }
    void setCurrentFrameIndex(int frameIndex); // For manual setting

    int getFrameCount() const { return m_frameCount; }
    bool isAsync() const { return m_isAsync; }
    int getLoopCount() const { return m_loopCount; } // -1 for ping-pong, 0 for infinite, >0 for N loops

    void setFrameDurations(const QVector<FrameDuration>& durations);
    FrameDuration getFrameDuration(int frameIndex) const;
    void setSingleFrameDuration(int frameIndex, int minDuration, int maxDuration);

    bool isAnimationComplete() const { return m_isComplete; }

private:
    void calculateSynchronousAnimation(qint64 totalElapsedTimeMs);
    int calculateNextFramePingPong();
    int calculateNextFrameLoop();
    int getEffectiveStartFrame() const;
    int getCurrentFrameDuration() const;


    int m_frameCount = 0;
    int m_startFrame = -1; // -1 indicates random start frame or default to 0
    int m_loopCount = 0;   // 0 for infinite loop, -1 for ping-pong, >0 for N loops
    bool m_isAsync = false; // If true, time is managed differently or by external calls

    QVector<FrameDuration> m_frameDurations;

    int m_currentFrameIndex = 0;
    int m_currentLoopIteration = 0;
    qint64 m_timeToNextFrame = 0; // Time remaining for the current frame in ms
    qint64 m_totalAnimationTimeNonAsync = 0; // Total duration for one full loop (non-async)

    AnimationDirection m_direction = ANIMATION_FORWARD;
    bool m_isComplete = false;

    // For async animations, last_time might be tracked differently or by the caller.
    // For non-async, QElapsedTimer can be used if Animator itself drives the updates.
    // However, the original seemed to rely on an external time source (g_gui.gfx.getElapsedTime()).
    // So, update(elapsedTimeMs) is preferred.
};

#endif // QT_ANIMATOR_H
