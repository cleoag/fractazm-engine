#pragma once
// Private host lifecycle only. No public ABI and no numerical continuation
// layout. One mutating driver owns this object; asynchronous callers enqueue
// requests to that driver, rather than mutating GPU state directly.
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <thread>

namespace mb3d::native_metal::continuation {
inline constexpr uint32_t Version=1;
inline constexpr uint32_t MaxAllowance=1u<<20;
using Identity=std::array<uint64_t,8>;
struct Token { Identity identity{}; uint64_t sequence=0; uint32_t version=Version; };
enum class State { Ready, Reserved, Running, Yielded, Paused, Cancelling, Completed, Cancelled, Failed };
enum class Reason { None, Invalid, Stale, Busy, Terminal, Deadline, Memory, Device, Cancelled };
struct Decision { bool accepted; Reason reason; };
class Control {
    const std::thread::id owner_=std::this_thread::get_id();
    void check_owner() const {if(std::this_thread::get_id()!=owner_)throw std::logic_error("Continuation driver thread mismatch");}
    Identity identity_;
    uint64_t deadline_, sequence_=0, submitted_=0, completed_=0;
    State state_=State::Ready;
    Reason reason_=Reason::None;
    bool disposal_=false;
    bool terminal() const {return state_==State::Completed||state_==State::Cancelled||state_==State::Failed;}
    void fail(Reason reason) {state_=State::Failed;reason_=reason;}
public:
    Control(Identity identity,uint64_t deadline,uint64_t retained,uint64_t incoming,uint64_t budget)
        :identity_(identity),deadline_(deadline) {
        if(!deadline)fail(Reason::Invalid);
        else if(retained>budget||incoming>budget-retained)fail(Reason::Memory);
    }
    Control(const Control&)=delete;
    Control& operator=(const Control&)=delete;
    Token token() const {check_owner();return {identity_,sequence_};}
    State state() const {check_owner();return state_;}
    Reason reason() const {check_owner();return reason_;}
    uint64_t submitted() const {check_owner();return submitted_;}
    uint64_t completed() const {check_owner();return completed_;}
    bool releasable() const {check_owner();return disposal_&&state_!=State::Running&&state_!=State::Cancelling;}
    Decision reserve(const Token& t,uint32_t allowance,uint64_t now) {
        check_owner();
        // Foreign/duplicate requests must not poison the actual live job.
        if(t.version!=Version)return {false,Reason::Invalid};
        if(t.identity!=identity_||t.sequence!=sequence_)return {false,Reason::Stale};
        if(!allowance||allowance>MaxAllowance)return {false,Reason::Invalid};
        if(terminal()||disposal_)return {false,Reason::Terminal};
        if(state_==State::Reserved||state_==State::Running||state_==State::Cancelling)return {false,Reason::Busy};
        if(now>=deadline_){fail(Reason::Deadline);return {false,Reason::Deadline};}
        if(sequence_==std::numeric_limits<uint64_t>::max()){fail(Reason::Invalid);return {false,Reason::Invalid};}
        state_=State::Reserved;return {true,Reason::None};
    }
    // Called by the same driver immediately before commit. No control callback
    // or asynchronous mutation may intervene between this and the real commit.
    Decision submit(uint64_t now) {
        check_owner();
        if(state_!=State::Reserved)return {false,Reason::Invalid};
        if(now>=deadline_){fail(Reason::Deadline);return {false,Reason::Deadline};}
        state_=State::Running;++submitted_;return {true,Reason::None};
    }
    Decision pause() {
        check_owner();
        if(state_!=State::Ready&&state_!=State::Yielded)return {false,Reason::Busy};
        state_=State::Paused;return {true,Reason::None};
    }
    void cancel() {
        check_owner();
        if(terminal())return;
        if(state_==State::Running||state_==State::Cancelling)state_=State::Cancelling;
        else {state_=State::Cancelled;reason_=Reason::Cancelled;}
    }
    void release() {check_owner();disposal_=true;cancel();}
    Decision finish(uint64_t sequence,bool device_ok,bool phase_complete,uint64_t now) {
        check_owner();
        if(sequence!=sequence_)return {false,Reason::Stale};
        if(state_!=State::Running&&state_!=State::Cancelling)return {false,Reason::Invalid};
        ++completed_;++sequence_;
        // An actual fault is never hidden by simultaneous cancellation.
        if(!device_ok)fail(Reason::Device);
        else if(now>=deadline_)fail(Reason::Deadline);
        else if(state_==State::Cancelling){state_=State::Cancelled;reason_=Reason::Cancelled;}
        else state_=phase_complete?State::Completed:State::Yielded;
        return {true,reason_};
    }
    // A commit failure before submission has no outstanding device use.
    void reject_reserved() {check_owner();if(state_==State::Reserved)fail(Reason::Device);}
};
}
