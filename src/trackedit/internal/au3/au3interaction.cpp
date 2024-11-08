#include "au3interaction.h"

#include <algorithm>

#include "ProjectRate.h"
#include "TempoChange.h"
#include "QualitySettings.h"

#include "global/types/number.h"
#include "global/concurrency/concurrent.h"

#include "libraries/lib-project/Project.h"
#include "libraries/lib-wave-track/WaveTrack.h"
#include "libraries/lib-track/Track.h"
#include "libraries/lib-wave-track/WaveClip.h"
#include "libraries/lib-wave-track/TimeStretching.h"

#include "au3wrap/internal/domaccessor.h"
#include "au3wrap/internal/domconverter.h"
#include "au3wrap/internal/wxtypes_convert.h"
#include "au3wrap/au3types.h"

#include "trackedit/dom/track.h"

#include "defer.h"
#include "log.h"
#include "trackediterrors.h"
#include "translation.h"

using namespace au::trackedit;
using namespace au::au3;

Au3Project& Au3Interaction::projectRef() const
{
    Au3Project* project = reinterpret_cast<Au3Project*>(globalContext()->currentProject()->au3ProjectPtr());
    return *project;
}

TrackIdList Au3Interaction::pasteIntoNewTracks(const std::vector<TrackData>& tracksData)
{
    auto& project = projectRef();
    auto& tracks = Au3TrackList::Get(project);
    auto prj = globalContext()->currentTrackeditProject();
    secs_t selectedStartTime = globalContext()->playbackState()->playbackPosition();

    TrackIdList tracksIdsPastedInto;

    Au3Track* pFirstNewTrack = NULL;
    for (auto data : tracksData) {
        auto pNewTrack = createNewTrackAndPaste(data.track, tracks, selectedStartTime);
        if (!pFirstNewTrack) {
            pFirstNewTrack = pNewTrack.get();
        }

        auto newTrack = DomConverter::track(pNewTrack.get());
        prj->notifyAboutTrackAdded(newTrack);
        for (const auto& clip : prj->clipList(pNewTrack->GetId())) {
            prj->notifyAboutClipAdded(clip);
        }

        tracksIdsPastedInto.push_back(newTrack.id);
    }

    pushProjectHistoryPasteState();

    return tracksIdsPastedInto;
}

Au3Track::Holder Au3Interaction::createNewTrackAndPaste(std::shared_ptr<Au3Track> track, Au3TrackList& list, secs_t begin)
{
    auto& trackFactory = WaveTrackFactory::Get(projectRef());
    auto& pSampleBlockFactory = trackFactory.GetSampleBlockFactory();

    //! NOTE: using dynamic_cast directly because when using UndoManager
    //! project may not contain track with given ID anymore
    Au3WaveTrack* waveTrack = dynamic_cast<Au3WaveTrack*>(track.get());
    IF_ASSERT_FAILED(waveTrack) {
        return nullptr;
    }
    auto pFirstTrack = waveTrack->EmptyCopy(pSampleBlockFactory);
    list.Add(pFirstTrack->SharedPointer());
    pFirstTrack->Paste(begin, *track);
    return pFirstTrack->SharedPointer();
}

TrackIdList Au3Interaction::determineDestinationTracksIds(const std::vector<Track>& tracks,
                                                          TrackId destinationTrackId, size_t tracksNum) const
{
    TrackIdList tracksIds;
    bool addingEnabled = false;

    for (const auto& track : tracks) {
        if (track.id == destinationTrackId) {
            addingEnabled = true;
        }
        if (addingEnabled) {
            tracksIds.push_back(track.id);
            if (tracksIds.size() == tracksNum) {
                break;
            }
        }
    }

    return tracksIds;
}

muse::Ret Au3Interaction::canPasteClips(const TrackIdList& dstTracksIds, const std::vector<TrackData>& clipsToPaste, secs_t begin) const
{
    IF_ASSERT_FAILED(dstTracksIds.size() <= clipsToPaste.size()) {
        return make_ret(trackedit::Err::NotEnoughDataInClipboard);
    }

    for (size_t i = 0; i < dstTracksIds.size(); ++i) {
        Au3WaveTrack* dstWaveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(dstTracksIds[i]));
        IF_ASSERT_FAILED(dstWaveTrack) {
            return make_ret(trackedit::Err::WaveTrackNotFound);
        }

        //! NOTE need to snap begin just like Paste() function do
        secs_t snappedBegin = dstWaveTrack->SnapToSample(begin);
        secs_t insertDuration = clipsToPaste.at(i).track.get()->GetEndTime();

        // throws incosistency exception if project tempo is not set, see:
        // au4/src/au3wrap/internal/au3project.cpp: 92
        // Paste func throws exception if there's not enough space to paste in (exception handler calls BasicUI logic)
        if (!dstWaveTrack->IsEmpty(snappedBegin, snappedBegin + insertDuration)) {
            LOGD() << "Not enough space to paste clip into";
            return make_ret(trackedit::Err::NotEnoughSpaceForPaste);
        }

        if (dstWaveTrack->NChannels() == 1 && clipboard()->trackData(i).track.get()->NChannels() == 2) {
            return make_ret(trackedit::Err::StereoClipIntoMonoTrack);
        }
    }

    return muse::make_ok();
}

