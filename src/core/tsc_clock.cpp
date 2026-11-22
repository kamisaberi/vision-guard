// File: /home/kami/vision-guard/src/core/tsc_clock.cpp

#include "vision_guard/core/tsc_clock.hpp"
#include <chrono>
#include <thread>

namespace vision_guard::core {

static double g_cycles_per_ns = 3.0;

void TscClock::calibrate() noexcept {
    const auto t0 = std::chrono::steady_clock::now();
    const uint64_t c0 = rdtsc();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const uint64_t c1 = rdtsc();
    const auto t1 = std::chrono::steady_clock::now();

    const double dt_ns = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    if (dt_ns > 0.0 && c1 > c0) {
        g_cycles_per_ns = static_cast<double>(c1 - c0) / dt_ns;
    }
}

double TscClock::cycles_to_nanoseconds(uint64_t cycles) noexcept {
    return (g_cycles_per_ns > 0.0) ? (static_cast<double>(cycles) / g_cycles_per_ns) : 0.0;
}

double TscClock::ghz() noexcept {
    return g_cycles_per_ns;
}

} // namespace vision_guard::core
