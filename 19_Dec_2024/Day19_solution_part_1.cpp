
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

static
bool checkStringSum( const std::string& line, std::map<std::string, std::size_t>& towelsMap,
                     std::size_t maxTowelLen, bool recomputeIndexes ){
   const auto Ntow = towelsMap.size();
   const auto N = line.size();

   if (recomputeIndexes){
      std::size_t i = 0;
      for( auto& [towel, idx] : towelsMap ){
         idx = i++; // to support (_1_)
      }
   }

   Sequences generator(towelsMap, N); // TODO: scale by minimum length of towels.

   auto genRes = generator.beginLexicographic();

   auto& [seqItMap, changedIdx, ok] = genRes;

   std::vector<std::size_t> partialBefore(N, 0);

   std::unordered_set<std::size_t> goodPositions;

   bool impossible = false;

   while(ok && !impossible){
      auto consume = partialBefore[changedIdx];
      std::size_t i = changedIdx;

      std::size_t changeIdx;

      
      for(; (i < N) && ok && !impossible; ++i){
         auto sIt = seqItMap[i];
         if (line.substr(consume, sIt->first.size()) == sIt->first){
            consume += sIt->first.size();
            if (consume == N) break; // not needed to use exactly 'N'

            if (i < N){
               partialBefore[i+1] = consume;
            }

            if (consume == 46){
               impossible = false;
            }

            if (!goodPositions.insert(consume).second){
               // passing already for the same position is useless, as
               // there is no limits on the number of each towel.
               // unconstrained programming !
               changeIdx = i;
               ok = false;
            }
         } else{
            changeIdx = i;
            ok = false;
         }
      }

      if (ok && (consume == N)){
         // check:
         std::string sCheck;
         if (!std::any_of( seqItMap.begin(), seqItMap.end(),
                        [&sCheck,line](auto sIt)
                        {
                        sCheck += sIt->first;
                        return (sCheck == line);
                        }))
            throw "error";
         
         return true;
      } else
      if (!impossible){
         // speed-up: look for a good string.
         std::size_t currentTried = generator.getIndexAt(changeIdx);
         std::size_t nextToTry = currentTried;
         auto sIt = seqItMap[changeIdx];

         if (ok){ //did not consume all, look for any longer
            // TODO: test this part, never hit.
            while((++nextToTry), towelsMap.cend() != ++sIt){
               if (sIt->first.size() == line.size()-partialBefore[N-1]){
                  break;
               }
            }
         } else{
            consume = partialBefore[changeIdx];

            bool found = false;
            for(std::size_t l = 1; l <= std::min(line.size()-consume, maxTowelLen); ++l){
               auto itCandidate = towelsMap.find(line.substr(consume,l));
               if (itCandidate != towelsMap.end()){
                  if (itCandidate->second > currentTried){
                  // based on the fact that towels was sorted <-> same order as in the map.
                     if ((!found) || (itCandidate->second < nextToTry)){
                        nextToTry = itCandidate->second; //(_1_)
                        found = true;
                        break; // because shorter strings come first.
                     }
                  }
               }
            }
            if (!found){
               nextToTry = Ntow;
            }
         }

         if (nextToTry == Ntow){
            bool impossibleBack = false;

            if (changeIdx == 0U){
               impossibleBack = true;
            }else --changeIdx;

            if (ok){ //did not consume all, look back for any longer in a faster way.
               // TODO: test this part, never hit.
               while ( line.size()-partialBefore[changeIdx] >
                        (N-changeIdx)*maxTowelLen ){
                  if (changeIdx == 0U){
                     impossibleBack = true;
                     break;
                  } else --changeIdx;
               }
            }

            impossible = impossibleBack;
         } else{
            generator.force(changeIdx, nextToTry-1U);
         }

         if (!impossible){
            genRes = generator.nextLexicographic(changeIdx,!ok);
         }
      }
   }

   return false;
}

NAoc__MR::TResult day19Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult count{0U};
   TResult bad = 0;

   std::vector<std::string> lines;
   std::vector<std::string> towels;
   std::map<std::string, std::size_t> towelsMap;
   std::size_t maxTowelLen;
   std::size_t Ntow;
   std::size_t nRemovedTowel = 0U;

   bool squencesBegun = false;

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines) break;

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         if (squencesBegun){
            std::cout << msgLine << "WARNING: empty line\n";
         }
         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---

      if (lineCount == 1){
         towels = split(line, ",", true);

         // TODO: compute after reduction. This may speed-up.
         maxTowelLen = std::accumulate( towels.begin(), towels.end(), std::size_t(0U),
                                        [](auto acc, auto towel){return std::max(acc,towel.size());} );

         Ntow = towels.size();

         std::sort(towels.begin(), towels.end());
         for(std::size_t i = 0; i < Ntow; ++i){
            if (towels[i].empty()){
               throw std::invalid_argument(msgLine + "some empty towel");
            }
            towelsMap[towels[i]] = i;
         }

         auto towelsMapRed = towelsMap;

         for(auto [towel, idx] : towelsMap){
            towelsMapRed.erase(towel);

            if (!checkStringSum(towel,towelsMapRed,Ntow-1U,true)){
               towelsMapRed[towel] = idx;
            } else{
               ++nRemovedTowel;
               if (IsShortTest){
                  std::cout << "Removed useless towel " << towel << std::endl; // speed-up
               }
            }
         }
         towelsMap = std::move(towelsMapRed);

         // correct 'reduced' index to support (_1_)
         if (nRemovedTowel > 0U){
            std::size_t j = 0;
            for(std::size_t i = 0; i < Ntow; ++i){
               if (auto it = towelsMap.find(towels[i]); it != towelsMap.end()){
                  it->second = j++;
               }
            }
         }
      } else{
         squencesBegun = true;

         std::unordered_set<std::size_t> alreadyAhead;

         std::function<bool(std::string_view)> checkStringSumRec;
         checkStringSumRec =
            [&towelsMap = std::as_const(towelsMap), &alreadyAhead, &line = std::as_const(line), &checkStringSumRec]( std::string_view residualLine ){
               auto idxStarView = std::distance(&*(line.cbegin()), &*(residualLine.begin()));

               if (alreadyAhead.insert(idxStarView).second){
                  for(const auto& [towel,idx] : towelsMap){
                     if (residualLine.find(towel) == 0U){
                        if (checkStringSumRec(std::string_view(std::next(residualLine.begin(),towel.size()), residualLine.end()))){
                           return true;
                        }
                     }
                  }
               }

               return residualLine.empty();
            };

         bool ok = checkStringSumRec(std::string_view{line});

         /*if (checkStringSum(line, towelsMap, maxTowelLen, false) != ok){
            throw "Fix code!";
         }*/

         if (ok){
            ++count;
         } else{
            if (IsShortTest) std::cout << "lineCount = " << lineCount << " : " << line << std::endl;
            ++bad;
         }
      }

      lines.push_back(std::move(line));
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << count  << std::endl;
   std::cout << "\nn. bad: " << bad << std::endl;
   std::cout << "\nn. removed useless towels: " << nRemovedTowel << std::endl;

   std::cout << std::endl;

   return count;
}