
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)
#include INCLUDE_FILE(HEADER_PART_2)


namespace{

const std::vector<std::string_view> Examples =
{
"AAAA\n"
"BBCD\n"
"BBCC\n"
"EEEC\n"
""sv,

"OOOOO\n"
"OXOXO\n"
"OOOOO\n"
"OXOXO\n"
"OOOOO\n"
""sv,

"EEEEE\n"
"EXXXX\n"
"EEEEE\n"
"EXXXX\n"
"EEEEE\n"
""sv,

"AAAAAA\n"
"AAABBA\n"
"AAABBA\n"
"ABBAAA\n"
"ABBAAA\n"
"AAAAAA\n"
""sv,

"RRRRIICCFF\n"
"RRRRIICCCF\n"
"VVRRRCCFFF\n"
"VVRCCCJFFF\n"
"VVVVCJJCFE\n"
"VVIVCCJJEE\n"
"VVIIICJJEE\n"
"MIIIIIJJEE\n"
"MIIISIJEEE\n"
"MMMISSJEEE\n"
""sv,

}
;

std::size_t ExampleStartIndex = 0U;

// INPUT_PATH supposed to be a defined , including quotes
constexpr std::string_view InputFilePath = INPUT_PATH "/" QUOTE(DAY) "_input_file.txt";

// constexpr std::string_view Input = IsShortTest ? ShortInputTest : InputFilePath;
}

int main()
{
   try {
      if (ExampleStartIndex > Examples.size()){
         ExampleStartIndex = Examples.size();
      }

      for(std::size_t idxEx = ExampleStartIndex; idxEx <= Examples.size(); ++idxEx){
         NAoc__MR::IsShortTest = (idxEx < Examples.size());

         const auto getInputStream =
            [idxEx = idxEx, IsShortTest = NAoc__MR::IsShortTest]() -> std::shared_ptr<std::istream>{
               if (IsShortTest) {
                  auto sstream = std::make_shared<std::stringstream>();
                  (*sstream) << Examples[idxEx];
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


         if (NAoc__MR::IsShortTest){
            std::cout << "\n--- E X A M P L E    n . " << (idxEx+1U) << " ---\n";
         } else{
            std::cout << "\n--- R E A L   I N P U T ---\n";

         }

         static_cast<void>(day12Part1(getInputStream()));
         static_cast<void>(day12Part2(getInputStream()));
      }
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
