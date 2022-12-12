#include <common.hpp>

// #define RUN_TESTS

using Map = std::vector<std::vector<char>>;

Map parseMap(std::istream&& input)
{
  return rv::getlines(input) | rv::views::transform([](auto v) { return std::vector<char>(v.begin(), v.end()); }) |
         rv::to<Map>();
}

std::tuple<std::vector<std::vector<u32>>, u32> calculateDistances(Map map)
{
  std::vector<std::vector<u32>> distances(map.size(), std::vector<u32>(map[0].size(), -1U));
  using Cursor = std::array<std::size_t, 2>;
  Cursor start{};
  Cursor target{};

  auto distance = [&](Cursor c) -> u32& { return distances[c[1]][c[0]]; };
  auto height = [&](Cursor c) -> char& { return map[c[1]][c[0]]; };

  for (std::size_t y = 0; y < map.size(); y++) {
    for (std::size_t x = 0; x < map[y].size(); x++) {
      if (height({x, y}) == 'S') {
        start = {x, y};
        height({x, y}) = 'a';
      } else if (height({x, y}) == 'E') {
        target = {x, y};
        height({x, y}) = 'z';
      }
    }
  }

  std::queue<Cursor> searchQueue;
  searchQueue.push(start);
  distance(start) = 0;

  while (!searchQueue.empty()) {

    auto c = searchQueue.front();
    searchQueue.pop();

    auto cursorHeight = height(c);
    auto cursorDistance = distance(c);

    auto processNeigh = [&](Cursor neigh) {
      if (distance(neigh) == -1U && height(neigh) <= cursorHeight + 1) {
        distance(neigh) = cursorDistance + 1;
        searchQueue.push(neigh);
      }
    };
    if (c[0] > 0)
      processNeigh(Cursor{c[0] - 1, c[1]});
    if (c[1] > 0)
      processNeigh(Cursor{c[0], c[1] - 1});
    if (c[0] < map[c[1]].size() - 1)
      processNeigh(Cursor{c[0] + 1, c[1]});
    if (c[1] < map.size() - 1)
      processNeigh(Cursor{c[0], c[1] + 1});
  }

  // for (auto& row : distances) {
  //   fmt::print("{}\n", fmt::join(row, ", "));
  // }

  auto targetDistance = distance(target);

  return {std::move(distances), targetDistance};
}

u32 calculateShortestPath(Map map)
{
  u32 shortest = -1U;
  // Remove starting S
  for (auto& row : map) {
    for (auto& col : row) {
      if (col == 'S') {
        col = 'a';
      }
    }
  }

  // Start from every A
  for (auto& row : map) {
    for (auto& col : row) {
      if (col == 'a') {
        col = 'S';
        auto [distances, length] = calculateDistances(map);
        shortest = std::min(shortest, length);
      }
    }
  }

  return shortest;
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto map = parseMap(std::fstream("../../src/day12/input.txt"));
  auto [distances, targetDistance] = calculateDistances(map);
  fmt::print("Task1 Result: {}\n", targetDistance);
  fmt::print("Task2 Result: {}\n", calculateShortestPath(map));
}

#else

TEST_CASE("Example Task1")
{
  auto map = parseMap(std::stringstream(1 + R"(
Sabqponm
abcryxxl
accszExk
acctuvwj
abdefghi)"));

  SECTION("Parsing")
  {
    REQUIRE(map.size() == 5);
    for (auto row : map) {
      REQUIRE(row.size() == 8);
    }
    REQUIRE(map[0] == std::vector<char>{'S', 'a', 'b', 'q', 'p', 'o', 'n', 'm'});
    REQUIRE(map[1] == std::vector<char>{'a', 'b', 'c', 'r', 'y', 'x', 'x', 'l'});
    REQUIRE(map[2] == std::vector<char>{'a', 'c', 'c', 's', 'z', 'E', 'x', 'k'});
    REQUIRE(map[3] == std::vector<char>{'a', 'c', 'c', 't', 'u', 'v', 'w', 'j'});
    REQUIRE(map[4] == std::vector<char>{'a', 'b', 'd', 'e', 'f', 'g', 'h', 'i'});
  }

  SECTION("Find distances")
  {
    auto [distances, targetDistance] = calculateDistances(map);
    REQUIRE(distances[2][5] == 31);
    REQUIRE(targetDistance == 31);
  }
  SECTION("Reverse distance")
  {
    auto reverseDistance = calculateShortestPath(map);
    REQUIRE(reverseDistance == 29);
  }
}

#endif
