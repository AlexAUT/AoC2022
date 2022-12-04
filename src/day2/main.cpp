#include <common.hpp>

// #define RUN_TESTS

enum class Action
{
  Rock,
  Paper,
  Scissors
};

template <typename T>
T fromString(std::string_view v);

template <>
Action fromString(std::string_view v)
{
  if (v.compare("A") == 0 || v.compare("X") == 0)
    return Action::Rock;
  if (v.compare("B") == 0 || v.compare("Y") == 0)
    return Action::Paper;
  if (v.compare("C") == 0 || v.compare("Z") == 0)
    return Action::Scissors;

  throw std::runtime_error("Invalid enum");
}

struct Game
{
  Action player1;
  Action player2;
};

enum class Result
{
  Win,
  Loss,
  Draw
};

template <>
Result fromString(std::string_view v)
{
  if (v.compare("X") == 0)
    return Result::Loss;
  if (v.compare("Y") == 0)
    return Result::Draw;
  if (v.compare("Z") == 0)
    return Result::Win;

  throw std::runtime_error("Invalid enum");
}

std::vector<Game> parseGames(std::istream&& input)
{
  std::vector<Game> games;
  std::string line;
  while (std::getline(input, line)) {
    auto splitPos = line.find_first_of(' ');
    games.push_back(Game{.player1 = fromString<Action>(std::string_view(line.data(), splitPos)),
                         .player2 = fromString<Action>(std::string_view(line.data() + splitPos + 1))});
  }
  return games;
}

u32 scoreAction(Action action)
{
  switch (action) {
  case Action::Rock:
    return 1;
  case Action::Paper:
    return 2;
  case Action::Scissors:
    return 3;
  }
  unreachable();
}

Result evaluateResult(Game game)
{
  if (game.player1 == game.player2) {
    return Result::Draw;
  }
  int a1 = static_cast<int>(game.player1);
  int a2 = static_cast<int>(game.player2);

  if (a2 == ((a1 + 2) % 3))
    return Result::Loss;
  return Result::Win;
}

u32 scoreResult(Result result)
{
  switch (result) {
  case Result::Win:
    return 6;
  case Result::Loss:
    return 0;
  case Result::Draw:
    return 3;
  }

  unreachable();
}

u32 scoreGame(Game game)
{
  return scoreResult(evaluateResult(game)) + scoreAction(game.player2);
}

u32 scoreGames(const std::vector<Game>& games)
{
  return rv::accumulate(games | rv::views::transform([](Game g) { return scoreGame(g); }), 0u);
}

Action requiredAction(Action player1, Result result)
{
  if (result == Result::Draw)
    return player1;
  if (result == Result::Loss)
    return static_cast<Action>((static_cast<int>(player1) + 2) % 3);
  else
    return static_cast<Action>((static_cast<int>(player1) + 1) % 3);
}

std::vector<Game> parseGamesTask2(std::istream&& input)
{
  std::vector<Game> games;
  std::string line;
  while (std::getline(input, line)) {
    auto splitPos = line.find_first_of(' ');
    Action player1 = fromString<Action>(std::string_view(line.data(), splitPos));
    Result result = fromString<Result>(std::string_view(line.data() + splitPos + 1));
    games.push_back(Game{.player1 = player1, .player2 = requiredAction(player1, result)});
  }
  return games;
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto gamesTask1 = parseGames(std::fstream("../../src/day2/input.txt"));
  auto gamesTask2 = parseGamesTask2(std::fstream("../../src/day2/input.txt"));
  fmt::print("Task1 Result: {}\n", scoreGames(gamesTask1));
  fmt::print("Task2 Result: {}\n", scoreGames(gamesTask2));
}

#else
TEST_CASE("Parse input")
{
  REQUIRE(fromString<Action>("A") == Action::Rock);
  REQUIRE(fromString<Action>("B") == Action::Paper);
  REQUIRE(fromString<Action>("C") == Action::Scissors);
  REQUIRE(fromString<Action>("X") == Action::Rock);
  REQUIRE(fromString<Action>("Y") == Action::Paper);
  REQUIRE(fromString<Action>("Z") == Action::Scissors);
};

TEST_CASE("Score Action")
{
  REQUIRE(scoreAction(Action::Rock) == 1);
  REQUIRE(scoreAction(Action::Paper) == 2);
  REQUIRE(scoreAction(Action::Scissors) == 3);
}

