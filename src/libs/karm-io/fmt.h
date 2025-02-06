#pragma once

#include <karm-base/backtrace.h>
#include <karm-base/box.h>
#include <karm-base/cow.h>
#include <karm-base/endian.h>
#include <karm-base/enum.h>
#include <karm-base/map.h>
#include <karm-base/rc.h>
#include <karm-base/time.h>
#include <karm-base/tuple.h>
#include <karm-base/vec.h>
#include <karm-io/aton.h>
#include <karm-io/impls.h>
#include <karm-io/sscan.h>
#include <karm-io/traits.h>
#include <karm-meta/signess.h>

namespace Karm::Io {

template <typename T>
struct Formatter;

struct _Args {
    virtual ~_Args() = default;
    virtual usize len() = 0;
    virtual Res<usize> format(Io::SScan& scan, Io::TextWriter& writer, usize index) = 0;
};

template <typename... Ts>
struct Args : public _Args {
    Tuple<Ts...> _tuple{};

    Args(Ts&&... ts) : _tuple(std::forward<Ts>(ts)...) {}

    usize len() override {
        return _tuple.len();
    }

    Res<usize> format(Io::SScan& scan, Io::TextWriter& writer, usize index) override {
        Res<usize> result = Error::invalidData("format index out of range");
        usize i = 0;
        _tuple.visit([&](auto const& t) {
            if (index == i) {
                using U = Meta::RemoveConstVolatileRef<decltype(t)>;
                Formatter<U> formatter;
                if constexpr (requires() {
                                  formatter.parse(scan);
                              }) {
                    formatter.parse(scan);
                }
                result = formatter.format(writer, t);
            }
            i++;

            return true;
        });
        return result;
    }
};

Res<usize> _format(Io::TextWriter& writer, Str format, _Args& args);

inline Res<usize> format(Io::TextWriter& writer, Str format) {
    return writer.writeStr(format);
}

template <typename... Ts>
inline Res<usize> format(Io::TextWriter& writer, Str format, Ts&&... ts) {
    Args<Ts...> args{std::forward<Ts>(ts)...};
    return _format(writer, format, args);
}

inline Res<String> format(Str format) {
    return Ok(format);
}

template <typename... Ts>
inline Res<String> format(Str format, Ts&&... ts) {
    Io::StringWriter writer{};
    Args<Ts...> args{std::forward<Ts>(ts)...};
    try$(_format(writer, format, args));
    return Ok(writer.take());
}

template <typename T>
inline Res<String> toStr(T const& t, Str format = "") {
    Io::StringWriter writer{};
    Formatter<T> formatter;
    if constexpr (requires(Io::SScan& scan) {
                      formatter.parse(scan);
                  }) {
        Io::SScan scan{format};
        formatter.parse(scan);
    }
    try$(formatter.format(writer, t));
    return Ok(writer.take());
}

// MARK: Align Formatting ------------------------------------------------------

enum struct Align {
    LEFT,
    RIGHT,
    CENTER,
};

template <typename T>
struct Aligned {
    T _inner;
    Align _align;
    usize _width;
};

inline auto aligned(auto inner, Align align, usize width) {
    return Aligned<decltype(inner)>{inner, align, width};
}

template <typename T>
struct Formatter<Aligned<T>> {
    Formatter<T> _innerFmt{};

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          _innerFmt.parse(scan);
                      }) {
            _innerFmt.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Aligned<T> val) {
        Io::StringWriter buf;
        try$(_innerFmt.format(buf, val._inner));
        usize width = buf.len();
        usize result = 0;

        if (width < val._width) {
            usize pad = val._width - width;
            switch (val._align) {
            case Align::LEFT:
                result += try$(writer.writeStr(buf.str()));
                for (usize i = 0; i < pad; i++)
                    result += try$(writer.writeRune(' '));
                break;
            case Align::RIGHT:
                for (usize i = 0; i < pad; i++)
                    result += try$(writer.writeRune(' '));
                result += try$(writer.writeStr(buf.str()));
                break;
            case Align::CENTER:
                for (usize i = 0; i < pad / 2; i++)
                    result += try$(writer.writeRune(' '));
                result += try$(writer.writeStr(buf.str()));
                for (usize i = 0; i < pad / 2; i++)
                    result += try$(writer.writeRune(' '));
                break;
            }
        } else {
            result += try$(writer.writeStr(buf.str()));
        }

        return Ok(result);
    }
};

