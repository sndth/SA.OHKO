#pragma once

// Standard includes
#include <cstdint>

class Patches
{
  auto static constexpr refresh_rate_ms_ = 20;
  auto static constexpr game_running_address_ = uint32_t{ 0x00400000 };
  auto static constexpr player_state_offset_ = uint32_t{ 0x530 };
  auto static constexpr health_offset_ = uint32_t{ 0x540 };
  auto static constexpr armour_offset_ = uint32_t{ 0x548 };
  auto static constexpr max_health_target_ = 5.682f;
  auto static constexpr max_health_target_actual_ = 1.0001f;
  auto static constexpr max_armour_target_ = 0.0f;
  auto static constexpr max_health_original_ = 569.0f;
  auto static constexpr max_armour_original_ = uint8_t{ 100 };

public:
  auto static IsGameRunning() -> bool;
  auto static PatchHealthBar(uint32_t offset_version) -> bool;
  auto static StartOHKOThread(uint32_t offset_version) -> bool;

private:
  auto static WINAPI OHKOThread(LPVOID parameter) -> DWORD;
};
