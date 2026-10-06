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

} // namespace dw3
