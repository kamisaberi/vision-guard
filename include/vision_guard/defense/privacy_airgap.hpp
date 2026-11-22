// File: /home/kami/vision-guard/include/vision_guard/defense/privacy_airgap.hpp

#pragma once

#include <atomic>

namespace vision_guard::defense {

class PrivacyAirGap {
public:
    void trip() noexcept { tripped_.store(true, std::memory_order_release); }
    void reset() noexcept { tripped_.store(false, std::memory_order_release); }
    [[nodiscard]] bool is_tripped() const noexcept { return tripped_.load(std::memory_order_acquire); }

private:
    std::atomic<bool> tripped_{false};
};

} // namespace vision_guard::defense
