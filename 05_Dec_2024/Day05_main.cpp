
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)
#include INCLUDE_FILE(HEADER_PART_2)


namespace{

constexpr std::string_view ShortInputTest =

"47|53\n"
"97|13\n"
"97|61\n"
"97|47\n"
"75|29\n"
"61|13\n"
"75|53\n"
"29|13\n"
"97|29\n"
"53|29\n"
"61|53\n"
"97|53\n"
"61|29\n"
"47|13\n"
"75|47\n"
"97|75\n"
"47|61\n"
"75|61\n"
"47|29\n"
"75|13\n"
"53|13\n"
"\n"
"75,47,61,53,29\n"
"97,61,53,29,13\n"
"75,29,13\n"
"75,97,47,61,53\n"
"61,13,29\n"
"97,13,75,29,47\n"
;

constexpr bool IsShortTest =  false;

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

       static_cast<void>(day05Part1(getInputStream()));
       static_cast<void>(day05Part2(getInputStream()));
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
