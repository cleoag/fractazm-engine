#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <stdexcept>
#include <thread>

// A stalled GPU wait cannot service cooperative RenderControl callbacks. This
// process-owned watchdog therefore bounds the complete frame independently.
class FrameDeadline {
public:
    using Clock=std::chrono::steady_clock;
    const Clock::time_point started=Clock::now();
    const double limit;
    explicit FrameDeadline(double seconds):limit(seconds) {
        if(!(seconds>0&&seconds<=300))throw std::invalid_argument("frame deadline outside (0,300]");
        worker=std::thread([this]{
            std::unique_lock lock(mutex);
            if(!cv.wait_until(lock,started+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(limit)),[this]{return done;})) {
                std::fputs("FRAME_DEADLINE_EXCEEDED\n",stderr);std::fflush(stderr);std::_Exit(124);
            }
        });
    }
    double complete(Clock::time_point end) {
        const double elapsed=std::chrono::duration<double>(end-started).count();
        if(elapsed>limit){std::fputs("FRAME_DEADLINE_EXCEEDED\n",stderr);std::fflush(stderr);std::_Exit(124);}
        stop();return elapsed;
    }
    ~FrameDeadline(){stop();}
    FrameDeadline(const FrameDeadline&)=delete;
    FrameDeadline& operator=(const FrameDeadline&)=delete;
private:
    std::mutex mutex;std::condition_variable cv;bool done=false;std::thread worker;
    void stop(){ {std::lock_guard lock(mutex);done=true;}cv.notify_all();if(worker.joinable())worker.join(); }
};
