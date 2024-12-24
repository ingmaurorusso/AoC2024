
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day17Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   std::vector<std::string> lines;

   using Value = unsigned long;
   constexpr std::size_t REG_A = 0U;
   constexpr std::size_t REG_B = 1U;
   constexpr std::size_t REG_C = 2U;
   std::array<Value,3U> regs;

   using OpCode = unsigned char;
   std::vector<OpCode> memory;
   constexpr OpCode ADV = 0;
   constexpr OpCode BXL = 1;
   constexpr OpCode BST = 2;
   constexpr OpCode JNZ = 3;
   constexpr OpCode BXC = 4;
   constexpr OpCode OUT = 5;
   constexpr OpCode BDV = 6;
   constexpr OpCode CDV = 7;

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines) break;

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         std::cout << msgLine << "WARNING: empty line\n";
         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---

      //  --- END LINE CHECKS ---

      std::smatch smatch;
      if (std::regex_search(line.cbegin(), line.cend(), smatch, std::regex{R"__((.*?)\: ([\d,]*))__"})){
         if (smatch.size() != 3){
            throw std::invalid_argument(msgLine + "wrong regex extraction");
         }

         auto title = std::string_view(smatch[1].first, smatch[1].second);

         auto sValues = std::string_view(smatch[2].first, smatch[2].second);

         auto values = getAllValues<Value>(sValues);
         if (values.empty()){
            throw std::invalid_argument(msgLine + "no values");
         }

         if (title == "Register A"){
            regs[REG_A] = values[0];
         } else
         if (title == "Register B"){
            regs[REG_B] = values[0];
         } else
         if (title == "Register C"){
            regs[REG_C] = values[0];
         } else
         if (title == "Program"){
            std::transform( values.begin(), values.end(),
                            std::back_inserter(memory),
                            [](Value v){return OpCode(v & 7);} );
         } else{
            throw std::invalid_argument(msgLine + "unexpected title");
         }
      }

      lines.push_back(std::move(line));
   }

   std::vector<Value> output;

   std::size_t pc = 0U;
   std::size_t nCycles = 0U;
   while (pc < memory.size()){
      ++nCycles;

      if (pc+1U == memory.size()){
         throw std::invalid_argument("program with no-operand at the cycle n. "s + std::to_string(nCycles));
      }

      auto litOperand = memory[pc+1];
      auto comboOperand = (litOperand < 4)
                          ? litOperand
                          : ( (litOperand == 7)? 0 : regs[litOperand-4]);
      bool jumped = false;
      bool isComboOp = false;
      switch (memory[pc]){
      case ADV:
         regs[REG_A] >>= comboOperand;
         isComboOp = true;
         break;
      case BXL:
         regs[REG_B] ^= litOperand;
         break;
      case BST:
         regs[REG_B] = (comboOperand & 7);
         isComboOp = true;
         break;
      case JNZ:
         if (regs[REG_A] != 0){
            pc = litOperand;
            jumped = true;
         }
         break;
      case BXC:
         regs[REG_B] ^= regs[REG_C];
         break;
      case OUT:
         output.push_back(comboOperand & 7);
         isComboOp = true;
         break;
      case BDV:
         regs[REG_B] = (regs[REG_A] >> comboOperand);
         isComboOp = true;
         break;
      //case CDV:
      default:
         regs[REG_C] = (regs[REG_A] >> comboOperand);
         isComboOp = true;
         break;
      }

      if (isComboOp && (litOperand == 7)){
         throw std::invalid_argument("combo operand = 7 at the cycle n. "s + std::to_string(nCycles));
      }

      if (!jumped) pc += 2U;
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << join(output,",",[](auto v){return std::to_string(v);}) << std::endl;
   std::cout << std::endl;

   //std::stringstream ss;
   //printIterableOnStream(ss, output, ",");

   return output.size();
}