#include <common.hpp>

// #define RUN_TESTS

struct Move
{
  std::size_t count;
  std::size_t from;
  std::size_t to;

  auto operator<=>(const Move&) const = default;
};

char parseCrate(std::string_view v)
{
  return v[1];
}

template <>
Move fromString(std::string_view v)
{
  std::stringstream sstr(std::string{v});
  std::string dummy;
  Move move;
  sstr >> dummy >> move.count >> dummy >> move.from >> dummy >> move.to;
  return move;
}

std::tuple<std::vector<std::vector<char>>, std::vector<Move>> parseInput(std::istream&& input)
{
  std::vector<std::vector<char>> stacks;
  std::vector<Move> moves;
  std::string line;
  while (std::getline(input, line)) {
    if (line[0] != 'm') {
      for (std::size_t pos = 0; pos < line.size(); pos += 4) {
        std::size_t stackIndex = pos / 4;
        if (line[pos] == '[') {
          stacks.resize(std::max(stacks.size(), stackIndex + 1));
          stacks[stackIndex].insert(stacks[stackIndex].begin(), parseCrate(line.substr(pos, 3)));
        } else if (line[pos == ' ']) {
        }
      }
    } else {
      moves.push_back(fromString<Move>(line));
    }
  }
  return {stacks, moves};
}

void executeMovesCrateMover9000(std::vector<std::vector<char>>& stacks, const std::vector<Move>& moves)
{
  for (auto move : moves) {
    for (std::size_t i = 0; i < move.count; i++) {
      stacks[move.to - 1].push_back(stacks[move.from - 1].back());
      stacks[move.from - 1].pop_back();
    }
  }
}

void executeMovesCrateMover9001(std::vector<std::vector<char>>& stacks, const std::vector<Move>& moves)
{
  for (auto move : moves) {
    auto& from = stacks[move.from - 1];
    auto& to = stacks[move.to - 1];
    auto count = move.count;

    to.resize(to.size() + count);
    std::copy(from.end() - count, from.end(), to.end() - count);
    from.resize(from.size() - count);
  }
}

std::string getTopCrates(const std::vector<std::vector<char>> stacks)
{
  std::string output;
  output.reserve(stacks.size());
  for (auto& stack : stacks) {
    output += stack.back();
  }
  return output;
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  {
    auto [stacks, moves] = parseInput(std::fstream("../../src/day5/input.txt"));
    executeMovesCrateMover9000(stacks, moves);
    fmt::print("Task1 Result: {}\n", getTopCrates(stacks));
  }
  {
    auto [stacks, moves] = parseInput(std::fstream("../../src/day5/input.txt"));
    executeMovesCrateMover9001(stacks, moves);
    fmt::print("Task2 Result: {}\n", getTopCrates(stacks));
  }
}

#else

TEST_CASE("Parse crate")
{
  REQUIRE(parseCrate("[A]") == 'A');
  REQUIRE(parseCrate("[Z]") == 'Z');
}

TEST_CASE("Parse crate stacks")
{
  std::string input = 1 + R"(
    [D]
[N] [C]
[Z] [M] [P]
 1   2   3)";

  auto [stacks, moves] = parseInput(std::stringstream(input));
  REQUIRE(stacks.size() == 3);
  REQUIRE(stacks[0] == std::vector{'Z', 'N'});
  REQUIRE(stacks[1] == std::vector{'M', 'C', 'D'});
  REQUIRE(stacks[2] == std::vector{'P'});
}

TEST_CASE("Parse moves")
{
  std::string input = 1 + R"(
move 1 from 2 to 1
move 3 from 1 to 3
move 2 from 2 to 1
move 1 from 1 to 2)";

  auto [stacks, moves] = parseInput(std::stringstream(input));

  REQUIRE(moves.size() == 4);
  REQUIRE(moves[0] == Move{1, 2, 1});
  REQUIRE(moves[1] == Move{3, 1, 3});
  REQUIRE(moves[2] == Move{2, 2, 1});
  REQUIRE(moves[3] == Move{1, 1, 2});
}

TEST_CASE("Execute moves 9000 + print")
{
  std::string input = 1 + R"(
    [D]
[N] [C]
[Z] [M] [P]
 1   2   3
move 1 from 2 to 1
move 3 from 1 to 3
move 2 from 2 to 1
move 1 from 1 to 2)";
  auto [stacks, moves] = parseInput(std::stringstream(input));

  REQUIRE(stacks.size() == 3);
  REQUIRE(moves.size() == 4);

  executeMovesCrateMover9000(stacks, moves);

  REQUIRE(stacks[0] == std::vector<char>{'C'});
  REQUIRE(stacks[1] == std::vector<char>{'M'});
  REQUIRE(stacks[2] == std::vector<char>{'P', 'D', 'N', 'Z'});

  REQUIRE(getTopCrates(stacks) == "CMZ");
}

TEST_CASE("Execute moves 9001 + print")
{
  std::string input = 1 + R"(
    [D]
[N] [C]
[Z] [M] [P]
 1   2   3
move 1 from 2 to 1
move 3 from 1 to 3
move 2 from 2 to 1
move 1 from 1 to 2)";
  auto [stacks, moves] = parseInput(std::stringstream(input));

  REQUIRE(stacks.size() == 3);
  REQUIRE(moves.size() == 4);

  executeMovesCrateMover9001(stacks, moves);

  REQUIRE(stacks[0] == std::vector<char>{'M'});
  REQUIRE(stacks[1] == std::vector<char>{'C'});
  REQUIRE(stacks[2] == std::vector<char>{'P', 'Z', 'N', 'D'});

  REQUIRE(getTopCrates(stacks) == "MCD");
}

#endif
