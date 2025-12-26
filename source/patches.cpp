// Windows includes
#include <Windows.h>

// Standard includes
#include <cstring>

// Project includes
#include "memory.hpp"
#include "patches.hpp"
#include "vehicle.hpp"

auto
Patches::IsGameRunning() -> bool
{
  __try {
    (void)Memory::ReadMemory<uint8_t>(game_running_address_);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

auto
Patches::PatchHealthBar(uint32_t ver_offset) -> bool
{
  uint32_t address = 0;

  if (!ver_offset)
    address = 0x00589395;
  else if (0x2680 == ver_offset)
    address = 0x00589B65;
  else if (0x75130 == ver_offset)
    address = 0x00597263;

  if (!address)
    return false;

  DWORD old_protect = 0;
  DWORD tmp_old_protect = 0;
  if (!VirtualProtect(reinterpret_cast<void*>(address),
                      5,
                      PAGE_EXECUTE_READWRITE,
                      &old_protect)) {
    return false;
  }

  std::memset(reinterpret_cast<void*>(address), 0x90, 5);
  VirtualProtect(
    reinterpret_cast<void*>(address), 5, old_protect, &tmp_old_protect);
  FlushInstructionCache(
    GetCurrentProcess(), reinterpret_cast<void*>(address), 5);
  return true;
}

struct OHKOArgs
{
  uint32_t ver_offset;
  bool die_after_bike_fall;
};

auto
Patches::StartOHKOThread(uint32_t ver_offset, bool die_after_bike_fall) -> bool
{
  auto* args = new OHKOArgs{ ver_offset, die_after_bike_fall };
  auto handle =
    CreateThread(nullptr, 0, &Patches::OHKOThread, args, 0, nullptr);

  if (!handle) {
    delete args;
    return false;
  }

  CloseHandle(handle);
  return true;
}

auto WINAPI
Patches::OHKOThread(LPVOID param) -> DWORD
{
  auto args = *reinterpret_cast<OHKOArgs*>(param);
  delete reinterpret_cast<OHKOArgs*>(param);

  uint32_t max_health_address = 0;
  uint32_t max_armour_address = 0;
  uint32_t player_pointer = 0;

  if (args.ver_offset == 0x75130) {
    max_health_address = 0x00C0BDC8;
    max_armour_address = 0x00C0F9E0;
    player_pointer = 0x00C0F890;
  } else {
    max_health_address = 0x00B793E0 + args.ver_offset;
    max_armour_address = 0x00B7CEE8 + args.ver_offset;
    player_pointer = 0x00B7CD98 + args.ver_offset;
  }

  while (true) {
    auto player = Memory::ReadMemory<uint32_t>(player_pointer);

    if (!player) {
      Sleep(20);
      continue;
    }

    static constexpr uint32_t k_health_offset = 0x540;
    auto health_address = uint32_t{ player + k_health_offset };

    if (args.die_after_bike_fall) {
      static constexpr uint32_t k_vehicle_model_id_offset = 0x22;
      static constexpr float k_crash_hp_delta = 0.15f;
      static uint32_t last_vehicle = 0;
      static float last_health = 1.0001f;
      auto current_vehicle = Memory::ReadMemory<uint32_t>(0x00BA18FC);
      auto current_health = Memory::ReadMemory<float>(health_address);

      if (last_vehicle != 0 && current_vehicle == 0) {
        auto model_id = Memory::ReadMemory<uint16_t>(last_vehicle +
                                                     k_vehicle_model_id_offset);

        if (Vehicle::IsBikeModel(model_id)) {
          auto delta = last_health - current_health;

          if (delta > k_crash_hp_delta) {
            *reinterpret_cast<float*>(health_address) = 0.0f;
          }
        }
      }

      last_vehicle = current_vehicle;
      last_health = current_health;
    }

    auto armour_address = uint32_t{ player + 0x548 };

    if (Memory::ReadMemory<float>(max_health_address) > 5.682f)
      *reinterpret_cast<float*>(max_health_address) = 5.682f;

    if (Memory::ReadMemory<uint8_t>(max_armour_address) > 0)
      *reinterpret_cast<uint8_t*>(max_armour_address) = 0;

    if (Memory::ReadMemory<float>(health_address) > 1.0001f)
      *reinterpret_cast<float*>(health_address) = 1.0001f;

    if (Memory::ReadMemory<float>(armour_address) > 0.0f)
      *reinterpret_cast<float*>(armour_address) = 0.0f;

    Sleep(20);
  }

  return 0;
}
