#include <karm-io/expr.h>

#include "emit.h"
#include "fmt.h"

namespace Karm::Io {

static auto const RE_SEP =
    Re::single(' ', '\t', '.', '_', '/', '-');

using CaseFn = auto(Rune, usize, usize) -> Rune;

Res<usize> _changeCase(SScan &s, Io::TextWriter &w, CaseFn fn) {
    bool wasLower = false;

    usize si = 0, wi = 0;
    usize written = 0;

    s.eat(RE_SEP);

    while (not s.ended()) {
        if (s.eat(RE_SEP)) {
            auto sep = fn(' ', si, wi);
            if (sep)
                written += try$(w.writeRune(sep));
            wi = 0;
            wasLower = false;
        } else {
            if (wasLower and isAsciiUpper(s.peek())) {
                auto sep = fn(' ', si, wi);
                if (sep)
                    written += try$(w.writeRune(sep));
                wi = 0;
            }

            wasLower = isAsciiLower(s.peek()) and isAsciiAlpha(s.peek());
            written += try$(w.writeRune(fn(s.next(), si, wi)));
            si++;
            wi++;
        }
    }

    return Ok(written);
}

Res<String> _changeCase(Str str, CaseFn fn) {
    Io::StringWriter writer;
    Io::SScan scan{str};
    try$(_changeCase(scan, writer, fn));
    return Ok(writer.take());
}

// MARK: Case functions --------------------------------------------------------

// si: string index
// wi: word index

Rune _toNoCase(Rune rune, usize, usize) {
    return toAsciiLower(rune);
}

Rune _toPascalCase(Rune rune, usize, usize wi) {
    if (rune == ' ')
        return '\0';

    if (wi == 0)
        return toAsciiUpper(rune);

    return toAsciiLower(rune);
}

Rune _toCamelCase(Rune rune, usize si, usize wi) {
    if (si == 0)
        return toAsciiLower(rune);

    return _toPascalCase(rune, si, wi);
}

Rune _toTitleCase(Rune rune, usize, usize wi) {
    if (wi == 0)
        return toAsciiUpper(rune);
    return toAsciiLower(rune);
}

Rune _toConstantCase(Rune rune, usize, usize) {
    if (rune == ' ')
        return '_';
    return toAsciiUpper(rune);
}

Rune _toDotCase(Rune rune, usize, usize) {
    if (rune == ' ')
        return '.';
    return rune;
}

Rune _toHeaderCase(Rune rune, usize, usize wi) {
    if (rune == ' ')
        return '-';
    if (wi == 0)
        return toAsciiUpper(rune);
    return toAsciiLower(rune);
}

Rune _toParamCase(Rune rune, usize, usize) {
    if (rune == ' ')
        return '-';
    return toAsciiLower(rune);
}

Rune _toPathCase(Rune rune, usize, usize) {
    if (rune == ' ')
        return '/';
    return rune;
}

Rune _toSentenceCase(Rune rune, usize si, usize) {
    if (si == 0)
        return toAsciiUpper(rune);
    return toAsciiLower(rune);
}

Rune _toSnakeCase(Rune rune, usize, usize) {
    if (rune == ' ')
        return '_';
    return toAsciiLower(rune);
}

Rune _toSwapCase(Rune rune, usize, usize) {
    if (isAsciiLower(rune))
        return toAsciiUpper(rune);
    if (isAsciiUpper(rune))
        return toAsciiLower(rune);
    return rune;
}

Rune _toLowerCase(Rune rune, usize, usize) {
    return toAsciiLower(rune);
}

Rune _toLowerFirstCase(Rune rune, usize si, usize) {
    if (si == 0)
        return toAsciiLower(rune);

    return rune;
}

Rune _toUpperCase(Rune rune, usize, usize) {
    return toAsciiUpper(rune);
}

Rune _toUpperFirstCase(Rune rune, usize si, usize) {
    if (si == 0)
        return toAsciiUpper(rune);
    return rune;
}

Rune _toSpongeCase(Rune rune, usize si, usize) {
    if (si % 2 == 0)
        return toAsciiUpper(rune);
    return toAsciiLower(rune);
}

// MARK: Public API ------------------------------------------------------------

Res<String> toDefaultCase(Str str) {
    return Ok(str);
}

Res<String> toCamelCase(Str str) {
    return _changeCase(str, _toCamelCase);
}

Res<String> toCapitalCase(Str str) {
    return _changeCase(str, _toTitleCase);
}

Res<String> toConstantCase(Str str) {
    return _changeCase(str, _toConstantCase);
}

Res<String> toDotCase(Str str) {
    return _changeCase(str, _toDotCase);
}

Res<String> toHeaderCase(Str str) {
    return _changeCase(str, _toHeaderCase);
}

Res<String> toNoCase(Str str) {
    return _changeCase(str, _toNoCase);
}

Res<String> toParamCase(Str str) {
    return _changeCase(str, _toParamCase);
}

Res<String> toPascalCase(Str str) {
    return _changeCase(str, _toPascalCase);
}

Res<String> toPathCase(Str str) {
    return _changeCase(str, _toPathCase);
}

Res<String> toSentenceCase(Str str) {
    return _changeCase(str, _toSentenceCase);
}

Res<String> toSnakeCase(Str str) {
    return _changeCase(str, _toSnakeCase);
}

Res<String> toTitleCase(Str str) {
    return _changeCase(str, _toTitleCase);
}

Res<String> toSwapCase(Str str) {
    return _changeCase(str, _toSwapCase);
}

Res<String> toLowerCase(Str str) {
    return _changeCase(str, _toLowerCase);
}

Res<String> toLowerFirstCase(Str str) {
    return _changeCase(str, _toLowerFirstCase);
}

Res<String> toUpperCase(Str str) {
    return _changeCase(str, _toUpperCase);
}

Res<String> toUpperFirstCase(Str str) {
    return _changeCase(str, _toUpperFirstCase);
}

Res<String> toSpongeCase(Str str) {
    return _changeCase(str, _toSpongeCase);
}

Res<String> changeCase(Str str, Case toCase) {
    switch (toCase) {
    case Case::CAMEL:
        return toCamelCase(str);

    case Case::CAPITAL:
        return toCapitalCase(str);

    case Case::CONSTANT:
        return toConstantCase(str);

    case Case::DOT:
        return toDotCase(str);

    case Case::HEADER:
        return toHeaderCase(str);

    case Case::NO:
        return toNoCase(str);

    case Case::PARAM:
        return toParamCase(str);

    case Case::PASCAL:
        return toPascalCase(str);

    case Case::PATH:
        return toPathCase(str);

    case Case::SENTENCE:
        return toSentenceCase(str);

    case Case::SNAKE:
        return toSnakeCase(str);

    case Case::TITLE:
        return toTitleCase(str);

    case Case::SWAP:
        return toSwapCase(str);

    case Case::LOWER:
        return toLowerCase(str);

    case Case::LOWER_FIRST:
        return toLowerFirstCase(str);

    case Case::UPPER:
        return toUpperCase(str);

    case Case::UPPER_FIRST:
        return toUpperFirstCase(str);

    case Case::SPONGE:
        return toSpongeCase(str);

    default:
        return toDefaultCase(str);
    }
}

} // namespace Karm::Io
