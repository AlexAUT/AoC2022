#include <common.hpp>

#include <set>
// #define RUN_TESTS

std::string parseInput(std::istream&& stream)
{
  std::string content;
  std::getline(stream, content);
  return content;
}

std::size_t findUniqueSequence(std::string_view input, std::size_t length)
{
  // clang-format off
  auto result = input |
    ranges::views::sliding(length) |
    ranges::views::transform([length](auto view) {
      return ranges::accumulate(view, std::bitset<256>(), [](auto& set, auto c) {
               set.set(c);
               return set;
             }).count() == length;
    }) |
    ranges::views::enumerate |
    ranges::views::filter([](auto v) { return std::get<1>(v) == true; }) |
    ranges::views::transform([](auto t) { return std::get<0>(t); }) |
    ranges::views::take(1);
  // clang-format on

  if (result.empty())
    return 0;
  return *result.begin() + length;
}

std::size_t findMarker(std::string_view input)
{
  return findUniqueSequence(input, 4);
}

std::size_t findMessageMarker(std::string_view input)
{
  return findUniqueSequence(input, 14);
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  std::string input = parseInput(std::fstream("../../src/day6/input.txt"));
  fmt::print("Task1 Result: {}\n", findMarker(input));
  fmt::print("Task2 Result: {}\n", findMessageMarker(input));
}

#else

TEST_CASE("Read string")
{
  REQUIRE(parseInput(std::stringstream(std::string("asfaasasfasf"))) == "asfaasasfasf");
}

TEST_CASE("Find Marker")
{
  REQUIRE(findMarker("aaaa") == 0);
  REQUIRE(findMarker("aaad") == 0);
  REQUIRE(findMarker("bccd") == 0);
  REQUIRE(findMarker("abcd") == 4);
  REQUIRE(findMarker("abcdasfqwe") == 4);
  REQUIRE(findMarker("aabcd") == 5);
  REQUIRE(findMarker("abcae") == 5);
  REQUIRE(findMarker("aabcd") == 5);
  REQUIRE(findMarker("mjqjpqmgbljsphdztnvjfqwrcgsmlb") == 7);
  REQUIRE(findMarker("bvwbjplbgvbhsrlpgdmjqwftvncz") == 5);
  REQUIRE(findMarker("nppdvjthqldpwncqszvftbrmjlhg") == 6);
  REQUIRE(findMarker("nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg") == 10);
  REQUIRE(findMarker("zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw") == 11);

  REQUIRE(findMessageMarker("mjqjpqmgbljsphdztnvjfqwrcgsmlb") == 19);
  REQUIRE(findMessageMarker("bvwbjplbgvbhsrlpgdmjqwftvncz") == 23);
  REQUIRE(findMessageMarker("nppdvjthqldpwncqszvftbrmjlhg") == 23);
  REQUIRE(findMessageMarker("nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg") == 29);
  REQUIRE(findMessageMarker("zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw") == 26);
}

#endif
