#include "types.hpp"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <scn/scn.h>

#include <catch2/catch_test_macros.hpp>
#include <range/v3/all.hpp>

#include <charconv>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace rv = ranges;
using namespace std::string_literals;
using namespace std::string_view_literals;

[[noreturn]] inline void unreachable()
{
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#ifdef __GNUC__ // GCC, Clang, ICC
  __builtin_unreachable();
#elifdef _MSC_VER // MSVC
  __assume(false);
#endif
}

template <typename T>
T fromString(std::string_view v);