Au3Interaction::Au3Interaction()
{
    m_progress = std::make_shared<muse::Progress>();
}

muse::Ret Au3Interaction::makeRoomForClip(const ClipKey& clipKey)
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();

    WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return make_ret(trackedit::Err::WaveTrackNotFound);
    }

    std::shared_ptr<WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return make_ret(trackedit::Err::ClipNotFound);
    }

    std::list<std::shared_ptr<WaveClip> > clips = DomAccessor::waveClipsAsList(waveTrack);

    clips.sort([](const std::shared_ptr<WaveClip>& c1, const std::shared_ptr<WaveClip>& c2) {
        return c1->GetPlayStartTime() < c2->GetPlayStartTime();
    });

    for (const auto& otherClip : clips) {
        if (clip == otherClip) {
            //! NOTE do not modify the clip the action was taken on
            continue;
        }

        trimOrDeleteOverlapping(waveTrack, clip->GetPlayStartTime(), clip->GetPlayEndTime(), otherClip);
    }

    return muse::make_ret(muse::Ret::Code::Ok);
}

muse::Ret Au3Interaction::makeRoomForDataOnTracks(const std::vector<TrackId>& tracksIds, const std::vector<TrackData>& trackData,
                                                  secs_t begin)
{
    IF_ASSERT_FAILED(tracksIds.size() <= trackData.size()) {
        return make_ret(trackedit::Err::NotEnoughDataInClipboard);
    }

    for (size_t i = 0; i < tracksIds.size(); ++i) {
        WaveTrack* dstWaveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(tracksIds.at(i)));
        IF_ASSERT_FAILED(dstWaveTrack) {
            return make_ret(trackedit::Err::WaveTrackNotFound);
        }

        //! NOTE need to snap begin just like Paste() function do
        secs_t snappedBegin = dstWaveTrack->SnapToSample(begin);
        secs_t insertDuration = trackData.at(i).track.get()->GetEndTime();

        auto ok = makeRoomForDataOnTrack(tracksIds.at(i), snappedBegin, snappedBegin + insertDuration);
        if (!ok) {
            return make_ret(trackedit::Err::FailedToMakeRoomForClip);
        }
    }

    return muse::make_ok();
}

muse::Ret Au3Interaction::makeRoomForDataOnTrack(const TrackId trackId, secs_t begin, secs_t end)
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();

    WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return make_ret(trackedit::Err::WaveTrackNotFound);
    }

    std::list<std::shared_ptr<WaveClip> > clips = DomAccessor::waveClipsAsList(waveTrack);

    clips.sort([](const std::shared_ptr<WaveClip>& c1, const std::shared_ptr<WaveClip>& c2) {
        return c1->GetPlayStartTime() < c2->GetPlayStartTime();
    });

    for (const auto& otherClip : clips) {
        trimOrDeleteOverlapping(waveTrack, begin, end, otherClip);
    }

    return muse::make_ret(muse::Ret::Code::Ok);
}

void Au3Interaction::trimOrDeleteOverlapping(WaveTrack* waveTrack, secs_t begin, secs_t end, std::shared_ptr<WaveClip> otherClip)
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();

    if (muse::RealIsEqualOrLess(begin, otherClip->GetPlayStartTime())
        && muse::RealIsEqualOrMore(end, otherClip->GetPlayEndTime())) {
        waveTrack->RemoveInterval(otherClip);
        prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

        return;
    }

    //! NOTE check if clip boundaries are within other clip
    if (!muse::RealIsEqualOrLess(begin, otherClip->GetPlayStartTime())
        && !muse::RealIsEqualOrMore(end, otherClip->GetPlayEndTime())) {
        secs_t otherClipStartTime = otherClip->GetPlayStartTime();
        secs_t otherClipEndTime = otherClip->GetPlayEndTime();

        auto leftClip = waveTrack->CopyClip(*otherClip, true);
        waveTrack->InsertInterval(std::move(leftClip), false);
        prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

        secs_t rightClipOverlap = (end - otherClip->GetPlayStartTime());
        otherClip->TrimLeft(rightClipOverlap);
        prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, otherClip.get()));

        leftClip->SetPlayStartTime(otherClipStartTime);
        secs_t leftClipOverlap = (otherClipEndTime - begin);
        leftClip->TrimRight(leftClipOverlap);
        prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, leftClip.get()));

        prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

        return;
    }

    //! NOTE check if clip hovers left side of the clip
    if (muse::RealIsEqualOrLess(begin, otherClip->GetPlayStartTime())
        && !muse::RealIsEqualOrMore(end, otherClip->GetPlayEndTime())
        && muse::RealIsEqualOrMore(end, otherClip->GetPlayStartTime())) {
        secs_t overlap = (end - otherClip->GetPlayStartTime());
        otherClip->TrimLeft(overlap);
        prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, otherClip.get()));

        return;
    }

    //! NOTE check if clip hovers right side of the clip
    if (!muse::RealIsEqualOrLess(begin, otherClip->GetPlayStartTime())
        && muse::RealIsEqualOrLess(begin, otherClip->GetPlayEndTime())
        && muse::RealIsEqualOrMore(end, otherClip->GetPlayEndTime())) {
        secs_t overlap = (otherClip->GetPlayEndTime() - begin);
        otherClip->TrimRight(overlap);
        prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, otherClip.get()));

        return;
    }
}

