#include <common.hpp>

#include <variant>

// #define RUN_TESTS

struct List
{
  using Element = std::variant<List, int>;
  std::vector<Element> items;
};

List parseSequence(std::string_view v)
{
  List list;

  if (v.empty())
    return {};

  std::stack<std::reference_wrapper<List>> listStack;
  listStack.push(list);

  v = std::string_view(v.begin() + 1, v.end());

  while (!v.empty()) {
    if (v[0] == '[') {
      listStack.push(std::get<0>(listStack.top().get().items.emplace_back(List())));
      v.remove_prefix(1);
    } else if (v[0] == ']') {
      listStack.pop();
      v.remove_prefix(1);
    } else {
      auto end = v.find_first_of(",]");
      int value;
      std::from_chars(v.begin(), v.begin() + end, value);
      listStack.top().get().items.emplace_back(value);
      if (v[end] == ']')
        listStack.pop();

      v = v.substr(end + 1);
    }
  }

  return list;
}

std::vector<List> parseSequences(std::istream&& input)
{
  return rv::getlines(input) | rv::views::filter([](auto v) { return !v.empty(); }) |
         rv::views::transform(parseSequence) | rv::to<std::vector<List>>;
}

int compare(const List::Element& e1, const List::Element& e2)
{
  auto* v1 = std::get_if<int>(&e1);
  auto* v2 = std::get_if<int>(&e2);
  auto* l1 = std::get_if<List>(&e1);
  auto* l2 = std::get_if<List>(&e2);

  if (v1 && v2) {
    return *v2 - *v1;
  } else if (l1 && l2) {
    for (const auto& [child1, child2] : ranges::views::zip(l1->items, l2->items)) {
      int c = compare(child1, child2);
      if (c != 0)
        return c;
    }
    return static_cast<int>(l2->items.size()) - static_cast<int>(l1->items.size());
  } else if (l1 && v2) {
    return compare(*l1, List{{*v2}});
  } else if (v1 && l2) {
    return compare(List{{*v1}}, *l2);
  }
  throw std::runtime_error("Invalid graph?");
};

std::vector<bool> checkPairsSequenceOrder(const std::vector<List>& lists)
{
  return lists | rv::views::chunk(2) | rv::views::transform([](auto v) {
           return compare(*(v | rv::views::take(1)).begin(), *(v | rv::views::drop(1)).begin()) > 0;
         }) |
         rv::to<std::vector<bool>>;
}

u32 sumOfRightOrderIndices(const std::vector<bool>& orders)
{
  return ranges::accumulate(orders | ranges::views::enumerate | ranges::views::transform([](auto index_result) {
                              return std::get<1>(index_result) ? std::get<0>(index_result) + 1 : 0;
                            }),
                            0);
}

std::tuple<int, int> dividerLocations(const std::vector<List>& lists)
{
  List divider1{{List{{2}}}};
  List divider2{{List{{6}}}};
  std::tuple<int, int> dividerLocations{1, 2};
  for (const auto list : lists) {
    if (compare(list, divider1) >= 0)
      std::get<0>(dividerLocations)++;
    if (compare(list, divider2) >= 0)
      std::get<1>(dividerLocations)++;
  }

  return dividerLocations;
}

int dividerScore(const std::vector<List>& lists)
{
  auto locs = dividerLocations(lists);
  return std::get<0>(locs) * std::get<1>(locs);
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto sequences = parseSequences(std::fstream("../../src/day12/input.txt"));
  auto orders = checkPairsSequenceOrder(sequences);
  fmt::print("Task1 Result: {}\n", sumOfRightOrderIndices(orders));
  fmt::print("Task2 Result: {}\n", dividerScore(sequences));
}

#else

TEST_CASE("Test packets")
{

  auto isInOrder = [](std::string_view sequence1, std::string_view sequence2) {
    auto list1 = parseSequence(sequence1);
    auto list2 = parseSequence(sequence2);

    return compare(list1, list2) >= 0;
  };
  // REQUIRE(isInOrder("[]", "[]") == true);
  REQUIRE(isInOrder("[1]", "[1]") == true);
  REQUIRE(isInOrder("[1,1]", "[1,1]") == true);
  REQUIRE(isInOrder("[1,2]", "[4,4]") == true);
  REQUIRE(isInOrder("[1,2]", "[1,1]") == false);
  REQUIRE(isInOrder("[[]]", "[[]]") == true);
  REQUIRE(isInOrder("[[]]", "[[1]]") == true);
  REQUIRE(isInOrder("[[1]]", "[[]]") == false);
  REQUIRE(isInOrder("[[1]]", "[[1]]") == true);
  REQUIRE(isInOrder("[[1,2]]", "[[1,3]]") == true);
  REQUIRE(isInOrder("[1,1,3,1,1]", "[1,1,5,1,1]") == true);
  REQUIRE(isInOrder("[[1],[2,3,4]]", "[[1],4]") == true);
  REQUIRE(isInOrder("[9]", "[[8,7,6]]") == false);
  REQUIRE(isInOrder("[[4,4],4,4]", "[[4,4],4,4,4]") == true);
  REQUIRE(isInOrder("[7,7,7,7]", "[7,7,7]") == false);
  REQUIRE(isInOrder("[]", "[3]") == true);
  REQUIRE(isInOrder("[[[]]]", "[[]]") == false);
  REQUIRE(isInOrder("[1,[2,[3,[4,[5,6,7]]]],8,9]", "[1,[2,[3,[4,[5,6,0]]]],8,9]") == false);
}

TEST_CASE("Example Task1")
{
  std::string input = 1 + R"(
[1,1,3,1,1]
[1,1,5,1,1]

[[1],[2,3,4]]
[[1],4]

[9]
[[8,7,6]]

[[4,4],4,4]
[[4,4],4,4,4]

[7,7,7,7]
[7,7,7]

[]
[3]

[[[]]]
[[]]

[1,[2,[3,[4,[5,6,7]]]],8,9]
[1,[2,[3,[4,[5,6,0]]]],8,9])";

  auto sequences = parseSequences(std::stringstream(input));
  SECTION("Task1")
  {
    std::vector<bool> orders = checkPairsSequenceOrder(sequences);
    REQUIRE(orders == std::vector{true, true, false, true, false, true, false, false});
    REQUIRE(sumOfRightOrderIndices(orders) == 13);
  }

  SECTION("Task2")
  {
    REQUIRE(dividerLocations(sequences) == std::tuple(10, 14));
    REQUIRE(dividerScore(sequences) == 140);
  }
}

#endif
