
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)
#include INCLUDE_FILE(HEADER_PART_2)


namespace{

constexpr std::string_view ShortInputTest =

"p=0,4 v=3,-3\n"
"p=6,3 v=-1,-3\n"
"p=10,3 v=-1,2\n"
"p=2,0 v=2,-1\n"
"p=0,0 v=1,3\n"
"p=3,0 v=-2,-2\n"
"p=7,6 v=-1,-3\n"
"p=3,0 v=-1,-2\n"
"p=9,3 v=2,3\n"
"p=7,3 v=-1,2\n"
"p=2,4 v=2,-3\n"
"p=9,5 v=-3,-3\n"

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

       static_cast<void>(day14Part1(getInputStream()));
       static_cast<void>(day14Part2(getInputStream()));
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
