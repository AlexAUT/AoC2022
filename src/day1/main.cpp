#include <common.hpp>

// #define RUN_TESTS

auto parseCalories(std::istream&& input) -> std::vector<u64>
{
  std::vector<u64> elf_calories;

  std::string line;
  while (std::getline(input, line)) {
    if (elf_calories.empty()) {
      elf_calories.emplace_back();
    }
    if (line.empty()) {
      elf_calories.emplace_back();
    } else {
      elf_calories.back() += std::atoi(line.c_str());
    }
  }
  return elf_calories;
}

auto maxCalories(const std::vector<u64>& calories) -> u64
{
  if (calories.empty()) {
    return 0;
  }
  return *rv::max_element(calories);
}

auto sumOfTop3Calories(std::vector<u64> calories) -> u64
{
  rv::sort(calories);
  return rv::accumulate(rv::views::reverse(calories) | rv::views::take(3), 0UL);
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto calories = parseCalories(std::fstream("../../src/day1/input.txt"));
  fmt::print("Task1 Result: {}", maxCalories(calories));
  fmt::print("Task2 Result: {}", sumOfTop3Calories(calories));
}

#else
TEST_CASE("Empty input")
{
  auto calories = parseCalories(std::stringstream(""));
  REQUIRE(calories.empty());
};

TEST_CASE("Test max")
{
  REQUIRE(maxCalories({}) == 0);
  REQUIRE(maxCalories({10, 100, 10}) == 100);

  REQUIRE(sumOfTop3Calories({}) == 0);
  REQUIRE(sumOfTop3Calories({10}) == 10);
  REQUIRE(sumOfTop3Calories({1, 3}) == 4);
  REQUIRE(sumOfTop3Calories({5, 3, 1}) == 9);
  REQUIRE(sumOfTop3Calories({0, 0, 0}) == 0);
  REQUIRE(sumOfTop3Calories({10, 100, 100, 1000}) == 1200);
}

TEST_CASE("Example input")
{
  std::string input = 1 + R"(
1000
2000
3000

4000

5000
6000

7000
8000
9000

10000)";
  std::vector<u64> calories = parseCalories(std::stringstream(input));

  REQUIRE(calories.size() == 5);
  REQUIRE(calories[0] == 6000);
  REQUIRE(calories[1] == 4000);
  REQUIRE(calories[2] == 11000);
  REQUIRE(calories[3] == 24000);
  REQUIRE(calories[4] == 10000);

  REQUIRE(maxCalories(calories) == 24000);
  REQUIRE(sumOfTop3Calories(calories) == 45000);
}
#endif
