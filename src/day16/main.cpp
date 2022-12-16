#include <common.hpp>

// #define RUN_TESTS

struct Room
{
  int label;
  int flow_rate;
  std::vector<int> connectedTo;

  bool operator==(const Room&) const = default;
};

auto convertLabel = [](std::string_view v) {
  if (v.size() != 2)
    throw std::runtime_error("Invalid label");
  return static_cast<int>(v[1] - 'A' + 1) + static_cast<int>((v[0] - 'A' + 1) * 100);
};

using System = std::unordered_map<int, Room>;

Room parseRoom(std::string_view v)
{
  // Valve AA has flow rate=0; tunnels lead to valves DD, II, BB
  Room r;
  std::string label;

  auto rest = scn::scan(v, "Valve {} has flow rate={};", label, r.flow_rate);
  if (!rest)
    throw std::runtime_error(fmt::format("Parse error, rest: {}", rest));

  r.label = convertLabel(label);

  std::string dummy;
  auto list = scn::scan(rest, " {} {} to {}", dummy, dummy, dummy);
  if (!list)
    throw std::runtime_error(fmt::format("Parse error, rest: {}", list));

  std::vector<std::string> labels;
  if (!scn::scan_list_ex(list.range() | rv::replace(',', ' '), labels, scn::list_separator(',')))
    throw std::runtime_error("Failed to parse list");

  r.connectedTo = labels | rv::transform(convertLabel) | rn::to<std::vector<int>>;

  return r;
}

std::vector<Room> parseRooms(std::istream&& input)
{
  return rn::getlines(input) | rv::transform(parseRoom) | rn::to<std::vector<Room>>;
}

System buildSystem(const std::vector<Room>& rooms)
{
  System system;
  for (auto& room : rooms) {
    system[room.label] = room;
  }

  return system;
}

using Distances = std::vector<std::vector<int>>;

Distances calculateDistances(const std::vector<Room>& rooms)
{
  Distances distances;

  System system = buildSystem(rooms);

  distances.reserve(rooms.size());

  struct QueueEle
  {
    int label;
    int depth;
  };

  for (auto& room : rooms) {
    auto& d = distances.emplace_back(rooms.size(), std::numeric_limits<int>::max());
    std::queue<QueueEle> searchQueue;
    searchQueue.push({room.label, 0});

    std::unordered_set<int> visitedLabels;

    while (!searchQueue.empty()) {
      int label = searchQueue.front().label;
      int depth = searchQueue.front().depth;
      auto& room = system[label];
      searchQueue.pop();
      if (!visitedLabels.insert(label).second)
        continue;
      auto index = rn::distance(rn::begin(rooms), rn::find_if(rooms, [=](const Room& r) { return r.label == label; }));
      d[index] = depth;
      for (int neigh : room.connectedTo) {
        searchQueue.push({neigh, depth + 1});
      }
    }
  }

  return distances;
}

int64_t getMostPressureRelief(const std::vector<Room>& rooms, const Distances& distances, int minutes = 30,
                              std::bitset<64> valves_to_be_ignored = {})
{
  int64_t maxPressureRelief{};
  // std::unordered_set<int> open_valves;
  if (rooms.size() > 64)
    throw std::runtime_error("64 rooms hard limit");
  std::bitset<64> open_valves = valves_to_be_ignored;
  // Just open all flow_rate == 0 valves to reduce search space
  for (const auto& [i, r] : rooms | rv::enumerate) {
    if (r.flow_rate == 0)
      open_valves.set(i);
  }
  struct Ele
  {
    int roomLabel{};
    int walkTime{};
    int visitedNeigCount{};
  };

  auto roomIndex = [&rooms](int label) {
    return rn::distance(rn::begin(rooms), rn::find_if(rooms, [=](const Room& r) { return r.label == label; }));
  };

  // DFS
  std::stack<Ele> history;
  history.push({convertLabel("AA"), 0, 0});

  int currentMin{};
  int currentFlow{};
  int64_t pressureReliefed{};

  auto advanceMin = [&](int offset) {
    currentMin += offset;
    pressureReliefed += currentFlow * offset;
    if (pressureReliefed < 0)
      throw std::runtime_error("Elephants died, lack of oxygen!");
    if (currentMin > minutes)
      throw std::runtime_error("Overtime, elephants died due to vulcano!");
  };

  while (!history.empty()) {
    int room_label = history.top().roomLabel;
    int room_index = roomIndex(room_label);
    const auto& room = rooms[room_index];

    // Try to open valve
    if (!open_valves.test(room_index)) {
      open_valves.set(room_index);
      advanceMin(1);
      currentFlow += room.flow_rate;
    }
    // Push next destination
    bool advanced = false;
    while (history.top().visitedNeigCount < rooms.size()) {
      int destIndex = history.top().visitedNeigCount;
      history.top().visitedNeigCount++;
      auto dest = rooms[destIndex];
      if (!open_valves.test(destIndex)) {
        auto d = distances[room_index][destIndex];
        if (currentMin + d < minutes - 1) {
          // Check if we visited this one already
          advanced = true;
          history.push(Ele{.roomLabel = dest.label, .walkTime = d, .visitedNeigCount = 0});
          // Go to room
          advanceMin(history.top().walkTime);
          // fmt::print("Push: Stack size: {}\n", history.size());
          break;
        }
      }
    }
    if (!advanced) {
      // Advance time to 30
      auto time_left = minutes - currentMin;
      advanceMin(time_left);
      maxPressureRelief = std::max(maxPressureRelief, pressureReliefed);
      // Roll back
      advanceMin(-time_left);
      currentFlow -= room.flow_rate;
      advanceMin(-history.top().walkTime);
      if (room.flow_rate > 0) {
        open_valves.reset(room_index);
        advanceMin(-1);
      }
      history.pop();
      // fmt::print("Pop: Stack size: {}\n", history.size());
      if (history.size() == 1) {
        // fmt::print("Back to beginning!");
      }
    }
  }

  return maxPressureRelief;
}

