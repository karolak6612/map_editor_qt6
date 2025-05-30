#include "Animator.h"
#include <QtGlobal> // For qrand() if needed, or use QRandomGenerator
#include <QRandomGenerator> // Preferred for Qt 5.10+

Animator::Animator(QObject *parent)
    : QObject(parent), m_frameCount(0), m_startFrame(-1), m_loopCount(0), m_isAsync(false),
      m_currentFrameIndex(0), m_currentLoopIteration(0), m_timeToNextFrame(0),
      m_totalAnimationTimeNonAsync(0), m_direction(AnimationDirection::Forward), m_isComplete(false) {
}

Animator::Animator(int frameCount, int startFrame, int loopCount, bool isAsync, QObject *parent)
    : QObject(parent) {
    setup(frameCount, startFrame, loopCount, isAsync);
}

Animator::~Animator() {
    // m_frameDurations will be cleared automatically as it's a QVector of objects
}

void Animator::setup(int frameCount, int startFrame, int loopCount, bool isAsync) {
    m_frameCount = qMax(0, frameCount); // Ensure frameCount is not negative
    m_startFrame = startFrame;
    m_loopCount = loopCount;
    m_isAsync = isAsync;

    if (m_frameCount > 0 && m_frameDurations.size() != m_frameCount) {
        m_frameDurations.resize(m_frameCount); // Default FrameDuration constructor will be used
    }
    
    m_totalAnimationTimeNonAsync = 0;
    if (!m_isAsync && m_frameCount > 0) {
        for (const auto& fd : m_frameDurations) {
            m_totalAnimationTimeNonAsync += fd.max; // Use max for worst-case total time
        }
    }
    reset();
}

void Animator::reset() {
    m_currentLoopIteration = 0;
    m_direction = AnimationDirection::Forward;
    m_isComplete = (m_frameCount == 0); // Animation is complete if there are no frames

    if (m_frameCount > 0) {
        m_currentFrameIndex = getEffectiveStartFrame();
        m_timeToNextFrame = getCurrentFrameDuration();
    } else {
        m_currentFrameIndex = 0;
        m_timeToNextFrame = 0;
    }
}

int Animator::getEffectiveStartFrame() const {
    if (m_startFrame >= 0 && m_startFrame < m_frameCount) {
        return m_startFrame;
    }
    if (m_frameCount > 0) {
        // Original wxwidgets version used uniform_random(0, frame_count - 1)
        // For Qt 5.10+, QRandomGenerator is preferred.
        return QRandomGenerator::global()->bounded(m_frameCount);
    }
    return 0;
}

int Animator::getCurrentFrameDuration() const {
    if (m_frameCount == 0 || m_currentFrameIndex < 0 || m_currentFrameIndex >= m_frameDurations.size()) {
        return 0;
    }
    return m_frameDurations[m_currentFrameIndex].getDuration();
}

void Animator::update(qint64 elapsedTimeMs) {
    if (m_isComplete || m_frameCount == 0 || elapsedTimeMs <= 0) {
        return;
    }

    if (m_isAsync) {
        m_timeToNextFrame -= elapsedTimeMs;
        while (m_timeToNextFrame <= 0) {
            if (m_loopCount < 0) { // Ping-pong
                m_currentFrameIndex = calculateNextFramePingPong();
            } else { // Normal loop or fixed count
                m_currentFrameIndex = calculateNextFrameLoop();
            }

            if (m_isComplete) { // Animation ended
                break;
            }
            m_timeToNextFrame += getCurrentFrameDuration(); // Add new frame's duration
        }
    } else { // Synchronous animation
        // For synchronous, the expectation is that totalElapsedTimeMs is the time since animation start.
        // This is a bit different from passing delta. The original used a global timer.
        // Let's assume elapsedTimeMs here is total time for simplicity in this model
        // or that the caller handles providing the correct total elapsed time for sync mode.
        // A more robust sync mode might need a QElapsedTimer internally if not provided.
        // For now, we'll make it behave like async if not driven by calculateSynchronousAnimation.
        // The original calculateSynchronous() was based on total_duration and global time.
        // This simplified update() assumes elapsedTimeMs is a delta.
        
        // Simplified: treat sync like async for delta updates for now.
        // Proper synchronous might involve calculateSynchronousAnimation(totalTime).
         m_timeToNextFrame -= elapsedTimeMs;
        while (m_timeToNextFrame <= 0) {
            if (m_loopCount < 0) { // Ping-pong
                m_currentFrameIndex = calculateNextFramePingPong();
            } else { // Normal loop or fixed count
                m_currentFrameIndex = calculateNextFrameLoop();
            }

            if (m_isComplete) { 
                break;
            }
            m_timeToNextFrame += getCurrentFrameDuration();
        }
    }
}

