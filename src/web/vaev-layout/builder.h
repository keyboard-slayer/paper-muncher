#pragma once

#include "base.h"

namespace Vaev::Layout {

Box build(Style::Computer &c, Markup::Document const &doc);

Box buildForPseudoElement(Strong<Style::Computed> style);

} // namespace Vaev::Layout
