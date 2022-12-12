#include <common.hpp>

#include <variant>

// #define RUN_TESTS

struct Command
{
  enum class Type
  {
    ChangeDirectory,
    ListFiles,
  } cmd;
  std::string arg;

  bool operator<=>(const Command&) const = default;
};

using HistoryEntry = std::variant<Command, std::string>;

template <>
Command::Type fromString(std::string_view v)
{
  if (v.compare("cd") == 0)
    return Command::Type::ChangeDirectory;
  if (v.compare("ls") == 0)
    return Command::Type::ListFiles;

  throw std::runtime_error("Invalid enum");
}

template <>
Command fromString(std::string_view v)
{
  auto cmd_start = v.find_first_of(' ');
  auto arg_start = v.find_first_of(' ', cmd_start + 1);

  return {.cmd = fromString<Command::Type>(
              v.substr(cmd_start + 1, arg_start != std::string::npos ? arg_start - cmd_start - 1 : std::string::npos)),
          .arg = arg_start != std::string::npos ? std::string(v.substr(arg_start + 1)) : ""};
}

std::vector<HistoryEntry> parseHistory(std::istream&& input)
{
  return ranges::getlines(input) | ranges::views::transform([](std::string_view v) {
           if (v[0] == '$')
             return HistoryEntry(fromString<Command>(v));
           return HistoryEntry(std::string(v));
         }) |
         ranges::to<std::vector<HistoryEntry>>();
}

struct File
{
  std::string name;
  u64 size;

  bool operator<=>(const File&) const = default;
};

struct Directory
{
  std::string name;
  std::vector<Directory> dirs;
  std::vector<File> files;

  std::size_t size{};

  bool operator==(const Directory&) const = default;
};

Directory parseFilesystemFromHistory(const std::vector<HistoryEntry>& history, bool storeDirSizes = true)
{
  if (std::get<Command>(history[0]) != Command{Command::Type::ChangeDirectory, "/"})
    throw std::runtime_error("First cmd has to be cd /!");
  Directory root{"/"};

  std::vector<Directory*> cwd;
  cwd.push_back(&root);

  auto create_dir = [&](const std::string& name, bool enter) {
    auto found = ranges::find_if(cwd.back()->dirs, [&](const Directory& d) { return d.name == name; });
    if (found != cwd.back()->dirs.end()) {
      if (enter)
        cwd.push_back(&*found);
    } else {
      cwd.back()->dirs.emplace_back(name);
      if (enter)
        cwd.push_back(&cwd.back()->dirs.back());
    }
  };

  for (bool first{true}; auto& entry : history) {
    if (first) {
      first = false;
      continue;
    }

    const Command* cmd = std::get_if<Command>(&entry);
    const std::string* output = std::get_if<std::string>(&entry);

    if (cmd) {
      if (cmd->cmd == Command::Type::ChangeDirectory) {
        if (cmd->arg == "..") {
          cwd.pop_back();
        } else {
          create_dir(cmd->arg, true);
        }
      }
    } else if (output) {
      if ((*output)[0] == 'd') {
        create_dir(std::string(output->substr(4)), false);
      } else {
        auto space_loc = output->find_first_of(' ');
        std::string name = std::string(output->substr(space_loc + 1, output->size() - space_loc - 1));
        int size = std::atoi(output->c_str());
        cwd.back()->files.emplace_back(name, size);

        if (storeDirSizes) {
          for (auto& dir : cwd) {
            dir->size += size;
          }
        }
      }
    }
  }

  return root;
}

u64 dirSizeSumWithThreshold(const Directory& dir, u64 threshold)
{
  u64 sum{};
  std::stack<const Directory*> stack;
  stack.push(&dir);

  while (!stack.empty()) {
    const Directory* dir = stack.top();
    stack.pop();

    if (dir->size < threshold)
      sum += dir->size;

    for (auto& d : dir->dirs)
      stack.push(&d);
  }

  return sum;
}

u64 freeSpace(const Directory& dir, u64 diskSpace, u64 targetFreeSpace)
{
  auto toFree = targetFreeSpace - (diskSpace - dir.size);

  const Directory* smallest = nullptr;
  std::stack<const Directory*> stack;
  stack.push(&dir);

  while (!stack.empty()) {
    const Directory* dir = stack.top();
    stack.pop();

    if (dir->size >= toFree) {
      if (!smallest || smallest->size > dir->size)
        smallest = dir;
    }

    for (auto& d : dir->dirs)
      stack.push(&d);
  }

  if (smallest == nullptr)
    throw std::runtime_error("No dir large enough?");

  return smallest->size;
}

#ifndef RUN_TESTS
#include <fstream>

auto main() -> int
{
  auto history = parseHistory(std::fstream("../../src/day7/input.txt"));
  auto dir = parseFilesystemFromHistory(history);
  fmt::print("Task1 Result: {}\n", dirSizeSumWithThreshold(dir, 100000));
  fmt::print("Task2 Result: {}\n", freeSpace(dir, 70000000, 30000000));
}

#else