// MARK: Case Formatting -------------------------------------------------------

enum struct Case {
    DEFAULT,
    CAMEL,
    CAPITAL,
    CONSTANT,
    DOT,
    HEADER,
    NO,
    PARAM,
    PASCAL,
    PATH,
    SENTENCE,
    SNAKE,
    TITLE,
    SWAP,
    LOWER,
    LOWER_FIRST,
    UPPER,
    UPPER_FIRST,
    SPONGE,
};

Res<String> toDefaultCase(Str str);

Res<String> toCamelCase(Str str);

Res<String> toCapitalCase(Str str);

Res<String> toConstantCase(Str str);

Res<String> toDotCase(Str str);

Res<String> toHeaderCase(Str str);

Res<String> toNoCase(Str str);

Res<String> toParamCase(Str str);

Res<String> toPascalCase(Str str);

Res<String> toPathCase(Str str);

Res<String> toSentenceCase(Str str);

Res<String> toSnakeCase(Str str);

Res<String> toTitleCase(Str str);

Res<String> toSwapCase(Str str);

Res<String> toLowerCase(Str str);

Res<String> toLowerFirstCase(Str str);

Res<String> toUpperCase(Str str);

Res<String> toUpperFirstCase(Str str);

Res<String> toSpongeCase(Str str);

Res<String> changeCase(Str str, Case toCase);

template <typename T>
struct Cased {
    T _inner;
    Case _case;
};

inline auto cased(auto inner, Case cased) {
    return Cased<decltype(inner)>{inner, cased};
}

template <typename T>
struct Formatter<Cased<T>> {
    Formatter<T> _innerFmt{};

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          _innerFmt.parse(scan);
                      }) {
            _innerFmt.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Cased<T> val) {
        Io::StringWriter sw;
        try$(_innerFmt.format(sw, val._inner));
        String result = try$(changeCase(sw.str(), val._case));
        return writer.writeStr(result.str());
    }
};

// MARK: Number Formatting -----------------------------------------------------

struct NumberFormatter {
    bool prefix = false;
    bool isChar = false;
    usize base = 10;
    usize width = 0;
    char fillChar = ' ';
    bool trailingZeros = false;
    usize precision = 6;

    Str formatPrefix();

    void parse(Str str);

    void parse(Io::SScan& scan);

    Res<usize> formatUnsigned(Io::TextWriter& writer, usize val);

    Res<usize> formatSigned(Io::TextWriter& writer, isize val);

    Res<usize> formatFloat(Io::TextWriter& writer, f64 val);

    Res<usize> formatRune(Io::TextWriter& writer, Rune val);
};

template <Meta::UnsignedIntegral T>
struct Formatter<T> : public NumberFormatter {
    Res<usize> format(Io::TextWriter& writer, T const& val) {
        if (isChar)
            return formatRune(writer, val);
        return formatUnsigned(writer, val);
    }
};

template <Meta::SignedIntegral T>
struct Formatter<T> : public NumberFormatter {
    Res<usize> format(Io::TextWriter& writer, T const& val) {
        if (isChar)
            return writer.writeRune(val);
        return formatSigned(writer, val);
    }
};

template <Meta::Float T>
struct Formatter<T> : public NumberFormatter {
    Res<usize> format(Io::TextWriter& writer, f64 const& val) {
        return formatFloat(writer, val);
    }
};

template <typename T>
struct Formatter<Be<T>> : public Formatter<T> {
    Res<usize> format(Io::TextWriter& writer, Be<T> const& val) {
        return Formatter<T>::format(writer, val.value());
    }
};

template <typename T>
struct Formatter<Le<T>> : public Formatter<T> {
    Res<usize> format(Io::TextWriter& writer, Le<T> const& val) {
        return Formatter<T>::format(writer, val.value());
    }
};

// MARK: Format Enums ----------------------------------------------------------

template <Meta::Enum T>
struct Formatter<T> {
    Res<usize> format(Io::TextWriter& writer, T val) {
        if constexpr (BoundedEnum<T>) {
            return writer.writeStr(nameOf<T>(val));
        } else {
            return Io::format(writer, "({} {})", nameOf<T>(), toUnderlyingType(val));
        }
    }
};

// MARK: Format Pointers -------------------------------------------------------

template <typename T>
struct Formatter<T*> {
    bool prefix = false;

    void parse(Io::SScan& scan) {
        prefix = scan.skip('#');
    }

