# File: /home/kami/vision-guard/apparatus_plugins/vision_conveyor_gige.py

import math
import struct


class VisionConveyorGigePlugin:
    def init_state(self, engine):
        engine.domain_state["conveyor"] = {
            "protocol": "GIGE VISION GVCP :3956",
            "belt_fps": 200,
            "ejects": 0,
            "defect_score": 0.03,
            "status": "LINE_NOMINAL_200FPS",
        }

    def decode_packet(self, engine, payload: bytes, is_attack: bool, src_ip: str = ""):
        c = engine.domain_state["conveyor"]
        if is_attack:
            c["defect_score"] = 0.93
            c["ejects"] += 1
            c["status"] = "MICRO_DEFECT_EJECTED"

    def emit_nominal_step(self, engine, elapsed: float):
        with engine.lock:
            c = engine.domain_state["conveyor"]
            if not engine.active_attack["active"]:
                c["defect_score"] = round(0.03 + abs(math.cos(elapsed * 1.7)) * 0.04, 3)
                c["status"] = "LINE_NOMINAL_200FPS"
        pkt = struct.pack(">BBHHH", 0x42, 0x01, 0x0080, 4, 1)
        engine.send_and_decode(pkt, is_attack=False)

    def emit_attack_step(self, engine, attack_type: str):
        pkt = struct.pack(">BBHHH", 0x42, 0x01, 0x0082, 8, 1)
        engine.send_and_decode(pkt, is_attack=True)

    def on_attack_triggered(self, engine, attack_type: str):
        c = engine.domain_state["conveyor"]
        c["defect_score"] = 0.93
        c["ejects"] += 1
        c["status"] = "MICRO_DEFECT_EJECTED"

    def on_attack_stopped(self, engine):
        self.init_state(engine)

    def handle_post(self, engine, path: str, req: dict):
        return None

    def get_snapshot(self, engine) -> dict:
        return {"conveyor": dict(engine.domain_state["conveyor"])}


APPARATUS_TYPES = {"vision_conveyor_gige": VisionConveyorGigePlugin}
