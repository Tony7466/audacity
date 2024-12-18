/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include <vector>

#include "global/types/string.h"
#include "global/types/ratio.h"
#include "actions/actiontypes.h"

class Effect;
class EffectInstanceEx;
struct EffectSettings;
class wxString;
namespace au::effects {
using secs_t = muse::number_t<double>;
using percent_t = muse::number_t<float>;
using ratio_t = muse::ratio_t;
using db_t = muse::db_t;
using rms_t = float;

using EffectId = muse::String;              // PluginID from AU3
using Effect = ::Effect;                    // Effect from AU3
using EffectInstanceId = uint64_t;
using EffectInstance = ::EffectInstanceEx;  // EffectInstanceEx from AU3
using EffectSettings = ::EffectSettings;
using EffectStateId = uintptr_t;
using TrackId = long;
using EffectChainLinkIndex = int;

struct EffectMeta {
    EffectId id;
    muse::String title;
    muse::String description;
    muse::String version;
    muse::String vendor;

    muse::String categoryId;

    bool isRealtimeCapable = false;
    bool supportsMultipleClipSelection = true;

    bool isValid() const { return !id.empty(); }
};

using EffectMetaList = std::vector<EffectMeta>;

constexpr const char16_t* EFFECT_OPEN_ACTION = u"action://effects/open?effectId=%1";
inline muse::actions::ActionQuery makeEffectOpenAction(const EffectId& id)
{
    return muse::actions::ActionQuery(muse::String(EFFECT_OPEN_ACTION).arg(id));
}

struct EffectCategory {
    muse::String id;
    muse::String title;

    bool isValid() const { return !id.empty(); }
};

using EffectCategoryList = std::vector<EffectCategory>;

constexpr const char16_t* VST_CATEGORY_ID = u"vst";
constexpr const char16_t* BUILTIN_CATEGORY_ID = u"builtin";

struct EffectChainLink {
    EffectChainLink(std::string effectName, EffectStateId effectStateId)
        : effectName{std::move(effectName)}, effectStateId{effectStateId} {}
    const std::string effectName;
    const EffectStateId effectStateId;
};

using EffectChainLinkPtr = std::shared_ptr<EffectChainLink>;

using PresetId = wxString;
using PresetIdList = std::vector<PresetId>;
}