    Res<usize> format(Io::TextWriter& writer, T* val) {
        usize written = 0;

        if (prefix) {
            written += try$(writer.writeRune('('));
            written += try$(writer.writeStr(nameOf<T>()));
            written += try$(writer.writeStr(" *)"s));
        }

        if (val) {
            NumberFormatter fmt;
            fmt.base = 16;
            fmt.fillChar = '0';
            fmt.width = sizeof(T*) * 2;
            fmt.prefix = true;
            written += try$(fmt.formatUnsigned(writer, (usize)val));
        } else {
            written += try$(writer.writeStr("nullptr"s));
        }

        return Ok(written);
    }
};

template <>
struct Formatter<std::nullptr_t> {
    Res<usize> format(Io::TextWriter& writer, std::nullptr_t) {
        return writer.writeStr("nullptr"s);
    }
};

// MARK: Format Optionals ------------------------------------------------------

template <>
struct Formatter<None> {
    Res<usize> format(Io::TextWriter& writer, None const&) {
        return writer.writeStr("None"s);
    }
};

template <>
struct Formatter<bool> {
    Res<usize> format(Io::TextWriter& writer, bool val) {
        return writer.writeStr(val ? "True"s : "False"s);
    }
};

template <typename T>
struct Formatter<Opt<T>> {
    Formatter<T> formatter;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          formatter.parse(scan);
                      }) {
            formatter.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Opt<T> const& val) {
        if (val)
            return formatter.format(writer, *val);
        return writer.writeStr("None"s);
    }
};

// MARK: Format Results --------------------------------------------------------

template <typename T>
struct Formatter<Ok<T>> {
    Formatter<T> formatter;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          formatter.parse(scan);
                      }) {
            formatter.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Ok<T> const& val) {
        if constexpr (Meta::Same<T, None>)
            return writer.writeStr("Ok"s);
        else
            return formatter.format(writer, val.inner);
    }
};

template <>
struct Formatter<Error> {
    Res<usize> format(Io::TextWriter& writer, Error const& val) {
        usize written = 0;
        Str msg = Str::fromNullterminated(val.msg());
        written += try$(writer.writeStr(msg));
        return Ok(written);
    }
};

template <typename T, typename E>
struct Formatter<Res<T, E>> {
    Formatter<T> _fmtOk;
    Formatter<E> _fmtErr;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          _fmtOk.parse(scan);
                      }) {
            _fmtOk.parse(scan);
        }

        if constexpr (requires() {
                          _fmtErr.parse(scan);
                      }) {
            _fmtErr.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Res<T, E> const& val) {
        if (val)
            return _fmtOk.format(writer, val.unwrap());
        return _fmtErr.format(writer, val.none());
    }
};

// MARK: Format Unions ---------------------------------------------------------

template <typename... Ts>
struct Formatter<Union<Ts...>> {

    Res<usize> format(Io::TextWriter& writer, Union<Ts...> const& val) {
        return val.visit([&](auto const& v) -> Res<usize> {
            return Io::format(writer, "{}", v);
        });
    }
};

// MARK: Format Ordering -------------------------------------------------------

template <>
struct Formatter<std::strong_ordering> {
    Res<usize> format(Io::TextWriter& writer, std::strong_ordering val) {
        if (val == std::strong_ordering::less)
            return writer.writeStr("Less"s);

        if (val == std::strong_ordering::greater)
            return writer.writeStr("Greater"s);

        return writer.writeStr("Equal"s);
    }
};

template <>
struct Formatter<std::weak_ordering> {
    Res<usize> format(Io::TextWriter& writer, std::weak_ordering val) {
        if (val == std::weak_ordering::less)
            return writer.writeStr("Less"s);

        if (val == std::weak_ordering::greater)
            return writer.writeStr("Greater"s);

        return writer.writeStr("Equivalent"s);
    }
};

template <>
struct Formatter<std::partial_ordering> {
    Res<usize> format(Io::TextWriter& writer, std::partial_ordering val) {
        if (val == std::partial_ordering::equivalent)
            return writer.writeStr("Equivalent"s);

        if (val == std::partial_ordering::less)
            return writer.writeStr("Less"s);

        if (val == std::partial_ordering::greater)
            return writer.writeStr("Greater"s);

        return writer.writeStr("Unordered"s);
    }
};

// MARK: Format References -----------------------------------------------------

template <typename T>
struct Formatter<Rc<T>> {
    Formatter<T> formatter;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          formatter.parse(scan);
                      }) {
            formatter.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Rc<T> const& val) {
        return formatter.format(writer, val.unwrap());
    }
};