muse::secs_t Au3Interaction::clipStartTime(const trackedit::ClipKey& clipKey) const
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return -1.0;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return -1.0;
    }

    return clip->Start();
}

bool Au3Interaction::changeClipStartTime(const trackedit::ClipKey& clipKey, secs_t newStartTime, bool completed)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    if (completed) {
        auto ok = makeRoomForClip(clipKey);
        if (!ok) {
            return false;
        }
    }

    if (!muse::RealIsEqualOrMore(newStartTime, 0.0)) {
        newStartTime = 0.0;
    }

    //! TODO Not sure what this method needs to be called to change the position, will need to clarify
    clip->SetPlayStartTime(newStartTime);
    // LOGD() << "changed PlayStartTime of track: " << clipKey.trackId
    //        << " clip: " << clipKey.index
    //        << " new PlayStartTime: " << newStartTime;

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    m_clipStartTimeChanged.send(clipKey, newStartTime, completed);

    if (completed) {
        //! TODO AU4: later when having keyboard arrow shortcut for moving clips
        //! make use of UndoPush::CONSOLIDATE arg in UndoManager
        projectHistory()->pushHistoryState("Clip moved", "Move clip");
    }

    return true;
}

muse::async::Channel<au::trackedit::ClipKey, secs_t /*newStartTime*/, bool /*completed*/>
Au3Interaction::clipStartTimeChanged() const
{
    return m_clipStartTimeChanged;
}

bool Au3Interaction::trimTracksData(const std::vector<TrackId>& tracksIds, secs_t begin, secs_t end)
{
    for (TrackId trackId : tracksIds) {
        Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
        IF_ASSERT_FAILED(waveTrack) {
            continue;
        }

        waveTrack->Trim(begin, end);

        trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
        prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));
    }

    pushProjectHistoryTracksTrimState(begin, end);

    return true;
}

bool Au3Interaction::silenceTracksData(const std::vector<trackedit::TrackId>& tracksIds, secs_t begin, secs_t end)
{
    for (TrackId trackId : tracksIds) {
        Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
        IF_ASSERT_FAILED(waveTrack) {
            return false;
        }

        waveTrack->Silence(begin, end, {});

        trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
        prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));
    }

    pushProjectHistoryTrackSilenceState(begin, end);

    return true;
}

bool Au3Interaction::changeTrackTitle(const TrackId trackId, const muse::String& title)
{
    Au3Track* track = DomAccessor::findTrack(projectRef(), Au3TrackId(trackId));
    IF_ASSERT_FAILED(track) {
        return false;
    }

    track->SetName(wxFromString(title));
    LOGD() << "changed name of track: " << trackId;

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackChanged(DomConverter::track(track));

    return true;
}

bool Au3Interaction::changeClipTitle(const trackedit::ClipKey& clipKey, const muse::String& newTitle)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    clip->SetName(wxFromString(newTitle));
    LOGD() << "changed name of clip: " << clipKey.clipId << ", track: " << clipKey.trackId;

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    return true;
}

bool Au3Interaction::changeClipPitch(const ClipKey& clipKey, int pitch)
{
    WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    clip->SetCentShift(pitch);
    LOGD() << "changed pitch of clip: " << clipKey.clipId << ", track: " << clipKey.trackId << ", pitch: " << pitch;

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    pushProjectHistoryChangeClipPitchState();

    return true;
}

bool Au3Interaction::resetClipPitch(const ClipKey& clipKey)
{
    WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    clip->SetCentShift(0);
    LOGD() << "reseted pitch of clip: " << clipKey.clipId << ", track: " << clipKey.trackId;

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    pushProjectHistoryResetClipPitchState();

    return true;
}

bool Au3Interaction::changeClipSpeed(const ClipKey& clipKey, double speed)
{
    WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    TimeStretching::SetClipStretchRatio(*waveTrack, *clip, speed);
    LOGD() << "changed speed of clip: " << clipKey.clipId << ", track: " << clipKey.trackId << ", speed: " << speed;

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    pushProjectHistoryChangeClipSpeedState();

    return true;
}

bool Au3Interaction::resetClipSpeed(const ClipKey& clipKey)
{
    WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    TimeStretching::SetClipStretchRatio(*waveTrack, *clip, 1);
    LOGD() << "reseted speed of clip: " << clipKey.clipId << ", track: " << clipKey.trackId;

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    pushProjectHistoryResetClipSpeedState();

    return true;
}

bool Au3Interaction::changeClipOptimizeForVoice(const ClipKey& clipKey, bool optimize)
{
    WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    clip->SetPitchAndSpeedPreset(optimize ? PitchAndSpeedPreset::OptimizeForVoice : PitchAndSpeedPreset::Default);
    LOGD() << "changed optimize for voice of clip: " << clipKey.clipId << ", track: " << clipKey.trackId << ", optimize: " << optimize;

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    return true;
}

