// Standard includes
#include <fstream>

// Project includes
#include "config.hpp"

// External includes
#include <nlohmann/json.hpp>
using json = nlohmann::json;

auto
Config::GetConfig() -> OHKOConfig&
{
  return m_config_;
}

auto
Config::FindConfig() -> std::filesystem::path
{
  namespace fs = std::filesystem;

  for (auto const& entry :
       fs::recursive_directory_iterator(fs::current_path())) {
    if (entry.path().filename() == "SA.OHKO.json") {
      return entry.path();
    }
  }

  return {};
}

auto
Config::LoadConfig(const std::filesystem::path& path) -> OHKOConfig
{
  auto ifs = std::ifstream(path);
  auto json = json::parse(ifs);
  auto config = OHKOConfig{};

  config.enable = json.value("Enable", true);
  config.die_after_bike_fall = json.value("DieAfterBikeFall", true);
  m_config_ = config;
  return config;
}
