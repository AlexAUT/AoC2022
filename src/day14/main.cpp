#include <common.hpp>

// #define RUN_TESTS

struct Point
{
  int x{};
  int y{};

  bool operator==(const Point& p) const = default;
};

using Line = std::vector<Point>;

Point pointFromRng(auto v)
{
  Point p;
  std::string v1 = v | rn::to<std::string>;
  if (!scn::scan(v1 | rv::replace(',', ' '), "{} {}", p.x, p.y))
    throw std::runtime_error("Invalid point!");
  return p;
}

template <>
Line fromString(std::string_view v)
{
  auto filter = [](char c) { return c != '-' && c != '>'; };
  return v | rv::filter(filter) | rv::split(' ') | rv::stride(2) |
         rv::transform([](auto v) { return pointFromRng(v); }) | rn::to<Line>;
}

std::vector<Line> parseLines(std::istream&& input)
{
  return rn::getlines(input) | rv::transform(fromString<Line>) | rn::to<std::vector<Line>>;
}

struct Map
{
  enum class Tile
  {
    Empty,
    Rock,
    Sand,
  };

  Point min;
  Point max;
  bool hasFloor;

  using TileMap = std::vector<std::vector<Tile>>;
  TileMap tiles = TileMap(max.x - min.x + 1, std::vector<Tile>(max.y - min.y + 1));

  Tile& operator[](Point p)
  {
    if (p.x < min.x || p.x > max.x || p.y < min.y || p.y > max.y)
      throw std::runtime_error("Out of bounds!");
    return tiles[p.x - min.x][p.y - min.y];
  }
};

Map createMap(const std::vector<Line>& lines, bool enableFoor)
{
  auto maxX = rn::max(lines | rv::join | rv::transform([](Point p) { return p.x; }));
  auto maxY = rn::max(lines | rv::join | rv::transform([](Point p) { return p.y; }));

  auto minX = rn::min(lines | rv::join | rv::transform([](Point p) { return p.x; }));
  auto minY = 0;

  if (enableFoor) {
    maxY += 2;
    auto height = maxY;
    minX -= height;
    maxX += height;
  }

  Map map{.min{minX, minY}, .max{maxX, maxY}, .hasFloor = enableFoor};

  // lines = std::vector<std::vector<Point>>
  for (const auto& line : lines) {
    for (const auto& segment : line | rv::sliding(2)) {
      auto startX = std::min(segment[0].x, segment[1].x);
      auto startY = std::min(segment[0].y, segment[1].y);
      auto endX = std::max(segment[0].x, segment[1].x);
      auto endY = std::max(segment[0].y, segment[1].y);

      for (int x = startX; x <= endX; x++) {
        for (int y = startY; y <= endY; y++) {
          map[Point{x, y}] = Map::Tile::Rock;
        }
      }
    }
  }

  return map;
}

u32 countUntilSandOffMap(Map map)
{
  using T = Map::Tile;
  for (u32 i = 0; i < std::numeric_limits<u32>::max(); i++) {
    Point loc{500, 0};

    try {
      while (map[loc] == T::Empty) {
        if (map.hasFloor && loc.y == map.max.y - 1) {
          map[loc] = T::Sand;
        } else if (map[Point{loc.x, loc.y + 1}] == T::Empty) {
          loc.y += 1;
        } else if (map[Point{loc.x - 1, loc.y + 1}] == T::Empty) {
          loc.x -= 1;
          loc.y += 1;
        } else if (map[Point{loc.x + 1, loc.y + 1}] == T::Empty) {
          loc.x += 1;
          loc.y += 1;
        } else {
          map[loc] = T::Sand;
        }
      }

      if (map.hasFloor && map[Point{500, 0}] == T::Sand) {
        return i + 1;
      }
    } catch (const std::runtime_error& e) {
      if (map.hasFloor)
        throw std::runtime_error("Out of bounds with floor?");
      return i;
    }
  }
  throw std::runtime_error("Overflow in iterations");
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto lines = parseLines(std::fstream("../../src/day14/input.txt"));
  {
    auto map = createMap(lines, false);
    fmt::print("Task1 Result: {}\n", countUntilSandOffMap(map));
  }
  {
    auto map = createMap(lines, true);
    fmt::print("Task2 Result: {}\n", countUntilSandOffMap(map));
  }
}

#else

TEST_CASE("Parsing")
{
  SECTION("Points")
  {
    for (int x = 400; x < 600; x++) {
      for (int y = 400; y < 600; y++) {
        REQUIRE(pointFromRng(fmt::format("{},{}", x, y)) == Point({x, y}));
      }
    }
  }
  SECTION("Multiple Points")
  {
    REQUIRE(fromString<std::vector<Point>>("10,10 -> 20,20") == std::vector<Point>{{10, 10}, {20, 20}});
    REQUIRE(fromString<std::vector<Point>>("10,10 -> 20,20 -> 30,30") ==
            std::vector<Point>{{10, 10}, {20, 20}, {30, 30}});
  }

  SECTION("Parse Lines")
  {
    std::string input = 1 + R"(
498,4 -> 498,6 -> 496,6
503,4 -> 502,4 -> 502,9 -> 494,9)";

    auto lines = parseLines(std::stringstream(input));

    REQUIRE(lines == std::vector{Line{{498, 4}, {498, 6}, {496, 6}}, Line{{503, 4}, {502, 4}, {502, 9}, {494, 9}}});
  }
}

TEST_CASE("Example")
{
  std::string input = 1 + R"(
498,4 -> 498,6 -> 496,6
503,4 -> 502,4 -> 502,9 -> 494,9)";

  auto lines = parseLines(std::stringstream(input));

  SECTION("Task1")
  {
    auto map = createMap(lines, false);
    REQUIRE(map.max == Point{503, 9});
    REQUIRE(map.min == Point{494, 0});

    REQUIRE(map[Point{503, 4}] == Map::Tile::Rock);
    REQUIRE(map[Point{502, 9}] == Map::Tile::Rock);
    REQUIRE(map[Point{496, 6}] == Map::Tile::Rock);
    REQUIRE(map[Point{494, 9}] == Map::Tile::Rock);
    REQUIRE(map[Point{500, 0}] == Map::Tile::Empty);

    REQUIRE(countUntilSandOffMap(createMap(lines, false)) == 24);
    REQUIRE(countUntilSandOffMap(createMap(lines, true)) == 93);
  }
}

#endif
