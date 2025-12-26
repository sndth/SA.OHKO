// Windows includes
#include <Windows.h>

// Standard includes
#include <fstream>

// Project includes
#include "patches.hpp"
#include "version.hpp"

// External includes
#include <nlohmann/json.hpp>
using json = nlohmann::json;

DWORD WINAPI
InitThread(LPVOID)
{
  try {
    bool found = false;
    namespace fs = std::filesystem;

    for (auto& item : fs::recursive_directory_iterator(fs::current_path())) {
      auto& path = item.path();

      if (path.filename() != "SA.OHKO.json")
        continue;

      found = true;
      auto ifs = std::ifstream(path);
      auto json = json::parse(ifs);
      auto version = Version::Verify(Version::GameName::GTASA);

      if (version == 0x75770) {
        MessageBoxA(nullptr,
                    "Unsupported game version",
                    "One Hit Knock Out",
                    MB_ICONERROR);
        return FALSE;
      }

      bool enable = json.value("Enable", true);
      bool die_after_bike_fall = json.value("DieAfterBikeFall", true);

      if (enable) {
        Patches::PatchHealthBar(version);
        Patches::StartOHKOThread(version, die_after_bike_fall);
      }

      break;
    }

    if (!found) {
      MessageBoxA(
        nullptr, "SA.OHKO.json not found", "One Hit Knock Out", MB_ICONERROR);
      return FALSE;
    }
  } catch (const json::exception& e) {
    MessageBoxA(
      nullptr,
      ("Failed to open SA.OHKO.json: " + std::string(e.what())).c_str(),
      "One Hit Knock Out",
      MB_ICONERROR);
    return FALSE;
  }

  return 0;
}

BOOL APIENTRY
DllMain(HMODULE module, DWORD reason, LPVOID)
{
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(module);
    CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
  }

  return TRUE;
}
