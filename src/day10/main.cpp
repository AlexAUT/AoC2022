#include <common.hpp>

// #define RUN_TESTS

struct Instruction
{
  enum class Op
  {
    Addx,
    Noop
  } op;

  i32 arg{};

  bool operator==(const Instruction&) const = default;
};

template <>
Instruction::Op fromString(std::string_view v)
{
  if (v.compare("addx") == 0)
    return Instruction::Op::Addx;
  else if (v.compare("noop") == 0)
    return Instruction::Op::Noop;

  throw std::runtime_error("Invalid Op");
}

std::ostream& operator<<(std::ostream& os, const Instruction& value)
{
  os << static_cast<int>(value.op) << ", " << value.arg;
  return os;
}

template <>
Instruction fromString(std::string_view v)
{
  int arg{};
  if (v.size() > 4) {
    std::from_chars(v.substr(5).begin(), v.substr(5).end(), arg);
  }
  return {.op = fromString<Instruction::Op>(v.substr(0, 4)), .arg = arg};
}

std::vector<Instruction> parseInstructions(std::istream&& input)
{
  return ranges::getlines(input) | ranges::views::transform(fromString<Instruction>) |
         ranges::to<std::vector<Instruction>>();
}

struct CPU
{
  i32 x{1};
  u32 cycle{0};

  bool operator==(const CPU&) const = default;

  void execute(Instruction instruction)
  {
    switch (instruction.op) {
    case Instruction::Op::Noop:
      cycle++;
      break;
    case Instruction::Op::Addx:
      x += instruction.arg;
      cycle += 2;
      break;
    }
  }
};

struct RegisterProber
{
  u32 firstProbe{20};
  u32 probeFrequency{40};

  std::vector<i32> probes;
  i32 lastRegisterValue{1};

  void update(CPU cpu)
  {
    auto addProbe = [this, cpu](u32 targetCycle) {
      // if (cpu.cycle == targetCycle)
      //   probes.push_back(cpu.x);
      // else
      probes.push_back(lastRegisterValue);
    };

    u32 nextProbeCycle{firstProbe};
    if (!probes.empty()) {
      nextProbeCycle = firstProbe + probes.size() * probeFrequency;
    }

    if (cpu.cycle >= nextProbeCycle)
      addProbe(nextProbeCycle);

    lastRegisterValue = cpu.x;
  }
};

struct Crt
{
  std::array<bool, 240> display{};
  u32 cycle{};

  i32 lastRegisterValue{1};

  void update(CPU cpu)
  {
    while (cycle < cpu.cycle && cycle < display.size()) {
      if (std::abs(static_cast<i32>(cycle % 40) - lastRegisterValue) <= 1) {
        display[cycle] = true;
      }
      cycle++;
    }
    lastRegisterValue = cpu.x;
  }

  std::string createImage() const
  {
    std::string result;
    result.reserve(display.size() + 6);
    for (std::size_t row = 0; row < 6; row++) {
      for (std::size_t col = 0; col < 40; col++) {
        result += display[row * 40 + col] ? "#" : ".";
      }
      result += '\n';
    }
    return result;
  }
};

u64 getSignalStrength(const RegisterProber& prober)
{
  using namespace ranges::views;
  u64 first =
      ranges::accumulate(prober.probes | take(1) | transform([&prober](u32 v) { return v * prober.firstProbe; }), 0ull);
  u64 rest = ranges::accumulate(prober.probes | enumerate | drop(1) | transform([&prober](const auto& indexView) {
                                  return std::get<1>(indexView) *
                                         (prober.firstProbe + prober.probeFrequency * std::get<0>(indexView));
                                }),
                                0ull);
  return first + rest;
}

void simulate(const std::vector<Instruction>& instructions, CPU& cpu, RegisterProber& prober, Crt& crt)
{
  prober.update(cpu);

  for (const auto& i : instructions) {
    cpu.execute(i);
    prober.update(cpu);
    crt.update(cpu);
  }
}

std::ostream& operator<<(std::ostream& os, const CPU& value)
{
  os << value.x << ", " << value.cycle;
  return os;
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto instructions = parseInstructions(std::fstream("../../src/day10/input.txt"));
  {
    CPU cpu{};
    RegisterProber prober{};
    Crt crt{};
    simulate(instructions, cpu, prober, crt);
    fmt::print("Task1 Result: {}\n", getSignalStrength(prober));
    fmt::print("Task2: \n{}", crt.createImage());
    // PLSFKAZS
  }
}

#else

TEST_CASE("Parsing")
{
  using Op = Instruction::Op;
  REQUIRE(fromString<Instruction::Op>("noop") == Op::Noop);
  REQUIRE(fromString<Instruction::Op>("addx") == Op::Addx);

  for (int i = -1000; i < 1001; i++) {
    REQUIRE(fromString<Instruction>(fmt::format("addx {}", i)) == Instruction{Op::Addx, i});
  }

  std::string input = 1 + R"(
noop
addx 3
addx -5
)";

  auto instructions = parseInstructions(std::stringstream(input));
  REQUIRE(instructions == std::vector<Instruction>{{Op::Noop}, {Op::Addx, 3}, {Op::Addx, -5}});
}

TEST_CASE("CPU Instructions")
{
  CPU cpu;

  REQUIRE(cpu == CPU{1, 0});

  using Op = Instruction::Op;
  cpu.execute({Op::Noop});
  REQUIRE(cpu == CPU{1, 1});
  cpu.execute({Op::Noop});
  REQUIRE(cpu == CPU{1, 2});
  cpu.execute({Op::Addx, 1});
  REQUIRE(cpu == CPU{2, 4});
  cpu.execute({Op::Addx, -5});
  REQUIRE(cpu == CPU{-3, 6});
}

TEST_CASE("Simulate")
{
  auto instructions = parseInstructions(std::fstream("../../src/day10/testInput.txt"));
  REQUIRE(instructions.size() == 146);

  CPU cpu{};
  RegisterProber prober{};
  Crt crt;
  simulate(instructions, cpu, prober, crt);

  SECTION("Prober")
  {
    REQUIRE(prober.probes.size() == 6);
    REQUIRE(prober.probes == std::vector{21, 19, 18, 21, 16, 18});

    REQUIRE(getSignalStrength(prober) == 13140);
  }

  SECTION("Crt")
  {
    std::string ref = 1 + R"(
##..##..##..##..##..##..##..##..##..##..
###...###...###...###...###...###...###.
####....####....####....####....####....
#####.....#####.....#####.....#####.....
######......######......######......####
#######.......#######.......#######.....
)";
    std::string img = crt.createImage();
    REQUIRE(img == ref);
  }
}
#endif