template <typename T>
struct Formatter<Weak<T>> {
    Formatter<T> formatter;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          formatter.parse(scan);
                      }) {
            formatter.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Weak<T> const& val) {
        auto inner = val.upgrade();
        if (not inner)
            return writer.writeStr("None"s);
        return formatter.format(writer, inner.unwrap().unwrap());
    }
};

template <typename T>
struct Formatter<Box<T>> {
    Formatter<T> formatter;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          formatter.parse(scan);
                      }) {
            formatter.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Box<T> const& val) {
        return formatter.format(writer, *val);
    }
};

template <typename T>
struct Formatter<Cow<T>> {
    Formatter<T> formatter;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          formatter.parse(scan);
                      }) {
            formatter.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Cow<T> const& val) {
        return formatter.format(writer, *val);
    }
};

// MARK: Format Reflectable ----------------------------------------------------

struct Emit;

template <typename T>
struct Repr;

template <typename T>
concept ReprMethod = requires(T t, Emit& emit) {
    t.repr(emit);
};

template <typename T>
concept Reprable =
    ReprMethod<T> or
    requires(T t, Emit& emit) {
        Repr<T>::repr(emit, t);
    };

// MARK: Format Sliceable ------------------------------------------------------

template <Sliceable T>
struct Formatter<T> {
    Formatter<typename T::Inner> inner;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          inner.parse(scan);
                      }) {
            inner.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, T const& val) {
        auto written = try$(writer.writeStr("["s));
        for (usize i = 0; i < val.len(); i++) {
            if (i != 0)
                written += try$(writer.writeStr(", "s));
            written += try$(inner.format(writer, val[i]));
        }
        return Ok(written + try$(writer.writeStr("]"s)));
    }
};

// MARK: Format Map ------------------------------------------------------------

template <typename K, typename V>
struct Formatter<Map<K, V>> {
    Formatter<K> keyFormatter;
    Formatter<V> valFormatter;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          keyFormatter.parse(scan);
                          valFormatter.parse(scan);
                      }) {
            keyFormatter.parse(scan);
            valFormatter.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Map<K, V> const& val) {
        usize written = 0;
        written += try$(writer.writeStr("{"s));
        bool first = true;
        for (auto const& [key, value] : val.iter()) {
            if (not first)
                written += try$(writer.writeStr(", "s));
            first = false;
            written += try$(keyFormatter.format(writer, key));
            written += try$(writer.writeStr(": "s));
            written += try$(valFormatter.format(writer, value));
        }
        written += try$(writer.writeStr("}"s));
        return Ok(written);
    }
};

// MARK: Format Range ----------------------------------------------------------

template <typename T, typename Tag>
struct Formatter<Range<T, Tag>> {

    Formatter<T> inner;

    void parse(Io::SScan& scan) {
        if constexpr (requires() {
                          inner.parse(scan);
                      }) {
            inner.parse(scan);
        }
    }

    Res<usize> format(Io::TextWriter& writer, Range<T, Tag> const& val) {
        usize written = 0;
        written += try$(writer.writeStr("["s));
        written += try$(inner.format(writer, val.start));
        written += try$(writer.writeStr("-"s));
        written += try$(inner.format(writer, val.end()));
        written += try$(writer.writeStr("]"s));
        return Ok(written);
    }
};

// MARK: Format String ---------------------------------------------------------

template <StaticEncoding E>
struct StringFormatter {
    bool prefix = false;

    void parse(Io::SScan& scan) {
        if (scan.skip('#'))
            prefix = true;
    }

    Res<usize> format(Io::TextWriter& writer, _Str<E> text) {
        if (not prefix)
            return writer.writeStr(text);

        usize written = 0;
        written += try$(writer.writeRune('"'));
        for (Rune c : iterRunes(text)) {
            if (c == '"')
                written += try$(writer.writeStr("\\\""s));
            else if (c == '\\')
                written += try$(writer.writeStr("\\\\"s));
            else if (c == '\a')
                written += try$(writer.writeStr("\\a"s));
            else if (c == '\b')
                written += try$(writer.writeStr("\\b"s));
            else if (c == '\f')
                written += try$(writer.writeStr("\\f"s));
            else if (c == '\n')
                written += try$(writer.writeStr("\\n"s));
            else if (c == '\r')
                written += try$(writer.writeStr("\\r"s));
            else if (c == '\t')
                written += try$(writer.writeStr("\\t"s));
            else if (c == '\v')
                written += try$(writer.writeStr("\\v"s));
            else if (not isAsciiPrint(c))
                written += try$(Io::format(writer, "\\u{x}", c));
            else
                written += try$(writer.writeRune(c));
        }
        written += try$(writer.writeRune('"'));
        return Ok(written);
    }
};

