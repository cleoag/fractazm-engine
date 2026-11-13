#ifndef MB3D_IO_M3P_PARSER_H
#define MB3D_IO_M3P_PARSER_H

#include "core/types.h"
#include <string>
#include <vector>
#include <cstdint>

namespace mb3d {

struct M3PLoadResult {
    TMandHeader10 header;
    THeaderCustomAddon addon;
    uint64_t raw_file_size = 0;
    uint64_t loaded_data_size = 0;
    uint64_t raw_header_size = 0;
    uint64_t pixel_data_size = 0;
    uint64_t addon_offset = 0;
    bool is_compressed = false;
    int32_t source_mand_id = 0;
    bool success;
    std::string error;
};

// Load a .m3p parameter file.
// Handles all format versions (MandId 1..44) with full version migration.
// Supports both uncompressed and zlib-compressed data.
M3PLoadResult LoadM3PFile(const std::string& filepath);

// Save parameters to a .m3p file (uncompressed raw binary).
bool SaveM3PFile(const std::string& filepath,
                 const TMandHeader10& header,
                 const THeaderCustomAddon& addon);

// ------------------------------------------------------------------
// Internal migration helpers (exposed for testing)
// ------------------------------------------------------------------

// Convert TLightingParas (oldest) / TLightingParas7 into TLightingParas8,
// handling files smaller than TLightingParas7.
void ConvertFromOldLightParas(TLightingParas8& light8, int fSize);

// Update TLightingParas8 color tables and version bits (version 0->2).
void UpdateLightVersion8(TLightingParas8& light8);

// Convert TLightingParas8 into TLightingParas9.
void ConvertLight8to9(const TLightingParas8& light8, TLightingParas9& light9);

// Convert TMandHeader4 into TMandHeader8 (for MandId < 7).
void Convert4to7paras(const TMandHeader4& para4, TMandHeader8& para8);

// Convert TMandHeader8/9 fields for MandId < 20.
bool ConvertHeaderFromOldParas(TMandHeader8& para8, THeaderCustomAddon* addon = nullptr);

// Apply formula option adjustments for MandId < 20.
void UpdateFormulaOptionTo20(THeaderCustomAddon& addon);

// Apply per-version field initializations for MandId 20..44.
void UpdateFormulaOptionAbove20(TMandHeader10& para, THeaderCustomAddon& addon);

// Update light parameters for versions above light version 3.
void UpdateLightParasAbove3(TLightingParas9& light);

// Parse/rename known external formula names.
void ParseExternFormulas(THeaderCustomAddon& addon);

// ------------------------------------------------------------------
// .m3i (full render cache) loading
// ------------------------------------------------------------------

struct M3ILoadResult {
    M3PLoadResult params;               // header + addon (reuse existing)
    std::vector<TsiLight5> siLight;     // pre-computed lighting buffer
    std::vector<uint32_t> image;        // pre-rendered color (optional, ARGB per pixel)
    bool has_silight = false;
    bool has_image = false;
    bool success = false;
    std::string error;
};

// Load a .m3i render cache file.
// Reads header (with full migration), siLight buffer (converting from
// siLight3/4 when needed), and optional fullSizeImage.
M3ILoadResult LoadM3IFile(const std::string& filepath);

} // namespace mb3d

#endif // MB3D_IO_M3P_PARSER_H
