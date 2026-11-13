#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

namespace mb3d::native_metal {
// Only these audited disjoint image phases can share a command. A phase ends
// before readback, axis/level/repeat transition or any reuse of its resources.
enum class NativeImagePhase : std::uint32_t {
    Standalone=0, ScreenBegin, ScreenWavelet, ThresholdEncode, ThresholdWavelet
};
struct NativeCommandLimits {
    std::uint32_t encoders=8;
    std::uint64_t invocations=65536;
};
class NativeImageCommandPolicy {
public:
    enum class Mode : std::uint32_t { Serial=0, BoundedSlices=1 };
private:
    Mode mode_;
    NativeCommandLimits limits_;
public:
    explicit NativeImageCommandPolicy(Mode mode=Mode::BoundedSlices,NativeCommandLimits limits={})
        :mode_(mode),limits_(limits) {
        if((mode!=Mode::Serial&&mode!=Mode::BoundedSlices)||limits.encoders<1||limits.encoders>8||
           limits.invocations<16384||limits.invocations>65536)
            throw std::invalid_argument("Invalid Native image command policy");
    }
    bool grouped(NativeImagePhase phase)const {
        switch(phase) {
        case NativeImagePhase::Standalone:return false;
        case NativeImagePhase::ScreenBegin:case NativeImagePhase::ScreenWavelet:
        case NativeImagePhase::ThresholdEncode:case NativeImagePhase::ThresholdWavelet:
            return mode_==Mode::BoundedSlices&&limits_.encoders>1;
        }
        throw std::invalid_argument("Unknown Native image command phase");
    }
    const NativeCommandLimits& limits()const{return limits_;}
    std::string describe()const {
        return "version=1,mode="+std::to_string(std::uint32_t(mode_))+",encoders="+
            std::to_string(limits_.encoders)+",invocations="+std::to_string(limits_.invocations)+",phases=1:2:3:4";
    }
};
}
