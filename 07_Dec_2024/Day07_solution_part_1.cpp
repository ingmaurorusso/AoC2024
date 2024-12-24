
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day07Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   using TValue = TResult;

   std::vector<std::string> lines;
   TResult sum{0U};
   unsigned count = 0U;
   //Tvalue sum2{0U};

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
      std::vector<TValue> res_s;
      std::vector<char> op_s;
      TValue v;
      while (lineStream >> v){
         v_s.push_back(v);
         op_s.push_back('+');

         res += v;
         res_s.push_back(res);
      }

      //2^(n-1) combinations
      bool okEq = (res == testValue);
      std::size_t i = v_s.size()-1U;

      while (res != testValue){
         bool ended = false;
         bool first = true;

         while(op_s[i] == '*'){
            if (i == 0){
               ended = true;
               break;
            }
            --i;
         }

         if (ended) break;

         op_s[i] = '*';
         res = ((i == 0)? first_v : res_s[i-1]) ;

         bool over = (res > std::numeric_limits<TValue>::max()/v_s[i]);
         res *= v_s[i];
         res_s[i++] = res; // may already over
         
         while((i < v_s.size()) && !over){
            over = (res > testValue);

            op_s[i] = '+';
            res += v_s[i];
            res_s[i++] = res;
         }
         --i; // prepare to go back

         okEq = (res == testValue) && !over;
      }

      if (okEq){
         ++count;

         if (sum > std::numeric_limits<TResult>::max() - testValue){
            throw std::runtime_error("Overflow, TResult size to be increased");
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
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}