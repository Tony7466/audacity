/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "au3interactiontypes.h"
#include "trackedittypes.h"
#include "dom/clip.h"
#include "au3wrap/au3types.h"
#include "global/types/ret.h"
#include "global/iinteractive.h"
#include <cstddef>

namespace au::trackedit::utils {
//! To avoid risk of `getWaveTrack(myTracks, myClipKey.trackId)` ...
struct TrackIndex {
    const size_t value;
};

using ProgressCb = std::function<void (double)>;
using CancelCb = std::function<bool ()>;

au3::Au3WaveTrack* getWaveTrack(au3::Au3TrackList& tracks, const au3::Au3TrackId& trackId);
au3::Au3WaveTrack* getWaveTrack(au3::Au3TrackList& tracks, const ClipKey& clip);
au3::Au3WaveTrack* getWaveTrack(au3::Au3TrackList& tracks, TrackIndex index);
const au3::Au3WaveTrack* getWaveTrack(const au3::Au3TrackList& tracks, TrackIndex index);

size_t getTrackIndex(const au3::Au3TrackList& tracks, const au3::Au3Track& track);
size_t getTrackIndex(const au3::Au3TrackList& tracks, const trackedit::TrackId& id);

void exchangeTrack(au3::Au3TrackList& tracks, au3::Au3WaveTrack& oldOne, au3::Au3WaveTrack& newOne);

au3::Au3WaveTrack* toggleStereo(au3::Au3TrackList& tracks, au3::Au3WaveTrack& track);
au3::Au3WaveTrack* toggleStereo(au3::Au3TrackList& tracks, size_t trackIndex);

/*!
 * @pre (trackFactory != nullptr && projectRate > 0) || tracks.GetOwner() != nullptr
 */
au3::Au3WaveTrack* appendWaveTrack(au3::Au3TrackList& tracks, size_t nChannels, const au3::Au3WaveTrackFactory* trackFactory = nullptr,
                                   double projectRate = 0.);

NeedsDownmixing moveClipsUpOrDown(int offset, const au3::Au3TrackList& orig, au3::Au3TrackList& copy,
                                  const trackedit::ClipKeyList& selectedClips);

au::trackedit::TrackListInfo getTrackListInfo(const au3::Au3TrackList& tracks);

bool clipIdSetsAreEqual(const au3::Au3WaveTrack& track1, const au3::Au3WaveTrack& track2);

std::vector<const Clip*> clipSetDifference(const Clips& set1, const Clips& set2);

muse::Ret withProgress(muse::IInteractive& interactive, const std::string& title, const std::function<bool(ProgressCb, CancelCb)>& action);
}
