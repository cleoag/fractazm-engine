#pragma once
// Native-only bounded command segments for audited disjoint image slices.
// Keep required host fault/readback boundaries outside a segment.
#include "comparison/metal_exact/metal_device.h"
#include "command_policy.h"
#include <cstdint>
#include <functional>

namespace mb3d::native_metal {
struct NativeCommandStatistics {
    std::uint64_t completed_commands=0,completed_dispatches=0,completed_invocations=0;
    double gpu_ms=0,max_command_ms=0;
};

// Each instance is a single dependency segment. It must be finished before a
// readback, level/axis transition or any reuse of its buffers. It cannot batch
// RNG/median continuations or erase a required first-fault CPU checkpoint.
class NativeCommandBatch {
    using Metal=metal_exact::gpu::Metal;
    Metal& metal_;
    std::function<void()> poll_;
    NativeCommandLimits limits_;
    NativeCommandStatistics statistics_;
    id<MTLCommandBuffer> command_=nil;
    std::uint32_t pending_dispatches_=0;
    std::uint64_t pending_invocations_=0;
    bool failed_=false;

    void wait_for_submitted() noexcept {
        // commit can fail before submission. Never wait on a merely created
        // or enqueued command; retain resources through any actual submission.
        if(command_!=nil&&(command_.status==MTLCommandBufferStatusCommitted||
                           command_.status==MTLCommandBufferStatusScheduled))
            [command_ waitUntilCompleted];
    }

    void discard_unsubmitted() noexcept {
        // A committed command is synchronously awaited by flush(). Never
        // drop its resource references while device work is outstanding.
        command_=nil;
        pending_dispatches_=0;
        pending_invocations_=0;
    }
    struct Operation {
        NativeCommandBatch& batch;
        bool succeeded=false;
        ~Operation() {
            if(!succeeded) {
                batch.failed_=true;
                batch.wait_for_submitted();
                batch.discard_unsubmitted();
            }
        }
    };
public:
    NativeCommandBatch(Metal& metal,const std::function<void()>& poll,NativeCommandLimits limits)
        :metal_(metal),poll_(poll),limits_(limits) {
        using metal_exact::gpu::require;
        require(bool(poll_)&&limits_.encoders>=1&&limits_.encoders<=8&&
                limits_.invocations>=1&&limits_.invocations<=65536,"Native command limits");
        require(metal_.arithmeticProfile==std::array<std::uint32_t,3>{2,4,32},
                "Native command batch requires actual Native profile");
    }
    NativeCommandBatch(const NativeCommandBatch&)=delete;
    NativeCommandBatch& operator=(const NativeCommandBatch&)=delete;
    ~NativeCommandBatch() {
        wait_for_submitted();
        // Unsubmitted commands are deliberately abandoned after encoding or
        // admission failure. Their admitted work is not completed work.
        // Earlier flushed chunks may have modified private buffers: failure
        // forbids publication, not all intermediate device writes.
    }

    void flush() {
        using metal_exact::gpu::require;
        Operation operation{*this};
        require(!failed_,"Native command batch already failed");
        if(!pending_dispatches_){operation.succeeded=true;return;}
        poll_();
        [command_ commit];
        [command_ waitUntilCompleted];
        auto command=command_;
        const auto dispatches=pending_dispatches_;
        const auto invocations=pending_invocations_;
        discard_unsubmitted();
        require(command.status==MTLCommandBufferStatusCompleted,
                command.error?command.error.description.UTF8String:"Native grouped command failed");
        const double ms=(command.GPUEndTime-command.GPUStartTime)*1000;
        require(command.GPUStartTime>0&&command.GPUEndTime>=command.GPUStartTime&&
                std::isfinite(ms)&&ms>=0,"Native grouped command timestamp missing");
        ++statistics_.completed_commands;
        statistics_.completed_dispatches+=dispatches;
        statistics_.completed_invocations+=invocations;
        statistics_.gpu_ms+=ms;metal_.noteCommand(ms);
        statistics_.max_command_ms=std::max(statistics_.max_command_ms,ms);
        metal_.afterDispatch();
        poll_();
        operation.succeeded=true;
    }

    void encode(id<MTLComputePipelineState> pipeline,std::uint32_t count,
                const std::function<void(id<MTLComputeCommandEncoder>)>& bind,
                std::uint32_t group_width=0) {
        using metal_exact::gpu::require;
        Operation operation{*this};
        require(!failed_,"Native command batch already failed");
        require(pipeline!=nil&&count>0&&count<=limits_.invocations&&bool(bind),
                "Native grouped dispatch arguments");
        const auto width=group_width?group_width:pipeline.threadExecutionWidth;
        require(width>0&&width<=pipeline.maxTotalThreadsPerThreadgroup,"Native grouped dispatch width");
        poll_();
        if(pending_dispatches_&&
           (pending_dispatches_==limits_.encoders||pending_invocations_+count>limits_.invocations))flush();
        metal_.beforeDispatch(count);
        if(command_==nil)command_=[metal_.queue commandBuffer];
        require(command_!=nil,"Cannot create Native grouped command");
        auto encoder=[command_ computeCommandEncoder];
        require(encoder!=nil,"Cannot encode Native grouped command");
        try {
            [encoder setComputePipelineState:pipeline];
            // The closure executes now. setBytes snapshots this dispatch's
            // values; no reference to a changing launch variable is deferred.
            bind(encoder);
            [encoder dispatchThreads:MTLSizeMake(count,1,1)
                threadsPerThreadgroup:MTLSizeMake(width,1,1)];
            [encoder endEncoding];
        } catch(...) {
            [encoder endEncoding];
            discard_unsubmitted();
            throw;
        }
        ++pending_dispatches_;
        pending_invocations_+=count;
        poll_(); // Encoding cancellation point, not a between-kernel CPU poll.
        if(pending_dispatches_==limits_.encoders||pending_invocations_==limits_.invocations)flush();
        operation.succeeded=true;
    }
    const NativeCommandStatistics& statistics()const{return statistics_;}
};
template<class Body>
NativeCommandStatistics RunNativeCommandSegment(metal_exact::gpu::Metal& metal,
    const std::function<void()>& poll,NativeCommandLimits limits,Body&& body) {
    NativeCommandBatch batch(metal,poll,limits);
    body(batch);
    batch.flush();
    return batch.statistics();
}
}
