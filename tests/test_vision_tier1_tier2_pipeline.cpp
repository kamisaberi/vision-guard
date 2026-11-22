// File: /home/kami/vision-guard/tests/test_vision_tier1_tier2_pipeline.cpp

#include "vision_guard/vision_guard.hpp"
#include "sentinel/sdk/abi.hpp"
#include <xinfer/xinfer.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

namespace fs = std::filesystem;

static std::string resolve_model(const std::string &model_name)
{
    const std::vector<fs::path> paths = {
        fs::path("/home/kami/vision-guard/scenarios/vision_edge_grid/models") / model_name,
        fs::path("/home/kami/sentinel-matrix/shared/models") / model_name,
        fs::path("/etc/sentinel/models") / model_name};
    for (const auto &p : paths)
        if (fs::exists(p)) return p.string();
    return "";
}

static float run_tier2_onnx(xinfer::Engine *engine, const std::vector<uint8_t> &pkt, float tier1_score)
{
    if (!engine) return tier1_score;
    try
    {
        auto &in_t = engine->get_input_tensor("input");
        float *in_ptr = in_t.data<float>();
        const size_t n = in_t.element_count();
        for (size_t i = 0; i < n; ++i)
            in_ptr[i] = (i < pkt.size()) ? (static_cast<float>(pkt[i]) / 255.0f) : tier1_score;
        engine->infer();
        auto &out_t = engine->get_output_tensor("scores");
        const float *out_ptr = out_t.data<float>();
        const size_t m = out_t.element_count();
        if (m == n && n > 1)
        {
            double mse = 0.0;
            for (size_t i = 0; i < m; ++i)
            {
                double d = static_cast<double>(in_ptr[i]) - static_cast<double>(out_ptr[i]);
                mse += d * d;
            }
            return std::clamp(std::max(tier1_score, static_cast<float>(std::sqrt(mse / m))), 0.0f, 1.0f);
        }
        return std::clamp(std::max(tier1_score, std::abs(out_ptr[0])), 0.0f, 1.0f);
    }
    catch (...)
    {
        return tier1_score;
    }
}

int main()
{
    std::cout << "================================================================================\n";
    std::cout << "   VISION-GUARD: TIER-1 (SHM/GIGE/RTSP) vs TIER-2 (OPENVINO ONNX) PIPELINE TEST \n";
    std::cout << "================================================================================\n";

    vision_guard::core::TscClock::calibrate();
    vision_guard::pipeline::GuardEngine engine;

    std::unique_ptr<xinfer::Engine> lidar_eng;
    const std::string mpath = resolve_model("lidar_spatial_detector.onnx");
    if (!mpath.empty())
    {
        lidar_eng = std::make_unique<xinfer::Engine>(xinfer::Target::OpenVINO);
        lidar_eng->load_model(mpath);
        std::cout << "[+] Loaded ONNX Tier-2 Engine: " << mpath << "\n";
    }

    // Tier-1 Deterministic Drops
    vision_guard::protocols::ShmVisionHeader priv_bad{vision_guard::protocols::ShmVisionValidator::MAGIC_VIS1, 1, 1, 640, 480, 1, 0, 0, 4.5f, 12.0f, 0.05f};
    std::vector<uint8_t> buf(sizeof(priv_bad));
    std::memcpy(buf.data(), &priv_bad, sizeof(priv_bad));
    assert(engine.process_shm_vision(buf) == vision_guard::GuardStatus::REJECT_VISION_PRIVACY_RETENTION_VIOLATION);
    std::cout << "  [1.1] HIPAA Disk Retention     -> TIER-1 KERNEL_DROP\n";

    engine.reset_airgap();
    const std::vector<uint8_t> gvcp_bad = {0x42, 0x01, 0x00, 0x82, 0x00, 0x08, 0x00, 0x01};
    assert(engine.process_gige_gvcp(gvcp_bad) == vision_guard::GuardStatus::REJECT_GIGE_UNAUTHORIZED_WRITE);
    std::cout << "  [1.2] GigE Vision WRITEREG     -> TIER-1 KERNEL_DROP\n";

    // Tier-2 ONNX Evaluation
    float s_ok = run_tier2_onnx(lidar_eng.get(), std::vector<uint8_t>(64, 10), 0.05f);
    float s_bad = run_tier2_onnx(lidar_eng.get(), std::vector<uint8_t>(64, 255), 0.05f);
    assert(s_ok < 0.85f && s_bad >= 0.85f);
    std::cout << "  [2.1] Tier-2 Spatial Vision AE -> Normal: " << s_ok << " (PASS) | Anomaly: " << s_bad << " (DROP)\n";
    std::cout << ">>> RESULT: SUCCESS (ALL VISION-GUARD TIER-1 & TIER-2 INVARIANTS VERIFIED) <<<\n";
    return 0;
}
