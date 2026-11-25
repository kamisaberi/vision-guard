// File: /home/kami/vision-guard/tools/aryo_vision/main.cpp

#include "vision_guard/vision_guard.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sentinel/fs/PathResolver.hpp>

static void print_help()
{
    std::cout << R"(================================================================================
ARYO VISION: AIR-GAPPED EDGE VISION, GIGE, RTSP & HIPAA PRIVACY (vision-guard)
================================================================================
Usage:
  aryo vision <action>

Actions:
  bench          Run 100,000-sample ShmVision frame verification (< 120 ns)
  verify-shm     Verify HIPAA RAM-only zero-retention & freeze guard
  verify-rtsp    Verify RTSP/ONVIF stream hijack blocking
  verify-gige    Verify GigE Vision GVCP unauthorized register write blocking
  airgap-status  Inspect real-time privacy air-gap latch state
  vision-up      Deploy vision_edge_grid simulation mesh
  vision-down    Teardown vision_edge_grid simulation mesh
  vision-status  Show running vision_edge_grid containers
  --plugin-desc  Print single-line extension metadata for master CLI
)" << std::endl;
}

static std::string locate_scenario_engine()
{
    namespace fs = std::filesystem;
    if (const char *m_root = std::getenv("MATRIX_ROOT"); m_root && *m_root)
    {
        fs::path p = fs::path(m_root) / "tools" / "scenario_engine.py";
        if (fs::exists(p))
            return p.string();
    }
    if (const char *home = std::getenv("HOME"); home && *home)
    {
        fs::path p = fs::path(home) / "sentinel-matrix" / "tools" / "scenario_engine.py";
        if (fs::exists(p))
            return p.string();
    }
    if (fs::exists("/opt/sentinel-matrix/tools/scenario_engine.py"))
        return "/opt/sentinel-matrix/tools/scenario_engine.py";
    return "scenario_engine.py";
}

static std::string resolve_cartridge_scenario(const std::string &cartridge, const std::string &slug)
{
    namespace fs = std::filesystem;
    fs::path cat_dir = sentinel::fs::PathResolver::resolve_dir(sentinel::fs::ObjectClass::ScenarioCatalog) / slug;
    if (fs::exists(cat_dir))
        return cat_dir.string();

    fs::path pack_dir = sentinel::fs::PathResolver::resolve_pack_uri("pack://" + cartridge + "/scenarios/" + slug);
    if (fs::exists(pack_dir))
        return pack_dir.string();

    return slug;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_help();
        return 0;
    }
    const std::string action = argv[1];

    if (action == "--plugin-desc")
    {
        std::cout << "[ext. vision]           aryo vision (bench, verify-shm, verify-rtsp, verify-gige, vision-up) [Air-Gapped Edge Vision Guard]";
        return 0;
    }
    if (action == "-h" || action == "--help" || action == "help")
    {
        print_help();
        return 0;
    }

    if (action == "vision-up")
    {
        const std::string cmd = "python3 \"" + locate_scenario_engine() + "\" load \"" +
                                resolve_cartridge_scenario("vision-guard", "vision_edge_grid") + "\"";
        return std::system(cmd.c_str());
    }
    if (action == "vision-down")
    {
        const std::string cmd = "python3 \"" + locate_scenario_engine() + "\" down --name vision_edge_grid";
        return std::system(cmd.c_str());
    }
    if (action == "vision-status")
    {
        const std::string cmd = "python3 \"" + locate_scenario_engine() + "\" status --name vision_edge_grid";
        return std::system(cmd.c_str());
    }
    vision_guard::core::TscClock::calibrate();
    vision_guard::pipeline::GuardEngine engine;

    if (action == "bench")
    {
        constexpr size_t ITERATIONS = 100'000;
        std::vector<double> lat_ns;
        lat_ns.reserve(ITERATIONS);

        vision_guard::protocols::ShmVisionHeader hdr{vision_guard::protocols::ShmVisionValidator::MAGIC_VIS1, 1, 1, 640, 480, 1, 1, 0, 4.5f, 12.0f, 0.05f};
        std::vector<uint8_t> buf(sizeof(hdr));

        for (size_t i = 1; i <= ITERATIONS; ++i)
        {
            hdr.frame_seq = static_cast<uint32_t>(i);
            std::memcpy(buf.data(), &hdr, sizeof(hdr));
            const uint64_t t0 = vision_guard::core::TscClock::rdtsc();
            const auto st = engine.process_shm_vision(buf);
            const uint64_t t1 = vision_guard::core::TscClock::rdtsc();
            if (st == vision_guard::GuardStatus::PASS)
                lat_ns.push_back(vision_guard::core::TscClock::cycles_to_nanoseconds(t1 - t0));
        }
        double total = 0.0;
        for (double v : lat_ns)
            total += v;
        std::cout << "[aryo-vision] Mean Latency (100k ShmVision frames): " << (total / lat_ns.size()) << " ns\n";
        return 0;
    }

    if (action == "verify-shm")
    {
        vision_guard::protocols::ShmVisionHeader bad{vision_guard::protocols::ShmVisionValidator::MAGIC_VIS1, 1, 1, 640, 480, 1, 0, 0, 4.5f, 12.0f, 0.05f};
        std::vector<uint8_t> buf(sizeof(bad));
        std::memcpy(buf.data(), &bad, sizeof(bad));
        const auto st = engine.process_shm_vision(buf);
        std::cout << "[aryo-vision] HIPAA RAM-Only Violation: " << vision_guard::status_to_string(st) << "\n";
        return (st == vision_guard::GuardStatus::REJECT_VISION_PRIVACY_RETENTION_VIOLATION) ? 0 : 1;
    }

    if (action == "verify-rtsp")
    {
        const std::string req = "TEARDOWN rtsp://10.240.80.75:8554/cam RTSP/1.0\r\n";
        const auto st = engine.process_rtsp_onvif(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(req.data()), req.size()));
        std::cout << "[aryo-vision] RTSP Hijack Result: " << vision_guard::status_to_string(st) << "\n";
        return (st == vision_guard::GuardStatus::REJECT_RTSP_STREAM_HIJACK) ? 0 : 1;
    }

    if (action == "verify-gige")
    {
        const std::vector<uint8_t> gvcp = {0x42, 0x01, 0x00, 0x82, 0x00, 0x08, 0x00, 0x01};
        const auto st = engine.process_gige_gvcp(gvcp);
        std::cout << "[aryo-vision] GigE WRITEREG Result: " << vision_guard::status_to_string(st) << "\n";
        return (st == vision_guard::GuardStatus::REJECT_GIGE_UNAUTHORIZED_WRITE) ? 0 : 1;
    }

    if (action == "airgap-status")
    {
        std::cout << "[aryo-vision] Privacy Air-Gap Tripped: " << (engine.is_airgap_tripped() ? "YES" : "NO") << "\n";
        return 0;
    }

    std::cerr << "[-] Unknown action: " << action << "\n";
    return 1;
}
