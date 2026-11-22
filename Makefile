# File: /home/kami/vision-guard/Makefile
# ==============================================================================
# ARYORITHM TECHNOLOGIES B.V. — AIR-GAPPED EDGE VISION & PRIVACY APPLIANCES
# Project: vision-guard (ShmRing Zero-Copy Video, GigE Vision, RTSP/ONVIF)
# Invariant: Sub-120 ns Deterministic Tier-1 Gate | $0.00 Cloud Egress
# ==============================================================================

SHELL := /bin/bash
.DEFAULT_GOAL := help

PROJECT_DIR   := $(shell pwd)
BUILD_DIR     := $(PROJECT_DIR)/build
SCENARIOS_DIR := $(PROJECT_DIR)/scenarios/vision_edge_grid
MATRIX_DIR    := /home/kami/sentinel-matrix
NEXUS_DIR     := /home/kami/sentinel-nexus
ETC_HARNESSES := /etc/sentinel/lab/harnesses
OPT_HARNESSES := /opt/sentinel-stack/harnesses
JOBS          ?= $(shell nproc)

CYAN   := \033[1;36m
GREEN  := \033[1;32m
YELLOW := \033[1;33m
WHITE  := \033[1;37m
RESET  := \033[0m

.PHONY: help
help:
	@echo -e "$(CYAN)================================================================================$(RESET)"
	@echo -e "$(WHITE)  VISION-GUARD: AIR-GAPPED EDGE VISION, GIGE, RTSP & HIPAA PRIVACY ENGINE$(RESET)"
	@echo -e "$(CYAN)================================================================================$(RESET)"
	@echo -e "  $(GREEN)make init$(RESET)         Hard copy all vision harnesses to /etc and /opt"
	@echo -e "  $(GREEN)make build$(RESET)        Compile release targets (-O3 -march=native)"
	@echo -e "  $(GREEN)make test$(RESET)         Run all unit, ABI & Tier-1/Tier-2 pipeline tests"
	@echo -e "  $(GREEN)make bench$(RESET)        Execute hardware latency benchmark (< 120 ns SLA)"
	@echo -e "  $(GREEN)make install$(RESET)      Install libraries, CLI & plugin system-wide"
	@echo -e "  $(GREEN)make sync-all$(RESET)     Full synchronization (install + sync-matrix + sync-web)"
	@echo -e "  $(GREEN)make vision-up$(RESET)    Deploy vision_edge_grid simulation mesh"
	@echo -e "  $(GREEN)make vision-down$(RESET)  Teardown vision_edge_grid"
	@echo -e "  $(GREEN)make vision-status$(RESET)Display running containers and ports"
	@echo -e "  $(GREEN)make vision-logs$(RESET)  Stream live logs from Breach Guard Sentry"
	@echo -e "$(CYAN)================================================================================$(RESET)"

.PHONY: init
init:
	@sudo mkdir -p $(ETC_HARNESSES) $(OPT_HARNESSES)
	@chmod +x harnesses/*.py 2>/dev/null || true
	@for f in harnesses/*.py; do \
		if [ -f "$$f" ]; then \
			base=$$(basename $$f); \
			hyphen_base=$$(echo $$base | tr '_' '-'); \
			sudo rm -f $(ETC_HARNESSES)/$$base $(ETC_HARNESSES)/$$hyphen_base; \
			sudo cp -f $$f $(ETC_HARNESSES)/$$base; \
			sudo cp -f $$f $(ETC_HARNESSES)/$$hyphen_base; \
			sudo rm -f $(OPT_HARNESSES)/$$base $(OPT_HARNESSES)/$$hyphen_base; \
			sudo cp -f $$f $(OPT_HARNESSES)/$$base; \
			sudo cp -f $$f $(OPT_HARNESSES)/$$hyphen_base; \
		fi \
	done
	@sudo chmod 755 $(ETC_HARNESSES)/*.py $(OPT_HARNESSES)/*.py 2>/dev/null || true
	@echo -e "$(GREEN)[+] Vision-Guard harnesses installed to $(ETC_HARNESSES) and $(OPT_HARNESSES).$(RESET)"

.PHONY: all build debug
all: build

build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)
	@echo -e "$(GREEN)[+] Build Complete: libvision_guard.so, vision_guard_dissector_plugin.so, aryo-vision.$(RESET)"

debug:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)

.PHONY: test bench
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@echo -e "$(GREEN)[+] All Vision-Guard Unit, ABI & Pipeline Tests Passed (100%).$(RESET)"

bench: build
	@$(BUILD_DIR)/bench_latency

.PHONY: install sync-matrix sync-web sync-all
install: build init
	@sudo $(MAKE) -C $(BUILD_DIR) install
	@sudo ldconfig

sync-matrix: build
	@mkdir -p $(MATRIX_DIR)/shared/lib
	@cp -f $(BUILD_DIR)/libvision_guard.so $(MATRIX_DIR)/shared/lib/

sync-web:
	@mkdir -p $(MATRIX_DIR)/web/plugins
	@if [ -d "$(NEXUS_DIR)/web" ]; then \
		cp -r $(NEXUS_DIR)/web/* $(MATRIX_DIR)/web/ 2>/dev/null || true; \
		mkdir -p $(MATRIX_DIR)/web/plugins; \
	fi

sync-all: install sync-matrix sync-web
	@echo -e "$(GREEN)[+] Full Vision-Guard Synchronization Complete.$(RESET)"

.PHONY: vision-up vision-down vision-status vision-logs
vision-up: sync-all
	@aryo vision vision-up
	@echo -e "$(GREEN)[+] Edge Vision & Privacy Grid Live: http://localhost:9443/$(RESET)"

vision-down:
	@aryo vision vision-down || true
	@docker network prune -f >/dev/null 2>&1 || true

vision-status:
	@docker ps --filter "name=matrix-" --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"

vision-logs:
	@docker logs -f matrix-edge-breach-01

.PHONY: clean distclean
clean:
	@rm -rf $(BUILD_DIR)

distclean: clean
	@$(MAKE) vision-down >/dev/null 2>&1 || true
	@rm -rf $(MATRIX_DIR)/.matrix_cache/docker-compose.vision_edge_grid.yml
