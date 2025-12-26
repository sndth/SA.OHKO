// Windows includes
#include <Windows.h>

// Project includes
#include "config.hpp"
#include "patches.hpp"
#include "version.hpp"

DWORD WINAPI
InitThread(LPVOID)
{
  try {
    auto const version = Version::Verify(Version::GameName::GTASA);

    if (version == 0x75770) {
      MessageBoxA(
        nullptr, "Unsupported game version", "One Hit Knock Out", MB_ICONERROR);
      return FALSE;
    }

    auto const path_config = Config::FindConfig();

    if (path_config.empty()) {
      MessageBoxA(
        nullptr, "SA.OHKO.json not found", "One Hit Knock Out", MB_ICONERROR);
      return FALSE;
    }

    auto const data_config = Config::LoadConfig(path_config);

    if (data_config.enable) {
      Patches::PatchHealthBar(version);
      Patches::StartOHKOThread(version);
    }
  } catch (const std::exception& e) {
    MessageBoxA(nullptr, e.what(), "One Hit Knock Out", MB_ICONERROR);
    return FALSE;
  }

  return TRUE;
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
