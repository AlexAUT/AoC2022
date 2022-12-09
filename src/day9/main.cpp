#include <common.hpp>

#include <set>

// #define RUN_TESTS

struct Movement
{
  enum class Dir
  {
    Up,
    Left,
    Down,
    Right
  } dir;
  i32 count;

  bool operator<=>(const Movement&) const = default;
};

template <>
Movement::Dir fromString(std::string_view v)
{
  if (v[0] == 'U')
    return Movement::Dir::Up;
  if (v[0] == 'L')
    return Movement::Dir::Left;
  if (v[0] == 'D')
    return Movement::Dir::Down;
  if (v[0] == 'R')
    return Movement::Dir::Right;

  throw std::runtime_error("Invalid direction");
}

template <>
Movement fromString(std::string_view v)
{
  return {.dir = fromString<Movement::Dir>(v), .count = std::atoi(std::string(v.substr(2)).c_str())};
}

std::vector<Movement> parseMovements(std::istream&& input)
{
  return ranges::getlines(input) | ranges::views::transform(fromString<Movement>) | ranges::to<std::vector<Movement>>;
}

struct Pos
{
  i32 x{};
  i32 y{};

  i32& operator[](int c)
  {
    return c == 0 ? x : y;
  }

  bool operator==(const Pos& o) const = default;
};

struct Rope
{
  Rope(std::size_t numberOfKnots, std::size_t knotIndexToTrack) :
      knots(numberOfKnots), knotIndexToTrack(knotIndexToTrack)
  {
  }
  std::vector<Pos> knots;
  std::size_t knotIndexToTrack;
  std::vector<Pos> uniqueKnotPosition = {{0, 0}};

  void execute(Movement movement)
  {
    auto tailValidPos = [this](std::size_t knotIndex) {
      return std::abs(knots[knotIndex - 1].x - knots[knotIndex].x) <= 1 &&
             std::abs(knots[knotIndex - 1].y - knots[knotIndex].y) <= 1;
    };

    auto updateKnots = [this, tailValidPos]() {
      for (std::size_t i = 1; i < knots.size(); i++) {
        if (!tailValidPos(i)) {
          for (int c{}; c < 2; c++) {
            auto diff = knots[i - 1][c] - knots[i][c];
            if (diff != 0) {
              knots[i][c] += std::signbit(diff) ? -1 : 1;
            }
          }
          if (i == knotIndexToTrack && !ranges::contains(uniqueKnotPosition, knots[knotIndexToTrack])) {
            uniqueKnotPosition.push_back(knots[knotIndexToTrack]);
          }
        }
      }
    };

    using D = Movement::Dir;
    for (i32 c = 0; c < movement.count; c++) {
      switch (movement.dir) {
      case D::Right: {
        knots[0].x++;
        updateKnots();
        break;
      }
      case D::Left: {
        knots[0].x--;
        updateKnots();
        break;
      }
      case D::Up: {
        knots[0].y++;
        updateKnots();
        break;
      }
      case D::Down: {
        knots[0].y--;
        updateKnots();
        break;
      }
      }
    }
  }
};

u64 countUniqueTailPositions(const std::vector<Movement>& movements, Rope rope)
{
  for (auto move : movements) {
    rope.execute(move);
  }

  return rope.uniqueKnotPosition.size();
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto movements = parseMovements(std::fstream("../../src/day9/input.txt"));
  fmt::print("Task1 Result: {}\n", countUniqueTailPositions(movements, Rope(2, 1)));
  fmt::print("Task2 Result: {}\n", countUniqueTailPositions(movements, Rope(10, 9)));
}

#else

TEST_CASE("Parsing")
{
  using D = Movement::Dir;
  REQUIRE(fromString<Movement>("U 1") == Movement{D::Up, 1});
  REQUIRE(fromString<Movement>("L 1") == Movement{D::Left, 1});
  REQUIRE(fromString<Movement>("R 1") == Movement{D::Right, 1});
  REQUIRE(fromString<Movement>("D 1") == Movement{D::Down, 1});

  REQUIRE(fromString<Movement>("U 3") == Movement{D::Up, 3});
  REQUIRE(fromString<Movement>("L 4") == Movement{D::Left, 4});
  REQUIRE(fromString<Movement>("R 5") == Movement{D::Right, 5});
  REQUIRE(fromString<Movement>("D 6") == Movement{D::Down, 6});

  std::string input = 1 + R"(
U 1
L 1
R 5
D 6)";
  auto movement = parseMovements(std::stringstream(input));

  REQUIRE(movement.size() == 4);
  REQUIRE(movement == std::vector{
                          Movement{D::Up, 1},
                          Movement{D::Left, 1},
                          Movement{D::Right, 5},
                          Movement{D::Down, 6},
                      });
}

TEST_CASE("Head tail moves")
{
  Rope rope(2, 1);
  REQUIRE(rope.knots == std::vector<Pos>{{0, 0}, {0, 0}});

  using D = Movement::Dir;

  rope.execute(Movement{D::Right, 1});
  REQUIRE(rope.knots == std::vector<Pos>{{1, 0}, {0, 0}});
  rope.execute(Movement{D::Right, 1});
  REQUIRE(rope.knots == std::vector<Pos>{{2, 0}, {1, 0}});
  rope.execute(Movement{D::Right, 2});
  REQUIRE(rope.knots == std::vector<Pos>{{4, 0}, {3, 0}});
  rope.execute(Movement{D::Left, 1});
  REQUIRE(rope.knots == std::vector<Pos>{{3, 0}, {3, 0}});
  rope.execute(Movement{D::Left, 2});
  REQUIRE(rope.knots == std::vector<Pos>{{1, 0}, {2, 0}});

  rope.execute(Movement{D::Up, 1});
  REQUIRE(rope.knots == std::vector<Pos>{{1, 1}, {2, 0}});
  rope.execute(Movement{D::Up, 1});
  REQUIRE(rope.knots == std::vector<Pos>{{1, 2}, {1, 1}});
  rope.execute(Movement{D::Up, 2});
  REQUIRE(rope.knots == std::vector<Pos>{{1, 4}, {1, 3}});
  rope.execute(Movement{D::Down, 1});
  REQUIRE(rope.knots == std::vector<Pos>{{1, 3}, {1, 3}});
  rope.execute(Movement{D::Down, 2});
  REQUIRE(rope.knots == std::vector<Pos>{{1, 1}, {1, 2}});
}

TEST_CASE("Unique tail positions")
{
  std::string input = 1 + R"(
R 4
U 4
L 3
D 1
R 4
D 1
L 5
R 2)";
  auto movements = parseMovements(std::stringstream(input));

  SECTION("Task1")
  {
    REQUIRE(countUniqueTailPositions(movements, Rope(2, 1)) == 13);
  }

  SECTION("Task2")
  {
    REQUIRE(countUniqueTailPositions(movements, Rope(10, 9)) == 1);

    std::string largerInput = 1 + R"(
R 5
U 8
L 8
D 3
R 17
D 10
L 25
U 20)";
    auto largerMovements = parseMovements(std::stringstream(largerInput));
    REQUIRE(countUniqueTailPositions(largerMovements, Rope(10, 9)) == 36);
  }
}

#endif