void Au3Interaction::renderClipPitchAndSpeed(const ClipKey& clipKey)
{
    muse::Concurrent::run([this, clipKey](){
        m_progress->started.notify();

        muse::ProgressResult result;

        DEFER {
            m_progress->finished.send(result);
        };

        WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), ::TrackId(clipKey.trackId));
        IF_ASSERT_FAILED(waveTrack) {
            return;
        }

        std::shared_ptr<WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
        IF_ASSERT_FAILED(clip) {
            return;
        }

        auto progressCallBack = [this](double progressFraction) {
            m_progress->progressChanged.send(progressFraction * 1000, 1000, "");
        };

        waveTrack->ApplyPitchAndSpeed({ { clip->GetPlayStartTime(), clip->GetPlayEndTime() } }, progressCallBack);
        LOGD() << "apply pitch and speed for clip: " << clipKey.clipId << ", track: " << clipKey.trackId;

        trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
        prj->notifyAboutTrackChanged(DomConverter::track(waveTrack)); //! todo: replase with onClipChanged

        pushProjectHistoryRenderClipStretchingState();
    });
}

void Au3Interaction::clearClipboard()
{
    clipboard()->clearTrackData();
}

muse::Ret Au3Interaction::pasteFromClipboard(secs_t begin, TrackId destinationTrackId)
{
    if (clipboard()->trackDataEmpty()) {
        return make_ret(trackedit::Err::TrackEmpty);
    }

    auto copiedData = clipboard()->trackData();
    if (destinationTrackId == -1) {
        auto tracksIdsToSelect = pasteIntoNewTracks(copiedData);
        selectionController()->setSelectedTracks(tracksIdsToSelect);
        return muse::make_ok();
    }

    project::IAudacityProjectPtr project = globalContext()->currentProject();
    //! TODO: we need to make sure that we get a trackList with order
    //! the same as in the TrackPanel
    auto tracks = project->trackeditProject()->trackList();
    size_t tracksNum = clipboard()->trackDataSize();

    // for multiple tracks copying
    TrackIdList dstTracksIds = determineDestinationTracksIds(tracks, destinationTrackId, tracksNum);

    bool newTracksNeeded = false;
    if (dstTracksIds.size() != tracksNum) {
        newTracksNeeded = true;
    }

    // check if copied data fits into selected area
    auto ret = canPasteClips(dstTracksIds, copiedData, begin);
    if (!ret) {
        if (!ret.code() == static_cast<int>(trackedit::Err::NotEnoughSpaceForPaste)) {
            return ret;
        }

        auto ok = makeRoomForDataOnTracks(dstTracksIds, copiedData, begin);
        if (!ok) {
            make_ret(trackedit::Err::FailedToMakeRoomForClip);
        }
    }

    //! NOTE This shouldn't happen, leaving this for now
    //! for debugging purposes
    if (!canPasteClips(dstTracksIds, copiedData, begin)) {
        return ret;
    }

    for (size_t i = 0; i < dstTracksIds.size(); ++i) {
        Au3WaveTrack* dstWaveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(dstTracksIds[i]));
        IF_ASSERT_FAILED(dstWaveTrack) {
            return make_ret(trackedit::Err::WaveTrackNotFound);
        }
        auto prj = globalContext()->currentTrackeditProject();

        // use an unordered_set to store the IDs of the clips before the paste
        std::unordered_set<int> clipIdsBefore;
        for (const auto& clip : prj->clipList(dstTracksIds[i])) {
            clipIdsBefore.insert(clip.key.clipId);
        }

        if (clipboard()->trackData(i).track.get()->NChannels() == 1 && dstWaveTrack->NChannels() == 2) {
            // When the source is mono, may paste its only channel
            // repeatedly into a stereo track
            const auto pastedTrack = std::static_pointer_cast<Au3WaveTrack>(clipboard()->trackData(i).track);
            pastedTrack->MonoToStereo();
            dstWaveTrack->Paste(begin, *pastedTrack);
        } else {
            dstWaveTrack->Paste(begin, *clipboard()->trackData(i).track);
        }

        // Check which clips were added and trigger the onClipAdded event
        for (const auto& clip : prj->clipList(dstTracksIds[i])) {
            if (clipIdsBefore.find(clip.key.clipId) == clipIdsBefore.end()) {
                prj->notifyAboutClipAdded(clip);
            }
        }
    }

    if (newTracksNeeded) {
        // remove already pasted elements from the clipboard and paste the rest into the new tracks
        copiedData.erase(copiedData.begin(), copiedData.begin() + dstTracksIds.size());
        auto tracksIdsToSelect = pasteIntoNewTracks(copiedData);
        dstTracksIds.insert(dstTracksIds.end(), tracksIdsToSelect.begin(), tracksIdsToSelect.end());
    }

    selectionController()->setSelectedTracks(dstTracksIds);

    pushProjectHistoryPasteState();

    return muse::make_ok();
}

bool Au3Interaction::cutClipIntoClipboard(const ClipKey& clipKey)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    auto track = waveTrack->Cut(clip->Start(), clip->End());
    clipboard()->addTrackData(TrackData { track, clipKey });

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipRemoved(DomConverter::clip(waveTrack, clip.get()));
    projectHistory()->pushHistoryState("Cut to the clipboard", "Cut");

    return true;
}

