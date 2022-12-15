#include <common.hpp>

// #define RUN_TESTS

using Sensor = std::pair<int, int>;
using Beacon = std::pair<int, int>;

std::pair<Sensor, Beacon> parseLine(std::string_view v)
{
  Sensor s;
  Beacon b;
  if (!scn::scan(v, "Sensor at x={}, y={}: closest beacon is at x={}, y={}", s.first, s.second, b.first, b.second))
    throw std::runtime_error("Parse error!");
  return {s, b};
}

std::vector<std::pair<Sensor, Beacon>> parse(std::istream&& input)
{
  return rn::getlines(input) | rv::transform(parseLine) | rn::to<std::vector<std::pair<Sensor, Beacon>>>;
}

auto manhattenDistance(auto pos1, auto pos2)
{
  return std::abs(std::get<0>(pos1) - std::get<0>(pos2)) + std::abs(std::get<1>(pos1) - std::get<1>(pos2));
}

std::set<int> blockedPositionsForRow(std::vector<std::pair<Sensor, Beacon>>& pairs, int row)
{
  std::set<int> blockedPositions;

  for (auto& [sensor, beacon] : pairs) {
    auto reach = manhattenDistance(sensor, beacon);

    int start = sensor.first - (reach - std::abs(sensor.second - row));
    int end = sensor.first + (reach - std::abs(sensor.second - row));
    for (int x = start; x <= end; x++) {
      if (std::make_pair(x, row) == beacon)
        continue;
      blockedPositions.insert(x);
    }
  }

  return blockedPositions;
}

std::pair<int, int> possibleLocationInArea(const std::vector<std::pair<Sensor, Beacon>>& pairs,
                                           std::pair<int, int> area)
{
  for (int y = area.first; y <= area.second; y++) {
    int x = area.first;
    while (x <= area.second) {
      bool possible{true};
      auto pos = std::make_pair(x, y);
      for (auto& [sensor, beacon] : pairs) {
        auto reach = manhattenDistance(sensor, beacon);
        auto distance = manhattenDistance(sensor, pos);
        if (reach >= distance) {
          // Shift to the end of this sensor
          x = sensor.first + (reach - std::abs(sensor.second - y)) + 1;
          possible = false;
          break;
        }
      }
      if (possible)
        return {x, y};
    }
  }

  throw std::runtime_error("No possible location found!");
}

int64_t tuningFrequency(std::pair<int, int> p)
{
  return static_cast<int64_t>(p.first) * 4000000ULL + static_cast<int64_t>(p.second);
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto pairs = parse(std::fstream("../../src/day15/input.txt"));
  fmt::print("Task1 Result: {}\n", blockedPositionsForRow(pairs, 2000000).size());
  auto loc = possibleLocationInArea(pairs, std::make_pair(0, 4000000));
  fmt::print("Task2 result: {}", tuningFrequency(loc));
}

#else

TEST_CASE("Day15")
{
  std::string input = 1 + R"(
Sensor at x=2, y=18: closest beacon is at x=-2, y=15
Sensor at x=9, y=16: closest beacon is at x=10, y=16
Sensor at x=13, y=2: closest beacon is at x=15, y=3
Sensor at x=12, y=14: closest beacon is at x=10, y=16
Sensor at x=10, y=20: closest beacon is at x=10, y=16
Sensor at x=14, y=17: closest beacon is at x=10, y=16
Sensor at x=8, y=7: closest beacon is at x=2, y=10
Sensor at x=2, y=0: closest beacon is at x=2, y=10
Sensor at x=0, y=11: closest beacon is at x=2, y=10
Sensor at x=20, y=14: closest beacon is at x=25, y=17
Sensor at x=17, y=20: closest beacon is at x=21, y=22
Sensor at x=16, y=7: closest beacon is at x=15, y=3
Sensor at x=14, y=3: closest beacon is at x=15, y=3
Sensor at x=20, y=1: closest beacon is at x=15, y=3)";

  SECTION("Parsing")
  {
    REQUIRE(parseLine("Sensor at x=2, y=18: closest beacon is at x=-2, y=15") ==
            std::make_pair(Sensor{2, 18}, Beacon{-2, 15}));
    auto pairs = parse(std::stringstream(input));
    REQUIRE(pairs.size() == 14);
    REQUIRE(pairs[0] == std::make_pair(Sensor{2, 18}, Beacon{-2, 15}));
    REQUIRE(pairs[13] == std::make_pair(Sensor{20, 1}, Beacon{15, 3}));
  }

  SECTION("Blocked positions on row")
  {
    auto pairs = parse(std::stringstream(input));
    auto blockedPositions = blockedPositionsForRow(pairs, 10);
    REQUIRE(blockedPositions.size() == 26);
  }

  SECTION("Possible location")
  {
    auto pairs = parse(std::stringstream(input));
    REQUIRE(possibleLocationInArea(pairs, {0, 20}) == std::make_pair(14, 11));
    REQUIRE(tuningFrequency(possibleLocationInArea(pairs, {0, 20})) == 56000011);
  }
}

#endif
