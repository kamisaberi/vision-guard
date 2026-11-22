# File: /home/kami/vision-guard/apparatus_plugins/vision_weapon_rtsp.py

import math
import time


class VisionWeaponRtspPlugin:
    def init_state(self, engine):
        engine.domain_state["breach"] = {
            "stream": "RTSP / H.265 4K",
            "fps": 30.0,
            "confidence": 0.04,
            "alerts_triggered": 0,
            "cloud_egress_bytes": 0,
            "status": "PERIMETER_CLEAR",
        }

    def decode_packet(self, engine, payload: bytes, is_attack: bool, src_ip: str = ""):
        b = engine.domain_state["breach"]
        if is_attack:
            b["confidence"] = 0.96
            b["alerts_triggered"] += 1
            b["status"] = "CONCEALED_WEAPON_ALERT"

    def emit_nominal_step(self, engine, elapsed: float):
        with engine.lock:
            b = engine.domain_state["breach"]
            if not engine.active_attack["active"]:
                b["confidence"] = round(0.04 + abs(math.sin(elapsed)) * 0.03, 3)
                b["status"] = "PERIMETER_CLEAR"
        engine.send_and_decode(b"PLAY rtsp://10.240.80.101:8554/gate01 RTSP/1.0\r\n", is_attack=False)

    def emit_attack_step(self, engine, attack_type: str):
        engine.send_and_decode(b"TEARDOWN rtsp://10.240.80.101:8554/gate01 RTSP/1.0\r\n", is_attack=True)

    def on_attack_triggered(self, engine, attack_type: str):
        b = engine.domain_state["breach"]
        b["confidence"] = 0.96
        b["alerts_triggered"] += 1
        b["status"] = "CONCEALED_WEAPON_ALERT"

    def on_attack_stopped(self, engine):
        self.init_state(engine)

    def handle_post(self, engine, path: str, req: dict):
        return None

    def get_snapshot(self, engine) -> dict:
        return {"breach": dict(engine.domain_state["breach"])}


APPARATUS_TYPES = {"vision_weapon_rtsp": VisionWeaponRtspPlugin}
