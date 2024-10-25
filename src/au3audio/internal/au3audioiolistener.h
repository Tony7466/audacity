/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "global/timer.h"
#include "global/async/asyncable.h"
#include "global/async/notification.h"
#include "global/async/channel.h"

#include "libraries/lib-audio-io/AudioIOListener.h"

#include "au3wrap/au3types.h"

namespace au::audio {
class Au3AudioIOListener : public AudioIOListener, public muse::async::Asyncable
{
public:
    Au3AudioIOListener()
        : m_timer(std::chrono::milliseconds(100))
    {
        m_timer.onTimeout(this, [this]() {
            m_updateRequested.notify();
        });
    }

    void OnAudioIORate(int /*rate*/) override { }
    void OnAudioIOStartRecording() override { m_timer.start(); }
    void OnAudioIONewBlocks() override { }
    void OnCommitRecording() override { m_commitRequested.notify(); }
    void OnSoundActivationThreshold() override { }
    void OnAudioIOStopRecording() override
    {
        m_finished.notify();
        m_timer.stop();
    }

    muse::async::Notification updateRequested() const { return m_updateRequested; }
    muse::async::Notification commitRequested() const { return m_commitRequested; }
    muse::async::Notification finished() const { return m_finished; }

    muse::async::Channel<au3::Au3TrackId, au3::Au3ClipId> recordingClipChanged() const { return m_recordingClipChanged; }

private:
    muse::async::Notification m_updateRequested;
    muse::async::Notification m_commitRequested;
    muse::async::Notification m_finished;

    muse::async::Channel<au3::Au3TrackId, au3::Au3ClipId> m_recordingClipChanged;

    muse::Timer m_timer;
};
}
