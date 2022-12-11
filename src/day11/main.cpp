#include <common.hpp>

// #define RUN_TESTS

struct Monkey
{
  std::vector<u64> items;
  std::function<u64(u64)> operation;
  std::function<u64(u64)> test;
  u64 divisbleBy;
};

template <>
Monkey fromString(std::string_view v)
{
  auto getline = [](std::string_view v) -> std::tuple<std::string_view, std::string_view> {
    return {v.substr(0, v.find_first_of('\n')), v.substr(v.find_first_of('\n') + 1)};
  };

  Monkey monkey;
  // Starting items
  auto [line, rest] = getline(v);
  line.remove_prefix("  Starting items: "sv.size());
  if (!scn::scan_list_ex(line, monkey.items, scn::list_separator(',')))
    throw std::runtime_error("Parsing error");

  // Operation
  std::tie(line, rest) = getline(rest);
  line.remove_prefix("  Operation: new = "sv.size());
  std::string arg1;
  std::string op;
  std::string arg2;
  if (!scn::scan(line, "{} {} {}", arg1, op, arg2))
    throw std::runtime_error("Parsing error");

  monkey.operation = [arg1, op, arg2](u64 old) {
    u64 x = arg1 == "old" ? old : static_cast<u64>(std::stoi(arg1));
    u64 y = arg2 == "old" ? old : static_cast<u64>(std::stoi(arg2));
    if (op == "+")
      return x + y;
    if (op == "*")
      return x * y;
    throw std::runtime_error("Invalid operation");
  };

  // Test
  std::tie(line, rest) = getline(rest);
  line.remove_prefix("  Test: divisible by "sv.size());
  u64 divisibleBy;
  if (!scn::scan(line, "{}", divisibleBy))
    throw std::runtime_error("Parsing error");
  monkey.divisbleBy = divisibleBy;

  std::tie(line, rest) = getline(rest);
  line.remove_prefix("    If true: throw to monkey "sv.size());
  u64 monkeyIfTrue;
  if (!scn::scan(line, "{}", monkeyIfTrue))
    throw std::runtime_error("Parsing error");

  std::tie(line, rest) = getline(rest);
  line.remove_prefix("    If false: throw to monkey "sv.size());
  u64 monkeyIfFalse;
  if (!scn::scan(line, "{}", monkeyIfFalse))
    throw std::runtime_error("Parsing error");

  monkey.test = [=](u64 value) { return (value % divisibleBy == 0) ? monkeyIfTrue : monkeyIfFalse; };

  return monkey;
}

std::vector<Monkey> parseMonkeys(std::istream&& input)
{
  return ranges::getlines(input) | ranges::views::chunk(7) | ranges::views::transform(ranges::views::drop(1)) |
         ranges::views::transform([](auto v) {
           std::string monkeyDesc;
           for (auto& l : v) {
             monkeyDesc += l;
             monkeyDesc += '\n';
           }
           return monkeyDesc;
         }) |
         ranges::views::transform(fromString<Monkey>) | ranges::to<std::vector<Monkey>>;
}

// Returns monkey inspect count
std::vector<u64> simulateRounds(std::vector<Monkey> monkeys, u32 numberOfRounds, bool divideWorryLevel = true)
{
  u64 ringValue = ranges::accumulate(monkeys, 1ull, [](u64 v, const Monkey& m) { return v * m.divisbleBy; });
  std::vector<u64> monkeyInspectCounts(monkeys.size());
  for (u32 r = 0; r < numberOfRounds; r++) {
    for (std::size_t monkeyId{}; monkeyId < monkeys.size(); monkeyId++) {
      auto& monkey = monkeys[monkeyId];

      // Inspect items
      monkeyInspectCounts[monkeyId] += monkey.items.size();
      for (auto& item : monkey.items) {
        item = monkey.operation(item);
        if (divideWorryLevel) {
          item /= 3;
        } else {
          item = item % ringValue;
        }
        auto throw_to = monkey.test(item);
        monkeys[throw_to].items.push_back(item);
      }
      monkey.items.clear();
    }
  }
  return monkeyInspectCounts;
}