// This would be used if we had a global timer as in the original
void Animator::calculateSynchronousAnimation(qint64 totalElapsedTimeMs) {
    if (m_isAsync || m_frameCount == 0 || m_totalAnimationTimeNonAsync == 0) {
        return; // Not applicable
    }

    qint64 effectiveElapsedTime = totalElapsedTimeMs % m_totalAnimationTimeNonAsync;
    qint64 timeAccumulator = 0;
    int frame = 0;
    for (int i = 0; i < m_frameCount; ++i) {
        frame = i;
        qint64 duration = m_frameDurations[i].getDuration(); // Using getDuration for consistency
        if (effectiveElapsedTime >= timeAccumulator && effectiveElapsedTime < timeAccumulator + duration) {
            m_currentFrameIndex = i;
            m_timeToNextFrame = duration - (effectiveElapsedTime - timeAccumulator);
            break;
        }
        timeAccumulator += duration;
    }
    // If loop completes, it might mean totalElapsedTimeMs was exactly m_totalAnimationTimeNonAsync
    if (frame == m_frameCount -1 && effectiveElapsedTime >= timeAccumulator) {
         m_currentFrameIndex = frame;
         m_timeToNextFrame = 0; // End of last frame
    }
    m_isComplete = false; // Synchronous animations usually loop indefinitely by definition of totalElapsedTime % totalDuration
}


int Animator::calculateNextFramePingPong() {
    int nextFrame = m_currentFrameIndex;
    if (m_direction == AnimationDirection::Forward) {
        nextFrame++;
        if (nextFrame >= m_frameCount) {
            nextFrame = m_frameCount - 2; // Move to second to last
            m_direction = AnimationDirection::Backward;
            if (m_frameCount <= 1) { // For 1 frame, it just stays
                 nextFrame = 0;
                 m_isComplete = true; // Or loop if loopCount == 0
            }
        }
    } else { // Backward
        nextFrame--;
        if (nextFrame < 0) {
            nextFrame = 1; // Move to second frame
            m_direction = AnimationDirection::Forward;
            if (m_frameCount <= 1) { // For 1 frame
                nextFrame = 0;
                m_isComplete = true;
            }
        }
    }
    if (m_frameCount <= 1) nextFrame = 0; // Ensure valid index for single frame
    return nextFrame;
}

int Animator::calculateNextFrameLoop() {
    int nextFrame = m_currentFrameIndex + 1;
    if (nextFrame >= m_frameCount) { // Reached end of sequence
        if (m_loopCount == 0) { // Infinite loop
            nextFrame = 0; // Wrap around
            m_currentLoopIteration = 0; // Reset loop iteration for clarity, though not strictly needed for infinite
        } else {
            m_currentLoopIteration++;
            if (m_currentLoopIteration >= m_loopCount) {
                m_isComplete = true;
                return m_currentFrameIndex; // Stay on last frame
            } else {
                nextFrame = 0; // Wrap around for next loop
            }
        }
    }
    return nextFrame;
}


void Animator::setCurrentFrameIndex(int frameIndex) {
    if (frameIndex >= 0 && frameIndex < m_frameCount) {
        m_currentFrameIndex = frameIndex;
        m_timeToNextFrame = getCurrentFrameDuration();
        m_isComplete = false; // Manually setting frame usually implies animation is not "naturally" complete
    }
}

void Animator::setFrameDurations(const QVector<FrameDuration>& durations) {
    if (durations.size() == m_frameCount) {
        m_frameDurations = durations;
        // Recalculate total animation time if needed
        m_totalAnimationTimeNonAsync = 0;
        if (!m_isAsync && m_frameCount > 0) {
            for (const auto& fd : m_frameDurations) {
                m_totalAnimationTimeNonAsync += fd.max;
            }
        }
        reset(); // Reset to apply new durations
    }
}

Animator::FrameDuration Animator::getFrameDuration(int frameIndex) const {
    if (frameIndex >= 0 && frameIndex < m_frameDurations.size()) {
        return m_frameDurations[frameIndex];
    }
    return FrameDuration(); // Return default if out of bounds
}

void Animator::setSingleFrameDuration(int frameIndex, int minDuration, int maxDuration) {
    if (frameIndex >= 0 && frameIndex < m_frameCount) {
        if (frameIndex >= m_frameDurations.size()) {
             m_frameDurations.resize(m_frameCount); // Should have been sized in setup
        }
        m_frameDurations[frameIndex].min = minDuration;
        m_frameDurations[frameIndex].max = maxDuration;
        if (m_frameDurations[frameIndex].min > m_frameDurations[frameIndex].max) {
           m_frameDurations[frameIndex].min = m_frameDurations[frameIndex].max;
        }
        // Recalculate total animation time
        m_totalAnimationTimeNonAsync = 0;
        if (!m_isAsync && m_frameCount > 0) {
            for (const auto& fd : m_frameDurations) {
                m_totalAnimationTimeNonAsync += fd.max;
            }
        }
        // No reset() here, to allow dynamic changes without restarting animation,
        // but this might affect m_timeToNextFrame if current frame's duration changed.
        // For safety, one might want to update m_timeToNextFrame if currentFrameIndex == frameIndex.
        if (m_currentFrameIndex == frameIndex) {
            m_timeToNextFrame = getCurrentFrameDuration(); // Re-fetch potentially new duration
        }
    }
}
