#include "core/lightmap.h"
#include "core/render_context.h"

namespace mb3d {

// ============================================================================
// Per-job state
// ============================================================================

MapResources::~MapResources() {
    for (auto& map : calc_maps) FreeLightMap(&map);
}

TLightMap (&GetCalcMaps())[3] {
    return CurrentRenderContext().maps.calc_maps;
}

std::atomic<LongBool>& GetMapMissingState() {
    return CurrentRenderContext().maps.missing;
}

// ============================================================================
// Map sequence support (from MapSequences.pas)
// ============================================================================

MapSequenceList& GetMapSequenceList() {
    return CurrentRenderContext().maps.sequences;
}

int GetCurrMapFrameNumber() {
    return CurrentRenderContext().maps.frame_number;
}

void SetCurrMapFrameNumber(int frame) {
    CurrentRenderContext().maps.frame_number = frame;
}

// static
std::string MapSequence::FormatFrameFilename(const std::string& filename, int frame) {
    // Find extension position (last '.')
    int posExtension = -1;
    for (int i = static_cast<int>(filename.size()) - 1; i >= 0; i--) {
        if (filename[i] == '.') { posExtension = i; break; }
    }
    if (posExtension < 0) return {};

    // Find start of trailing digits before extension
    int posNumStart = -1;
    for (int i = posExtension - 1; i >= 0; i--) {
        if (filename[i] < '0' || filename[i] > '9') {
            posNumStart = i + 1;
            break;
        }
    }
    if (posNumStart < 0 || posNumStart == posExtension) return {};

    int numLen = posExtension - posNumStart;
    std::string hs = std::to_string(frame);
    while (static_cast<int>(hs.size()) < numLen) hs = "0" + hs;

    return filename.substr(0, posNumStart) + hs + filename.substr(posExtension);
}

std::string MapSequence::GetFilename(int frame) const {
    int lFrame = (frame - 1) * increment + firstImage;
    int lLastImage = (lastImage / increment) * increment;
    if (lFrame < firstImage) {
        lFrame = firstImage;
    } else if (lFrame > lLastImage) {
        if (loop) {
            lFrame = (lFrame - 1) % (lLastImage - firstImage + 1) + firstImage;
        } else {
            lFrame = lLastImage;
        }
    }
    return FormatFrameFilename(imageFilename, lFrame);
}

const MapSequence* MapSequenceList::GetSequence(int channel) const {
    for (const auto& seq : sequences_) {
        if (seq.destChannel == channel) return &seq;
    }
    return nullptr;
}

void FreeLightMap(TPLightMap lm) {
    if (lm->LMa) {
        delete[] lm->LMa;
        lm->LMa = nullptr;
    }
    lm->LMnumber = 0;
    lm->LMWidth = 0;
    lm->LMHeight = 0;
    lm->iLMstart = 0;
}

} // namespace mb3d
