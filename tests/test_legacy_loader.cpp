// Full original LoadParameter records, captured before render overrides.
// A qualified comparison is deliberately distinct from byte-exact parity.
#include "io/m3p_parser.h"
#include "fixtures/legacy_loader_cases.h"
#include "windows_console_crash_guard.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <zlib.h>

namespace fs = std::filesystem;
using Bytes = std::vector<uint8_t>;
static int checks = 0;
static void require(bool condition, const std::string& description) {
    if (!condition) throw std::runtime_error(description);
    ++checks;
}
static Bytes read(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    require(bool(file), "missing fixture: " + path.string());
    return Bytes(std::istreambuf_iterator<char>(file), {});
}
static void write(const fs::path& path, const Bytes& bytes) {
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    require(bool(file), "cannot write temporary input");
}
template<class T> static T get(const Bytes& bytes, size_t offset) {
    T value; std::memcpy(&value, bytes.data()+offset, sizeof(value)); return value;
}
template<class T> static void put(Bytes& bytes, size_t offset, T value) {
    std::memcpy(bytes.data()+offset, &value, sizeof(value));
}
static uint64_t fingerprint(const Bytes& bytes) {
    uint64_t value = 14695981039346656037ULL;
    for (uint8_t byte : bytes) value = (value ^ byte) * 1099511628211ULL;
    return value;
}
static Bytes pack(const mb3d::M3PLoadResult& loaded) {
    Bytes bytes(sizeof(loaded.header)+sizeof(loaded.addon));
    std::memcpy(bytes.data(), &loaded.header, sizeof(loaded.header));
    std::memcpy(bytes.data()+sizeof(loaded.header), &loaded.addon, sizeof(loaded.addon));
    return bytes;
}
static bool compareQualified(const Bytes& original, const Bytes& actual,
                             int version, const std::string& name) {
    require(original.size()==1976 && actual.size()==1976, name+": packed record size");
    std::array<bool,1976> permitted{};
    auto allow = [&](size_t start, size_t size) {
        std::fill(permitted.begin()+start, permitted.begin()+start+size, true);
    };
    require(get<int32_t>(actual,0)==44, name+": canonical migrated marker");
    if (get<int32_t>(original,0)==version) allow(0,4);
    // Original Convert4to7paras leaves the Para9 local tail uninitialized.
    // Zero is the deterministic port policy, not equality with undefined bytes.
    if (version<7) {
        require(get<int32_t>(actual,414)==0, name+": deterministic old iMaxIts");
        allow(414,4);
    }
    if (version<17) {
        for (size_t offset=246; offset<318; offset+=8) {
            const double a=get<double>(original,offset), b=get<double>(actual,offset);
            require(std::isfinite(a) && std::isfinite(b) && std::fabs(a-b)<=2e-12,
                    name+": old camera Double tolerance");
            allow(offset,8);
        }
    }
    for (size_t slot=0; slot<6; ++slot) {
        const size_t start=848+188*slot, nameStart=start+12;
        size_t terminator=0;
        while (terminator<32 && original[nameStart+terminator]!=0 &&
               original[nameStart+terminator]==actual[nameStart+terminator]) ++terminator;
        // DivUtils.CustomFtoStr stops at this terminator; no unbounded comparison.
        if (terminator<32 && original[nameStart+terminator]==0 && actual[nameStart+terminator]==0)
            allow(nameStart+terminator+1,31-terminator);
        // IniHAddon sets every option[0]=8; the invalid-slot branch retains it.
        // Count alone is insufficient: a named dormant formula can be enabled.
        if (get<int32_t>(original,start)==0 && get<int32_t>(actual,start)==0 &&
            get<int32_t>(original,start+4)==-1 && get<int32_t>(actual,start+4)==-1 &&
            get<int32_t>(original,start+8)==0 && get<int32_t>(actual,start+8)==0 &&
            original[nameStart]==0 && actual[nameStart]==0 &&
            get<double>(original,start+60)==8.0 && get<double>(actual,start+60)==0.0)
            allow(start+60,8);
    }
    for (size_t i=0;i<actual.size();++i)
        require(actual[i]==original[i] || permitted[i], name+": unexplained byte at "+std::to_string(i));
    return actual==original;
}

