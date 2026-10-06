// The launcher's self-test (like the port's --input-test): no disc, no display needed (SDL's offscreen driver), so
// CI runs it. It checks the path helpers, the JSON writer, the settings directory's lookup order (4.2), the settings
// file's round trips (unknown members kept, a broken file kept aside, a newer schema never written), then opens the
// window and walks every screen with injected key events and a virtual gamepad, saving each screen as a PNG in
// <dir>/screens/.
#pragma once

#include <string>

namespace dw3 {

// `dir`: a scratch directory the test may fill (created when missing). True when every check passed.
bool self_test_run(const std::string &dir);

// The game's stand-in: the self-test starts this executable with SELF_TEST_GAME_ENV set to a mode, so the launch
// path (game_probe, GameRun, the interim options) is tested without the game or a disc. main() calls it first.
constexpr const char *SELF_TEST_GAME_ENV = "DW3_LAUNCHER_FAKE_GAME";
int self_test_fake_game(const char *mode, int argc, char **argv);

} // namespace dw3