template <StaticEncoding E>
struct Formatter<_Str<E>> : public StringFormatter<E> {};

template <StaticEncoding E>
struct Formatter<_String<E>> : public StringFormatter<E> {
    Res<usize> format(Io::TextWriter& writer, _String<E> const& text) {
        return StringFormatter<E>::format(writer, text.str());
    }
};

template <usize N>
struct Formatter<StrLit<N>> : public StringFormatter<Utf8> {};

template <>
struct Formatter<char const*> : public StringFormatter<Utf8> {
    Res<usize> format(Io::TextWriter& writer, char const* text) {
        _Str<Utf8> str = Str::fromNullterminated(text);
        return StringFormatter::format(writer, str);
    }
};

// MARK: Format Time -----------------------------------------------------------

template <>
struct Formatter<Duration> {
    Res<usize> format(Io::TextWriter& writer, Duration const& val) {
        return Io::format(writer, "{}.{03}s", val.toSecs(), val.toMSecs() % 1000);
    }
};

template <>
struct Formatter<Instant> {
    Res<usize> format(Io::TextWriter& writer, Instant const& val) {
        return Io::format(writer, "monotonic:{}", val._value);
    }
};

template <>
struct Formatter<SystemTime> {
    Res<usize> format(Io::TextWriter& writer, SystemTime const& val) {
        return Io::format(writer, "{}", DateTime::fromInstant(val));
    }
};

template <>
struct Formatter<Time> {
    Res<usize> format(Io::TextWriter& writer, Time const& val) {
        return Io::format(writer, "{02}:{02}:{02}", val.hour, val.minute, val.second);
    }
};

template <>
struct Formatter<Date> {
    Res<usize> format(Io::TextWriter& writer, Date const& val) {
        return Io::format(writer, "{04}-{02}-{02}", (isize)val.year, (usize)val.month + 1, (usize)val.day + 1);
    }
};

template <>
struct Formatter<DateTime> {
    Res<usize> format(Io::TextWriter& writer, DateTime const& val) {
        return Io::format(writer, "{} {}", val.date, val.time);
    }
};

// MARK: Format Tuple ----------------------------------------------------------

template <typename Car, typename Cdr>
struct Formatter<Pair<Car, Cdr>> {

    Res<usize> format(Io::TextWriter& writer, Pair<Car, Cdr> const& val) {
        usize written = 0;
        written += try$(writer.writeRune('{'));

        Formatter<Car> carFormatter;
        written += try$(carFormatter.format(writer, val.v0));
        written += try$(writer.writeStr(", "s));

        Formatter<Cdr> cdrFormatter;
        written += try$(cdrFormatter.format(writer, val.v1));
        written += try$(writer.writeRune('}'));
        return Ok(written);
    }
};

template <typename... Ts>
struct Formatter<Tuple<Ts...>> {

    Res<usize> format(Io::TextWriter& writer, Tuple<Ts...> const& val) {
        usize written = 0;
        bool first = true;
        written += try$(writer.writeRune('{'));
        try$(val.visit([&]<typename T>(T const& f) -> Res<usize> {
            if (not first)
                written += try$(writer.writeStr(", "s));

            Formatter<T> formatter;
            written += try$(formatter.format(writer, f));

            first = false;
            return Ok(0uz);
        }));
        written += try$(writer.writeRune('}'));
        return Ok(written);
    }
};

// MARK: Format backtrace ------------------------------------------------------

template <>
struct Formatter<Backtrace> {
    Res<usize> format(Io::TextWriter& writer, Backtrace const& val) {
        if (val.status() == Backtrace::DISABLED)
            return writer.writeStr("(backtrace disabled)"s);

        if (val.status() == Backtrace::UNSUPPORTED)
            return writer.writeStr("(backtrace unsupported)"s);

        usize index = 1;
        usize written = 0;
        for (auto const& frame : val.frames()) {
            written += try$(Io::format(writer, "#{}: {} at {}:{}\n", index, frame.desc, frame.file, frame.line));
            index++;
        }

        return Ok(written);
    }
};

} // namespace Karm::Io
