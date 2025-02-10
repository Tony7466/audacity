#pragma once

#include <optional>

#include "iwavepainter.h"

namespace au::projectscene {
class SamplesPainter : public IWavePainter
{
public:
    SamplesPainter() = default;
    void paint(int channelIndex, QPainter& painter, const WaveMetrics& metrics, const Style& style, const au::au3::Au3WaveTrack& track,
               const au::au3::Au3WaveClip& clip) override;
};
}
