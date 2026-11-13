#include "render/native_metal/continuation_control.h"
#include <cassert>
#include <iostream>
using namespace mb3d::native_metal::continuation;
int main(){
 Identity key{1,2,3,4,5,6,7,8};
 Control c(key,100,10,20,30);auto t=c.token();
 assert(c.state()==State::Ready);
 auto wrong_version=t;wrong_version.version=0;assert(!c.reserve(wrong_version,1,1).accepted);
 bool rejected=false;std::thread other([&]{try{c.cancel();}catch(const std::logic_error&){rejected=true;}});other.join();assert(rejected&&c.state()==State::Ready);
 assert(!c.reserve(t,0,1).accepted&&c.state()==State::Ready);
 assert(!c.reserve(t,MaxAllowance+1,1).accepted);
 for(unsigned i=0;i<key.size();++i){auto foreign=t;++foreign.identity[i];assert(!c.reserve(foreign,1,1).accepted);assert(c.state()==State::Ready);}
 assert(c.pause().accepted);assert(c.reserve(t,1,1).accepted);
 assert(!c.reserve(t,1,1).accepted);assert(c.submit(2).accepted);
 assert(!c.pause().accepted);assert(!c.submit(2).accepted);
 assert(!c.finish(1,true,true,3).accepted&&c.state()==State::Running);
 assert(c.finish(0,true,false,3).accepted&&c.state()==State::Yielded);
 assert(!c.finish(0,true,true,4).accepted);
 assert(!c.reserve(t,1,4).accepted);t=c.token();assert(t.sequence==1);
 assert(c.reserve(t,1,4).accepted&&c.submit(5).accepted);
 c.release();assert(!c.releasable());assert(c.state()==State::Cancelling);
 assert(c.finish(1,true,true,6).accepted&&c.state()==State::Cancelled);
 assert(c.releasable()&&c.submitted()==2&&c.completed()==2);
 assert(!c.reserve(c.token(),1,7).accepted);
 Control fault(key,100,0,1,1);assert(fault.reserve(fault.token(),1,1).accepted);assert(fault.submit(2).accepted);
 fault.cancel();assert(fault.finish(0,false,true,3).accepted);assert(fault.reason()==Reason::Device);
 Control complete(key,100,0,1,1);assert(complete.reserve(complete.token(),1,1).accepted);assert(complete.submit(2).accepted);
 assert(complete.finish(0,true,true,3).accepted);complete.cancel();assert(complete.state()==State::Completed);
 Control reserved(key,100,0,1,1);assert(reserved.reserve(reserved.token(),1,1).accepted);reserved.cancel();assert(!reserved.submit(2).accepted);assert(reserved.submitted()==0);
 Control timeout(key,10,0,1,1);assert(timeout.pause().accepted);assert(!timeout.reserve(timeout.token(),1,10).accepted);assert(timeout.reason()==Reason::Deadline);
 Control commit_timeout(key,10,0,1,1);assert(commit_timeout.reserve(commit_timeout.token(),1,9).accepted);assert(!commit_timeout.submit(10).accepted);
 Control in_flight(key,10,0,1,1);assert(in_flight.reserve(in_flight.token(),1,1).accepted);assert(in_flight.submit(2).accepted);in_flight.release();assert(!in_flight.releasable());assert(in_flight.finish(0,true,true,11).accepted&&in_flight.reason()==Reason::Deadline);assert(in_flight.releasable());
 Control overflow(key,100,UINT64_MAX,1,UINT64_MAX);assert(overflow.reason()==Reason::Memory);
 Control fail_before_submit(key,100,0,1,1);assert(fail_before_submit.reserve(fail_before_submit.token(),1,1).accepted);fail_before_submit.reject_reserved();assert(fail_before_submit.state()==State::Failed&&fail_before_submit.submitted()==0);
 std::cout<<"PASS: lifecycle, 8 identity lanes, sequencing, cancellation/fault races, deadlines, memory overflow\n";
}
