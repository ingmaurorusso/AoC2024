
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)
#include INCLUDE_FILE(HEADER_PART_2)


namespace{

const std::vector<std::string_view> Examples =
{
"x00: 1\n"
"x01: 0\n"
"x02: 1\n"
"x03: 1\n"
"x04: 0\n"
"y00: 1\n"
"y01: 1\n"
"y02: 1\n"
"y03: 1\n"
"y04: 1\n"

"ntg XOR fgs -> mjb\n"
"y02 OR x01 -> tnw\n"
"kwq OR kpj -> z05\n"
"x00 OR x03 -> fst\n"
"tgd XOR rvg -> z01\n"
"vdt OR tnw -> bfw\n"
"bfw AND frj -> z10\n"
"ffh OR nrd -> bqk\n"
"y00 AND y03 -> djm\n"
"y03 OR y00 -> psh\n"
"bqk OR frj -> z08\n"
"tnw OR fst -> frj\n"
"gnj AND tgd -> z11\n"
"bfw XOR mjb -> z00\n"
"x03 OR x00 -> vdt\n"
"gnj AND wpb -> z02\n"
"x04 AND y00 -> kjc\n"
"djm OR pbm -> qhw\n"
"nrd AND vdt -> hwm\n"
"kjc AND fst -> rvg\n"
"y04 OR y02 -> fgs\n"
"y01 AND x02 -> pbm\n"
"ntg OR kjc -> kwq\n"
"psh XOR fgs -> tgd\n"
"qhw XOR tgd -> z09\n"
"pbm OR djm -> kpj\n"
"x03 XOR y03 -> ffh\n"
"x00 XOR y04 -> ntg\n"
"bfw OR bqk -> z06\n"
"nrd XOR fgs -> wpb\n"
"frj XOR qhw -> z04\n"
"bqk OR frj -> z07\n"
"y03 OR x01 -> nrd\n"
"hwm AND bqk -> z03\n"
"tgd XOR rvg -> z12\n"
"tnw OR pbm -> gnj\n"
""sv,

// my own example

"x00: 0\n"
"x01: 0\n"
"x02: 0\n"
"y00: 0\n"
"y01: 0\n"
"y02: 0\n"
"\n"
"x00 AND y00 -> c0\n"
"x00 XOR y00 -> z00\n"


"x01 XOR y01 -> r10\n"
// "r10 XOR c0 -> z01\n"
"r10 XOR c0 -> z02\n"

"x01 AND y01 -> a10\n"
"c0 AND r10 -> a11\n"
"a10 OR a11 -> c1\n"


"x02 XOR y02 -> r20\n"
// "r20 XOR c1 -> z02\n"
"r20 XOR c1 -> z01\n"

"x02 AND y02 -> a20\n"
"c1 AND r20 -> a21\n"
"a20 OR a21 -> z03\n"
""sv
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

         static_cast<void>(day24Part1(getInputStream()));
         static_cast<void>(day24Part2(getInputStream()));
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
