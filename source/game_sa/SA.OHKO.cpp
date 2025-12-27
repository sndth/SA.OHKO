// Standard includes
#include <fstream>

// External includes
#include <plugin.h>
#include <CMessages.h>
using namespace plugin;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct Main
{
  Main()
  {
    bool bEnable = true;
    bool bDefaultDieAfterBikeFall = true;

    try {
      auto ifs = std::ifstream("SA.OHKO.json");
      auto json = json::parse(ifs);

      bEnable = json.value("Enable", true);
      bDefaultDieAfterBikeFall = json.value("DieAfterBikeFall", true);
    } catch (...) {
      MessageBoxA(nullptr,
                  "Failed to load SA.OHKO.json",
                  "SA.OHKO config warning",
                  MB_ICONWARNING);
    }

    if (bEnable) {
      Events::gameProcessEvent += [bDefaultDieAfterBikeFall] {
        gInstance.GameProcess(bDefaultDieAfterBikeFall);
      };
    }
  }

  void PatchBikes(CPlayerPed* player)
  {
    auto const pVehicle = player->m_pVehicle;

    if (!pVehicle)
      return;

    auto const bVehicleIsBike = VEHICLE_BMX == pVehicle->m_nVehicleSubClass ||
                                VEHICLE_BIKE == pVehicle->m_nVehicleSubClass;

    if (bVehicleIsBike && player->bFallenDown) {
      player->m_fHealth = 0;
    }
  }

  void PatchHealth(CPlayerPed* player)
  {
    player->m_fHealth = 1;
    player->m_fArmour = 0;

    auto& gvm = injector::address_manager::singleton();

    /*
     * Hide health bar,
     * works only in 1.0 (US/EU)
     */
    if (gvm.IsSA() && 1 == gvm.GetMajorVersion() &&
        0 == gvm.GetMinorVersion()) {
      injector::MakeNOP(0x589395, 5);
    }
  }

  void GameProcess(bool const bDieAfterBikeFall)
  {
    auto const pPlayer = FindPlayerPed();

    if (!pPlayer)
      return;

    PatchHealth(pPlayer);

    if (bDieAfterBikeFall) {
      PatchBikes(pPlayer);
    }
  }
} gInstance;
