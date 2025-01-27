/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "internal/irealtimeeffectpaneltrackselection.h"
#include "uicomponents/view/abstractmenumodel.h"
#include "trackedit/trackedittypes.h"
#include "effects/effects_base/ieffectsprovider.h"
#include "effects/effects_base/irealtimeeffectservice.h"
#include <QObject>

namespace au::projectscene {
class RealtimeEffectMenuModelBase : public muse::uicomponents::AbstractMenuModel
{
    Q_OBJECT
    Q_PROPERTY(bool isMasterTrack READ isMasterTrack WRITE prop_setIsMasterTrack NOTIFY isMasterTrackChanged)

    muse::Inject<IRealtimeEffectPanelTrackSelection> trackSelection;
public:
    explicit RealtimeEffectMenuModelBase(QObject* parent = nullptr);

    Q_INVOKABLE void load() final override;

protected:
    std::optional<au::trackedit::TrackId> trackId() const;
    void resetList();
    void removeTrack(const au::trackedit::TrackId& trackId);
    bool isMasterTrack() const { return m_isMasterTrack; }

    muse::Inject<effects::IEffectsProvider> effectsProvider;
    muse::Inject<effects::IRealtimeEffectService> realtimeEffectService;

signals:
    void isMasterTrackChanged();

private:
    void beginResetModel();
    void endResetModel();

    void prop_setIsMasterTrack(bool isMasterTrack);

    virtual void doLoad() = 0;
    virtual void populateMenu() = 0;
    virtual void doResetList() {}
    virtual void doRemoveTrack(const au::trackedit::TrackId&) {}
    virtual void onTrackIdChanged() {}

    bool m_isMasterTrack = false;
};
}
