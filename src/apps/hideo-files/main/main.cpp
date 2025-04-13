#include <karm-sys/entry.h>
#include <karm-ui/app.h>

import Hideo.Files;

Async::Task<> entryPointAsync(Sys::Context& ctx) {
    co_return co_await Ui::runAsync(ctx, Hideo::Files::app());
}