bool Au3Interaction::cutClipDataIntoClipboard(const TrackIdList& tracksIds, secs_t begin, secs_t end)
{
    for (const auto& trackId : tracksIds) {
        bool ok = cutTrackDataIntoClipboard(trackId, begin, end);
        if (!ok) {
            return false;
        }
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    projectHistory()->pushHistoryState("Cut to the clipboard", "Cut");

    return true;
}

bool Au3Interaction::cutTrackDataIntoClipboard(const TrackId trackId, secs_t begin, secs_t end)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    auto track = waveTrack->Cut(begin, end);
    trackedit::ClipKey dummyClipKey = trackedit::ClipKey();
    clipboard()->addTrackData(TrackData { track, dummyClipKey });

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

    return true;
}

bool Au3Interaction::copyClipIntoClipboard(const ClipKey& clipKey)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    auto track = waveTrack->Copy(clip->Start(), clip->End());
    clipboard()->addTrackData(TrackData { track, clipKey });

    return true;
}

bool Au3Interaction::copyClipDataIntoClipboard(const ClipKey& clipKey, secs_t begin, secs_t end)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    auto track = waveTrack->Copy(begin, end);
    clipboard()->addTrackData(TrackData { track, clipKey });

    return true;
}

bool Au3Interaction::copyTrackDataIntoClipboard(const TrackId trackId, secs_t begin, secs_t end)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    auto track = waveTrack->Copy(begin, end);
    trackedit::ClipKey dummyClipKey = trackedit::ClipKey();
    clipboard()->addTrackData(TrackData { track, dummyClipKey });

    return true;
}

bool Au3Interaction::removeClip(const trackedit::ClipKey& clipKey)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    secs_t start = clip->Start();
    secs_t end = clip->End();
    secs_t duration = end - start;

    waveTrack->Clear(clip->Start(), clip->End());

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

    pushProjectHistoryDeleteState(start, duration);

    return true;
}

bool Au3Interaction::removeClipsData(const std::vector<trackedit::ClipKey>& clipsKeys, secs_t begin, secs_t end)
{
    secs_t duration = end - begin;
    secs_t start = begin;

    for (const ClipKey& clipKey : clipsKeys) {
        Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
        IF_ASSERT_FAILED(waveTrack) {
            continue;
        }

        std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
        IF_ASSERT_FAILED(clip) {
            continue;
        }

        waveTrack->Clear(begin, end);

        trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
        prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));
    }

    pushProjectHistoryDeleteState(start, duration);

    return true;
}

bool Au3Interaction::splitTracksAt(const TrackIdList& tracksIds, secs_t pivot)
{
    for (const auto& trackId : tracksIds) {
        Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
        IF_ASSERT_FAILED(waveTrack) {
            return false;
        }

        waveTrack->SplitAt(pivot);

        trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
        prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));
    }

    projectHistory()->pushHistoryState("Split", "Split");

    return true;
}

bool Au3Interaction::mergeSelectedOnTrack(const TrackId trackId, secs_t begin, secs_t end)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    //! TODO fix this so it displays progress if there's
    //! a need to change pitch/speed
    ProgressReporter dummyProgressReporter;
    waveTrack->Join(begin, end, dummyProgressReporter);

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

    return true;
}

bool Au3Interaction::duplicateSelectedOnTrack(const TrackId trackId, secs_t begin, secs_t end)
{
    auto& tracks = Au3TrackList::Get(projectRef());
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    auto dest = waveTrack->Copy(begin, end, false);
    dest->MoveTo(std::max(static_cast<double>(begin), waveTrack->GetStartTime()));
    tracks.Add(dest);

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackAdded(DomConverter::track(dest.get()));

    return true;
}

bool Au3Interaction::splitCutSelectedOnTrack(const TrackId trackId, secs_t begin, secs_t end)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    auto track = waveTrack->SplitCut(begin, end);
    trackedit::ClipKey dummyClipKey = trackedit::ClipKey();
    clipboard()->addTrackData(TrackData { track, dummyClipKey });

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

    return true;
}

bool Au3Interaction::splitDeleteSelectedOnTrack(const TrackId trackId, secs_t begin, secs_t end)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    waveTrack->SplitDelete(begin, end);

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

    return true;
}

bool Au3Interaction::mergeSelectedOnTracks(const TrackIdList& tracksIds, secs_t begin, secs_t end)
{
    secs_t duration = end - begin;

    for (const auto& trackId : tracksIds) {
        bool ok = mergeSelectedOnTrack(trackId, begin, end);
        if (!ok) {
            return false;
        }
    }

    pushProjectHistoryJoinState(begin, duration);

    return true;
}

bool Au3Interaction::duplicateSelectedOnTracks(const TrackIdList& tracksIds, secs_t begin, secs_t end)
{
    for (const auto& trackId : tracksIds) {
        bool ok = duplicateSelectedOnTrack(trackId, begin, end);
        if (!ok) {
            return false;
        }
    }

    pushProjectHistoryDuplicateState();

    return true;
}

bool Au3Interaction::duplicateClip(const ClipKey& clipKey)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    trackedit::secs_t begin = clip->Start();
    trackedit::secs_t end = clip->End();

    bool ok = duplicateSelectedOnTrack(clipKey.trackId, begin, end);
    if (!ok) {
        return false;
    }

    pushProjectHistoryDuplicateState();

    return true;
}

bool Au3Interaction::clipSplitCut(const ClipKey& clipKey)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    auto track = waveTrack->SplitCut(clip->Start(), clip->End());
    trackedit::ClipKey dummyClipKey = trackedit::ClipKey();
    clipboard()->addTrackData(TrackData { track, dummyClipKey });

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

    projectHistory()->pushHistoryState("Split-cut to the clipboard", "Split cut");

    return true;
}

