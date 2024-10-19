#include "run_cmd.h"
#include "chat_completion_cmd.h"
#include "config/yaml_config.h"
#include "cortex_upd_cmd.h"
#include "database/models.h"
#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/logging_utils.h"

namespace commands {

namespace {
std::string Repo2Engine(const std::string& r) {
  if (r == kLlamaRepo) {
    return kLlamaEngine;
  } else if (r == kOnnxRepo) {
    return kOnnxEngine;
  } else if (r == kTrtLlmRepo) {
    return kTrtLlmEngine;
  }
  return r;
};
}  // namespace

void RunCmd::Exec(bool run_detach) {
  std::optional<std::string> model_id = model_handle_;

  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  auto address = host_ + ":" + std::to_string(port_);

  // Download model if it does not exist
  {
    auto related_models_ids = modellist_handler.FindRelatedModel(model_handle_);
    if (related_models_ids.has_error() || related_models_ids.value().empty()) {
      auto result = model_service_.DownloadModel(model_handle_);
      model_id = result.value();
      CTL_INF("model_id: " << model_id.value());
    } else if (related_models_ids.value().size() == 1) {
      model_id = related_models_ids.value().front();
    } else {  // multiple models with nearly same name found
      auto selection = cli_selection_utils::PrintSelection(
          related_models_ids.value(), "Local Models: (press enter to select)");
      if (!selection.has_value()) {
        return;
      }
      model_id = selection.value();
      CLI_LOG("Selected: " << selection.value());
    }
  }

  try {
    namespace fs = std::filesystem;
    namespace fmu = file_manager_utils;
    auto model_entry = modellist_handler.GetModelInfo(*model_id);
    if (model_entry.has_error()) {
      CLI_LOG("Error: " + model_entry.error());
      return;
    }
    yaml_handler.ModelConfigFromFile(
        fmu::ToAbsoluteCortexDataPath(
            fs::path(model_entry.value().path_to_model_yaml))
            .string());
    auto mc = yaml_handler.GetModelConfig();

    // Check if engine existed. If not, download it
    {
      auto required_engine =
          engine_service_.GetEngineInfo(Repo2Engine(mc.engine));

      if (!required_engine.has_value()) {
        throw std::runtime_error("Engine not found: " + mc.engine);
      }
      if (required_engine.value().status == EngineService::kIncompatible) {
        throw std::runtime_error("Engine " + mc.engine + " is incompatible");
      }
      if (required_engine.value().status == EngineService::kNotInstalled) {
        auto install_engine_result = engine_service_.InstallEngine(mc.engine);
        if (install_engine_result.has_error()) {
          throw std::runtime_error(install_engine_result.error());
        }
      }
    }

    // Start server if it is not running
    {
      if (!commands::IsServerAlive(host_, port_)) {
        CLI_LOG("Starting server ...");
        commands::ServerStartCmd ssc;
        if (!ssc.Exec(host_, port_)) {
          return;
        }
      }
    }

    // Always start model if not llamacpp
    // If it is llamacpp, then check model status first
    {
      if ((mc.engine.find(kLlamaRepo) == std::string::npos &&
           mc.engine.find(kLlamaEngine) == std::string::npos) ||
          !commands::ModelStatusCmd(model_service_)
               .IsLoaded(host_, port_, *model_id)) {

        auto result = model_service_.StartModel(host_, port_, *model_id);
        if (result.has_error()) {
          CLI_LOG("Error: " + result.error());
          return;
        }
        if (!result.value()) {
          CLI_LOG("Error: Failed to start model");
          return;
        }
      }
    }

    // Chat
    if (run_detach) {
      CLI_LOG(*model_id << " model started successfully. Use `"
                        << commands::GetCortexBinary() << " chat " << *model_id
                        << "` for interactive chat shell");
    } else {
      ChatCompletionCmd(model_service_).Exec(host_, port_, *model_id, mc, "");
    }
  } catch (const std::exception& e) {
    CLI_LOG("Fail to run model with ID '" + model_handle_ + "': " + e.what());
  }
}
};  // namespace commands
