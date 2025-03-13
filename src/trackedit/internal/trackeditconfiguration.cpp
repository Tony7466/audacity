/*
* Audacity: A Digital Audio Editor
*/
#include "trackeditconfiguration.h"

#include "settings.h"

namespace au::trackedit {
static const std::string moduleName("trackedit");

static const muse::Settings::Key ASK_BEFORE_CONVERTING_TO_MONO_OR_STEREO(moduleName, "trackedit/askBeforeConvertingToMonoOrStereo");

void TrackeditConfiguration::init()
{
    muse::settings()->setDefaultValue(ASK_BEFORE_CONVERTING_TO_MONO_OR_STEREO, muse::Val(true));
    muse::settings()->valueChanged(ASK_BEFORE_CONVERTING_TO_MONO_OR_STEREO).onReceive(nullptr, [this](const muse::Val& val) {
        m_askBeforeConvertingToMonoOrStereoChanged.notify();
    });
}

bool TrackeditConfiguration::askBeforeConvertingToMonoOrStereo() const
{
    return muse::settings()->value(ASK_BEFORE_CONVERTING_TO_MONO_OR_STEREO).toBool();
}

void TrackeditConfiguration::setAskBeforeConvertingToMonoOrStereo(bool ask)
{
    if (askBeforeConvertingToMonoOrStereo() == ask) {
        return;
    }
    muse::settings()->setSharedValue(ASK_BEFORE_CONVERTING_TO_MONO_OR_STEREO, muse::Val(ask));
}

muse::async::Notification TrackeditConfiguration::askBeforeConvertingToMonoOrStereoChanged() const
{
    return m_askBeforeConvertingToMonoOrStereoChanged;
}
}
