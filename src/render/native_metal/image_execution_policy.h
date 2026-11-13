#pragma once
// Host scheduling contracts for image-only stages. None of these policies
// authorize formula-lane reuse or a change to shader arithmetic/sample order.
#include "execution_policy.h"
#include "command_policy.h"
#include "native_ao_host.h"
#include "native_ao_math.h"
#include <vector>

namespace mb3d::native_metal {
enum class ImageOrder : uint32_t {
    IndependentPixels=1, SeedThenPixels, InterleavedRandomLanes,
    RasterMedian, HorizontalThenVertical, QuickSortPermutation,
    OrderedCandidates, OrderedGather, QuantizeEachPass, ReconstructThenNeighbors
};
enum class BatchUnit : uint32_t { Pixels=1, LogicalLanes, OwnerTasks, Tiles };
enum class ImageSemantics : uint32_t {
    AOSampling=1, AOSeedContinuation, AOAxisPingPong, AORasterMedian,
    DOFSortContinuation, DOFCollectiveCandidates, DOFOrderedAccumulation,
    DOFPassQuantization, ZNormalsNeighborPass
};
struct ImageEntryPolicy {
    LogicalOwner owner=LogicalOwner::Pixel;
    ImageOrder order=ImageOrder::IndependentPixels;
    uint32_t batch=0,group_width=0,continuation=0;
    bool shared_group_memory=false;
    BatchUnit batch_unit=BatchUnit::Pixels;
    ImageSemantics semantics=ImageSemantics::AOSampling;
    uint32_t continuation_chunks=1;
    // Image-stage faults remain specific to their ABI, not the formula
    // attempted-pixel commit contract. A stage cannot publish partial output.
};
inline std::string DescribeImageEntry(const ImageEntryPolicy& p) {
    std::ostringstream s;s<<"owner="<<uint32_t(p.owner)<<",order="<<uint32_t(p.order)
        <<",batch="<<p.batch<<",group="<<p.group_width<<",quota="<<p.continuation
        <<",collective="<<p.shared_group_memory<<",unit="<<uint32_t(p.batch_unit)
        <<",semantics="<<uint32_t(p.semantics)<<",chunks="<<p.continuation_chunks;return s.str();
}

class ScreenAOExecutionPolicy {
    DescriptorBits<mb3d_native_ao::Params> params_;
    std::vector<DescriptorBits<mb3d_native_ao::Level>> levels_;
    bool parallel_random_;
    uint32_t algorithm_;
    uint32_t raster_slice_;
    ImageEntryPolicy sampling_,rng_,wavelet_,median_;
    NativeImageCommandPolicy commands_;
    bool certificates_=false,force_unknown_=false;
    uint32_t seed_chunks_=1;
public:
    enum class LaneGrouping : uint32_t { LegacySimd=0, IndependentOwners=1 };
    enum class SeedGeometry : uint32_t { Ordered=0, CertifiedDirections=1, ForcedUnknown=2 };
    explicit ScreenAOExecutionPolicy(const NativeAOPreparation& p,bool parallel_random=true,
                                    LaneGrouping grouping=LaneGrouping::LegacySimd,
                                    NativeImageCommandPolicy commands=NativeImageCommandPolicy(),
                                    SeedGeometry geometry=SeedGeometry::Ordered,uint32_t seed_chunks=1)
        :params_(p.params),parallel_random_(parallel_random),algorithm_(p.params.algorithm),
         raster_slice_(p.params.algorithm==mb3d_native_ao::HiQ?16384:1024),commands_(commands) {
        using namespace mb3d_native_ao;
        if(!valid(p.params)||p.params.algorithm>9)
            throw std::invalid_argument("Invalid Native screen AO execution policy");
        if(grouping!=LaneGrouping::LegacySimd&&grouping!=LaneGrouping::IndependentOwners)
            throw std::invalid_argument("Invalid Native screen AO lane grouping");
        const uint32_t laneGroup=grouping==LaneGrouping::IndependentOwners?1:0;
        // Physical regrouping only: reset initializes row y=lane, then each
        // owner advances by logical_lanes. Its 64-byte angle blocks, surface,
        // records and seeds are disjoint. Keep the same 128-pixel quota and
        // whole-lane fault check after every logical chunk; the explicit collective
        // retains this checkpoint on-device. Never blindly batch continuations
        // or infer valid ownership from arbitrary externally poisoned cursors.
        for(const auto& l:p.levels)levels_.emplace_back(l);
        const bool random=random_mode(p.params.algorithm);
        if(geometry!=SeedGeometry::Ordered&&geometry!=SeedGeometry::CertifiedDirections&&geometry!=SeedGeometry::ForcedUnknown)
            throw std::invalid_argument("Invalid Native AO seed geometry policy");
        certificates_=parallel_random&&random&&geometry!=SeedGeometry::Ordered;
        force_unknown_=certificates_&&geometry==SeedGeometry::ForcedUnknown;
        if(seed_chunks<1||seed_chunks>8||
           (seed_chunks>1&&(!certificates_||grouping!=LaneGrouping::LegacySimd)))
            throw std::invalid_argument("Invalid Native AO seed continuation chunks");
        // A single bounded collective may replace host checkpoints only when
        // every owner fits one group. Each 128-pixel boundary still votes on
        // all owners' faults before any owner advances to the next chunk.
        seed_chunks_=p.params.logical_lanes<=32?seed_chunks:1;
        sampling_.batch=random?(parallel_random?16384:p.params.logical_lanes):raster_slice_;
        sampling_.order=random?(parallel_random?ImageOrder::SeedThenPixels:ImageOrder::InterleavedRandomLanes):ImageOrder::IndependentPixels;
        if(random&&!parallel_random){sampling_.owner=LogicalOwner::InterleavedRows;
            sampling_.continuation=128;sampling_.batch_unit=BatchUnit::LogicalLanes;sampling_.group_width=laneGroup;}
        rng_={LogicalOwner::InterleavedRows,ImageOrder::InterleavedRandomLanes,p.params.logical_lanes,laneGroup,128,false};
        if(seed_chunks_>1){rng_.group_width=p.params.logical_lanes;rng_.shared_group_memory=true;rng_.continuation_chunks=seed_chunks_;}
        wavelet_={LogicalOwner::OrderedPass,ImageOrder::HorizontalThenVertical,raster_slice_,0,0,false};
        median_={LogicalOwner::SerialTask,ImageOrder::RasterMedian,1,0,256,false};
        rng_.batch_unit=BatchUnit::LogicalLanes;rng_.semantics=ImageSemantics::AOSeedContinuation;
        wavelet_.semantics=ImageSemantics::AOAxisPingPong;
        median_.batch_unit=BatchUnit::OwnerTasks;median_.semantics=ImageSemantics::AORasterMedian;
    }
    void validate(const NativeAOPreparation& p) const {
        if(!params_.matches(p.params)||p.levels.size()!=levels_.size())
            throw std::invalid_argument("Native screen AO execution policy input mismatch");
        for(size_t i=0;i<levels_.size();++i)if(!levels_[i].matches(p.levels[i]))
            throw std::invalid_argument("Native screen AO execution policy level mismatch");
    }
    bool parallel_random() const{return parallel_random_;}
    bool uses_certificates() const{return certificates_;}
    bool force_unknown() const{return force_unknown_;}
    bool uses_chunk_prefix() const{return certificates_&&!force_unknown_&&seed_chunks_>1;}
    uint32_t seed_continuation_chunks() const{return seed_chunks_;}
    uint32_t certificate_pixel_batch() const{return 16384;}
    const NativeImageCommandPolicy& commands()const{return commands_;}
    const ImageEntryPolicy& sampling() const{return sampling_;}
    const ImageEntryPolicy& rng() const{return rng_;}
    const ImageEntryPolicy& wavelet() const{return wavelet_;}
    const ImageEntryPolicy& median() const{return median_;}
    uint32_t seeded_pixel_batch() const{return 16384;}
    uint32_t raster_slice_batch() const{return raster_slice_;}
    bool uses_rng() const{return mb3d_native_ao::random_mode(algorithm_);}
    bool uses_median() const{return !mb3d_native_ao::hiq(algorithm_);}
    // RNG is initialized per logical lane, level and repeat. Foreground/border
    // tests determine draws; seed planning must finish and validate before
    // sampling. A failed seed pixel leaves the failed cursor unadvanced.
};

// Preparation, resource allocation and runtime consume this same production
// choice. Direct component callers retain their explicit ordered default.
inline ScreenAOExecutionPolicy ProductionScreenAOExecutionPolicy(const NativeAOPreparation& p) {
    return ScreenAOExecutionPolicy(p,true,ScreenAOExecutionPolicy::LaneGrouping::LegacySimd,
        NativeImageCommandPolicy{},ScreenAOExecutionPolicy::SeedGeometry::CertifiedDirections,
        mb3d_native_ao::random_mode(p.params.algorithm)?8:1);
}

inline bool SameDOFLimits(const DOFLimits& a,const DOFLimits& b) {
    return a.spatial_candidates==b.spatial_candidates&&a.tile_batch==b.tile_batch&&
        a.source_slice==b.source_slice&&a.gather_slice==b.gather_slice&&a.sort_slice==b.sort_slice&&
        a.sort_work_limit==b.sort_work_limit&&a.gather_work_limit==b.gather_work_limit&&
        a.candidate_scan_work_limit==b.candidate_scan_work_limit;
}
class DOFExecutionPolicy {
    DescriptorBits<mb3d_dof::Params> params_;
    DOFLimits limits_;
    DOFAllocationPlan allocation_;
public:
    DOFExecutionPolicy(const mb3d_dof::Params& p,const DOFLimits& l={})
        :params_(p),limits_(l),allocation_(PlanNativeDOF(p,l)){}
    void validate(const mb3d_dof::Params& p,const DOFLimits& l) const {
        if(!params_.matches(p)||!SameDOFLimits(limits_,l))
            throw std::invalid_argument("Native DOF execution policy input mismatch");
    }
    const DOFLimits& limits() const{return limits_;}
    const DOFAllocationPlan& allocation() const{return allocation_;}
    ImageEntryPolicy sort() const{return {LogicalOwner::SerialTask,ImageOrder::QuickSortPermutation,1,0,limits_.sort_slice,false,BatchUnit::OwnerTasks,ImageSemantics::DOFSortContinuation};}
    ImageEntryPolicy spatial() const{return {LogicalOwner::CollectiveTile,ImageOrder::OrderedCandidates,limits_.tile_batch,256,0,true,BatchUnit::Tiles,ImageSemantics::DOFCollectiveCandidates};}
    ImageEntryPolicy gather() const{return {LogicalOwner::Pixel,ImageOrder::OrderedGather,limits_.tile_batch,0,limits_.gather_slice,false,BatchUnit::Tiles,ImageSemantics::DOFOrderedAccumulation};}
    ImageEntryPolicy finish() const{return {LogicalOwner::OrderedPass,ImageOrder::QuantizeEachPass,limits_.tile_batch,0,0,false,BatchUnit::Tiles,ImageSemantics::DOFPassQuantization};}
    bool use_spatial(uint32_t actual_maximum_bound) const{return limits_.spatial_candidates&&actual_maximum_bound<=24;}
    // Spatial selection consumes the completed GPU radius reduction. The
    // collective kernel uses 4096 shared entries and MUST keep 256 threads.
    // Sort keeps the original quicksort permutation, including equal keys;
    // incomplete/faulted sort state and order are not successful output.
};

class ZNormalsExecutionPolicy {
    DescriptorBits<mb3d_zn::Params> params_;
    ZNormalsAllocationPlan allocation_;
    uint32_t pixels_;
public:
    explicit ZNormalsExecutionPolicy(const mb3d_zn::Params& p)
        :params_(p),allocation_(PlanNativeZNormals(p)),pixels_(p.count){}
    void validate(const mb3d_zn::Params& p) const {
        if(!params_.matches(p))throw std::invalid_argument("Native Z normals execution policy input mismatch");
    }
    const ZNormalsAllocationPlan& allocation() const{return allocation_;}
    ImageEntryPolicy reconstruction_and_normals() const {
        return {LogicalOwner::Pixel,ImageOrder::ReconstructThenNeighbors,pixels_,0,0,false,BatchUnit::Pixels,ImageSemantics::ZNormalsNeighborPass};
    }
    // Reconstruction completes before neighbors read coordinates. Atomic fault
    // status is checked after each command; non-normal surface fields survive.
};
}
