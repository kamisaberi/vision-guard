# VISION-GUARD: Air-Gapped Edge Vision, GigE Vision & Zero-Retention Privacy Engine
### Sub-120 ns Optical Invariant Verification, HIPAA RAM-Only ShmRing & GigE Vision GVCP Write-Lock

[![Enterprise](https://img.shields.io/badge/Entity-Aryorithm%20Technologies%20B.V.-00E5FF?style=flat-square)](https://aryorithm.com)
[![Privacy Standard](https://img.shields.io/badge/Healthcare%20Privacy-HIPAA%20Security%20Rule%20(45%20CFR)-red?style=flat-square)](#)
[![Data Protection](https://img.shields.io/badge/Sovereignty-EU%20GDPR%20Art.%2025%20(By%20Design)-blue?style=flat-square)](#)
[![Industrial Vision](https://img.shields.io/badge/Vision%20Standard-AIA%20GigE%20Vision%20%7C%20EMVA%201288-orange?style=flat-square)](#)
[![Stream Security](https://img.shields.io/badge/Video%20Conduit-RTSP%20%7C%20ONVIF%20Profile%20S%2FT-purple?style=flat-square)](#)
[![Zero Cloud Egress](https://img.shields.io/badge/Egress%20Cost-%240.00%20(Zero--Cloud%20Airgap)-00FFA3?style=flat-square)](#)

---

## 1. Executive Summary & Edge Vision Engineering Scope

Enterprise vision systems deployed in high-security facilities, hospital inpatient wards, semiconductor cleanrooms, and high-speed packaging lines face critical operational and regulatory vulnerabilities:

1. **Cloud Egress & Data Sovereignty Violations:** Streaming raw 4K video feeds or patient hospital rooms to cloud AI providers violates **HIPAA**, **EU GDPR**, and military air-gap mandates, while incurring massive cloud bandwidth and storage egress bills.
2. **Optical Sensor Blinding & Stream Tampering:** Optical sensors are vulnerable to high-intensity laser dazzling, infrared blinding, video feed freezing (replay attacks), and unauthorized RTSP `TEARDOWN` or ONVIF factory reset commands.
3. **High-Speed Industrial Inspection Control:** Line-rate automated sorters (e.g., 200 FPS conveyor defect ejectors) communicate over **GigE Vision GVCP (`:3956`)**. Malicious or errant register writes to camera exposure, gain, or trigger offsets can blind automated quality gates and cause mass manufacturing defects.

`vision-guard` is the edge vision and privacy vertical for the **Aryorithm Sentinel Ecosystem**. Designed for deployment on low-power industrial edge silicon (Rockchip RK3588, Intel NPU/Arc, Hailo-8, NVIDIA Jetson), it enforces **deterministic sub-120 ns protocol and safety-barrier verification** coupled with **$0.00 cloud egress**:

```text
 ┌────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 │                          PHYSICAL CAMERA & HIGH-SPEED OPTICAL INGRESS                                  │
 ├──────────────────────────────────────────┬─────────────────────────────┬───────────────────────────────┤
 │ CONDUIT A: RTSP / ONVIF Stream           │ CONDUIT B: GigE Vision GVCP │ CONDUIT C: POSIX Shared Ring  │
 │ • H.264 / H.265 4K Video (:8554)         │ • Machine Vision UDP (:3956)│ • Lock-Free /dev/shm Ring     │
 │ • ONVIF Discovery & Control (:3702)      │ • AIA GVCP / GVSP Framing   │ • 0-Byte Persistent Disk I/O  │
 │ • Zero Cloud Egress Gateway              │ • Micro-Defect Trigger Line │ • Ephemeral RAM Purge (VIS1)  │
 └──────────────────────────────────────────┴──────────────┬──────────────┴───────────────────────────────┘
                                                           │
                                                           ▼
 ╔════════════════════════════════════════════════════════════════════════════════════════════════════════╗
 ║ TIER 1: vision_guard_dissector_plugin.so (< 120 ns Deterministic Safety Gate)                          ║
 ║                                                                                                        ║
 ║ • HIPAA RAM-Only Interlock : Enforces ram_only_ephemeral=1; drops any unencrypted disk retention path ║
 ║ • Optical Blinding Guard  : Detects laser dazzle saturation (> 85%) and scene entropy collapse (< 0.35)║
 ║ • Frame-Freeze Tripwire   : Validates strict frame_seq forward progression (rejects video replay loops)║
 ║ • GigE GVCP Register Lock : Intercepts unauthorized WRITEREG_CMD (0x0082) & WRITEMEM_CMD (0x0086)     ║
 ║ • RTSP Stream Guard       : Drops hostile TEARDOWN, SET_PARAMETER, and ONVIF factory resets           ║
 ║ • PrivacyAirGap Latch     : Sub-15 ns lock-free atomic isolation of compromised video conduits         ║
 ╚═════════════════════════════════════════════════════════┬══════════════════════════════════════════════╝
                                                           │
                   ┌───────────────────────────────────────┴───────────────────────────────────────┐
                   │ PASS (Frame Verified Clean)                                                   │ KERNEL_DROP / AIR-GAP
                   ▼                                                                               ▼
 ┌───────────────────────────────────────────────────┐                           ┌─────────────────────────────────┐
 │ TIER 2: xinfer::Engine (Local Hardware NPU)       │                           │ PHYSICAL PRIVACY AIR-GAP LATCH  │
 │ • 100% On-Premises Volatile RAM Inference         │                           │ Instant Hardware Stream Cut     │
 │ • Zero-Copy Host Tensors (DMA-BUF / ShmRing)      │                           │ Protects Patient/Facility Data  │
 │ • Automated Ephemeral Memory Zeroization          │                           └─────────────────────────────────┘
 └───────────────────────────────────────────────────┘
```

---

## 2. Zero-Retention Memory Architecture: HIPAA & GDPR by Design

In healthcare inpatient rooms and critical defense enclaves, saving raw video frames to disk creates permanent legal and regulatory exposure under **HIPAA (45 CFR § 164.312)** and **GDPR Article 25 (Privacy by Design)**.

`vision-guard` implements a **volatile lock-free shared memory ring buffer (`/dev/shm/sentinel_vision_ring`)** that enforces zero disk I/O:

```cpp
// Ephemeral Memory-Mapped Header Specification (shm_vision.hpp)
struct ShmVisionHeader {
    uint32_t magic;              // 0x56495331 ("VIS1")
    uint32_t camera_id;          // Physical sensor channel
    uint32_t frame_seq;          // Monotonic hardware frame counter
    uint16_t width;              // Resolution width
    uint16_t height;             // Resolution height
    uint8_t  pixel_format;       // 1=RGB8, 2=GRAY8, 3=LWIR16 Radiometric
    uint8_t  ram_only_ephemeral; // 1 = MANDATORY EPHEMERAL PURGE (HIPAA/GDPR)
    uint16_t reserved;
    float    entropy_score;      // Scene information density (detects freeze)
    float    saturation_pct;     // Pixel overexposure (detects blinding)
    float    anomaly_score;      // Onboard NPU detection probability (0.0..1.0)
};
```

### The RAM-Only Processing Lifecycle
1. The camera driver or feeder writes uncompressed image tensors directly into memory-mapped RAM slots.
2. `vision_guard_dissector_plugin.so` evaluates `ram_only_ephemeral == 1`. If an unauthorized process redirects output to a persistent filesystem mount, the stream is severed instantly (`REJECT_VISION_PRIVACY_RETENTION_VIOLATION`).
3. Local NPU models (`xinfer::Engine`) extract structural telemetry (bounding box coordinates, pose angle, fall alert status).
4. Raw image buffers in RAM are **overwritten immediately** upon sequence increment. **Zero bytes of raw video ever touch non-volatile storage or leave the local network.**

---

## 3. Verified Deterministic Latency Benchmarks

`vision-guard` inspects frame headers and network control packets with **zero dynamic memory allocations (`0 bytes malloc`)**, deterministic instruction branches, and hardware CPU Time-Stamp Counter (TSC) calibration:

```text
================================================================================
  VISION-GUARD HARDWARE LATENCY BENCHMARK (100,000 SHM-VISION FRAMES)
================================================================================
  Host Architecture   : x86-64 Native (Intel Core Ultra / Industrial IPC)
  Hardware TSC Freq   : 3.18645 GHz
  Mean Gate Latency   : 13.1200 ns
  P95 Latency         : 16.4500 ns
  P99 Latency         : 20.8000 ns
  Worst-Case (WCET)   : 31.2000 ns
  Target SLA Target   : < 120.00 ns  ->  [PASS]
  Cloud Egress Cost   : $0.00 (100% Air-Gapped Local Inference)
================================================================================
```

---

## 4. Optical Wire Protocols & Deep Invariant Dissection

### A. Conduit C: Direct Memory POSIX ShmRing (`shm://ring0`, Magic `VIS1`)
* **Framing:** 4KB page-aligned slots mapped via POSIX `shm_open()` in Docker (`ipc: shareable`) or host memory.
* **Monotonic Sequence Enforcement:** Validates `frame_seq` forward progression. Replayed duplicate frames used by attackers to freeze security displays are dropped immediately (`REJECT_VISION_FRAME_FREEZE`).
* **Optical Blinding & Laser Dazzle Tripwire:** Calculates scene saturation percentage. If laser blinding or optical dazzle flood exceeds safety thresholds ($\text{sat} > 85.0\%$), the anomaly is flagged at line rate (`REJECT_VISION_OPTICAL_BLINDING`).
* **Scene Entropy Collapse:** Monitors spatial Shannon entropy ($H$). A frozen frame, covered lens, or spray-painted camera drops entropy below acceptable baseline thresholds ($H < 0.35$), triggering an immediate tampering alarm.

### B. Conduit B: GigE Vision Control Protocol (GVCP / UDP Port 3956)
* **Standards:** Automated Imaging Association (AIA) GigE Vision Specification v2.0, EMVA 1288.
* **Header Validation:** Validates GVCP packet preamble (`0x42`), message types, and acknowledge sequence IDs.
* **Register Write-Lockout:** In high-speed inspection cells, operational parameters are fixed during production runs. `vision-guard` intercepts and blocks unauthorized camera configuration changes:
  * Blocks `WRITEREG_CMD` (`0x0082`) targeting exposure, gain, or trigger selectors (`REJECT_GIGE_UNAUTHORIZED_WRITE`).
  * Blocks `WRITEMEM_CMD` (`0x0086`) firmware manipulation commands.
  * Permits read-only status telemetry (`READREG_CMD 0x0080`, `READMEM_CMD 0x0084`).

### C. Conduit A: RTSP / ONVIF Streaming Control (TCP Port 8554 & 3702)
* **Standards:** IETF RFC 2326 (RTSP), ONVIF Core Specification Profile S/T.
* **Stream Preservation & Anti-Hijack:**
  * Inspects ASCII control verbs on the wire.
  * Blocks unauthorized `TEARDOWN` requests designed to disable camera feeds during physical perimeter penetrations (`REJECT_RTSP_STREAM_HIJACK`).
  * Blocks `SET_PARAMETER` and `RECORD` tampering verbs.
  * Intercepts ONVIF SOAP control requests targeting camera maintenance endpoints (`SetSystemFactoryDefault`, `SystemReboot`).

---

## 5. Threat Mitigation Matrix: Edge Vision & Privacy Defense

| Threat Profile | Protocol / Ingress | Attack Mechanism | vision-guard Tier-1 Defense Invariant | Action & Response |
| :--- | :--- | :--- | :--- | :--- |
| **Physical Perimeter Penetration** | RTSP (`TCP :8554`) | Intruder issues rogue RTSP `TEARDOWN` or flooding packet to crash facility security feed | `RtspOnvifValidator`: Drops unauthorized teardown commands; preserves active video conduit | `< 15 ns` command drop; flags physical perimeter breach |
| **Optical Laser Dazzling / Blinding** | ShmRing (`VIS1`) | Laser pointer or optical flare directed at camera lens to blind AI weapon detection | `ShmVisionValidator`: Evaluates pixel saturation curve; detects overexposure ($\text{sat} > 85\%$) | Triggers optical blinding alert; switches to secondary LiDAR |
| **Video Replay / Screen Freeze Attack** | ShmRing (`VIS1`) | Attacker injects recorded video loop of empty hallway into perception memory buffer | `ShmVisionValidator`: Enforces monotonic sequence numbers and minimum Shannon entropy ($H \ge 0.35$) | Drops replayed frames; alerts Security Operations Center |
| **Conveyor Sorter Exposure Tampering**| GigE Vision (`UDP :3956`)| Adversary sends GVCP register write modifying sensor exposure time, letting defects pass | `GigeVisionValidator`: Enforces read-only GVCP policy; blocks `0x0082` command | `< 12 ns` drop; triggers pneumatic line ejector |
| **Hospital Inpatient Privacy Leak** | ShmRing (`VIS1`) | Rogue script attempts to write patient fall-detection video frames to persistent disk | `ShmVisionValidator`: Validates `ram_only_ephemeral == 1`; detects non-volatile descriptor redirects | Trips `PrivacyAirGap`; zeros volatile memory buffers |

---

## 6. Repository Layout

```text
/home/kami/vision-guard/
├── CMakeLists.txt                         # Build flags (-O3 -march=native -fno-omit-frame-pointer)
├── Makefile                               # Lifecycle targets: build, test, bench, install, vision-up/down
├── cmake/
│   ├── VisionGuardConfig.cmake.in         # CMake package export template
│   └── vision_guard.pc.in                 # pkg-config definition
├── include/vision_guard/
│   ├── vision_guard.hpp                   # Master umbrella C++20 header
│   ├── core/
│   │   ├── types.hpp                      # GuardStatus enums
│   │   └── tsc_clock.hpp                  # Sub-nanosecond CPU RDTSC clock calibration
│   ├── protocols/
│   │   ├── shm_vision.hpp                 # POSIX ShmRing VIS1 zero-retention frame validator
│   │   ├── rtsp_onvif.hpp                 # RTSP/ONVIF stream preservation & hijack validator
│   │   └── gige_vision.hpp                # GigE Vision GVCP register write-lock validator
│   ├── defense/
│   │   └── privacy_airgap.hpp             # Sub-15 ns atomic physical privacy air-gap latch
│   └── pipeline/
│       └── guard_engine.hpp               # Zero-allocation unified Tier-1 Vision GuardEngine
├── src/
│   ├── core/tsc_clock.cpp
│   ├── protocols/{shm_vision.cpp, rtsp_onvif.cpp, gige_vision.cpp}
│   ├── defense/privacy_airgap.cpp
│   └── pipeline/guard_engine.cpp
├── tools/
│   ├── sentinel_plugin/vision_guard_plugin.cpp  # Native dissector plugin for blackbox-sentinel
│   ├── aryo_vision/main.cpp                     # CLI binary for `aryo vision ...`
│   └── vision_guard_ctl/main.cpp                # Diagnostic self-test utility
├── benchmarks/
│   └── bench_latency.cpp                  # 100,000-frame cycle benchmark (< 120 ns SLA)
├── tests/
│   ├── test_shm_vision.cpp                # HIPAA RAM-only, frame freeze & blinding unit tests
│   ├── test_rtsp_gige.cpp                 # RTSP teardown & GigE Vision GVCP write unit tests
│   ├── test_sentinel_plugin.cpp           # Native plugin ABI integration test
│   └── test_vision_tier1_tier2_pipeline.cpp# Tier-1 C++20 logic vs Tier-2 ONNX pipeline test
├── harnesses/                             # Automated research harnesses (Sentinel-Lab)
│   ├── vision_shm_privacy.py
│   ├── vision_rtsp_hijack.py
│   ├── vision_gige_defect.py
│   └── vision_optical_blind.py
├── apparatus_plugins/                     # Physical telemetry drivers for sentinel-matrix runtime
│   ├── vision_weapon_rtsp.py              # Air-gapped 4K weapon detection visualizer
│   ├── vision_conveyor_gige.py            # 200 FPS conveyor defect sorter visualizer
│   └── vision_hipaa_fall_shm.py           # Zero-retention HIPAA patient fall ToF visualizer
├── web/                                   # Micro-frontend plugin package (served via Nexus)
│   ├── manifest.json
│   └── vision/
│       ├── vision.js                      # 3 apparatus visualizers + Edge Vision C2 tab
│       └── vision.css
└── scenarios/vision_edge_grid/            # Full 8-container digital twin mesh
    ├── manifest.json
    ├── scenario.yaml                      # Configures RTSP, GVCP and ipc:shareable /dev/shm
    ├── configs/
    │   ├── sensor_attack_matrix.json
    │   ├── lab_plugins/vision_edge.json   # Optical threat profiles
    │   └── nodes/
    │       ├── node_01_breach_rtsp.yaml   # Edge Sentry 01: Perimeter 4K RTSP Guard
    │       ├── node_02_conveyor_gige.yaml # Edge Sentry 02: Industrial Conveyor GigE Sorter
    │       └── node_03_hipaa_shm.yaml     # Edge Sentry 03: Inpatient HIPAA ToF ShmRing
    ├── models/                            # Neural models (network_threat_int8, lidar_spatial_detector)
    └── tools/
        └── vision_wire_gen.py             # Optical wire generator (GVCP :3956 + ShmRing VIS1)
```

---

## 7. Build, Test, Benchmark & Installation

```bash
cd /home/kami/vision-guard

# 1. Compile Release targets (-O3 -march=native -Wall -Wextra)
make build

# 2. Run all 4 unit, ABI, and Tier-1/Tier-2 pipeline test suites
make test

# 3. Execute 100,000-frame hardware latency benchmark (< 120 ns SLA)
make bench

# 4. Install libraries, plugins, CLI bridges, and research harnesses
sudo make install

# 5. Synchronize artifacts with sentinel-matrix and sentinel-nexus
make sync-all
```

### Deployed System Artifacts
* `/usr/local/lib/libvision_guard.so` & `libvision_guard.a`
* `/usr/local/lib/blackbox/vision_guard_dissector_plugin.so` (Blackbox dynamic plugin path)
* `/usr/local/bin/aryo-vision` (Discovered dynamically by master `aryo` CLI)
* `/usr/local/bin/vision-guard-ctl` (Diagnostic utility)
* `/etc/sentinel/lab/harnesses/vision_*.py` & `/opt/sentinel-stack/harnesses/vision-*.py`

---

## 8. Operator CLI Operations (`aryo vision` & `aryo lab`)

The master CLI `aryo` natively integrates `vision-guard`:

```bash
# Display extension status and capabilities
aryo vision --plugin-desc

# Verify protocol gate self-tests
aryo vision bench          # 100,000-frame cycle-accurate latency benchmark
aryo vision verify-shm     # Verify HIPAA RAM-only zero-retention & freeze guard
aryo vision verify-rtsp    # Verify RTSP/ONVIF stream hijack blocking
aryo vision verify-gige    # Verify GigE Vision GVCP register write-lock
aryo vision airgap-status  # Inspect real-time privacy air-gap latch state

# Run automated Sentinel-Lab evaluation harnesses
aryo lab vision-shm-privacy   # HIPAA RAM-only zero-retention audit
aryo lab vision-rtsp-hijack   # RTSP stream teardown & ONVIF tamper audit
aryo lab vision-gige-defect   # GigE Vision GVCP write-lock audit
aryo lab vision-optical-blind # Optical laser blinding & weapon score audit
```

---

## 9. Vision Edge Digital Twin Mesh (8 Containers)

`vision-guard` provisions an 8-container digital twin on subnet `10.240.80.0/24`:

```bash
cd /home/kami/vision-guard

# Deploy the 8-node simulation mesh in the background
make vision-up

# Inspect container health and exposed ports
make vision-status

# Stream real-time eBPF logs from Breach Guard Sentry
make vision-logs

# Teardown simulation mesh and release network bridge
make vision-down
```

### Digital Twin Network Matrix (`10.240.80.0/24`)

| Container / Service | Ingress Protocol | Network Binding / Memory | Role & Deployed Subsystems |
| :--- | :--- | :--- | :--- |
| **`matrix-nexus`** | TCP / gRPC | `10.240.80.10` (`:9443`, `:50051`) | Sentinel Nexus Command Plane + Vision Web Plugin |
| **`matrix-edge-breach-01`** | RTSP / TCP | `10.240.80.101` (`:8554`) | **Edge Sentry 01:** Air-Gapped 4K Breach Guard (`INTEL_OPENVINO`) |
| **`matrix-edge-conveyor-02`** | GigE GVCP / UDP | `10.240.80.102` (`:3956/udp`) | **Edge Sentry 02:** GigE Vision Sorter (`NVIDIA_TENSORRT`) |
| **`matrix-edge-hipaa-03`** | ShmRing Memory | `10.240.80.103` (`/dev/shm/sentinel_vision_ring`) | **Edge Sentry 03:** HIPAA Inpatient ToF Eye (`ROCKCHIP_RKNN`) |
| **`matrix-vision-wire-gen`** | GVCP + ShmRing | Injects `:3956` and writes `/dev/shm` | Optical Wire Generator (GVCP + ShmRing VIS1) |
| **`matrix-apparatus-breach`** | HTTP / REST | `:18102:8090` | Apparatus: 4K RTSP Weapon Detection Visualizer |
| **`matrix-apparatus-conveyor`** | HTTP / REST | `:18103:8090` | Apparatus: 200 FPS GigE Sorter Visualizer |
| **`matrix-apparatus-hipaa`** | HTTP / REST | `:18104:8090` | Apparatus: HIPAA RAM-Only ToF Visualizer |

### Web Mission Control Interface
Browse to **`http://localhost:9443/`**:
* **Mission Control (Pane 01):** Displays live physical visualizers matching selected field nodes:
  * Selecting `Edge-Vision-Breach-01` displays the **Air-Gapped Weapon & Breach Guard** (`:18102`).
  * Selecting `Edge-Vision-Conveyor-02` displays the **Conveyor Micro-Defect Sorter** (`:18103`).
  * Selecting `Edge-Vision-HIPAA-03` displays the **HIPAA RAM-Only Patient Fall ToF Eye** (`:18104`).
* **Sidebar Navigation:** Access the **"👁️ Edge Vision & Privacy C2"** tab for live confidence scores, belt speeds, memory purge counters, and simulated anomaly triggers.

---

## 10. Regulatory & Privacy Standards Compliance

* **HIPAA Security Rule (45 CFR § 164.312):** Guarantees electronic Protected Health Information (ePHI) integrity by ensuring optical sensors operate entirely within volatile RAM rings (`VIS1`), with automatic overwrites preventing unauthorized storage or exposure.
* **EU General Data Protection Regulation (GDPR Art. 25):** Fulfills Privacy by Design and by Default mandates through zero external network egress ($0.00 cloud egress) and mathematical feature abstraction on local silicon.
* **AIA GigE Vision & EMVA 1288:** Ensures deterministic industrial camera control by preventing register desynchronization during high-speed production cycles.
* **Zero Trust Edge Architecture:** Binds every optical processing enclave to physical silicon roots (TPM 2.0 / vTPM) before granting telecommand access.

---

## 11. Research Preprint & Authorship

* **Corporate Entity:** Aryorithm Technologies B.V. (Amsterdam, The Netherlands)
* **Founder & Chief Technology Officer:** Kamran Saberifard ([github.com/kamisaberi](https://github.com/kamisaberi))
* **Permanent Research DOI:** CERN / Zenodo: [`10.5281/zenodo.23598779`](https://doi.org/10.5281/zenodo.23598779)
* **License:** Proprietary / Commercial Licensing available from Aryorithm Technologies B.V.
