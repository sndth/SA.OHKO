#pragma once

// Standard includes
#include <cstdint>

class Version
{
public:
  enum GameName : uint8_t
  {
    GTA3,
    GTAVC,
    GTASA
  };

  auto static Verify(GameName game) -> uint32_t;
};
