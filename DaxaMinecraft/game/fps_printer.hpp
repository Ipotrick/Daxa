#pragma once

#include <chrono>
#include <iostream>

template <typename ClockT>
struct FpsPrinter {
    typename ClockT::time_point fps_prev_time;
    u64 frames_since_last = 0;

    void update(const typename ClockT::time_point &now) {
        using namespace std::literals;
        ++frames_since_last;
        if (now - fps_prev_time > 1s) {
            std::cout << "fps: " << frames_since_last << std::endl;
            frames_since_last = 0;
            fps_prev_time = now;
        }
    }
};