u64 calculateMonkeyBusiness(std::vector<u64> inspectCounts)
{
  auto maxElement = ranges::max_element(inspectCounts);
  u64 max = *maxElement;
  *maxElement = 0;
  maxElement = ranges::max_element(inspectCounts);
  u64 secondMax = *maxElement;
  return max * secondMax;
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  {
    auto monkeys = parseMonkeys(std::fstream("../../src/day11/input.txt"));
    auto inspectCounts = simulateRounds(monkeys, 20);
    fmt::print("Task1 Result: {}\n", calculateMonkeyBusiness(inspectCounts));
  }
  {
    auto monkeys = parseMonkeys(std::fstream("../../src/day11/input.txt"));
    auto inspectCounts = simulateRounds(monkeys, 10000, false);
    fmt::print("Task2 Result: {}\n", calculateMonkeyBusiness(inspectCounts));
  }
}

#else

TEST_CASE("Parse Monkey")
{
  std::string input = 1 + R"(
  Starting items: 79, 98
  Operation: new = old * 19
  Test: divisible by 23
    If true: throw to monkey 2
    If false: throw to monkey 3)";

  auto monkey = fromString<Monkey>(input);

  SECTION("Starting items")
  {
    REQUIRE(monkey.items == std::vector<u64>{79, 98});
  }

  SECTION("Operation")
  {
    for (int i = 0; i < 100; i++) {
      REQUIRE(monkey.operation(i) == i * 19);
    }
  }

  SECTION("Test")
  {
    for (int i = 0; i < 100; i++) {
      REQUIRE(monkey.test(i) == ((i % 23 == 0) ? 2 : 3));
    }
  }
}

TEST_CASE("Example")
{
  std::string input = 1 + R"(
Monkey 0:
  Starting items: 79, 98
  Operation: new = old * 19
  Test: divisible by 23
    If true: throw to monkey 2
    If false: throw to monkey 3

Monkey 1:
  Starting items: 54, 65, 75, 74
  Operation: new = old + 6
  Test: divisible by 19
    If true: throw to monkey 2
    If false: throw to monkey 0

Monkey 2:
  Starting items: 79, 60, 97
  Operation: new = old * old
  Test: divisible by 13
    If true: throw to monkey 1
    If false: throw to monkey 3

Monkey 3:
  Starting items: 74
  Operation: new = old + 3
  Test: divisible by 17
    If true: throw to monkey 0
    If false: throw to monkey 1)";

  auto monkeys = parseMonkeys(std::stringstream(input));

  SECTION("Parse monkeys")
  {
    REQUIRE(monkeys.size() == 4);
    REQUIRE(monkeys[0].items == std::vector<u64>{79, 98});
    REQUIRE(monkeys[3].items == std::vector<u64>{74});
    REQUIRE(monkeys[3].operation(3) == 6);
    REQUIRE(monkeys[3].test(17) == 0);
    REQUIRE(monkeys[3].test(18) == 1);
  }

  SECTION("Simulate Task1")
  {
    auto inspectCounts = simulateRounds(monkeys, 20);
    REQUIRE(inspectCounts.size() == 4);
    REQUIRE(inspectCounts == std::vector<u64>{101, 95, 7, 105});
    REQUIRE(calculateMonkeyBusiness(inspectCounts) == 10605);
  }
  SECTION("Simulate Task2")
  {
    auto inspectCounts = simulateRounds(monkeys, 10000, false);
    REQUIRE(simulateRounds(monkeys, 1, false) == std::vector<u64>{2, 4, 3, 6});
    REQUIRE(simulateRounds(monkeys, 20, false) == std::vector<u64>{99, 97, 8, 103});
    REQUIRE(calculateMonkeyBusiness(inspectCounts) == 2713310158);
  }
}

#endif
