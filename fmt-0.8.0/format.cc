/*
 String formatting library for C++

 Copyright (c) 2012, Victor Zverovich
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Disable useless MSVC warnings.
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#undef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "format.h"

#include <cctype>
#include <cmath>
#include <cstdarg>

using fmt::ULongLong;

namespace {

inline int SignBit(double value) {
  // When compiled in C++11 mode signbit is no longer a macro but a function
  // defined in namespace std and the macro is undefined.
#ifdef signbit
  return signbit(value);
#else
  return std::signbit(value);
#endif
}

inline int IsInf(double x) {
#ifdef isinf
  return isinf(x);
#else
  return std::isinf(x);
#endif
}

#define FMT_SNPRINTF snprintf

const char RESET_COLOR[] = "\x1b[0m";
}

template <typename T>
int fmt::internal::CharTraits<char>::FormatFloat(
    char *buffer, std::size_t size, const char *format,
    unsigned width, int precision, T value) {
  if (width == 0) {
    return precision < 0 ?
        FMT_SNPRINTF(buffer, size, format, value) :
        FMT_SNPRINTF(buffer, size, format, precision, value);
  }
  return precision < 0 ?
      FMT_SNPRINTF(buffer, size, format, width, value) :
      FMT_SNPRINTF(buffer, size, format, width, precision, value);
}

template <typename T>
int fmt::internal::CharTraits<wchar_t>::FormatFloat(
    wchar_t *buffer, std::size_t size, const wchar_t *format,
    unsigned width, int precision, T value) {
  if (width == 0) {
    return precision < 0 ?
        swprintf(buffer, size, format, value) :
        swprintf(buffer, size, format, precision, value);
  }
  return precision < 0 ?
      swprintf(buffer, size, format, width, value) :
      swprintf(buffer, size, format, width, precision, value);
}

const char fmt::internal::DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

#define FMT_POWERS_OF_10(factor) \
  factor * 10, \
  factor * 100, \
  factor * 1000, \
  factor * 10000, \
  factor * 100000, \
  factor * 1000000, \
  factor * 10000000, \
  factor * 100000000, \
  factor * 1000000000

const uint32_t fmt::internal::POWERS_OF_10_32[] = {0, FMT_POWERS_OF_10(1)};
const uint64_t fmt::internal::POWERS_OF_10_64[] = {
  0,
  FMT_POWERS_OF_10(1),
  FMT_POWERS_OF_10(ULongLong(1000000000)),
  // Multiply several constants instead of using a single long long constants
  // to avoid warnings about C++98 not supporting long long.
  ULongLong(1000000000) * ULongLong(1000000000) * 10
};

void fmt::internal::ReportUnknownType(char code, const char *type) {
  if (std::isprint(static_cast<unsigned char>(code))) {
    throw fmt::FormatError(fmt::str(
        fmt::Format("unknown format code '{}' for {}") << code << type));
  }
  throw fmt::FormatError(
      fmt::str(fmt::Format("unknown format code '\\x{:02x}' for {}")
        << static_cast<unsigned>(code) << type));
}


// Fills the padding around the content and returns the pointer to the
// content area.
template <typename Char>
typename fmt::BasicWriter<Char>::CharPtr
  fmt::BasicWriter<Char>::FillPadding(CharPtr buffer,
    unsigned total_size, std::size_t content_size, wchar_t fill) {
  std::size_t padding = total_size - content_size;
  std::size_t left_padding = padding / 2;
  Char fill_char = static_cast<Char>(fill);
  std::fill_n(buffer, left_padding, fill_char);
  buffer += left_padding;
  CharPtr content = buffer;
  std::fill_n(buffer + content_size, padding - left_padding, fill_char);
  return content;
}

template <typename Char>
typename fmt::BasicWriter<Char>::CharPtr
  fmt::BasicWriter<Char>::PrepareFilledBuffer(
    unsigned size, const AlignSpec &spec, char sign) {
  unsigned width = spec.width();
  if (width <= size) {
    CharPtr p = GrowBuffer(size);
    *p = sign;
    return p + size - 1;
  }
  CharPtr p = GrowBuffer(width);
  CharPtr end = p + width;
  Alignment align = spec.align();
  // TODO: error if fill is not convertible to Char
  Char fill = static_cast<Char>(spec.fill());
  if (align == ALIGN_LEFT) {
    *p = sign;
    p += size;
    std::fill(p, end, fill);
  } else if (align == ALIGN_CENTER) {
    p = FillPadding(p, width, size, fill);
    *p = sign;
    p += size;
  } else {
    if (align == ALIGN_NUMERIC) {
      if (sign) {
        *p++ = sign;
        --size;
      }
    } else {
      *(end - size) = sign;
    }
    std::fill(p, end - size, fill);
    p = end;
  }
  return p - 1;
}

template <typename Char>
template <typename T>
void fmt::BasicWriter<Char>::FormatDouble(
    T value, const FormatSpec &spec, int precision) {
  // Check type.
  char type = spec.type();
  bool upper = false;
  switch (type) {
  case 0:
    type = 'g';
    break;
  case 'e': case 'f': case 'g':
    break;
  case 'F':
    // Fall through.
  case 'E': case 'G':
    upper = true;
    break;
  default:
    internal::ReportUnknownType(type, "double");
    break;
  }

  char sign = 0;
  // Use SignBit instead of value < 0 because the latter is always
  // false for NaN.
  if (SignBit(static_cast<double>(value))) {
    sign = '-';
    value = -value;
  } else if (spec.sign_flag()) {
    sign = spec.plus_flag() ? '+' : ' ';
  }

  if (value != value) {
    // Format NaN ourselves because sprintf's output is not consistent
    // across platforms.
    std::size_t size = 4;
    const char *nan = upper ? " NAN" : " nan";
    if (!sign) {
      --size;
      ++nan;
    }
    CharPtr out = FormatString(nan, size, spec);
    if (sign)
      *out = sign;
    return;
  }

  if (IsInf(static_cast<double>(value))) {
    // Format infinity ourselves because sprintf's output is not consistent
    // across platforms.
    std::size_t size = 4;
    const char *inf = upper ? " INF" : " inf";
    if (!sign) {
      --size;
      ++inf;
    }
    CharPtr out = FormatString(inf, size, spec);
    if (sign)
      *out = sign;
    return;
  }

  std::size_t offset = buffer_.size();
  unsigned width = spec.width();
  if (sign) {
    buffer_.reserve(buffer_.size() + (std::max)(width, 1u));
    if (width > 0)
      --width;
    ++offset;
  }

  // Build format string.
  enum { MAX_FORMAT_SIZE = 10}; // longest format: %#-*.*Lg
  Char format[MAX_FORMAT_SIZE];
  Char *format_ptr = format;
  *format_ptr++ = '%';
  unsigned width_for_sprintf = width;
  if (spec.hash_flag())
    *format_ptr++ = '#';
  if (spec.align() == ALIGN_CENTER) {
    width_for_sprintf = 0;
  } else {
    if (spec.align() == ALIGN_LEFT)
      *format_ptr++ = '-';
    if (width != 0)
      *format_ptr++ = '*';
  }
  if (precision >= 0) {
    *format_ptr++ = '.';
    *format_ptr++ = '*';
  }
  if (internal::IsLongDouble<T>::VALUE)
    *format_ptr++ = 'L';
  *format_ptr++ = type;
  *format_ptr = '\0';

  // Format using snprintf.
  Char fill = static_cast<Char>(spec.fill());
  for (;;) {
    std::size_t size = buffer_.capacity() - offset;
    Char *start = &buffer_[offset];
    int n = internal::CharTraits<Char>::FormatFloat(
        start, size, format, width_for_sprintf, precision, value);
    if (n >= 0 && offset + n < buffer_.capacity()) {
      if (sign) {
        if ((spec.align() != ALIGN_RIGHT && spec.align() != ALIGN_DEFAULT) ||
            *start != ' ') {
          *(start - 1) = sign;
          sign = 0;
        } else {
          *(start - 1) = fill;
        }
        ++n;
      }
      if (spec.align() == ALIGN_CENTER &&
          spec.width() > static_cast<unsigned>(n)) {
        unsigned width = spec.width();
        CharPtr p = GrowBuffer(width);
        std::copy(p, p + n, p + (width - n) / 2);
        FillPadding(p, spec.width(), n, fill);
        return;
      }
      if (spec.fill() != ' ' || sign) {
        while (*start == ' ')
          *start++ = fill;
        if (sign)
          *(start - 1) = sign;
      }
      GrowBuffer(n);
      return;
    }
    buffer_.reserve(n >= 0 ? offset + n + 1 : 2 * buffer_.capacity());
  }
}

// Throws Exception(message) if format contains '}', otherwise throws
// FormatError reporting unmatched '{'. The idea is that unmatched '{'
// should override other errors.
template <typename Char>
void fmt::BasicFormatter<Char>::ReportError(
    const Char *s, StringRef message) const {
  for (int num_open_braces = num_open_braces_; *s; ++s) {
    if (*s == '{') {
      ++num_open_braces;
    } else if (*s == '}') {
      if (--num_open_braces == 0)
        throw fmt::FormatError(message);
    }
  }
  throw fmt::FormatError("unmatched '{' in format");
}

// Parses an unsigned integer advancing s to the end of the parsed input.
// This function assumes that the first character of s is a digit.
// 参数是指针引用, 会修改引用的内容
template <typename Char>
unsigned fmt::BasicFormatter<Char>::ParseUInt(const Char *&s) const {
  assert('0' <= *s && *s <= '9');
  unsigned value = 0;
  do {
    unsigned new_value = value * 10 + (*s++ - '0');
    if (new_value < value)  // Check if value wrapped around.
      ReportError(s, "number is too big in format");
    value = new_value;
  } while ('0' <= *s && *s <= '9');
  return value;
}

// 这里是解析 {} 中的内容, 获取对应的 Arg 对象
// 参数有点特别, 指针引用, PaserUInt 会更新 s
// 解析完, s 指向 '}' 或者 ':'
template <typename Char>
inline const typename fmt::BasicFormatter<Char>::Arg
    &fmt::BasicFormatter<Char>::ParseArgIndex(const Char *&s) {
  unsigned arg_index = 0;
  // 如果不是数字, 说明就是按默认的顺序从左往右
  if (*s < '0' || *s > '9') {
    if (*s != '}' && *s != ':')
      ReportError(s, "invalid argument index in format string");
    if (next_arg_index_ < 0) {
      // 要么全是自动顺序的, 要么全是指定顺序
      ReportError(s, "cannot switch from manual to automatic argument indexing");
    }
    arg_index = next_arg_index_++;
  } else {
    if (next_arg_index_ > 0) {
      ReportError(s, "cannot switch from automatic to manual argument indexing");
    }
    next_arg_index_ = -1;
    arg_index = ParseUInt(s);
  }
  if (arg_index >= args_.size())
    ReportError(s, "argument index is out of range in format");
  return *args_[arg_index];
}

template <typename Char>
void fmt::BasicFormatter<Char>::CheckSign(const Char *&s, const Arg &arg) {
  char sign = static_cast<char>(*s);
  if (arg.type > LAST_NUMERIC_TYPE) {
    ReportError(s,
        Format("format specifier '{}' requires numeric argument") << sign);
  }
  if (arg.type == UINT || arg.type == ULONG || arg.type == ULONG_LONG) {
    ReportError(s,
        Format("format specifier '{}' requires signed argument") << sign);
  }
  ++s;
}

// 整体语法参考 https://fmt.dev/latest/syntax.html?highlight=align#format-string-syntax
// format_spec ::=  [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
// fill        ::=  <a character other than '{' or '}'>
// align       ::=  "<" | ">" | "^"
// sign        ::=  "+" | "-" | " "
// width       ::=  integer | "{" [arg_id] "}"
// precision   ::=  integer | "{" [arg_id] "}"
// type        ::=  "a" | "A" | "b" | "B" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G" |
//                  "o" | "p" | "s" | "x" | "X"
template <typename Char>
void fmt::BasicFormatter<Char>::DoFormat() {
  const Char *start = format_;
  format_ = 0;
  next_arg_index_ = 0;
  const Char *s = start;
  BasicWriter<Char> &writer = *writer_;
  while (*s) {
    Char c = *s++;
    if (c != '{' && c != '}') continue;
    if (*s == c) {
      writer.buffer_.append(start, s);
      start = ++s;
      continue;
    }
    if (c == '}')
      throw FormatError("unmatched '}' in format");
    // 走到这里 c 肯定是 {
    num_open_braces_= 1;
    // 从上一次解析完成的位置到 { 之前的内容拷贝到输出缓冲区
    writer.buffer_.append(start, s - 1);
    // s 此刻指向 { 后面一个字符
    // 找到 {} 匹配的 Arg 对象
    const Arg &arg = ParseArgIndex(s);
    // s 此刻指向 : 或者 }
    FormatSpec spec;
    int precision = -1;
    if (*s == ':') {
      ++s;

      // Parse fill and alignment.
      if (Char c = *s) {
        const Char *p = s + 1;
        spec.align_ = ALIGN_DEFAULT;
        // unclear: 为啥要搞个 do while
        // 因为它其实会看两个字符, 因为规则中 [[fill]align]
        // 说明可以只有 align, 没有 fill
        // 这里先看 : 后面第二个字符是不是 align, 如果是, 那么第一个字符肯定是 fill
        // 如果第二个字符不是 align, 再看第一个字符是不是 align
        do {
          switch (*p) {
            case '<':
              spec.align_ = ALIGN_LEFT;
              break;
            case '>':
              spec.align_ = ALIGN_RIGHT;
              break;
            case '=':
              spec.align_ = ALIGN_NUMERIC;
              break;
            case '^':
              spec.align_ = ALIGN_CENTER;
              break;
          }
          // 如果 *p 是 <, >, =, ^ 中的一个
          if (spec.align_ != ALIGN_DEFAULT) {
            if (p != s) {
              // {:} 这种也允许, 虽然没什么意义
              if (c == '}') break;
              if (c == '{') ReportError(s, "invalid fill character '{'");
              // s 跳过 fill 和 align
              s += 2;
              // : 后面第一个字符表示 fill
              spec.fill_ = c;
            } 
            // 走到这里说明第一个字符是 align, 没有 fill, s 跳过 align
            else {
              ++s;
            }
            if (spec.align_ == ALIGN_NUMERIC && arg.type > LAST_NUMERIC_TYPE)
              ReportError(s, "format specifier '=' requires numeric argument");
            break;
          }
          // 走到这里说明 : 后面的第二个字符不是 align, 那么再回头看第一个字符是不是 align
        } while (--p >= s);
      }

      // Parse sign.
      // 对于有符号正整数, + 会打印 +, - 不打印符号, ' ' 会打印 ' '
      // 对于有符号负整数, 都会打印 -
      switch (*s) {
        case '+':
          CheckSign(s, arg);
          spec.flags_ |= SIGN_FLAG | PLUS_FLAG;
          break;
        case '-':
          CheckSign(s, arg);
          break;
        case ' ':
          CheckSign(s, arg);
          spec.flags_ |= SIGN_FLAG;
          break;
      }
      // 只用于数值类型
      // unclear: # 对 double 的作用是啥?
      if (*s == '#') {
        if (arg.type > LAST_NUMERIC_TYPE)
          ReportError(s, "format specifier '#' requires numeric argument");
        spec.flags_ |= HASH_FLAG;
        ++s;
      }

      // Parse width and zero flag.
      if ('0' <= *s && *s <= '9') {
        // 0 开头表示填充 0, 此时就没有对齐一说了
        if (*s == '0') {
          if (arg.type > LAST_NUMERIC_TYPE)
            ReportError(s, "format specifier '0' requires numeric argument");
          spec.align_ = ALIGN_NUMERIC;
          spec.fill_ = '0';
        }
        // Zero may be parsed again as a part of the width, but it is simpler
        // and more efficient than checking if the next char is a digit.
        unsigned value = ParseUInt(s);
        if (value > INT_MAX)
          ReportError(s, "number is too big in format");
        spec.width_ = value;
      }

      // Parse precision.
      if (*s == '.') {
        // 跳过 .
        ++s;
        precision = 0;
        // 数字开头表示是一个整数表示精度
        if ('0' <= *s && *s <= '9') {
          unsigned value = ParseUInt(s);
          if (value > INT_MAX)
            ReportError(s, "number is too big in format");
          precision = value;
        } 
        // 如果是 { 开头说明是一个 Arg 表示精度
        else if (*s == '{') {
          ++s;
          ++num_open_braces_;
          const Arg &precision_arg = ParseArgIndex(s);
          ULongLong value = 0;
          switch (precision_arg.type) {
            case INT:
              if (precision_arg.int_value < 0)
                ReportError(s, "negative precision in format");
              value = precision_arg.int_value;
              break;
            case UINT:
              value = precision_arg.uint_value;
              break;
            case LONG:
              if (precision_arg.long_value < 0)
                ReportError(s, "negative precision in format");
              value = precision_arg.long_value;
              break;
            case ULONG:
              value = precision_arg.ulong_value;
              break;
            case LONG_LONG:
              if (precision_arg.long_long_value < 0)
                ReportError(s, "negative precision in format");
              value = precision_arg.long_long_value;
              break;
            case ULONG_LONG:
              value = precision_arg.ulong_long_value;
              break;
            default:
              ReportError(s, "precision is not integer");
          }
          if (value > INT_MAX)
            ReportError(s, "number is too big in format");
          precision = static_cast<int>(value);
          if (*s++ != '}')
            throw FormatError("unmatched '{' in format");
          --num_open_braces_;
        } else {
          ReportError(s, "missing precision in format");
        }
        // 只有浮点数可以使用精度
        if (arg.type != DOUBLE && arg.type != LONG_DOUBLE) {
          ReportError(s,
              "precision specifier requires floating-point argument");
        }
      }

      // Parse type.
      if (*s != '}' && *s)
        spec.type_ = static_cast<char>(*s++);
    }

    if (*s++ != '}')
      throw FormatError("unmatched '{' in format");
    // 开始下一段的解析, s 指向 } 后一个字符
    start = s;

    // 如果 {} 中没有 :, 走到这里的时 spec 还是默认值
    //   width = 0, type = 0, fill = ' '
    // Format argument.
    switch (arg.type) {
      case INT:
        // 这里调用的是 BasicFormatter::FormatInt()
        // BasicWriter 内部也有个 FormatInt()
        FormatInt(arg.int_value, spec);
        break;
      case UINT:
        FormatInt(arg.uint_value, spec);
        break;
      case LONG:
        FormatInt(arg.long_value, spec);
        break;
      case ULONG:
        FormatInt(arg.ulong_value, spec);
        break;
      case LONG_LONG:
        FormatInt(arg.long_long_value, spec);
        break;
      case ULONG_LONG:
        FormatInt(arg.ulong_long_value, spec);
        break;
      case DOUBLE:
        writer.FormatDouble(arg.double_value, spec, precision);
        break;
      case LONG_DOUBLE:
        writer.FormatDouble(arg.long_double_value, spec, precision);
        break;
      case CHAR: {
        if (spec.type_ && spec.type_ != 'c')
          internal::ReportUnknownType(spec.type_, "char");
        typedef typename BasicWriter<Char>::CharPtr CharPtr;
        CharPtr out = CharPtr();
        if (spec.width_ > 1) {
          Char fill = static_cast<Char>(spec.fill());
          out = writer.GrowBuffer(spec.width_);
          if (spec.align_ == ALIGN_RIGHT) {
            std::fill_n(out, spec.width_ - 1, fill);
            out += spec.width_ - 1;
          } else if (spec.align_ == ALIGN_CENTER) {
            out = writer.FillPadding(out, spec.width_, 1, fill);
          } else {
            std::fill_n(out + 1, spec.width_ - 1, fill);
          }
        } else {
          out = writer.GrowBuffer(1);
        }
        *out = static_cast<Char>(arg.int_value);
        break;
      }
      case STRING: {
        if (spec.type_ && spec.type_ != 's')
          internal::ReportUnknownType(spec.type_, "string");
        const Char *str = arg.string.value;
        std::size_t size = arg.string.size;
        if (size == 0) {
          if (!str)
            throw FormatError("string pointer is null");
          if (*str)
            size = std::char_traits<Char>::length(str);
        }
        writer.FormatString(str, size, spec);
        break;
      }
      case POINTER:
        if (spec.type_ && spec.type_ != 'p')
          internal::ReportUnknownType(spec.type_, "pointer");
        spec.flags_= HASH_FLAG;
        spec.type_ = 'x';
        FormatInt(reinterpret_cast<uintptr_t>(arg.pointer_value), spec);
        break;
      case CUSTOM:
        if (spec.type_)
          internal::ReportUnknownType(spec.type_, "object");
        arg.custom.format(writer, arg.custom.value, spec);
        break;
      default:
        assert(false);
        break;
    }
  }
  writer.buffer_.append(start, s);
}

void fmt::ColorWriter::operator()(const fmt::BasicWriter<char> &w) const {
  char escape[] = "\x1b[30m";
  escape[3] = '0' + static_cast<char>(color_);
  std::fputs(escape, stdout);
  std::fwrite(w.data(), 1, w.size(), stdout);
  std::fputs(RESET_COLOR, stdout);
}

// Explicit instantiations for char.

template void fmt::BasicWriter<char>::FormatDouble<double>(
    double value, const FormatSpec &spec, int precision);

template void fmt::BasicWriter<char>::FormatDouble<long double>(
    long double value, const FormatSpec &spec, int precision);

template fmt::BasicWriter<char>::CharPtr
  fmt::BasicWriter<char>::FillPadding(CharPtr buffer,
    unsigned total_size, std::size_t content_size, wchar_t fill);

template fmt::BasicWriter<char>::CharPtr
  fmt::BasicWriter<char>::PrepareFilledBuffer(
    unsigned size, const AlignSpec &spec, char sign);

template void fmt::BasicFormatter<char>::ReportError(
    const char *s, StringRef message) const;

template unsigned fmt::BasicFormatter<char>::ParseUInt(const char *&s) const;

template const fmt::BasicFormatter<char>::Arg
    &fmt::BasicFormatter<char>::ParseArgIndex(const char *&s);

template void fmt::BasicFormatter<char>::CheckSign(
    const char *&s, const Arg &arg);

template void fmt::BasicFormatter<char>::DoFormat();

// Explicit instantiations for wchar_t.

template void fmt::BasicWriter<wchar_t>::FormatDouble<double>(
    double value, const FormatSpec &spec, int precision);

template void fmt::BasicWriter<wchar_t>::FormatDouble<long double>(
    long double value, const FormatSpec &spec, int precision);

template fmt::BasicWriter<wchar_t>::CharPtr
  fmt::BasicWriter<wchar_t>::FillPadding(CharPtr buffer,
      unsigned total_size, std::size_t content_size, wchar_t fill);

template fmt::BasicWriter<wchar_t>::CharPtr
  fmt::BasicWriter<wchar_t>::PrepareFilledBuffer(
    unsigned size, const AlignSpec &spec, char sign);

template void fmt::BasicFormatter<wchar_t>::ReportError(
    const wchar_t *s, StringRef message) const;

template unsigned fmt::BasicFormatter<wchar_t>::ParseUInt(
    const wchar_t *&s) const;

template const fmt::BasicFormatter<wchar_t>::Arg
    &fmt::BasicFormatter<wchar_t>::ParseArgIndex(const wchar_t *&s);

template void fmt::BasicFormatter<wchar_t>::CheckSign(
    const wchar_t *&s, const Arg &arg);

template void fmt::BasicFormatter<wchar_t>::DoFormat();