bool Au3Interaction::clipSplitDelete(const ClipKey& clipKey)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    waveTrack->SplitDelete(clip->Start(), clip->End());

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackChanged(DomConverter::track(waveTrack));

    pushProjectHistorySplitDeleteState(clip->Start(), clip->End() - clip->Start());

    return true;
}

bool Au3Interaction::splitCutSelectedOnTracks(const TrackIdList tracksIds, secs_t begin, secs_t end)
{
    for (const auto& trackId : tracksIds) {
        bool ok = splitCutSelectedOnTrack(trackId, begin, end);
        if (!ok) {
            return false;
        }
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    projectHistory()->pushHistoryState("Split-cut to the clipboard", "Split cut");

    return true;
}

bool Au3Interaction::splitDeleteSelectedOnTracks(const TrackIdList tracksIds, secs_t begin, secs_t end)
{
    secs_t duration = end - begin;

    for (const auto& trackId : tracksIds) {
        bool ok = splitDeleteSelectedOnTrack(trackId, begin, end);
        if (!ok) {
            return false;
        }
    }

    pushProjectHistorySplitDeleteState(begin, duration);

    return true;
}

bool Au3Interaction::trimClipLeft(const ClipKey& clipKey, secs_t deltaSec, bool completed)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    if (completed) {
        auto ok = makeRoomForClip(clipKey);
        if (!ok) {
            return false;
        }
    }

    clip->TrimLeft(deltaSec);

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    if (completed) {
        projectHistory()->pushHistoryState("Clip trimmed", "Trim clip");
    }

    return true;
}

bool Au3Interaction::trimClipRight(const ClipKey& clipKey, secs_t deltaSec, bool completed)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return false;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return false;
    }

    if (completed) {
        auto ok = makeRoomForClip(clipKey);
        if (!ok) {
            make_ret(trackedit::Err::FailedToMakeRoomForClip);
        }
    }

    clip->TrimRight(deltaSec);

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutClipChanged(DomConverter::clip(waveTrack, clip.get()));

    if (completed) {
        projectHistory()->pushHistoryState("Clip trimmed", "Trim clip");
    }

    return true;
}

void Au3Interaction::newMonoTrack()
{
    auto& project = projectRef();
    auto& tracks = Au3TrackList::Get(project);
    auto& trackFactory = ::WaveTrackFactory::Get(project);

    sampleFormat defaultFormat = QualitySettings::SampleFormatChoice();
    auto rate = ::ProjectRate::Get(project).GetRate();

    auto track = trackFactory.Create(defaultFormat, rate);
    track->SetName(tracks.MakeUniqueTrackName(Au3WaveTrack::GetDefaultAudioTrackNamePreference()));

    tracks.Add(track);

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    prj->notifyAboutTrackAdded(DomConverter::track(track.get()));

    selectionController()->setSelectedTracks(TrackIdList(TrackId(track->GetId())));

    pushProjectHistoryTrackAddedState();
}

void Au3Interaction::newStereoTrack()
{
    auto& project = projectRef();
    auto& tracks = Au3TrackList::Get(project);
    auto& trackFactory = ::WaveTrackFactory::Get(project);

    sampleFormat defaultFormat = QualitySettings::SampleFormatChoice();
    auto rate = ::ProjectRate::Get(project).GetRate();

    tracks.Add(trackFactory.Create(2, defaultFormat, rate));
    auto& newTrack = **tracks.rbegin();
    newTrack.SetName(tracks.MakeUniqueTrackName(Au3WaveTrack::GetDefaultAudioTrackNamePreference()));

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    auto track = *tracks.rbegin();
    prj->notifyAboutTrackAdded(DomConverter::track(track));

    selectionController()->setSelectedTracks(TrackIdList(TrackId(track->GetId())));

    pushProjectHistoryTrackAddedState();
}

void Au3Interaction::newLabelTrack()
{
    NOT_IMPLEMENTED;
}

void Au3Interaction::deleteTracks(const TrackIdList& trackIds)
{
    auto& project = projectRef();
    auto& tracks = Au3TrackList::Get(project);

    for (const auto& trackId : trackIds) {
        Au3Track* au3Track = DomAccessor::findTrack(project, Au3TrackId(trackId));
        IF_ASSERT_FAILED(au3Track) {
            continue;
        }
        auto track = DomConverter::track(au3Track);

        tracks.Remove(*au3Track);

        trackedit::ITrackeditProjectPtr trackEdit = globalContext()->currentTrackeditProject();
        trackEdit->notifyAboutTrackRemoved(track);
    }

    projectHistory()->pushHistoryState("Delete track", "Delete track");
}

void Au3Interaction::duplicateTracks(const TrackIdList& trackIds)
{
    auto& project = projectRef();
    auto& tracks = Au3TrackList::Get(project);

    for (const auto& trackId : trackIds) {
        Au3Track* au3Track = DomAccessor::findTrack(project, Au3TrackId(trackId));

        IF_ASSERT_FAILED(au3Track) {
            continue;
        }

        auto au3Clone = au3Track->Duplicate();
        Au3TrackList::AssignUniqueId(au3Clone);

        tracks.Add(au3Clone, ::TrackList::DoAssignId::Yes);

        auto clone = DomConverter::track(au3Clone.get());

        trackedit::ITrackeditProjectPtr trackEdit = globalContext()->currentTrackeditProject();
        trackEdit->notifyAboutTrackInserted(clone, tracks.Size());
    }
    projectHistory()->pushHistoryState("Duplicate track", "Duplicate track");
}

