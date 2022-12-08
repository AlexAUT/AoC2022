#include <common.hpp>

#define RUN_TESTS

using Wood = std::vector<std::vector<u32>>;

Wood parseWood(std::istream&& input)
{
  // using namespace ranges;
  using namespace ranges::views;
  using ranges::getlines;
  using ranges::to;
  return getlines(input) |
         transform([](auto& view) { return view | transform([](char c) { return c - '0'; }) | to<std::vector<u32>>; }) |
         to<std::vector<std::vector<u32>>>;
}

bool isTreeVisible(const Wood wood, std::size_t x, std::size_t y)
{
  using namespace ranges;
  auto view_left = wood[y] | views::take(x);
  auto view_right = wood[y] | views::drop(x + 1);

  auto view_topdown = wood | views::transform([x](const auto& view) { return view[x]; });
  auto view_top = view_topdown | views::take(y);
  auto view_bottom = view_topdown | views::drop(y + 1);

  auto isSmaller = [compare = wood[y][x]](u32 h) { return h < compare; };

  return ranges::all_of(view_left, isSmaller) || ranges::all_of(view_right, isSmaller) ||
         ranges::all_of(view_top, isSmaller) || ranges::all_of(view_bottom, isSmaller);
}

u64 countVisibleTrees(const Wood& wood)
{
  using namespace ranges::views;
  return ranges::accumulate(iota(0u, wood.size()) | transform([&wood](const auto& row_index) {
                              return ranges::count_if(iota(0u, wood[row_index].size()),
                                                      [&wood, row_index](auto col_index) {
                                                        return isTreeVisible(wood, col_index, row_index);
                                                      });
                            }),
                            0u);
}

std::array<u64, 4> visibleTreesInAllDirections(const Wood wood, std::size_t x, std::size_t y)
{
  using namespace ranges;
  auto view_left = wood[y] | views::take(x) | views::reverse;
  auto view_right = wood[y] | views::drop(x + 1);

  auto view_topdown = wood | views::transform([x](const auto& view) { return view[x]; });
  auto view_top = view_topdown | views::take(y) | views::reverse;
  auto view_bottom = view_topdown | views::drop(y + 1);

  auto count_visible_trees = [height = wood[y][x]](auto& view) {
    return static_cast<u64>(ranges::count_if(view, [height, blocked = false](u32 h) mutable {
      if (!blocked && h >= height) {
        blocked = true;
        return true;
      }
      return !blocked;
    }));
  };

  return std::array<u64, 4>{count_visible_trees(view_top), count_visible_trees(view_left),
                            count_visible_trees(view_bottom), count_visible_trees(view_right)};
}
u64 scenicScore(std::array<u64, 4> values)
{
  return ranges::accumulate(values, 1u, std::multiplies<u64>());
}

u64 findHighestScenicScore(const Wood& wood)
{
  using namespace ranges::views;
  return ranges::max(iota(1u, wood.size() - 1) | transform([&wood](const auto& row_index) {
                       return ranges::max(iota(1u, wood[row_index].size() - 1) |
                                          transform([&wood, row_index](auto col_index) {
                                            return scenicScore(visibleTreesInAllDirections(wood, col_index, row_index));
                                          }));
                     }));
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto wood = parseWood(std::fstream("../../src/day8/input.txt"));
  fmt::print("Task1 Result: {}\n", countVisibleTrees(wood));
  fmt::print("Task2 Result: {}\n", findHighestScenicScore(wood));
}

#else

TEST_CASE("Task1 wood")
{
  std::string input = 1 + R"(
30373
25512
65332
33549
35390)";

  Wood wood = parseWood(std::stringstream(input));

  SECTION("Parsing")
  {
    REQUIRE(wood.size() == 5);
    for (const auto& line : wood)
      REQUIRE(line.size() == 5);

    REQUIRE(wood[0] == std::vector<u32>{3, 0, 3, 7, 3});
    REQUIRE(wood[1] == std::vector<u32>{2, 5, 5, 1, 2});
    REQUIRE(wood[2] == std::vector<u32>{6, 5, 3, 3, 2});
    REQUIRE(wood[3] == std::vector<u32>{3, 3, 5, 4, 9});
    REQUIRE(wood[4] == std::vector<u32>{3, 5, 3, 9, 0});
  }

  SECTION("Visible")
  {
    SECTION("Border")
    {
      for (std::size_t i = 0; i < 5; i++) {
        REQUIRE(isTreeVisible(wood, i, 0));
        REQUIRE(isTreeVisible(wood, i, 4));
        REQUIRE(isTreeVisible(wood, 0, i));
        REQUIRE(isTreeVisible(wood, 4, i));
      }
    }
    SECTION("Inside")
    {
      REQUIRE(isTreeVisible(wood, 1, 1));
      REQUIRE(isTreeVisible(wood, 2, 1));
      REQUIRE(!isTreeVisible(wood, 3, 1));
      REQUIRE(isTreeVisible(wood, 1, 2));
      REQUIRE(!isTreeVisible(wood, 2, 2));
      REQUIRE(isTreeVisible(wood, 3, 2));
      REQUIRE(isTreeVisible(wood, 2, 3));
      REQUIRE(!isTreeVisible(wood, 1, 3));
      REQUIRE(!isTreeVisible(wood, 3, 3));
    }
  }

  SECTION("Count visible")
  {
    REQUIRE(countVisibleTrees(wood) == 21);
  }

  SECTION("Task2")
  {
    REQUIRE(visibleTreesInAllDirections(wood, 2, 1) == std::array<u64, 4>{1, 1, 2, 2});
    REQUIRE(visibleTreesInAllDirections(wood, 2, 3) == std::array<u64, 4>{2, 2, 1, 2});

    REQUIRE(scenicScore(visibleTreesInAllDirections(wood, 2, 1)) == 4);
    REQUIRE(scenicScore(visibleTreesInAllDirections(wood, 2, 3)) == 8);

    REQUIRE(findHighestScenicScore(wood) == 8);
  }
}

#endif
