#pragma once

#include <karm-ui/node.h>
#include <vaev-markup/dom.h>

namespace Vaev::Browser {

Ui::Child app(Mime::Url url, Res<Strong<Vaev::Markup::Document>> dom);

} // namespace Vaev::Browser