void Au3Interaction::moveTracks(const TrackIdList& trackIds, const TrackMoveDirection direction)
{
    if (trackIds.empty()) {
        return;
    }

    TrackIdList sortedTrackIds = trackIds;
    auto isAscending = (direction == TrackMoveDirection::Up || direction == TrackMoveDirection::Bottom);
    std::sort(sortedTrackIds.begin(), sortedTrackIds.end(), [this, isAscending](const TrackId& a, const TrackId& b) {
        return isAscending ? trackPosition(a) < trackPosition(b) : trackPosition(a) > trackPosition(b);
    });

    auto canMoveWithoutPushing = [this, direction, &sortedTrackIds](const TrackId trackId) {
        int currentPos = trackPosition(trackId);
        int targetPos = (direction == TrackMoveDirection::Up) ? currentPos - 1 : currentPos + 1;

        for (const auto& id : sortedTrackIds) {
            if (trackPosition(id) == targetPos) {
                return false;
            }
        }
        return canMoveTrack(trackId, direction);
    };

    for (const auto& trackId : sortedTrackIds) {
        if (direction == TrackMoveDirection::Top || direction == TrackMoveDirection::Bottom || canMoveWithoutPushing(trackId)) {
            moveTrack(trackId, direction);
        }
    }
    projectHistory()->pushHistoryState("Move track", "Move track");
}

void Au3Interaction::moveTracksTo(const TrackIdList& trackIds, int to)
{
    if (trackIds.empty()) {
        return;
    }

    TrackIdList sortedTrackIds = trackIds;
    auto isAscending = (to > trackPosition(trackIds.front()));
    std::sort(sortedTrackIds.begin(), sortedTrackIds.end(), [this, isAscending](const TrackId& a, const TrackId& b) {
        return isAscending ? trackPosition(a) < trackPosition(b) : trackPosition(a) > trackPosition(b);
    });

    for (const auto& trackId : sortedTrackIds) {
        moveTrackTo(trackId, to);
    }

    projectHistory()->pushHistoryState("Move track", "Move track");
}

bool Au3Interaction::canMoveTrack(const TrackId trackId, const TrackMoveDirection direction)
{
    auto& project = projectRef();
    auto& tracks = ::TrackList::Get(project);
    Au3Track* au3Track = DomAccessor::findTrack(project, Au3TrackId(trackId));

    switch (direction) {
    case TrackMoveDirection::Up:
    case TrackMoveDirection::Top:
        return tracks.CanMoveUp(*au3Track);
    case TrackMoveDirection::Down:
    case TrackMoveDirection::Bottom:
        return tracks.CanMoveDown(*au3Track);
    }
}

int Au3Interaction::trackPosition(const TrackId trackId)
{
    auto& project = projectRef();
    auto& tracks = ::TrackList::Get(project);
    Au3Track* au3Track = DomAccessor::findTrack(project, Au3TrackId(trackId));

    return std::distance(tracks.begin(), tracks.Find(au3Track));
}

void Au3Interaction::moveTrackTo(const TrackId trackId, int to)
{
    auto& project = projectRef();
    auto& tracks = ::TrackList::Get(project);
    Au3Track* au3Track = DomAccessor::findTrack(project, Au3TrackId(trackId));

    IF_ASSERT_FAILED(au3Track) {
        return;
    }

    int from = std::distance(tracks.begin(), tracks.Find(au3Track));

    if (to == from) {
        return;
    }

    int pos = from;
    if (pos < to) {
        while (pos != to && tracks.CanMoveDown(*au3Track)) {
            pos++;
            tracks.MoveDown(*au3Track);
        }
    } else {
        while (pos != to && tracks.CanMoveUp(*au3Track)) {
            pos--;
            tracks.MoveUp(*au3Track);
        }
    }

    if (pos != to) {
        LOGW("Can't move track from position %d to %d, track was moved to position %d", from, to, pos);
    }

    auto track = DomConverter::track(au3Track);

    trackedit::ITrackeditProjectPtr trackEdit = globalContext()->currentTrackeditProject();
    trackEdit->notifyAboutTrackMoved(track, pos);
}

