
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

using PointAndDir = std::pair<PointUnsign,Direction4>;
using PositionsWithSource = std::unordered_map<PointUnsign,PointAndDir>;

using PositionsWithDir = std::unordered_set<PointAndDir>; // rely on hash_custom.h

constexpr char Obstacle = '#';
constexpr char FreePos = '.';
constexpr std::string Expected{Arrows4 + std::string(1,Obstacle) + std::string(1,FreePos)};

// false if there is loop
bool findPattern(const Field2D& field, PointUnsign guardPos, Direction4 guardDir, PositionsWithSource &passedPositions){
   passedPositions.clear();
   PositionsWithDir passedPosWithDir;

   passedPositions.insert({guardPos,{guardPos,guardDir}});
   passedPosWithDir.insert(std::make_pair(guardPos,guardDir));

   // check position is closed in any direction
   if ( (guardPos.x > 0U) && (field(guardPos.x-1U,guardPos.y) == Obstacle) &&
        (guardPos.x+1 < field.dimX()) && (field(guardPos.x+1U,guardPos.y) == Obstacle) &&
        (guardPos.y > 0U) && (field(guardPos.x,guardPos.y-1U) == Obstacle) &&
        (guardPos.y+1 < field.dimY()) && (field(guardPos.x,guardPos.y+1U) == Obstacle) ){
      return false; // loop, impossible to escape from starting position
   }

   bool ended = false;
   while(!ended){
      auto oldPos = guardPos;
      auto oldDir = guardDir;

      ended = !field.movePoint(guardPos, guardDir);

      if (ended) break;

      if (field[guardPos] == Obstacle){
         guardPos = oldPos;
         dir4rotate(guardDir,false);
         // false=anti-clockwise, because Down here represents
         // reducing 'y', that is 'up' in relaity, and the puzzle
         // wants clockwise in reality.
         continue;
      }

      if (!passedPosWithDir.insert(std::make_pair(guardPos,guardDir)).second){
         return false;
      }
      passedPositions.insert({guardPos,{oldPos,oldDir}});
   }

   return true;
}

NAoc__MR::TResult day06Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   std::vector<std::string> lines;

   Point guardPos;
   bool foundPos = false;

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
      if (!lines.empty()){
         if (line.size() != lines[0].size()){
            throw std::invalid_argument(msgLine + "non-uniform lines length");
         }
      }

      if (auto p = line.find_first_not_of(Expected); p != std::string::npos){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,line[p]));
      }

      if (auto p1 = line.find_first_of(Arrows4); p1 != std::string::npos){
         auto p2 = line.find_last_of(Arrows4);

         if (foundPos || (p1 != p2)){
            throw std::invalid_argument(msgLine + "multiple starting positions");
         }

         guardPos = {p1, lineCount-1U};
         foundPos = true;
      }
      //  --- END LINE CHECKS ---

      lines.emplace_back(std::move(line));
   }

   if (!foundPos){
      throw std::invalid_argument("No initial position");
   }

   Direction4 guardDir = dir4FromArrow(lines[guardPos.y][guardPos.x]).first;

   Field2D field{std::move(lines)};

   PositionsWithSource passedPositions{};

   if (!findPattern(field, guardPos, guardDir, passedPositions)){
      throw std::invalid_argument("loop for the original conditions");
   }

   const auto startPos = guardPos;
   const auto startDir = guardDir;
   TResult countLoop = 0U;

   // Start only from positions of the original pattern,
   // just before hitting an obstacle.
   for (const auto [p, source] : passedPositions){
      // if (field[p] == Obstacle) continue; not possible
      if (p == startPos) continue;

      guardPos = source.first;
      guardDir = source.second;

      field[p] = Obstacle;

      PositionsWithSource loopPositions{};
      if (!findPattern(field, guardPos, guardDir, loopPositions)){
         checkSumResult(countLoop,1U);
         ++countLoop;
      }

      field[p] = FreePos;
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << countLoop << std::endl;
   std::cout << std::endl;

   return countLoop;
}