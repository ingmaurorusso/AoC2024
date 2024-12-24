
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day02Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult nSafeReports{};

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
      if ( std::find_if(std::begin(line), std::end(line), [](char ch){return !(std::isdigit(ch) || std::isspace(ch));})
          != std::end(line) ){
         throw std::invalid_argument(msgLine + "Not only numbers in: <" + line + ">");
      }
      //  --- END LINE CHECKS ---

      struct ManageCheck{
         bool first = true, second{};
         bool increase{};
         unsigned val{}, oldVal{};
      };
      ManageCheck mc{};

      struct ManageRecovery{
         std::size_t idx = 0, idxBad{0};
         bool firstBad = true, secondBad{};
         unsigned recoveryVal{}, oldVal2{}, oldVal3{};
      };
      ManageRecovery mr{};

      bool goodReport = true;

      const auto check = [&mc](){
         if (!mc.first){
            bool newInc = (mc.val > mc.oldVal);
            if (mc.second){
               mc.increase = newInc;
               mc.second = false;
            } else
            if (mc.increase != newInc){
               return false;
            }

            const auto diff = mc.increase? mc.val - mc.oldVal : (mc.oldVal - mc.val);
            if ((diff == 0) || (diff > 3)){
               return false;
            }
         } else{
            mc.first = false;
            mc.second = true;
         }

         return true;
      };

      enum class SolvingMode{ LessCode, FastCode, Mixed };
      using enum SolvingMode;

      constexpr SolvingMode solvingMode = FastCode;

      if constexpr(solvingMode == LessCode  ||  solvingMode == Mixed){
         std::list<unsigned> values;
         {
            unsigned val{};
            while (lineStream >> val){
               values.push_back(val);
            }

            if (val == std::numeric_limits<decltype(val)>::max()){
               throw std::invalid_argument(msgLine + "Too wide number in: <" + line + ">");
            }
         }

         const auto checkList = [&check, &mc](auto begin, auto end, unsigned avoid){
            mc.first = true;

            std::size_t idx = 0U;
            return std::all_of( begin, end,
               [&mc, check, &idx, avoid](auto newVal){
                  bool res = true;
                  if (idx != avoid){
                     mc.val = newVal;
                     res = check();

                     mc.oldVal = mc.val;
                  }
                  ++idx;
                  return res;
               }
            );
         };

         const auto checkListTolerance = [&check, &mc](auto begin, auto end) -> std::pair<bool, std::size_t>{
            auto dist = std::distance(begin, end);

            for(std::size_t avoid = 0; (avoid < dist); ++avoid){
               mc.first = true;

               std::size_t idx = 0U;
               if (std::all_of( begin, end,
                  [&mc, check, &idx, avoid](auto newVal){
                     if (idx != avoid){
                        mc.val = newVal;
                        if (!check()){
                           return false;
                        }

                        mc.oldVal = mc.val;
                     }
                     ++idx;
                     return true;
                  }))
                  return {true, avoid};
            };

            return {(dist == 0U), std::size_t(dist)};
         };

         if (!checkList(values.cbegin(), values.cend(), values.size())){
            if constexpr(solvingMode == LessCode){
               // 1) Less-code solution: quadratic run-time.
               goodReport = checkListTolerance(values.cbegin(), values.cend()).first;
            } else // SolvingMode::Mixed: linear-time but factor 3 or 4.
            if (values.size() <= 4){
               goodReport = checkListTolerance(values.cbegin(), values.cend()).first;
            } else{
               // Use sub-lists by 4: as soon as one is found needing tolerance,
               // enlarge by 1 and 2 still accepting tolerance and then these 2
               // must begin a tail sub-list acceptable without tolerance.
               auto ahead4 = std::next(values.cbegin(), 4U);
               auto init = values.cbegin();

               goodReport = false;
               while(checkList(init, ahead4, 4U)){
                  ++init;
                  if (values.cend() == (ahead4++)){
                     goodReport = true;
                     break;
                  }
               }

               if (!goodReport){
                  // One list of 4 was found bad.
                  if (std::distance(ahead4, values.cend()) < 2U){
                     // There are less than 2 elements after ahead3.
                     goodReport = checkListTolerance(init, values.cend()).first;
                  } else{
                     if (auto [good, excluded] = checkListTolerance(init, ahead4); good){
                        if (checkList(++init, ++ahead4, (excluded > 0U)? excluded-1U : 4U ) &&
                            checkList(++init, ++ahead4, (excluded > 1U)? excluded-2U : 4U )){
                           ++init;
                           ++init;
                           goodReport = checkList(init, values.cend(), values.size());
                        }
                     }
                  }
               }
            }
         }
      } else
      if constexpr(solvingMode == FastCode){
         // 2) Fast-code solution: liner run-time.

         while ((lineStream >> mc.val) && goodReport){
            ++mr.idx;

            goodReport = check();

            if (!goodReport && mr.firstBad){
               // three chances: remove 'val' or remove 'oldVal',
               //                or remove 'oldVal2'
               // (_1_)
               // Start removing 'val'.
               mr.recoveryVal = mc.val;
               mr.firstBad = false;
               mr.secondBad = true;
               goodReport = true;
               mr.idxBad = mr.idx;

               if (mr.idx == 2){
                  mc.second = true;
               }

               continue; // avoid update oldVal, as 'val' has been discarded.
            } else
            if (!goodReport && (mr.idx == mr.idxBad+1U) /*&& mr.secondBad*/){
               // (_2_)
               auto newVal = mc.val;
               goodReport = true;
               auto frozenOldVal = mc.oldVal;

               // Try replacing oldVal with recoveryVal
               if (mr.idx > 3){
                  mc.oldVal = mr.oldVal2;
                  mc.val = mr.recoveryVal;
                  mc.second = (mr.idx == 4);
                  goodReport = check();
               } else{ //idx == 3
                  mc.second = true;
               }

               if (goodReport){ // not strictly needed
                  mc.oldVal = mr.recoveryVal;
                  mc.val = newVal;
                  goodReport = check();
               }

               if (!goodReport && (mr.idx > 3)){
                  // Another chance is to use both
                  // recoveryVal and the previous one,
                  // removing the second previous.
                  goodReport = true;
                  if (mr.idx > 4){ // else: oldVal3 does not exist
                     mc.oldVal = mr.oldVal3;
                     mc.val = frozenOldVal;
                     goodReport = check();
                  } else{
                     mc.second = true;
                  }

                  if (goodReport){ // not strictly needed
                     mc.oldVal = frozenOldVal; // = val (if idx > 4)
                     mc.val = mr.recoveryVal;
                     goodReport = check();
                  }

                  if (goodReport){ // not strictly needed
                     // useless to set mr.oldVal2, as oldVal3 will be never used again.
                     // mr.oldVal2 = frozenOldVal; // = oldVal
                     
                     mc.oldVal = mr.recoveryVal; // = val
                     mc.val = newVal;
                     goodReport = check();
                  }
               }
            } else
            if (!goodReport && (mr.idx == 4) && mr.secondBad){
               auto newVal = mc.val;
               goodReport = true;
               auto frozenOldVal = mc.oldVal;

               // idx==4 => idxBad==2 (cannot be 1 and, if it was 3,
               // the code (_2_) would have been executed).
               // There still is the chance that second_to_third was
               // in opposite order compared to first_to_third, as
               // well as in the same order of third_to_fourth,
               // where the second was the discarded one in (_1_).
               // Note that idx > 4 does not give any chance to
               // switch direction. Moreover, this try does not make sense
               // if the new bad couple third_to_fourth is because
               // of bad distance.

               mc.oldVal = mr.recoveryVal;
               mc.val = frozenOldVal;
               mc.second = true;
               goodReport = check();

               if (goodReport){ // not strictly needed
                  // useless to set oldVal2, as oldVal3 will be never used again.
                  // oldVal2 = recoveryVal; // = oldVal

                  mc.oldVal = frozenOldVal;
                  mc.val = newVal;
                  goodReport = check();
               }
            }
            
            mr.oldVal3 = mr.oldVal2;
            mr.oldVal2 = mc.oldVal;
            mc.oldVal = mc.val;
         }

         if (mc.val == std::numeric_limits<decltype(mc.val)>::max()){
            throw std::invalid_argument(msgLine + "Too wide number in: <" + line + ">");
         }         
      } else{
         throw std::domain_error("Bad definition for solvingMode");
      }

      if (goodReport){
         ++nSafeReports;
      }
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << nSafeReports << std::endl;
   std::cout << std::endl;

   return nSafeReports;
}