void Au3Interaction::moveTrack(const TrackId trackId, const TrackMoveDirection direction)
{
    auto& project = projectRef();
    auto& tracks = Au3TrackList::Get(project);
    Au3Track* au3Track = DomAccessor::findTrack(project, Au3TrackId(trackId));

    IF_ASSERT_FAILED(au3Track) {
        return;
    }

    int initialPosition = std::distance(tracks.begin(), tracks.Find(au3Track));

    IF_ASSERT_FAILED(initialPosition >= 0 && initialPosition < static_cast<int>(tracks.Size())) {
        return;
    }

    int targetPosition = initialPosition;

    switch (direction) {
    case TrackMoveDirection::Up:
        if (!tracks.CanMoveUp(*au3Track)) {
            break;
        }
        targetPosition--;
        tracks.MoveUp(*au3Track);
        break;
    case TrackMoveDirection::Down:
        if (!tracks.CanMoveDown(*au3Track)) {
            break;
        }
        targetPosition++;
        tracks.MoveDown(*au3Track);
        break;
    case TrackMoveDirection::Top:
        while (tracks.CanMoveUp(*au3Track)) {
            targetPosition--;
            tracks.MoveUp(*au3Track);
        }
        break;
    case TrackMoveDirection::Bottom:
        while (tracks.CanMoveDown(*au3Track)) {
            targetPosition++;
            tracks.MoveDown(*au3Track);
        }
        break;
    default:
        return;
    }

    if (initialPosition == targetPosition) {
        LOGW() << "Can't move track to " << &direction;
        return;
    }

    auto track = DomConverter::track(au3Track);

    trackedit::ITrackeditProjectPtr trackEdit = globalContext()->currentTrackeditProject();
    trackEdit->notifyAboutTrackMoved(track, targetPosition);
}

muse::secs_t Au3Interaction::clipDuration(const trackedit::ClipKey& clipKey) const
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return -1.0;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return -1.0;
    }

    return clip->End() - clip->Start();
}

void Au3Interaction::pushProjectHistoryJoinState(secs_t start, secs_t duration)
{
    std::stringstream ss;
    ss << "Joined " << duration << " seconds at " << start;

    projectHistory()->pushHistoryState(ss.str(), "Join");
}

void Au3Interaction::undo()
{
    if (!canUndo()) {
        return;
    }

    auto trackeditProject = globalContext()->currentProject()->trackeditProject();

    projectHistory()->undo();

    // Undo removes all tracks from current state and
    // inserts tracks from the previous state so we need
    // to reload whole model
    trackeditProject->reload();
}

bool Au3Interaction::canUndo()
{
    return projectHistory()->undoAvailable();
}

void Au3Interaction::redo()
{
    if (!canRedo()) {
        return;
    }

    auto trackeditProject = globalContext()->currentProject()->trackeditProject();

    projectHistory()->redo();

    // Redo removes all tracks from current state and
    // inserts tracks from the previous state so we need
    // to reload whole model
    trackeditProject->reload();
}

bool Au3Interaction::canRedo()
{
    return projectHistory()->redoAvailable();
}

void Au3Interaction::toggleStretchEnabled(const ClipKey &clipKey)
{
    Au3WaveTrack* waveTrack = DomAccessor::findWaveTrack(projectRef(), Au3TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(waveTrack) {
        return;
    }

    std::shared_ptr<Au3WaveClip> clip = DomAccessor::findWaveClip(waveTrack, clipKey.clipId);
    IF_ASSERT_FAILED(clip) {
        return;
    }

    clip->SetStretchEnabled(!clip->GetStretchEnabled());
}

muse::ProgressPtr Au3Interaction::progress() const
{
    return m_progress;
}

void Au3Interaction::pushProjectHistoryDuplicateState()
{
    projectHistory()->pushHistoryState("Duplicated", "Duplicate");
}

void Au3Interaction::pushProjectHistorySplitDeleteState(secs_t start, secs_t duration)
{
    std::stringstream ss;
    ss << "Split-deleted " << duration << " seconds at " << start;

    projectHistory()->pushHistoryState(ss.str(), "Split delete");
}

void Au3Interaction::pushProjectHistoryTrackAddedState()
{
    projectHistory()->pushHistoryState("Created new audio track", "New track");
}

void Au3Interaction::pushProjectHistoryTracksTrimState(secs_t start, secs_t end)
{
    std::stringstream ss;
    ss << "Trim selected audio tracks from " << start << " seconds to " << end << " seconds";

    projectHistory()->pushHistoryState(ss.str(), "Trim Audio");
}

void Au3Interaction::pushProjectHistoryTrackSilenceState(secs_t start, secs_t end)
{
    std::stringstream ss;
    ss << "Silenced selected tracks for " << start << " seconds at " << end << "";

    projectHistory()->pushHistoryState(ss.str(), "Silence");
}

void Au3Interaction::pushProjectHistoryPasteState()
{
    projectHistory()->pushHistoryState("Pasted from the clipboard", "Paste");
}

void Au3Interaction::pushProjectHistoryDeleteState(secs_t start, secs_t duration)
{
    std::stringstream ss;
    ss << "Delete " << duration << " seconds at " << start;

    projectHistory()->pushHistoryState(ss.str(), "Delete");
}

void Au3Interaction::pushProjectHistoryChangeClipPitchState()
{
    projectHistory()->pushHistoryState("Pitch Shift", "Changed Pitch Shift");
}

void Au3Interaction::pushProjectHistoryResetClipPitchState()
{
    projectHistory()->pushHistoryState("Reset Clip Pitch", "Reset Clip Pitch");
}

void Au3Interaction::pushProjectHistoryChangeClipSpeedState()
{
    projectHistory()->pushHistoryState("Changed Speed", "Changed Speed");
}

void Au3Interaction::pushProjectHistoryResetClipSpeedState()
{
    projectHistory()->pushHistoryState("Reset Clip Speed", "Reset Clip Speed");
}

void Au3Interaction::pushProjectHistoryRenderClipStretchingState()
{
    projectHistory()->pushHistoryState("Rendered time-stretched audio", "Render");
}
