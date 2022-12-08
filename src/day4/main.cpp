#include <common.hpp>

// #define RUN_TESTS

struct CleaningRange
{
  u32 start;
  u32 end;

  friend auto operator<=>(const CleaningRange&, const CleaningRange&) = default;
};

using CleaningPair = std::array<CleaningRange, 2>;

template <>
CleaningRange fromString(std::string_view v)
{
  auto del = v.find_first_of('-');
  u32 v1;
  u32 v2;
  std::from_chars(v.data(), v.data() + del, v1);
  std::from_chars(v.data() + del + 1, v.end(), v2);

  return {.start = std::min(v1, v2), .end = std::max(v1, v2)};
}

template <>
CleaningPair fromString(std::string_view v)
{
  auto del = v.find_first_of(',');
  return {fromString<CleaningRange>(v.substr(0, del)), fromString<CleaningRange>(v.substr(del + 1))};
}

std::vector<CleaningPair> parsePairs(std::istream&& input)
{
  return rv::getlines(input) | rv::views::transform(fromString<CleaningPair>) | rv::to<std::vector<CleaningPair>>();
}

bool fullyContained(CleaningPair pair)
{
  return (pair[0].start <= pair[1].start && pair[0].end >= pair[1].end) ||
         (pair[1].start <= pair[0].start && pair[1].end >= pair[0].end);
}

bool partiallyContained(CleaningPair pair)
{
  return fullyContained(pair) || (pair[0].start >= pair[1].start && pair[0].start <= pair[1].end) ||
         (pair[0].end >= pair[1].start && pair[0].end <= pair[1].end);
}

u32 countFullyContained(const std::vector<CleaningPair>& pairs)
{
  return rv::count(pairs | rv::views::transform(fullyContained), true);
}

u32 countPartiallyContained(const std::vector<CleaningPair>& pairs)
{
  return rv::count(pairs | rv::views::transform(partiallyContained), true);
}

// #ifndef RUN_TESTS
#include <fstream>

#ifndef RUN_TESTS
auto main() -> int
{
  auto pairs = parsePairs(std::fstream("../../src/day4/input.txt"));
  fmt::print("Task1 Result: {}\n", countFullyContained(pairs));
  fmt::print("Task2 Result: {}\n", countPartiallyContained(pairs));
}

#else

TEST_CASE("Parsing ranges")
{
  REQUIRE(fromString<CleaningRange>("1-2") == CleaningRange{1, 2});
  REQUIRE(fromString<CleaningRange>("5-10") == CleaningRange{5, 10});
  REQUIRE(fromString<CleaningRange>("5-1") == CleaningRange{1, 5});
}

TEST_CASE("Parsing range pairs")
{
  CleaningPair p = fromString<CleaningPair>("2-4,6-8");
  REQUIRE(p[0] == CleaningRange{2, 4});
  REQUIRE(p[1] == CleaningRange{6, 8});
}

TEST_CASE("Fully Contained")
{
  // No overlap
  REQUIRE(fullyContained(CleaningPair{CleaningRange{1, 2}, CleaningRange{3, 4}}) == false);
  REQUIRE(fullyContained(CleaningPair{CleaningRange{3, 6}, CleaningRange{1, 2}}) == false);

  // Full overlap
  REQUIRE(fullyContained(CleaningPair{CleaningRange{1, 4}, CleaningRange{2, 3}}) == true);
  REQUIRE(fullyContained(CleaningPair{CleaningRange{2, 3}, CleaningRange{1, 4}}) == true);

  // Partial overlap
  REQUIRE(fullyContained(CleaningPair{CleaningRange{1, 3}, CleaningRange{2, 4}}) == false);
  REQUIRE(fullyContained(CleaningPair{CleaningRange{2, 4}, CleaningRange{1, 3}}) == false);
}

TEST_CASE("Partial Contained")
{
  // No overlap
  REQUIRE(partiallyContained(CleaningPair{CleaningRange{1, 2}, CleaningRange{3, 4}}) == false);
  REQUIRE(partiallyContained(CleaningPair{CleaningRange{3, 6}, CleaningRange{1, 2}}) == false);

  // Full overlap
  REQUIRE(partiallyContained(CleaningPair{CleaningRange{1, 4}, CleaningRange{2, 3}}) == true);
  REQUIRE(partiallyContained(CleaningPair{CleaningRange{2, 3}, CleaningRange{1, 4}}) == true);

  // Partial overlap
  REQUIRE(partiallyContained(CleaningPair{CleaningRange{1, 3}, CleaningRange{2, 4}}) == true);
  REQUIRE(partiallyContained(CleaningPair{CleaningRange{2, 4}, CleaningRange{1, 3}}) == true);
}

TEST_CASE("Example input")
{
  std::string input = 1 + R"(
2-4,6-8
2-3,4-5
5-7,7-9
2-8,3-7
6-6,4-6
2-6,4-8)";

  auto pairs = parsePairs(std::stringstream(input));

  REQUIRE(pairs.size() == 6);
  REQUIRE(pairs[0][0] == CleaningRange{2, 4});
  REQUIRE(pairs[5][1] == CleaningRange{4, 8});

  REQUIRE(countFullyContained(pairs) == 2);
}

#endif