TEST_CASE("Parse cd")
{
  using T = Command::Type;
  REQUIRE(fromString<Command>("$ cd /") == Command{T::ChangeDirectory, {"/"}});
  REQUIRE(fromString<Command>("$ cd abc") == Command{T::ChangeDirectory, {"abc"}});
  REQUIRE(fromString<Command>("$ cd dir") == Command{T::ChangeDirectory, {"dir"}});
  REQUIRE(fromString<Command>("$ cd ls") == Command{T::ChangeDirectory, {"ls"}});
}

TEST_CASE("Parse ls")
{
  using T = Command::Type;

  REQUIRE(fromString<Command>("$ ls") == Command{T::ListFiles});
}

TEST_CASE("Parse History")
{
  using T = Command::Type;

  std::string input = 1 + R"(
$ cd /
$ ls
dir a
14848514 b.txt
8504156 c.dat
dir d
$ cd a
$ ls
dir e
29116 f
2557 g
62596 h.lst
$ cd e
$ ls
584 i
$ cd ..
$ cd ..
$ cd d
$ ls
4060174 j
8033020 d.log
5626152 d.ext
7214296 k)";

  auto history = parseHistory(std::stringstream(input));

  REQUIRE(history.size() == 23);
  REQUIRE(std::get_if<Command>(&history[0]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[1]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[2]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[3]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[4]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[5]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[6]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[7]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[8]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[9]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[10]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[11]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[12]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[13]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[14]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[15]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[16]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[17]) != nullptr);
  REQUIRE(std::get_if<Command>(&history[18]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[19]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[20]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[21]) != nullptr);
  REQUIRE(std::get_if<std::string>(&history[22]) != nullptr);

  REQUIRE(std::get<Command>(history[0]) == Command{T::ChangeDirectory, "/"});
  REQUIRE(std::get<Command>(history[1]) == Command{T::ListFiles});
  REQUIRE(std::get<std::string>(history[21]) == std::string{"5626152 d.ext"});
  REQUIRE(std::get<std::string>(history[22]) == std::string{"7214296 k"});
}

TEST_CASE("cd /")
{
  using T = Command::Type;

  std::string input = 1 + R"(
$ cd /)";
  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history);
  REQUIRE(dir == Directory{"/"});
}

TEST_CASE("cd into folder")
{
  using T = Command::Type;

  std::string input = 1 + R"(
$ cd /
$ cd test
)";
  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history);
  REQUIRE(dir == Directory{"/", {Directory{"test"}}});
}

TEST_CASE("cd into folder and go back and into another folder")
{
  using T = Command::Type;

  std::string input = 1 + R"(
$ cd /
$ cd test
$ cd ..
$ cd test2
)";
  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history);
  REQUIRE(dir == Directory{"/", {Directory{"test"}, Directory{"test2"}}});
}

TEST_CASE("empty ls")
{
  using T = Command::Type;

  std::string input = 1 + R"(
$ cd /
$ ls
)";
  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history);
  REQUIRE(dir == Directory{"/"});
}

TEST_CASE("ls with dirs")
{
  using T = Command::Type;

  std::string input = 1 + R"(
$ cd /
$ ls
dir d
dir e
)";
  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history);
  REQUIRE(dir == Directory{"/", {Directory{"d"}, Directory{"e"}}});
}

TEST_CASE("ls with files")
{
  using T = Command::Type;

  std::string input = 1 + R"(
$ cd /
$ ls
2557 f.lst
62000 abc.txt)";
  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history, false);
  REQUIRE(dir == Directory{"/", {}, {File{"f.lst", 2557}, File{"abc.txt", 62000}}});
}

TEST_CASE("Dir size")
{
  std::string input = 1 + R"(
$ cd /
$ ls
2 f.lst
4 abc.txt
$ cd a
8 f.lst
10 abc.txt)";

  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history);
  REQUIRE(dir.size == 24);
  REQUIRE(dir.dirs[0].size == 18);
}

TEST_CASE("Sum of size smaller than threshold")
{
  std::string input = 1 + R"(
$ cd /
$ ls
2 f.lst
4 abc.txt
$ cd a
8 f.lst
10 abc.txt
$ cd ..
$ cd b
$ ls
22 asf.asf
$ cd ..
$ cd c
$ ls
10 qwe.asf)";

  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history);
  REQUIRE(dirSizeSumWithThreshold(dir, 20) == 18 + 10);
}

TEST_CASE("Example Task1 and 2")
{
  std::string input = 1 + R"(
$ cd /
$ ls
dir a
14848514 b.txt
8504156 c.dat
dir d
$ cd a
$ ls
dir e
29116 f
2557 g
62596 h.lst
$ cd e
$ ls
584 i
$ cd ..
$ cd ..
$ cd d
$ ls
4060174 j
8033020 d.log
5626152 d.ext
7214296 k)";

  auto history = parseHistory(std::stringstream(input));
  auto dir = parseFilesystemFromHistory(history);
  REQUIRE(dirSizeSumWithThreshold(dir, 100000) == 95437);

  REQUIRE(freeSpace(dir, 70000000, 30000000) == 24933642);
}

#endif
