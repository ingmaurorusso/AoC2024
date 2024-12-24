
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)
#include INCLUDE_FILE(HEADER_PART_2)


namespace{

constexpr std::string_view ShortInputTest =

"....#.....\n"
".........#\n"
"..........\n"
"..#.......\n"
".......#..\n"
"..........\n"
".#..^.....\n"
"........#.\n"
"#.........\n"
"......#...\n"

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

       static_cast<void>(day06Part1(getInputStream()));
       static_cast<void>(day06Part2(getInputStream()));
   } catch (std::invalid_argument& ex) {
      std::cerr << "Bad input: " << ex.what() << std::endl;
      return 1;
   } catch (std::exception& ex) {
      std::cerr << "Error: " << ex.what() << std::endl;
      return 1;
   } catch (...) {
      std::cerr << "Unknown error: " << std::endl;
      return 1;
   }

   return 0;
}
