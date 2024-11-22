#pragma once

#include <json/value.h>
#include <string>
#include <vector>
#include "utils/result.hpp"

// TODO: namh think of the other name
struct DefaultEngineVariant {
  std::string engine;
  std::string version;
  std::string variant;

  Json::Value ToJson() const {
    Json::Value root;
    root["engine"] = engine;
    root["version"] = version;
    root["variant"] = variant;
    return root;
  }
};

// TODO: namh think of the other name
struct EngineVariantResponse {
  std::string name;
  std::string version;
  std::string engine;

  Json::Value ToJson() const {
    Json::Value root;
    root["name"] = name;
    root["version"] = version;
    root["engine"] = engine;
    return root;
  }
};

class EngineServiceI {
 public:
  virtual ~EngineServiceI() {}

  virtual cpp::result<DefaultEngineVariant, std::string>
  SetDefaultEngineVariant(const std::string& engine, const std::string& version,
                          const std::string& variant) = 0;

  virtual cpp::result<DefaultEngineVariant, std::string>
  GetDefaultEngineVariant(const std::string& engine) = 0;

  virtual cpp::result<std::vector<EngineVariantResponse>, std::string>
  GetInstalledEngineVariants(const std::string& engine) const = 0;

  virtual cpp::result<void, std::string> LoadEngine(
      const std::string& engine_name) = 0;

  virtual cpp::result<void, std::string> UnloadEngine(
      const std::string& engine_name) = 0;
};