// File: /home/kami/vision-guard/web/vision/vision.js
// ==============================================================================
// ARYORITHM VISION-GUARD UI PLUGIN:
//   1. Air-Gapped Weapon & Breach Guard (:18102)
//   2. Conveyor Micro-Defect GigE Vision Sorter (:18103)
//   3. HIPAA RAM-Only Patient Fall ToF Eye (:18104)
//   4. Edge Vision & Zero-Retention Privacy C2 Tab
// ==============================================================================

(function () {
    if (!window.SentinelUI) return;

    const HOST = window.location.hostname || "127.0.0.1";
    const BREACH_API   = `http://${HOST}:18102`;
    const CONVEYOR_API = `http://${HOST}:18103`;
    const HIPAA_API    = `http://${HOST}:18104`;

    const isVisionNode = (node) =>
        /breach|conveyor|hipaa|vision|gige/i.test(`${node?.site || ""} ${node?.hostname || ""}`);

    function mountCanvas(apiUrl, drawFn) {
        return function (container) {
            container.innerHTML = `<div style="position:relative; width:100%; height:100%; background:#05080C; border-radius:4px; overflow:hidden; aspect-ratio:1.25;"><canvas width="750" height="600" style="width:100%; height:100%; object-fit:cover; display:block;"></canvas></div>`;
            const canvas = container.querySelector("canvas");
            const state = { animId: null, alive: true, data: null, tick: 0 };
            const poll = async () => {
                try { const r = await fetch(`${apiUrl}/telemetry`, { cache: "no-store" }); if (r.ok) state.data = await r.json(); } catch (_) { }
            };
            poll();
            const timer = setInterval(poll, 200);
            function loop() { if (!state.alive) return; state.tick += 0.04; drawFn(canvas, state); state.animId = requestAnimationFrame(loop); }
            loop();
            state.cleanup = () => clearInterval(timer);
            return state;
        };
    }

    function unmountCanvas(h) { if (!h) return; h.alive = false; if (h.animId) cancelAnimationFrame(h.animId); if (h.cleanup) h.cleanup(); }

    window.SentinelUI.registerApparatus({
        id: "vision_weapon_rtsp",
        title: "AIR-GAPPED WEAPON & BREACH GUARD (RTSP 4K)",
        shortLabel: "🛡️ BREACH VISION",
        aspectRatio: 1.25,
        protocol: "RTSP :8554 / RAM-ONLY",
        matches: (n) => isVisionNode(n) && /breach/i.test(`${n?.site} ${n?.hostname}`),
        mount: mountCanvas(BREACH_API, (canvas, st) => {
            const ctx = canvas.getContext("2d"), w = canvas.width, h = canvas.height;
            const b = st.data?.breach || {};
            const alert = Boolean(st.data?.active_attack?.active);
            ctx.fillStyle = "#040710"; ctx.fillRect(0, 0, w, h);

            ctx.strokeStyle = "rgba(0,229,255,0.12)";
            for (let x = 0; x < w; x += 60) { ctx.beginPath(); ctx.moveTo(x, 30); ctx.lineTo(x, h - 30); ctx.stroke(); }
            for (let y = 30; y < h - 30; y += 60) { ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke(); }

            const bx = w / 2 + Math.sin(st.tick) * 140, by = h / 2 + Math.cos(st.tick * 0.7) * 80;
            ctx.strokeStyle = alert ? "#FF3366" : "#00FFA3"; ctx.lineWidth = 2.2;
            ctx.strokeRect(bx - 55, by - 75, 110, 150);
            ctx.font = "bold 10px monospace"; ctx.fillStyle = alert ? "#FF3366" : "#00FFA3";
            ctx.fillText(alert ? "ROI: THREAT [0.96]" : `ROI: CLEAR [${b.confidence || 0.04}]`, bx - 52, by - 82);

            ctx.fillStyle = "rgba(7,11,20,0.88)"; ctx.fillRect(0, 0, w, 28); ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace"; ctx.fillStyle = alert ? "#FF3366" : "#00FFA3";
            ctx.fillText(alert ? `[!] WEAPON / STREAM BREACH DETECTED | CLOUD EGRESS: $0.00` : `[+] AIR-GAPPED 4K EDGE VISION | 30 FPS | CLOUD EGRESS: $0.00`, 12, 18);
            ctx.font = "9px monospace"; ctx.fillStyle = "#94A3B8";
            ctx.fillText(`LOCAL RAM INFERENCE | ALERTS: ${b.alerts_triggered || 0}`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({ apiBase: BREACH_API, attacks: [{ label: "[!] Trigger Weapon / RTSP Breach Event", pcap: "", attack_type: "breach", mode: "overlay", duration_sec: 15 }] })
    });

    window.SentinelUI.registerApparatus({
        id: "vision_conveyor_gige",
        title: "CONVEYOR MICRO-DEFECT SORTER (GIGE VISION)",
        shortLabel: "🏭 GIGE SORTER",
        aspectRatio: 1.25,
        protocol: "GIGE GVCP :3956 / 200 FPS",
        matches: (n) => isVisionNode(n) && /conveyor|gige/i.test(`${n?.site} ${n?.hostname}`),
        mount: mountCanvas(CONVEYOR_API, (canvas, st) => {
            const ctx = canvas.getContext("2d"), w = canvas.width, h = canvas.height;
            const c = st.data?.conveyor || {};
            const alert = Boolean(st.data?.active_attack?.active);
            ctx.fillStyle = "#040810"; ctx.fillRect(0, 0, w, h);

            ctx.fillStyle = "#0F172A"; ctx.fillRect(0, h / 2 - 60, w, 120);
            for (let i = 0; i < 6; i++) {
                const ix = ((i * 140 + st.tick * 160) % (w + 100)) - 50;
                const isBad = alert && i === 2;
                ctx.fillStyle = isBad ? "rgba(255,51,102,0.7)" : "rgba(0,229,255,0.45)";
                ctx.fillRect(ix, h / 2 - 28, 48, 56);
                ctx.strokeStyle = isBad ? "#FF3366" : "#00FFA3";
                ctx.strokeRect(ix, h / 2 - 28, 48, 56);
            }

            ctx.fillStyle = "rgba(7,11,20,0.88)"; ctx.fillRect(0, 0, w, 28); ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace"; ctx.fillStyle = alert ? "#FF3366" : "#00E5FF";
            ctx.fillText(alert ? `[!] MICRO-DEFECT / GVCP WRITE BLOCKED | EJECTOR FIRED` : `[+] GIGE VISION SORTER | 200 FPS | DEFECT SCORE: ${c.defect_score || 0.03}`, 12, 18);
            ctx.font = "9px monospace"; ctx.fillStyle = "#94A3B8";
            ctx.fillText(`GVCP :3956 WRITE-LOCK ARMED | EJECTS: ${c.ejects || 0}`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({ apiBase: CONVEYOR_API, attacks: [{ label: "[!] Inject Micro-Defect / GVCP Tamper", pcap: "", attack_type: "defect", mode: "overlay", duration_sec: 15 }] })
    });

    window.SentinelUI.registerApparatus({
        id: "vision_hipaa_fall_shm",
        title: "HIPAA RAM-ONLY PATIENT FALL TOF EYE",
        shortLabel: "🏥 HIPAA SHM-TOF",
        aspectRatio: 1.25,
        protocol: "POSIX /dev/shm (0B DISK)",
        matches: (n) => isVisionNode(n) && /hipaa/i.test(`${n?.site} ${n?.hostname}`),
        mount: mountCanvas(HIPAA_API, (canvas, st) => {
            const ctx = canvas.getContext("2d"), w = canvas.width, h = canvas.height, cx = w / 2, cy = h / 2;
            const hp = st.data?.hipaa || {};
            const alert = Boolean(st.data?.active_attack?.active);
            ctx.fillStyle = "#040712"; ctx.fillRect(0, 0, w, h);

            ctx.strokeStyle = alert ? "#FF3366" : "#A855F7"; ctx.lineWidth = 2.5;
            if (alert) {
                ctx.strokeRect(cx - 110, cy + 40, 220, 48); // Horizontal posture (fall!)
            } else {
                ctx.strokeRect(cx - 35, cy - 110, 70, 210); // Upright posture
            }

            ctx.fillStyle = "rgba(7,11,20,0.88)"; ctx.fillRect(0, 0, w, 28); ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace"; ctx.fillStyle = alert ? "#FF3366" : "#00FFA3";
            ctx.fillText(alert ? `[!] PATIENT FALL ALERT (SCORE ${hp.fall_score}) | RAW FRAME PURGED` : `[+] HIPAA TOF EYE | ENTROPY: ${hp.entropy || 4.5} | DISK RETENTION: 0 BYTES`, 12, 18);
            ctx.font = "9px monospace"; ctx.fillStyle = "#94A3B8";
            ctx.fillText(`BUS C: /dev/shm RING | RAM FRAMES PURGED: ${hp.ram_purged_frames || 0}`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({ apiBase: HIPAA_API, attacks: [{ label: "[!] Simulate Patient Fall Anomaly", pcap: "", attack_type: "fall", mode: "overlay", duration_sec: 15 }] })
    });

    let c2Timer = null;
    function renderVisionC2(container) {
        if (c2Timer) clearInterval(c2Timer);
        container.innerHTML = `
            <div style="display:flex; flex-direction:column; gap:14px;">
                <div class="vis-c2-bar">
                    <span style="font-size:12px; font-weight:800; color:#00E5FF;">VISION-GUARD: AIR-GAPPED EDGE VISION &amp; HIPAA PRIVACY C2</span>
                    <span class="badge badge-green" style="font-size:9px;">$0.00 CLOUD EGRESS | 0B DISK RETENTION</span>
                </div>
                <div class="vis-c2-grid">
                    <div id="vis-card-breach" class="vis-c2-card">
                        <div style="color:#00FFA3; font-weight:800;">🛡️ WEAPON &amp; BREACH (:18102)</div>
                        <div id="vis-breach-kpis">Connecting...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="vis-c2-btn danger" data-v-atk="breach">TRIGGER BREACH</button>
                            <button class="vis-c2-btn" data-v-stop="breach">RESTORE</button>
                        </div>
                    </div>
                    <div id="vis-card-conv" class="vis-c2-card">
                        <div style="color:#00E5FF; font-weight:800;">🏭 GIGE SORTER (:18103)</div>
                        <div id="vis-conv-kpis">Connecting...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="vis-c2-btn danger" data-v-atk="conv">INJECT DEFECT</button>
                            <button class="vis-c2-btn" data-v-stop="conv">RESTORE</button>
                        </div>
                    </div>
                    <div id="vis-card-hipaa" class="vis-c2-card">
                        <div style="color:#A855F7; font-weight:800;">🏥 HIPAA SHM-TOF (:18104)</div>
                        <div id="vis-hipaa-kpis">Connecting...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="vis-c2-btn danger" data-v-atk="hipaa">TRIGGER FALL</button>
                            <button class="vis-c2-btn" data-v-stop="hipaa">RESTORE</button>
                        </div>
                    </div>
                </div>
            </div>
        `;
        const eps = {
            breach: { url: BREACH_API,   body: { attack_type: "breach", mode: "overlay", duration_sec: 15 } },
            conv:   { url: CONVEYOR_API, body: { attack_type: "defect", mode: "overlay", duration_sec: 15 } },
            hipaa:  { url: HIPAA_API,    body: { attack_type: "fall",   mode: "overlay", duration_sec: 15 } }
        };
        container.querySelectorAll("[data-v-atk]").forEach(b => b.addEventListener("click", () => {
            const c = eps[b.getAttribute("data-v-atk")];
            if (c) fetch(`${c.url}/attack/inject`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(c.body) }).catch(() => {});
        }));
        container.querySelectorAll("[data-v-stop]").forEach(b => b.addEventListener("click", () => {
            const c = eps[b.getAttribute("data-v-stop")];
            if (c) fetch(`${c.url}/attack/stop`, { method: "POST" }).catch(() => {});
        }));
        async function refresh() {
            const b = await fetch(`${BREACH_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            const c = await fetch(`${CONVEYOR_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            const h = await fetch(`${HIPAA_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            if (b?.breach)   document.getElementById("vis-breach-kpis").innerHTML = `<div class="vis-c2-kpi"><span>Confidence</span><b>${b.breach.confidence}</b></div><div class="vis-c2-kpi"><span>Status</span><b>${b.breach.status}</b></div>`;
            if (c?.conveyor) document.getElementById("vis-conv-kpis").innerHTML   = `<div class="vis-c2-kpi"><span>Defect</span><b>${c.conveyor.defect_score}</b></div><div class="vis-c2-kpi"><span>Status</span><b>${c.conveyor.status}</b></div>`;
            if (h?.hipaa)    document.getElementById("vis-hipaa-kpis").innerHTML  = `<div class="vis-c2-kpi"><span>Purged Frames</span><b>${h.hipaa.ram_purged_frames}</b></div><div class="vis-c2-kpi"><span>Status</span><b>${h.hipaa.status}</b></div>`;
        }
        refresh();
        c2Timer = setInterval(refresh, 500);
    }

    window.SentinelUI.registerTab({ id: "tab-vision-c2", label: "Edge Vision & Privacy C2", icon: "👁️", group: "Operations", renderView: renderVisionC2 });
})();