int64_t getMostPressureReliefWithHelp(const std::vector<Room>& rooms, const Distances& distances, int minutes = 26)
{
  int64_t max_pressure{};
  int numberOfPossibilities = 1 << 14;
  for (uint32_t i = 0; i < numberOfPossibilities; i++) {
    std::bitset<64> my_valves;
    std::bitset<64> ele_valves;

    for (uint32_t j = 0; j < 15; j++) {
      if (i & (1 << j)) {
        my_valves.set(j);
      } else {
        ele_valves.set(j);
      }
    }
    auto my_val = getMostPressureRelief(rooms, distances, minutes, my_valves);
    auto ele_val = getMostPressureRelief(rooms, distances, minutes, ele_valves);
    max_pressure = std::max(max_pressure, my_val + ele_val);
  }
  return max_pressure;
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto rooms = parseRooms(std::fstream("../../src/day16/input.txt"));
  auto distances = calculateDistances(rooms);
  fmt::print("Task1 Result: {}\n", getMostPressureRelief(rooms, distances));
  fmt::print("Task1 Result: {}\n", getMostPressureReliefWithHelp(rooms, distances));
}

#else

TEST_CASE("Day16")
{
  std::string input = 1 + R"(
Valve AA has flow rate=0; tunnels lead to valves DD, II, BB
Valve BB has flow rate=13; tunnels lead to valves CC, AA
Valve CC has flow rate=2; tunnels lead to valves DD, BB
Valve DD has flow rate=20; tunnels lead to valves CC, AA, EE
Valve EE has flow rate=3; tunnels lead to valves FF, DD
Valve FF has flow rate=0; tunnels lead to valves EE, GG
Valve GG has flow rate=0; tunnels lead to valves FF, HH
Valve HH has flow rate=22; tunnel leads to valve GG
Valve II has flow rate=0; tunnels lead to valves AA, JJ
Valve JJ has flow rate=21; tunnel leads to valve II)";

  auto rooms = parseRooms(std::stringstream(input));
  auto distances = calculateDistances(rooms);

  SECTION("Parsing")
  {
    REQUIRE(rooms.size() == 10);
    REQUIRE(rooms[0] ==
            Room{convertLabel("AA"), 0, std::vector{convertLabel("DD"), convertLabel("II"), convertLabel("BB")}});
    REQUIRE(rooms[9] == Room{convertLabel("JJ"), 21, std::vector{convertLabel("II")}});
  }

  SECTION("Distances")
  {
    for (std::size_t i = 0; i < rooms.size(); i++) {
      REQUIRE(distances[i][i] == 0);
    }
    REQUIRE(distances[0][1] == 1);
    REQUIRE(distances[1][0] == 1);
    REQUIRE(distances[0][2] == 2);
  }

  SECTION("Task1")
  {
    REQUIRE(getMostPressureRelief(rooms, distances) == 1651);
  }
  SECTION("Task2")
  {
    REQUIRE(getMostPressureReliefWithHelp(rooms, distances) == 1707);
  }
}

#endif
