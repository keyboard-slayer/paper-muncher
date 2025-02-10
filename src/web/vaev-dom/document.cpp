#include "document.h"

#include "element.h"

namespace Vaev::Dom {

String Document::title() const {
    String res = ""s;
    iterDepthFirst([&](auto& node) {
        if (auto element = node.template is<Element>()) {
            if (element->tagName == Html::TITLE) {
                res = element->textContent();
                return Iter::BREAK;
            }
        }
        return Iter::CONTINUE;
    });
    return res;
}

} // namespace Vaev::Dom
