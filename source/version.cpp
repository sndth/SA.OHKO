#include <stdexcept>

// Project includes
#include "Memory.hpp"
#include "Version.hpp"

auto
Version::Verify(GameName game) -> uint32_t
{
  switch (game) {
    case GameName::GTA3: {
      uint32_t constexpr MAGIC = 0x53E58955;

      if (MAGIC == Memory::ReadMemory<uint32_t>(0x005C1E70))
        return -0x10140; // 1.0 NoCD

      if (MAGIC == Memory::ReadMemory<uint32_t>(0x005C2130))
        return -0x10140; // 1.1 NoCD

      if (MAGIC == Memory::ReadMemory<uint32_t>(0x005C6FD0))
        return 0; // 1.1 Steam

      throw std::runtime_error("Unknown GTA III version");
    }

    case GameName::GTAVC: {
      auto const value = Memory::ReadMemory<uint8_t>(0x00608578);

      switch (value) {
        case 0x5D:
          return 0; // 1.0

        case 0x81:
          return 8; // 1.1

        case 0x5B:
          return -0xFF8; // Steam

        case 0x44:
          return -0x2FF8; // Japanese

        default:
          throw std::runtime_error("Unknown GTA Vice City version");
      }
    }

    case GameName::GTASA: {
      uint32_t constexpr SA_MAGIC = 0x94BF;

      struct Check
      {
        uint32_t addr;
        uint32_t offset;
      };

      Check constexpr checks[] = {
        { 0x0082457C, 0 },       // 1.0 US
        { 0x008245BC, 0 },       // 1.0 EU / Hoodlum / Downgrade
        { 0x008252FC, 0x2680 },  // 1.01 US
        { 0x0082533C, 0x2680 },  // 1.01 EU
        { 0x0085EC4A, 0x75130 }, // 3.0 Steam
        { 0x0085DEDA, 0x75770 }  // 1.01 Steam?
      };

      for (auto const& item : checks) {
        if (Memory::ReadMemory<uint32_t>(item.addr) == SA_MAGIC) {
          return item.offset;
        }
      }

      throw std::runtime_error("Unknown GTA San Andreas version");
    }

    default:
      throw std::runtime_error("Invalid game enum");
  }
}
