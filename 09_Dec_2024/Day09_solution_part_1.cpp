
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day09Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   using TSize = std::size_t;
   TResult sum{0U};

   using Tid = unsigned;
   std::list<std::pair<TSize, Tid>> regions;

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
      if (lineCount > 1){
         throw std::invalid_argument(msgLine + "unexpected line after first one");
      }
      if ( const auto it = std::find_if( line.cbegin(), line.cend(),
                                         [](char ch){return !std::isdigit(ch);} );
           it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected non-digit character: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      bool full = true;
      auto id = 1U;
      auto pos = 0;
      auto nFiles = 0U;
      std::size_t nZeroFree = 0U;;
      for(auto it = line.cbegin(); it != line.cend(); ++it){
         auto ch = *it;
         auto size = ch - '0';

         regions.push_back({full? id: 0U, size});

         if ((size == 0) && !full) ++nZeroFree;
         
         full = !full;
         if (full) ++id;
      }

      auto iBack = regions.size()-1U;
      auto itBack = regions.rbegin();

      if (iBack%2){
         --iBack;
         ++itBack;
      }

      auto iNextFree = 1U;
      auto itNextFree = std::next(regions.begin());

      while (itNextFree->second == 0){
         iNextFree += 2;
         if (iNextFree >= regions.size()) break;
         ++itNextFree;
         ++itNextFree;
      }

      const auto printRegions = [&regions](){
         std::for_each(regions.cbegin(), regions.cend(), [](auto pair){
            std::cout << "[id=" << pair.first << ", s=" << pair.second << "] ; ";
         });
         std::cout << '\n';
      };
         

      while(iBack > iNextFree){
         itNextFree->first = itBack->first;
         const auto freezeSize = itNextFree->second;
         const auto freezeBack = itBack->second;

         bool eq = (itNextFree->second <= itBack->second);
         if (freezeSize <= freezeBack){
            itBack->second -= freezeSize;

            iNextFree += 2U;
            if (iNextFree < iBack){
               ++itNextFree;
               ++itNextFree;

               while (itNextFree->second == 0){
                  iNextFree += 2U;
                  if (iNextFree >= regions.size()) break;
                  ++itNextFree;
                  ++itNextFree;
               }
            }
         }
         if (freezeSize >= freezeBack){
            auto residual = freezeSize - freezeBack;
            if (!eq){ //changes managed in previous 'if'
               itNextFree->second = itBack->second;
            }
            if (residual > 0U){ // 0 in case of eq==true
               ++itNextFree; // because the new has to be inserted after
               itNextFree = regions.insert( itNextFree, {0U, residual} );
            }

            itBack->first = 0U;
            itBack->second = 0U; // useless
            if (iNextFree+2 < iBack){
               iBack -= 2;
               ++itBack;
               ++itBack;
            } else iBack = 0;
         }

         if (IsShortTest) printRegions();
      }

      std::size_t idNew = 0U;
      sum += std::accumulate(regions.cbegin(), regions.cend(), TResult{0U}, [&idNew] (TResult sum, const auto pair){
         auto idOrg = pair.first;
         if (idOrg == 0) return sum; //and might break
         --idOrg;

         auto size = pair.second;
         auto add = (idNew * size + size*(size-1)/2) * idOrg;

         if (sum > std::numeric_limits<TResult>::max() - add){
            throw std::runtime_error("Type TResult has to be wider");
         }

         sum += add;

         idNew += size;
         return sum;
      });
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}