int main(int argc, char** argv) {
    install_test_console_crash_handlers();
    try {
        if (argc!=3) throw std::runtime_error("expected fixture directory and temporary directory");
        const fs::path fixtures=argv[1], temp=argv[2];
        fs::create_directories(temp);
        int exact=0, qualified=0, undefined=0;
        for (const auto& c : legacy_loader_fixture::cases) {
            const auto input=read(fixtures/(std::string(c.id)+".m3p"));
            const auto original=read(fixtures/(std::string(c.id)+".pascal.bin"));
            require(fingerprint(input)==c.inputHash && fingerprint(original)==c.expectedHash,
                    std::string(c.id)+": independent fixture integrity");
            const auto loaded=mb3d::LoadM3PFile((fixtures/(std::string(c.id)+".m3p")).string());
            require(loaded.success,std::string(c.id)+": "+loaded.error);
            require(loaded.source_mand_id==c.sourceVersion,std::string(c.id)+": source version provenance");
            const bool equal=compareQualified(original,pack(loaded),c.sourceVersion,c.id);
            equal ? ++exact : ++qualified;
            if(c.sourceVersion<7) ++undefined;
            if(c.addonOffset) {
                require(loaded.addon_offset==c.addonOffset,std::string(c.id)+": independent cache addon offset");
                require(loaded.pixel_data_size==c.storedBytes,std::string(c.id)+": independent stored byte count");
            }
        }
        for (const char* id : {"v15_ABoxMod1_named","v15_ABoxMod1_hybrid"}) {
            const auto loaded=mb3d::LoadM3PFile((fixtures/(std::string(id)+".m3p")).string());
            require(loaded.success,"ABoxMod1 loaded");
            const auto& first=loaded.addon.Formulas[0];
            require(first.iOptionCount==7 && first.dOptionValue[0]==6.25 &&
                first.dOptionValue[1]==.75 && first.dOptionValue[2]==2.0,
                "ABoxMod1 saved prefix retained; legacy sentinel42 replaced by independent default2");
            if(std::strstr(id,"hybrid")) {
                for(int slot : {1,2}) {
                    const auto& f=loaded.addon.Formulas[slot];
                    require(f.iOptionCount==7 && f.dOptionValue[0]==2.0 &&
                        f.dOptionValue[1]==0.0 && f.dOptionValue[2]==2.0,
                        "ABoxMod1 later old hybrid slots use complete independent defaults");
                }
            }
        }
        const fs::path temporary=temp/"bounded-input.m3p";
        auto reject = [&](const Bytes& bytes,const std::string& label) {
            write(temporary,bytes);
            const auto loaded=mb3d::LoadM3PFile(temporary.string());
            require(!loaded.success && !loaded.error.empty(),label+": explicit load failure");
        };
        int malformed=0;
        for (const char* id : {"version_00","version_04","version_07","version_08","version_20"}) {
            const auto valid=read(fixtures/(std::string(id)+".m3p"));
            const int version=get<int32_t>(valid,0);
            const size_t size=version<4?200:version<7?272:version<8?580:version<20?704:840;
            for(size_t length : {size_t(0),size_t(1),size_t(3),size_t(19),size_t(99),size-1}) {
                reject(Bytes(valid.begin(),valid.begin()+length),std::string(id)+" truncated header");++malformed;
            }
        }
        for (const char* id : {"version_44","v44_old_addon"}) {
            const auto valid=read(fixtures/(std::string(id)+".m3p"));
            for(size_t length : {size_t(0),size_t(1),size_t(3),size_t(4),valid.size()-841}) {
                reject(Bytes(valid.begin(),valid.begin()+840+length),"truncated versioned addon");++malformed;
            }
        }
        for(size_t size : {size_t(0),size_t(1),size_t(3),size_t(19),size_t(99)}) {
            Bytes raw(size,0), compressed(compressBound(static_cast<uLong>(size))+20);
            uLongf length=static_cast<uLongf>(compressed.size());
            const uint8_t empty=0;
            require(compress(compressed.data(),&length,size?raw.data():&empty,static_cast<uLong>(size))==Z_OK,
                    "construct bounded zlib safety input");
            // Keep file-level minimum satisfied; trailing bytes are legal to uncompress.
            compressed.resize(std::max<size_t>(20,length),0);
            reject(compressed,"short decompressed record");++malformed;
        }
        const auto modern=read(fixtures/"version_44.m3p");
        for(const char* id : {"version_16","version_17","version_18"}) {
            auto huge=read(fixtures/(std::string(id)+".m3p"));
            put<int32_t>(huge,4,INT32_MAX);put<int32_t>(huge,8,INT32_MAX);
            reject(huge,"old cache byte-count overflow");++malformed;
        }
        for(uint32_t tiling : {uint32_t(1u<<28),uint32_t((1u<<28)|1u),uint32_t((1u<<28)|(1u<<7))}) {
            auto invalid=modern;put<uint32_t>(invalid,428,tiling);
            reject(invalid,"original zero tile divisor");++malformed;
        }
        // Independent MakeHeader readback captured modes 0,3,7,255; direct core
        // mode3 is separately covered by the original CheckHybridOptions oracle.
        // Existing modes1/2 are identity controls, including upper option bits.
        for(const std::array<uint8_t,2> mode : {std::array<uint8_t,2>{0,0},{1,1},{2,2},{3,0},{7,4},{255,252}}) {
            auto input=modern;input[841]=mode[0];write(temporary,input);
            const auto loaded=mb3d::LoadM3PFile(temporary.string());
            require(loaded.success && loaded.addon.bOptions1==mode[1],"file-only hybrid GUI readback");
        }
        for(uint8_t optic : {3,6,255}) {
            auto input=modern;input[148]=optic;
            reject(input,"original file optic selector failure");++malformed;
        }
        for(uint8_t optic : {0,1,2}) {
            auto input=modern;input[148]=optic;write(temporary,input);
            const auto loaded=mb3d::LoadM3PFile(temporary.string());
            require(loaded.success && loaded.header.bPlanarOptic==optic,"valid file optics unchanged");
        }
        for(uint8_t low=0;low<=7;++low) {
            auto input=modern;input[343]=32|low;write(temporary,input);
            const auto loaded=mb3d::LoadM3PFile(temporary.string());
            require(loaded.success && loaded.header.bVolLightNr==(32|std::min<int>(6,low)),
                    "file volume selector preserves valid modes and upper bits");
        }
        const auto recovery=mb3d::LoadM3PFile((fixtures/"version_44.m3p").string());
        require(recovery.success && pack(recovery)==read(fixtures/"version_44.pascal.bin"),
                "valid exact load after malformed inputs in same process");
        std::printf("Legacy loader: %d exact records, %d qualified records; %d original-undefined iMaxIts cases explicitly excluded.\n",
                    exact,qualified,undefined);
        std::printf("ABoxMod1 default tails, cache offsets, 6 hybrid file modes, %d bounded rejects and recovery: %d checks passed.\n",malformed,checks);
        return 0;
    } catch(const std::exception& e) {
        std::fprintf(stderr,"Legacy loader regression failed: %s\n",e.what());return 1;
    }
}
