#include "replaced.h"

namespace Vaev::Layout {

struct ReplacedFormatingContext : public FormatingContext {
    Output run(Tree &tree, Box &box, Input input, [[maybe_unused]] usize startAt, [[maybe_unused]] Opt<usize> stopAt) override {
        Vec2Px size = {};

        if (auto image = box.content.is<Karm::Image::Picture>()) {
            size = image->bound().size().cast<Px>();
        } else {
            panic("unsupported replaced content");
        }

        if (tree.fc.allowBreak() and not tree.fc.acceptsFit(
                                         input.position.y,
                                         size.y,
                                         input.pendingVerticalSizes
                                     )) {
            return {
                .size = {},
                .completelyLaidOut = false,
                .breakpoint = Breakpoint::buildOverflow()
            };
        }

        return {
            .size = size,
            .completelyLaidOut = true,
        };
    }
};

Strong<FormatingContext> constructReplacedFormatingContext(Box &) {
    return makeStrong<ReplacedFormatingContext>();
}

} // namespace Vaev::Layout
