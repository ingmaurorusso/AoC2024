
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;


NAoc__MR::TResult day15Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   std::vector<std::string> fieldLines;
   std::vector<std::string> moveLines;

   PointUnsign robotPos;
   using Positions = std::unordered_set<PointUnsign>;

   Positions boxes;
   // Positions walls;

   constexpr char Wall = '#';
   constexpr std::string_view DoubleWall = "##"sv;
   constexpr char Box = 'O';
   constexpr std::string_view DoubleBox = "[]"sv;
   constexpr char OpenBox = DoubleBox[0];
   constexpr char CloseBox = DoubleBox[1];
   constexpr char Free = '.';
   constexpr std::string_view DoubleFree = ".."sv;
   constexpr char Robot = '@';
   constexpr std::string_view DoubleRobot = "@."sv;

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
         // TODO: improve performance wil a single-scroll.
         stringReplaceRgx(line,std::string(1,Box),std::string(DoubleBox));
         stringReplaceRgx(line,std::string(1,Wall),std::string(DoubleWall));
         stringReplaceRgx(line,std::string("\\."),std::string(DoubleFree));
         // robot at the end, otherwise new Free is doubled again by Free->DoubleFree
         stringReplaceRgx(line,std::string(1,Robot),std::string(DoubleRobot));

         if (auto p = line.find(Robot); p != std::string::npos){
            robotPos = {p,lineCount-1U};
         }

         for(std::size_t i = 0U; i < line.size(); ++i){
            switch(line[i]){
            //case Wall: walls.insert(PointUnsign{i,lineCount-1U});
            case OpenBox:
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

         if ((!bad) && (DoubleBox.find(ch) != std::string_view::npos)){
            // look for a move on free position (or wall).
            std::unordered_set<Positions::const_iterator> touchedBoxes;
            {
               auto box = robotPos;
               if (ch == CloseBox) box.x--;
               touchedBoxes.insert(boxes.find(box));
            }

            auto posBeyond = robotPos;
            Positions allPosBeyond;
            allPosBeyond.insert(robotPos);

            const auto addBox2ToPositions =
               [&field = std::as_const(field)](auto& positions, PointUnsign pOfBox){
                  positions.insert(pOfBox);

                  auto posSide = pOfBox;
                  field.movePoint(posSide, (field[pOfBox] == OpenBox) ? Direction4::Right : Direction4::Left);
                  positions.insert(posSide);
               };

            if (dir4IsVertical(dir)){
               addBox2ToPositions(allPosBeyond, robotPos);
            }

            // Assume walls on boundaris, otherwise not specified
            // what to do if a box has to go outside the field.
            bool anyWall = false, allFree = false;
            do{
               bool anyNewBox = false;

               Positions newPositions;

               std::for_each( allPosBeyond.begin(), allPosBeyond.end(),
                         [ &field = std::as_const(field),
                           &boxes = std::as_const(boxes),
                           &newPositions, &addBox2ToPositions,
                           &touchedBoxes, &anyNewBox, &anyWall, dir ](auto pos)
                         {
                           field.movePoint(pos, dir, 1+!!dir4IsHorizontal(dir));

                           bool close = false;
                           switch(field[pos]){
                           case Wall: anyWall = true; break;
                           case CloseBox :
                              close = true;
                              //[[fallthrough]]
                           case OpenBox :
                              anyNewBox = true;
                              {
                                 auto box = pos;
                                 if (close) box.x--;
                                 touchedBoxes.insert(boxes.find(box));
                              }

                              if (dir4IsVertical(dir)){
                                 addBox2ToPositions(newPositions, pos);
                              } else{
                                 newPositions.insert(pos);
                              }
                           // case Free:
                           }
                         });
               allFree = !(anyWall || anyNewBox);

               allPosBeyond = std::move(newPositions);
            } while (!(allFree || anyWall));

            if (!allFree){ // <=> anyWall==true
               bad = true;
            }else{
               // TODO: not best performance... as now is write Free and then replace with Box
               std::for_each( touchedBoxes.cbegin(), touchedBoxes.cend(),
                              [&field, dir](auto citBox){
                                 field[*citBox] = Free;
                                 field(citBox->x+1U, citBox->y) = Free;
                              } );

               Positions newBoxes;
               std::for_each( touchedBoxes.begin(), touchedBoxes.end(),
                              [&field, &boxes, &newBoxes, dir](auto itBox){
                                 auto p = *itBox;
                                 field.movePoint(p, dir);
                                 newBoxes.insert(p);

                                 field[p] = OpenBox;
                                 field(p.x+1U, p.y) = CloseBox;
                                 
                                 boxes.erase(itBox);
                              } );

               boxes.merge(newBoxes);

               /* old code
               // move back to change points in boxes and the field.
               auto invDir = dir;
               dir4Invert(invDir);
               auto fixRobotPos = robotPos;

               if (field[fixRobotPos] == CloseBox){ // means 'dir' cannot be right.
                  // move to open as boxes contains open-positions.
                  fixRobotPos.x--;
                  posBeyond.x--;
               }

               if (dir4IsVertical(dir)){
                  boxes.insert(posBeyond); // add last new position
                  field[posBeyond] = OpenBox;

                  field[fixRobotPos] = Free;
                  boxes.erase(fixRobotPos); // remove first old position
                  
                  // do the same for close.
                  posBeyond.x++;
                  field[posBeyond] = CloseBox;

                  fixRobotPos.x++;
                  field[fixRobotPos] = Free;

                  // unchanged for intermediate positions.
               } else{
                  // need to change by 1 also intermediate positions.
                  do{
                     field.movePoint(posBeyond, invDir, 2);

                     boxes.erase(posBeyond);

                     auto newPos = posBeyond;
                     field.movePoint(posBeyond, dir);

                     boxes.insert(newPos);

                     field[newPos] = OpenBox;
                     newPos.x++; // not posBeyond if dir==Right
                     field[newPos] = CloseBox;
                  }while(posBeyond != fixRobotPos);
               }
               */
            }
         }

         if (bad){
            robotPos = oldP;
            // useless if move failed due to box near the walls.
         } else{
            field[oldP] = Free;
            field[robotPos] = Robot;
         }

         if (IsShortTest) std::cout << field << std::endl;
      }
   }

   for(auto& box : boxes){
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
   std::cout << "\nResult P2: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}
