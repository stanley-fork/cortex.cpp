#include "engine_unload_cmd.h"
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace commands {
cpp::result<void, std::string> EngineUnloadCmd::Exec(
    const std::string& host, int port, const std::string& engine) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return cpp::fail("Failed to start server");
    }
  }

  auto load_engine_url = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"v1", "engines", engine, "load"},
  };
  auto load_engine_result =
      curl_utils::SimpleDeleteJson(load_engine_url.ToFullPath());
  if (load_engine_result.has_error()) {
    CTL_ERR(load_engine_result.error());
    return cpp::fail("Failed to unload engine: " + load_engine_result.error());
  }

  CLI_LOG("Engine unloaded successfully");
  return {};
}
};  // namespace commands