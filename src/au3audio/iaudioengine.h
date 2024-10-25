/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "async/notification.h"
#include "async/channel.h"

#include "global/modularity/imoduleinterface.h"

#include "au3wrap/au3types.h"

struct TransportSequences;
struct AudioIOStartStreamOptions;
namespace au::audio {
class IAudioEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioEngine);
public:
    virtual ~IAudioEngine() = default;

    virtual bool isBusy() const = 0;

    virtual int startStream(const TransportSequences& sequences, double startTime, double endTime, double mixerEndTime, // Time at which mixer stops producing, maybe > endTime
                            const AudioIOStartStreamOptions& options) = 0;

    virtual muse::async::Notification updateRequested() const = 0;
    virtual muse::async::Notification commitRequested() const = 0;
    virtual muse::async::Notification finished() const = 0;
    virtual muse::async::Channel<au3::Au3TrackId, au3::Au3ClipId> recordingClipChanged() const = 0;
};
}
