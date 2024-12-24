
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;


NAoc__MR::TResult day17Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

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

   // Operate instructions in the opposite way, until
   // reachingthe state of pc==0 and output of correct size.
   // However, since output prints modulo 8, this determines
   // a set of possible values for the registers.

   // The approach is customized for the particular input got
   // In acse of the example:
   // 0,3, A >>= 3
   // 5,4, OUT A&7
   // 3,0  JNZ
   // which means that bits from A are printer 4 per times,
   // starting from the lowest (excluded the LSBs that
   // are rulted out on the first shift by 3 bits), therefore
   // in octal notation the value of A is:
   // 034530X and then X=0 in order to get the minimum value.
   // Indeed, this is 117440 in decimal notation.
   //On the real input:
   // 2,4, B = A & 7
   // 1,7, B ^= 7
   // 7,5, C = A >> B
   // 0,3, A >>= 3
   // 4,0, B ^= C
   // 1,7, B ^= 7
   // 5,5, OUT B&7
   // 3,0  JNZ (exit if A==0 or restart from begin)
   // Since for the real input the program has 16 printed values,
   // then A has from 46 to 48 (significant) bits as fourth
   // instruction shifts it down by 3 bits. Note that this time
   // the uppest 3-bits of A are read (first instruction) before
   // the shift. However, these uppest 3-bits might be also 0 (leading
   // to 43-45 significant bits for A) in case such configuration
   // may lead to print 0 as last value.... well, this is really possible
   // as we get B = 0 & 7 = 0 ; B ^= 7 = 7 ; C = A >> B = A >> 7 = 0 ;
   // B = B ^ C = 7 ^ 0 = 7 ; B = B ^ 7 = 7 ^ 7 = 0 -> OUT 0. Therefore,
   // we can establish that the 3 uppest digits are 0, as we look for the
   // minimum value of A leading to the result.... really, provided that
   // they can influence the next values to be correct.
   // Now, for each cycle (in opposite order), the three digits of B depend
   // on the 3 further lower digits of A but also from upper digits (due to
   // the instruction C = A >> B), previously established,
   // and so we get a constraint: for each of the 8 possible values of A&7
   // we get a needed value for 3 bits whose position is given by A&7, for
   // example if A&7=1, then the interesting bits of C are the ones in
   // positions from 1 to 3 from A.
   // Since we look for the minimum of A, we may look for the lowest triplet
   // that leads to the value to be printed, based on the previous established
   // triplets.

   const std::size_t N = memory.size();

   Sequences triplets(std::string_view{"01234567"}, N);

   auto tuple = triplets.beginLexicographic();
   auto& [seq, idxChanged, ok] = tuple;


   std::size_t sequenceNumber = 0;

   while(ok){
      ++sequenceNumber;

      std::size_t idxToChange{};
      for(std::size_t i = N; (i > 0U) && ok;){
         --i;

         if (idxChanged > N-1 - i) continue;
         
         Value treeBits = *(seq[N-1 - i]) - '0';

         // look for constraints on higher bits.
         auto treeBitsInv = 7 - treeBits; // B ^= 7

         Value baseBitPos = i*3;
         auto beginHigherPos = baseBitPos+treeBitsInv; // C = A >> B

         Value treeBitsHigher;
         {
            auto j = beginHigherPos/3;
            if (j < N){
               treeBitsHigher = *(seq[N-1 - j]) - '0';

               auto down = beginHigherPos % 3;

               Value treeBitsHigher2 = 0U;
               if ((j+1 < N) && (down > 0U)){
                  treeBitsHigher2 = *(seq[N-1 - (j+1)]) - '0';
               }

               treeBitsHigher >>= down;
               treeBitsHigher |= ((treeBitsHigher2 << (3-down)) & 7);
            } else{
               // bits too high, for sure zeros.
               treeBitsHigher = 0U;
            }
         }

         treeBits ^= treeBitsHigher; // B ^= C
         // don't mention second B ^= 7 because the first was applied
         // only to 'treeBitsInv' to support the correct evaluation
         // of "C = A >> B".

         if (treeBits != memory[i]){
            // This configuration is not good, must be changed.
            idxToChange = N-1 - i;
            ok = false;
         }
      }

      if (!ok){
         tuple = triplets.nextLexicographic(idxToChange,true);
         // this updates seq, idxChanged, and ok.
      } else break; // ok for found good sequence ! (not 'next' sequence).
   }

   Value A = 0U;
   if (ok){
      for(std::size_t i = N; i > 0U;){
         --i;

         Value treeBits = *(seq[N-1 - i]) - '0';

         (A <<= 3) |= treeBits;
      }
   }


   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << (ok? std::to_string(A) : "not found!") << std::endl;
   std::cout << "Tried number of triplets sequences: " << sequenceNumber << std::endl;
   std::cout << std::endl;

   //std::stringstream ss;
   //printIterableOnStream(ss, output, ",");

   return A;
}