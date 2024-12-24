
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)
#include INCLUDE_FILE(HEADER_PART_2)


namespace{

constexpr std::string_view ShortInputTest =

/*
"########\n"
"#..O.O.#\n"
"##@.O..#\n"
"#...O..#\n"
"#.#.O..#\n"
"#...O..#\n"
"#......#\n"
"########\n"
"\n"
"<^^>>>vv<v>>v<<\n"
*/

/*
"#######\n"
"#...#.#\n"
"#.....#\n"
"#..OO@#\n"
"#..O..#\n"
"#.....#\n"
"#######\n"
"\n"
"<vv<<^^<<^^\n"
*/


"##########\n"
"#..O..O.O#\n"
"#......O.#\n"
"#.OO..O.O#\n"
"#..O@..O.#\n"
"#O#..O...#\n"
"#O..O..O.#\n"
"#.OO.O.OO#\n"
"#....O...#\n"
"##########\n"
"\n"
"<vv>^<v^>v>^vv^v>v<>v^v<v<^vv<<<^><<><>>v<vvv<>^v^>^<<<><<v<<<v^vv^v>^\n"
"vvv<<^>^v^^><<>>><>^<<><^vv^^<>vvv<>><^^v>^>vv<>v<<<<v<^v>^<^^>>>^<v<v\n"
"><>vv>v^v^<>><>>>><^^>vv>v<^^^>>v^v^<^^>v^^>v^<^v>v<>>v^v^<v>v^^<^^vv<\n"
"<<v<^>>^^^^>>>v^<>vvv^><v<<<>^^^vv^<vvv>^>v<^^^^v<>^>vvvv><>>v^<<^^^^^\n"
"^><^><>>><>^^<<^^v>>><^<v>^<vv>>v>>>^v><>^v><<<<v>>v<v<v>vvv>^<><<>^><\n"
"^>><>^v<><^vvv<^^<><v<<<<<><^v<<<><<<^^<v<^^^><^>>^<v^><<<^>>^v<v^v<v^\n"
">^>>^v>vv>^<<^v<>><<><<v<<v><>v<^vv<<<>^^v^>^^>>><<^v>>v^v><^^>>^<>vv^\n"
"<><^^>^^^<><vvvvv^v<v<<>^v<v>v<<^><<><<><<<^^<<<^<<>><<><^^^>^^<>^>v<>\n"
"^^>vv<^v^v<vv>^<><v<^v>^^^>>>^^vvv^>vvv<>>>^<^>>>>>^<<^v>^vvv<>^<><<v>\n"
"v^^>>><<^^<>>^v^<v^vv<>v^<<>^<^v^v><^<<<><<^<v><v<>vv>>v><v^<vv<>v^<<^\n"

;

constexpr bool IsShortTest = false;

// INPUT_PATH supposed to be a defined , including quotes
constexpr std::string_view InputFilePath = INPUT_PATH "/" QUOTE(DAY) "_input_file.txt";

// constexpr std::string_view Input = IsShortTest ? ShortInputTest : InputFilePath;
}

int main()
{
   NAoc__MR::IsShortTest = IsShortTest;

   try {
      const auto getInputStream =
         []() -> std::shared_ptr<std::istream>{
             if constexpr (IsShortTest) {
                auto sstream = std::make_shared<std::stringstream>();
                (*sstream) << ShortInputTest;
                // std::move(sstream) for C++20 or more.
                return sstream; //std::static_pointer_cast<std::istream>(sstream);
             } else{
                if (std_fs::is_directory(InputFilePath) || ! std_fs::exists(InputFilePath)){
                   throw std::invalid_argument(std::string(InputFilePath) + " is not a file!");
                } //else:
                return std::static_pointer_cast<std::istream>(
                       std::make_shared<std::ifstream>(std::string(InputFilePath)));
             }
       };

       static_cast<void>(day15Part1(getInputStream()));
       static_cast<void>(day15Part2(getInputStream()));
   } catch (std::invalid_argument& ex) {
      std::cerr << "Bad input: " << ex.what() << std::endl;
      return 1;
   } catch (std::exception& ex) {
      std::cerr << "Error: " << ex.what() << std::endl;
      return 1;
   } catch (std::string& ex) {
      std::cerr << "String error: " << ex << std::endl;
      return 1;
   } catch (...) {
      std::cerr << "Unknown error: " << std::endl;
      return 1;
   }

   return 0;
}
