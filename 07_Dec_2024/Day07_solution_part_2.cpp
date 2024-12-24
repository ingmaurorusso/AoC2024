
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day07Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   using TValue = TResult;

   TResult sum{0U};
   unsigned count{0U};

   static const auto digitNumber10 = [](TValue v){
      unsigned n = 1;
      while(v >= 10){
         v /= 10;
         ++n;
      }
      return n;
   };

   constexpr auto operations = "+*|";

   Sequences<std::string> opSequence(operations);

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
      if ( auto it = std::find_if( line.cbegin(), line.cend(),
                         [](const auto ch){return !(std::isdigit(ch) || std::isspace(ch) || (ch == ':'));});
           it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      if (line.find_first_of (":") != line.find_last_of (":")){
         throw std::invalid_argument(msgLine + "multiple 'columns char'");
      }
      //  --- END LINE CHECKS ---

      TValue testValue;

      if (!(lineStream >> testValue)){
         throw std::invalid_argument(msgLine + "no initial test value");
      }

      char ch = ' ';
      do{
         if (!(lineStream >> ch)) break;
      } while (std::isspace(ch));

      if (ch != ':'){
         throw std::invalid_argument(msgLine + "no 'column char' in right position");
      }

      TValue first_v;
      lineStream >> first_v;
      TValue res = first_v;

      std::vector<TValue> v_s;
      
      TValue v;
      while (lineStream >> v){
         v_s.push_back(v);
      }
      std::vector<TValue> res_s(v_s.size(), TValue{});

      bool okEq = false;
      if (v_s.empty()){
         okEq = (first_v == testValue);
      } else{
         opSequence.reset(v_s.size());

         auto [ops_idx, changeIdx, notEnded] = opSequence.beginLexicographic();

         const auto compute = [ first_v, &v_s = std::as_const(v_s), &res_s ](const auto& ops_idx, const auto changeIdx){
            auto newRes = (changeIdx == 0)? first_v : res_s[changeIdx-1];

            bool over = false;
            auto idx = changeIdx;
            for(; idx < ops_idx.size(); ++idx){
               auto add = v_s[idx];
               auto factor = add;
               switch(*ops_idx[idx]){
               case '+': factor = 1U; break;
               case '*': add = 0U; break;
               // case '|' :
                  // might use pre-conversions to string, then concat
                  // and finally convert-back to int.
               default:
                  factor = std::pow(10,digitNumber10(add));
               }

               over = (newRes > std::numeric_limits<TValue>::max()/factor);
               newRes *= factor;
               over = (newRes > std::numeric_limits<TValue>::max()-add);
               newRes += add;

               res_s[idx] = newRes;
            }

            return std::make_pair(idx,!over);
         };

         while(notEnded && !okEq){
            auto [idxOver, notOver] = compute(ops_idx, changeIdx);

            if (notOver){
               okEq = (res_s.back() == testValue);
            }

            auto [next_ops_idx, next_changeIdx, next_notEnded] = opSequence.nextLexicographic(idxOver, !notOver);

            ops_idx = std::move(next_ops_idx);
            changeIdx = next_changeIdx;
            notEnded = next_notEnded;
         }
      }

      if (okEq){
         ++count;

         if (sum > std::numeric_limits<TValue>::max() - testValue){
            throw std::runtime_error("OVerflow, TResult size to be increased");
         }
         sum += testValue;

         /*
         // check
         TValue test2 = first_v;
         for(int i = 0; i < v_s.size(); ++i){
            if (op_s[i] == '+') test2 += v_s[i];
            else test2 *= v_s[i];
         }
         if (test2 != testValue)
            bug = true;
         */
      }

      /*
      // check method 2;
      bool okEq2 = false;
      const auto n = v_s.size();
      unsigned nComb = (1U << n);
      for(unsigned c = 0; (c < nComb) && !okEq2; ++c){
         auto j = 1U;
         TValue test2 = first_v;
         bool over = false;
         for(int i = 0; (i < v_s.size()) && !over; ++i){
            if (!(c & j)) test2 += v_s[i];
            else{
               over = (test2 > std::numeric_limits<TValue>::max()/v_s[i]);

               test2 *= v_s[i];
            }
            j <<= 1;

            over |= (test2 > testValue);
         }
         okEq2 = (test2 == testValue) && !over;
      }
      if (okEq2) sum2 += testValue;
      if (okEq != okEq2)
         bug = true;
      */

      // lines.emplace_back(std::move(line));
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "N. good values P2: " << count << std::endl;
   std::cout << "\nResult P2: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}