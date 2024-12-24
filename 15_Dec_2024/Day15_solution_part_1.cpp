
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day15Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   std::vector<std::string> fieldLines;
   std::vector<std::string> moveLines;

   PointUnsign robotPos;
   std::unordered_set<PointUnsign> boxes;
   // std::unordered_set<PointUnsign> walls;

   constexpr char Wall = '#';
   constexpr char Box = 'O';
   constexpr char Free = '.';
   constexpr char Robot = '@';

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
      static constexpr std::string FieldElems = {Wall, Box, Robot, Free};
      std::size_t p1 = line.find_first_not_of(FieldElems);
      std::size_t p2 = line.find_first_not_of(Arrows4);
      auto badP = std::string::npos;
      if ((p1 != std::string::npos) && (Arrows4.find(line[p1]) == std::string::npos)){
         badP = p1;
      } else
      if ((p2 != std::string::npos) && (FieldElems.find(line[p2]) == std::string::npos)){
         badP = p2;
      } else
      if ((p1 != std::string::npos) && (p2 != std::string::npos)){
         badP = std::max(p1,p2);
      }

      if (badP != std::string::npos){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,line[badP]));
      }
      //  --- END LINE CHECKS ---

      if (line.find_first_of(Arrows4) != std::string::npos){
         moveLines.push_back(std::move(line));
      } else{
         if (auto p = line.find(Robot); p != std::string::npos){
            robotPos = {p,lineCount-1U};
         }

         for(std::size_t i = 0U; i < line.size(); ++i){
            switch(line[i]){
            //case Wall: walls.insert(PointUnsign{i,lineCount-1U});
            case Box:
               boxes.insert(PointUnsign{i,lineCount-1U});
            }
         }

         fieldLines.push_back(std::move(line)); //TODO: remove when not needed
      }
   }

   Field2D field(fieldLines);
   if (IsShortTest) std::cout << field << std::endl;

   for(const auto& moveRow : moveLines){
      for(auto arrow : moveRow){
         auto dir = dir4FromArrow(arrow).first;

         auto oldP = robotPos;

         char ch;
         bool bad = !field.movePoint(robotPos, dir);
         if (!bad){
            ch = field[robotPos];
            bad = (ch == Wall);
         }

         if ((!bad) && (ch == Box)){
            // look for a move on free position (or wall).
            auto posBeyond = robotPos;

            // Assume walls on boundaris, otherwise not specified
            // what to do if a box has to go outside the field.
            bool wall = false, free = false;
            while (!(free || wall)){
               field.movePoint(posBeyond, dir);
               switch(field[posBeyond]){
               case Free:
                  free = true;
                  break;
               case Wall:
                  wall = true;
               }
            }

            if (!free){
               bad = true;
            }else{
               // move back to change points in boxes and the field.
               // dir4Invert(dir);

               boxes.insert(posBeyond); // add last new position
               field[posBeyond] = Box;

               field[robotPos] = Free;
               boxes.erase(robotPos); // remove first old position
               // unchanged positions in the middle.
            }
         }

         if (bad){
            robotPos = oldP;
            // useless if move failed due to box near the walls.
         } else{
            field[oldP] = Free;
            field[robotPos] = Robot;
         }

         if (IsShortTest)
            std::cout << field << std::endl;
      }
   }

   for(auto box : boxes){
      // TODO: add overflow check also for 'add' computation
      auto add = box.x + 100*box.y;
      if (sum > std::numeric_limits<TResult>::max()-add){
         throw std::runtime_error("TResult to be wider");
      }
      sum += add;
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "Number of boxes: " << boxes.size() << std::endl;
   // TODO: print walls on boundaries
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}