TEST_CASE("Evaluate Result")
{
  SECTION("Draw")
  {
    REQUIRE(evaluateResult({Action::Rock, Action::Rock}) == Result::Draw);
    REQUIRE(evaluateResult({Action::Paper, Action::Paper}) == Result::Draw);
    REQUIRE(evaluateResult({Action::Scissors, Action::Scissors}) == Result::Draw);
  }
  SECTION("Win")
  {
    REQUIRE(evaluateResult({Action::Rock, Action::Paper}) == Result::Win);
    REQUIRE(evaluateResult({Action::Paper, Action::Scissors}) == Result::Win);
    REQUIRE(evaluateResult({Action::Scissors, Action::Rock}) == Result::Win);
  }
  SECTION("Loss")
  {
    REQUIRE(evaluateResult({Action::Rock, Action::Scissors}) == Result::Loss);
    REQUIRE(evaluateResult({Action::Paper, Action::Rock}) == Result::Loss);
    REQUIRE(evaluateResult({Action::Scissors, Action::Paper}) == Result::Loss);
  }
}

TEST_CASE("Score Result")
{
  REQUIRE(scoreResult(Result::Win) == 6);
  REQUIRE(scoreResult(Result::Draw) == 3);
  REQUIRE(scoreResult(Result::Loss) == 0);
}

TEST_CASE("Example input Task1")
{
  std::string input = 1 + R"(
A Y
B X
C Z)";

  auto games = parseGames(std::stringstream(input));

  REQUIRE(games.size() == 3);
  REQUIRE(games[0].player1 == Action::Rock);
  REQUIRE(games[0].player2 == Action::Paper);
  REQUIRE(games[1].player1 == Action::Paper);
  REQUIRE(games[1].player2 == Action::Rock);
  REQUIRE(games[2].player1 == Action::Scissors);
  REQUIRE(games[2].player2 == Action::Scissors);

  REQUIRE(scoreGame(games[0]) == 8);
  REQUIRE(scoreGame(games[1]) == 1);
  REQUIRE(scoreGame(games[2]) == 6);

  REQUIRE(scoreGames(games) == (scoreGame(games[0]) + scoreGame(games[1]) + scoreGame(games[2])));
}

TEST_CASE("Parse Result")
{
  REQUIRE(fromString<Result>("X") == Result::Loss);
  REQUIRE(fromString<Result>("Y") == Result::Draw);
  REQUIRE(fromString<Result>("Z") == Result::Win);
}

TEST_CASE("Required Action")
{
  REQUIRE(requiredAction(Action::Rock, Result::Win) == Action::Paper);
  REQUIRE(requiredAction(Action::Rock, Result::Draw) == Action::Rock);
  REQUIRE(requiredAction(Action::Rock, Result::Loss) == Action::Scissors);

  REQUIRE(requiredAction(Action::Paper, Result::Win) == Action::Scissors);
  REQUIRE(requiredAction(Action::Paper, Result::Draw) == Action::Paper);
  REQUIRE(requiredAction(Action::Paper, Result::Loss) == Action::Rock);

  REQUIRE(requiredAction(Action::Scissors, Result::Win) == Action::Rock);
  REQUIRE(requiredAction(Action::Scissors, Result::Draw) == Action::Scissors);
  REQUIRE(requiredAction(Action::Scissors, Result::Loss) == Action::Paper);
}

TEST_CASE("Example input Task2")
{
  std::string input = 1 + R"(
A Y
B X
C Z)";

  auto games = parseGamesTask2(std::stringstream(input));

  REQUIRE(games.size() == 3);
  REQUIRE(games[0].player1 == Action::Rock);
  REQUIRE(games[0].player2 == Action::Rock);
  REQUIRE(games[1].player1 == Action::Paper);
  REQUIRE(games[1].player2 == Action::Rock);
  REQUIRE(games[2].player1 == Action::Scissors);
  REQUIRE(games[2].player2 == Action::Rock);

  REQUIRE(scoreGame(games[0]) == 4);
  REQUIRE(scoreGame(games[1]) == 1);
  REQUIRE(scoreGame(games[2]) == 7);

  REQUIRE(scoreGames(games) == (scoreGame(games[0]) + scoreGame(games[1]) + scoreGame(games[2])));
}
#endif
