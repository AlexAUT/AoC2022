#include <common.hpp>

#include <set>

// #define RUN_TESTS

struct Rucksack
{
  std::vector<char> compartment1;
  std::vector<char> compartment2;

  std::vector<char> intersectionOfCompartments() const;
};

template <>
Rucksack fromString(std::string_view v)
{
  if (v.size() % 2 == 1)
    throw std::runtime_error("Rucksack odd number of items");

  std::size_t comp_len = v.size() / 2;

  return Rucksack{.compartment1 = std::vector<char>(v.begin(), v.begin() + comp_len),
                  .compartment2 = std::vector<char>(v.begin() + comp_len, v.end())};
}

std::vector<char> Rucksack::intersectionOfCompartments() const
{
  std::set<char> s1(compartment1.begin(), compartment1.end());
  std::set<char> s2(compartment2.begin(), compartment2.end());
  std::vector<char> inter;
  rv::set_intersection(s1, s2, rv::back_inserter(inter));

  return inter;
}

u32 priorityOfItem(char item)
{
  if (item <= 'Z') {
    return 52 - ('Z' - item);
  }
  if (item <= 'z') {
    return 26 - ('z' - item);
  }
  return 0;
}

std::vector<Rucksack> parseRucksacks(std::istream&& input)
{
  std::vector<Rucksack> rucksacks;
  std::string line;
  while (std::getline(input, line)) {
    rucksacks.push_back(fromString<Rucksack>(line));
  }

  return rucksacks;
}

u32 prioritiesOfIntersections(const std::vector<Rucksack>& rucksacks)
{
  return rv::accumulate(rucksacks | rv::views::transform([](const Rucksack& r) {
                          return priorityOfItem(r.intersectionOfCompartments()[0]);
                        }),
                        0u);
}

auto intersectionOfGroup = [](rv::viewable_range auto&& range) -> char {
  std::array<std::set<char>, 3> sets{};
  for (std::size_t index{}; const Rucksack& rucksack : range) {
    sets[index].insert(rucksack.compartment1.begin(), rucksack.compartment1.end());
    sets[index++].insert(rucksack.compartment2.begin(), rucksack.compartment2.end());
  }
  std::vector<char> v1;
  rv::set_intersection(sets[0], sets[1], rv::back_inserter(v1));
  std::set<char> i1(v1.begin(), v1.end());
  std::vector<char> v2;
  rv::set_intersection(i1, sets[2], rv::back_inserter(v2));

  return v2[0];
};

std::vector<char> intersectionOfGroups(const std::vector<Rucksack>& rucksacks)
{
  return rucksacks | rv::views::chunk(3) | rv::views::transform(intersectionOfGroup) | rv::to<std::vector<char>>();
}

u32 sumPrioritiesOfLabels(std::vector<char> labels)
{
  return rv::accumulate(labels | rv::views::transform([](char c) { return priorityOfItem(c); }), 0u);
}

// #ifndef RUN_TESTS
#include <fstream>

#ifndef RUN_TESTS
auto main() -> int
{
  auto rucksacks = parseRucksacks(std::fstream("../../src/day3/input.txt"));
  fmt::print("Task1 Result: {}\n", prioritiesOfIntersections(rucksacks));
  fmt::print("Task2 Result: {}\n", sumPrioritiesOfLabels(intersectionOfGroups(rucksacks)));
}

#else

TEST_CASE("Parse rucksack")
{
  Rucksack r = fromString<Rucksack>("aabbccdd");
  REQUIRE(r.compartment1.size() == r.compartment2.size());
  REQUIRE(r.compartment1 == std::vector<char>{'a', 'a', 'b', 'b'});
  REQUIRE(r.compartment2 == std::vector<char>{'c', 'c', 'd', 'd'});
}

TEST_CASE("Union of compartments")
{
  Rucksack r = fromString<Rucksack>("aabbccaa");
  std::vector<char> u = r.intersectionOfCompartments();
  REQUIRE(u.size() == 1);
  REQUIRE(u.front() == 'a');
}

TEST_CASE("Priority")
{
  REQUIRE(priorityOfItem('a') == 1);
  REQUIRE(priorityOfItem('z') == 26);
  REQUIRE(priorityOfItem('A') == 27);
  REQUIRE(priorityOfItem('Z') == 52);
}

TEST_CASE("Example Input")
{
  std::string input = 1 + R"(
vJrwpWtwJgWrhcsFMMfFFhFp
jqHRNqRjqzjGDLGLrsFMfFZSrLrFZsSL
PmmdzqPrVvPwwTWBwg
wMqvLMZHhHMvwLHjbvcjnnSBnvTQFn
ttgJtRGJQctTZtZT
CrZsJsPPZsGzwwsLwLmpwMDw)";

  auto rucksacks = parseRucksacks(std::stringstream(input));

  REQUIRE(rucksacks.size() == 6);

  REQUIRE(rucksacks[0].intersectionOfCompartments() == std::vector{'p'});
  REQUIRE(rucksacks[1].intersectionOfCompartments() == std::vector{'L'});
  REQUIRE(rucksacks[2].intersectionOfCompartments() == std::vector{'P'});
  REQUIRE(rucksacks[3].intersectionOfCompartments() == std::vector{'v'});
  REQUIRE(rucksacks[4].intersectionOfCompartments() == std::vector{'t'});
  REQUIRE(rucksacks[5].intersectionOfCompartments() == std::vector{'s'});

  REQUIRE(priorityOfItem(rucksacks[0].intersectionOfCompartments()[0]) == 16);
  REQUIRE(priorityOfItem(rucksacks[1].intersectionOfCompartments()[0]) == 38);
  REQUIRE(priorityOfItem(rucksacks[2].intersectionOfCompartments()[0]) == 42);
  REQUIRE(priorityOfItem(rucksacks[3].intersectionOfCompartments()[0]) == 22);
  REQUIRE(priorityOfItem(rucksacks[4].intersectionOfCompartments()[0]) == 20);
  REQUIRE(priorityOfItem(rucksacks[5].intersectionOfCompartments()[0]) == 19);

  REQUIRE(prioritiesOfIntersections(rucksacks) == 157);

  SECTION("Task2")
  {
    auto intersections = intersectionOfGroups(rucksacks);

    REQUIRE(intersections.size() == 2);
    REQUIRE(intersections[0] == 'r');
    REQUIRE(intersections[1] == 'Z');

    REQUIRE(sumPrioritiesOfLabels(intersections) == 70);
  }
}

#endif
