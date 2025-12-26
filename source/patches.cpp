// Windows includes
#include <Windows.h>

// Standard includes
#include <cstring>

// Project includes
#include "config.hpp"
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
Patches::PatchHealthBar(uint32_t offset_version) -> bool
{
  uint32_t address = 0;

  if (!offset_version)
    address = 0x00589395;
  else if (0x2680 == offset_version)
    address = 0x00589B65;
  else if (0x75130 == offset_version)
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

auto
Patches::StartOHKOThread(uint32_t offset_version) -> bool
{
  auto addr = new uint32_t{ offset_version };
  auto handle =
    CreateThread(nullptr, 0, &Patches::OHKOThread, addr, 0, nullptr);

  if (!handle) {
    delete addr;
    return false;
  }

  CloseHandle(handle);
  return true;
}

auto WINAPI
Patches::OHKOThread(LPVOID parameter) -> DWORD
{
  auto address = *reinterpret_cast<uint32_t*>(parameter);
  delete reinterpret_cast<uint32_t*>(parameter);

  uint32_t player_pointer = 0;
  // uint32_t max_health_address = 0;
  // uint32_t max_armour_address = 0;

  if (address == 0x75130) {
    player_pointer = 0x00C0F890;
    // max_health_address = 0x00C0BDC8;
    // max_armour_address = 0x00C0F9E0;
  } else {
    player_pointer = address + 0x00B7CD98;
    // max_health_address = address + 0x00B793E0;
    // max_armour_address = address + 0x00B7CEE8;
  }

  while (true) {
    auto player = Memory::ReadMemory<uint32_t>(player_pointer);

    if (!player) {
      Sleep(20);
      continue;
    }

    static constexpr uint32_t k_health_offset = 0x540;
    auto health_address = uint32_t{ player + k_health_offset };

    if (Config::GetConfig().die_after_bike_fall) {
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

    // if (Memory::ReadMemory<float>(max_health_address) > 5.682f)
    //   *reinterpret_cast<float*>(max_health_address) = 5.682f;

    // if (Memory::ReadMemory<uint8_t>(max_armour_address) > 0)
    //   *reinterpret_cast<uint8_t*>(max_armour_address) = 0;

    if (Memory::ReadMemory<float>(health_address) > 1.0001f)
      *reinterpret_cast<float*>(health_address) = 1.0001f;

    if (Memory::ReadMemory<float>(armour_address) > 0.0f)
      *reinterpret_cast<float*>(armour_address) = 0.0f;

    Sleep(20);
  }

  return 0;
}
