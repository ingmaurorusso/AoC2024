
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day09Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   using TSize = std::size_t;
   TResult sum{0U};

   using Tid = unsigned;
   using File = std::pair<TSize, Tid>;
   using Regions = std::list<std::pair<TSize, File>>; //absolute original position and range
   Regions regions;
   std::map<TSize, std::map<std::size_t,decltype(Regions{}.begin())>> freeRegionsBySize;

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
      std::size_t fullPosIndex = 0U;
      std::size_t nZeroFree = 0U;;
      for(auto it = line.cbegin(); it != line.cend(); ++it){
         auto ch = *it;
         auto size = ch - '0';

         if (size > 0U) // no need to insert empty regions for Part 2,
                        // as the algorithm does not relay on 2 moves per time,
            regions.push_back({fullPosIndex, {full? id: 0U,size}});
         if (!full){
            if (size > 0U) // speed-up
               freeRegionsBySize[size].insert({fullPosIndex,std::prev(regions.end())});
            if (size > 8){
               ++nZeroFree; // TODO: remove
            }
         }

         if ((size == 0) && !full) ++nZeroFree;
         
         full = !full;
         if (full) ++id;

         fullPosIndex += size;
      }

      const auto printRegions = [&regions](){
         std::for_each(regions.cbegin(), regions.cend(), [](auto pair){
            std::cout << "[pos=" << pair.first << ", id=" << pair.second.first << ", s=" << pair.second.second << "] ; ";
         });
         std::cout << "\n\n";
      };

      if (IsShortTest)
         printRegions();

      constexpr auto mergeFreedBlocks = true; // not needed

      // for each non-empty, try to move earlier
      auto itBack = regions.rbegin();
      while(itBack->second.first == 0U){
         ++itBack;
      }

      while(itBack != regions.rend()){
         auto& fileRef = itBack->second;
         const auto fileOrg = itBack->second;

         auto& filePosRef = itBack->first;
         const auto filePosOrg = itBack->first;

         auto& fileIdRef = fileRef.first;
         auto& fileSizeRef = fileRef.second;
         const auto fileSizeOrg = fileOrg.second;

         auto itFreeMap = freeRegionsBySize.lower_bound(fileSizeOrg);

         if (itFreeMap != freeRegionsBySize.end()){
            // there is a free region that fit
            // find wider on further left, if any.
            auto spacePosOrg = itFreeMap->second.begin()->first;
            for(auto itFreeMap2 = std::next(itFreeMap); itFreeMap2 != freeRegionsBySize.end(); ++itFreeMap2){
               auto candidatePos = itFreeMap2->second.begin()->first;
               if (candidatePos < spacePosOrg){
                  itFreeMap = itFreeMap2;
                  spacePosOrg = candidatePos;
               }
            }

            auto& freeMapRef = itFreeMap->second;
            auto itFree = freeMapRef.begin()->second;

            auto& spaceRef = itFree->second;
            // auto spacePosOrg = itFree->first; set above
            auto& spacePosRef = itFree->first;

            if (spacePosOrg < filePosOrg){ // avoid to move right again
               const auto spaceSizeOrg = spaceRef.second;

               spaceRef = fileOrg;

               const auto residual = spaceSizeOrg - fileSizeOrg;

               if (residual > 0U){
                  ++itFree; // because the new has to be inserted after
                  itFree = regions.insert(itFree, {spacePosOrg+fileSizeOrg, File{0U, residual}} );

                  freeRegionsBySize[residual].insert({freeMapRef.begin()->first + fileSizeOrg, itFree});
               }

               freeMapRef.erase(freeMapRef.begin());
               if (freeMapRef.empty()){
                  freeRegionsBySize.erase(itFreeMap);
               }

               fileIdRef = 0U; // nullify the block

               if constexpr (mergeFreedBlocks){
                  // since itBack remains free, it may be merged to others adjacent
                  // and must be anyway inserted in free regions map.
                  auto newSpacePos = filePosOrg;

                  auto itOther = std::next(itBack); // on the left, with i==0
                  for(int i = 0; i < 2; ++i){
                     if (itOther != regions.rend()){ // check needed for i==0
                        const auto otherSpacePosOrg = itOther->first;
                        const auto otherSpaceOrg = itOther->second;
                        if (otherSpaceOrg.first == 0U){ // id==0 <-> another free region   
                           const auto otherSizeOrg = otherSpaceOrg.second;

                           if (otherSizeOrg > 0U){
                              fileSizeRef += otherSizeOrg;

                              auto itFreeMapOther = freeRegionsBySize.find(otherSizeOrg);
                              auto& freeMapOther = itFreeMapOther->second;
                              auto itFreeOther = freeMapOther.find(otherSpacePosOrg);

                              freeMapOther.erase(itFreeOther);
                              if (freeMapOther.empty()){
                                 freeRegionsBySize.erase(itFreeMapOther);
                              }

                              if (i == 0){
                                 filePosRef = otherSpacePosOrg;
                              }
                           }

                           ++itBack; // otherwise for i==1, removing its 'base' forward node
                                     // would make itBack inconsistent.
                           regions.erase(std::prev(itOther.base()));
                           --itBack;
                        }
                     }

                     if (itBack == regions.rbegin()) break;

                     itOther = std::prev(itBack); // on the right, with i==1
                  }

                  freeRegionsBySize[fileSizeRef].insert({filePosRef,std::prev(itBack.base())});
               }

               if (IsShortTest)
                  printRegions();
            }
         }

         do{
            if (regions.rend() == ++itBack) break;
         }while(itBack->second.first == 0U); // avoid free spaces (marked with id==0)
      }

      TSize idNew = 0U;
      sum += std::accumulate(regions.cbegin(), regions.cend(), TResult{0U}, [&idNew] (TResult sum, const auto pair){
         const auto region = pair.second;
         auto idOrg = region.first;
         const auto size = region.second;
         if (idOrg > 0U){
            --idOrg;

            TSize add = (idNew * size + size*(size-1)/2) * idOrg;

            checkSumResult(sum, add);
            sum += add;
         }

         idNew += size;
         return sum;
      });

      if (IsShortTest)
         printRegions();
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}