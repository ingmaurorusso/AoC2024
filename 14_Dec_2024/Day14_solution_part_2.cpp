
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

using PosAndSpeedMap = std::unordered_map<TResult, std::pair<PointSign,PointSign>>;

using GraphP = Graph<PointUnsign,GraphKind::Indirect>;

GraphP buildConnectionsFromField(const PosAndSpeedMap& posAndSpeedMap, const Field2D &field){
   GraphP connections;

   for(auto& [k_, p_v] : posAndSpeedMap){
      auto [pCopy, v_] = p_v;

      PointUnsign pU = {CoordUnsign(pCopy.x), CoordUnsign(pCopy.y)};

      // only two directions: it's indirect graph.
      for(std::underlying_type_t<Direction4> d = 0; d < 2U; ++d){
         auto p2 = pU;

         if (field.movePoint(p2,Direction4{d})){
            if (field[p2] == '*'){
               try{
                  connections << GraphP::Edge{p2, pU};
               }catch(std::invalid_argument){
                  // case of edge already present: robot positions may overlap.
               }
            }
         }
      }
   }

   return connections;
}

NAoc__MR::TResult day14Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   std::vector<std::string> lines;

   std::unordered_map<TResult, std::pair<PointSign,PointSign>> posAndSpeedByRobot;

   unsigned X = IsShortTest ? 11 : 101;
   unsigned Y = IsShortTest ? 7 : 103;

   constexpr unsigned times = 100;

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
      std::smatch matchInfo{};

      if (!std::regex_search(line.cbegin(), line.cend(), matchInfo, std::regex{R"__((.*?p=.*?)(-?[\d]+),(-?[\d]+)(.*?v=.*?)(-?[\d]+),(-?[\d]+).*)__"})){
         throw std::invalid_argument(msgLine + "unmatched syntax");
      }
      //  --- END LINE CHECKS ---

      auto values = getAllValues<CoordSign>(line);
      if (values.size() != 4U){
         throw std::invalid_argument(msgLine + "not exactly 4 numbers in the line");
      }

      CoordSign px = values[0];
      CoordSign py = values[1];
      if ((px < 0) || (py < 0)){
         throw std::runtime_error("negative initial position");
      }

      CoordSign vx = values[2];
      while (vx < 0) vx += X;
      CoordSign vy = values[3];
      while (vy < 0) vy += Y;

      posAndSpeedByRobot[lineCount] = {{px,py},{vx,vy}};


      lines.emplace_back(std::move(line)); //TODO: remove when not needed
   }

   Field2D fieldEmpty(X,Y);

   TResult lcm = 1;

   for(auto& [k_, p_v] : posAndSpeedByRobot){
      auto& [p_,v] = p_v;

      auto x1 = X/std::gcd(X,v.x);
      auto y1 = Y/std::gcd(Y,v.y);
      lcm = std::lcm(lcm, std::lcm(x1, y1));

      if ((lcm%x1 != 0) || (lcm%y1 != 0)){
         throw std::runtime_error("TResult to be wider");
      }
   }

   // lcm is the period after which the position is the same as beginning.
   auto posAndSpeedByRobot2 = posAndSpeedByRobot;
   for(auto& [k_, p_v] : posAndSpeedByRobot){
      auto& [p,v] = p_v;

      checkProdResult(std::max(v.x,v.y), lcm);
      checkProdResult(std::max(p.x,p.y), std::max(v.x,v.y)*lcm);
      p += (v*lcm);

      p.y %= Y;
      p.x %= X;
   }
   if (posAndSpeedByRobot != posAndSpeedByRobot2){
      throw std::runtime_error("fix code! lcm did not work!");
   }

   const auto middleX = X/2; // indexes are from 0
   const auto middleY = Y/2;

   TResult res = 0U;

   CoordUnsign maxBar = 0;

   TResult bestNumber = 0;
   bool foundAnyConnected = false;
   //Field2D bestField(X,Y);
   PointUnsign bestP;
   TResult maxCandidate = 0;

   for(TResult i = 0; i < lcm; ++i){
      Field2D fieldNew = fieldEmpty;

      std::array<TResult,4> qs = {0,0,0,0};
      std::array<TResult,3> qs2 = {0,0,0};
      for(auto& [k_, p_v] : posAndSpeedByRobot){
         auto& [p,v_] = p_v;

         fieldNew[PointUnsign(p.x,p.y)] = '*';

         if ((p.x != middleX) && (p.y != middleY)){
            unsigned idxQ = 0;
            if (p.y > middleY) idxQ += 2;
            if (p.x > middleX) idxQ++;

            qs[idxQ]++;
         }

         if (p.x < X/3.) qs2[0]++;
         else if (p.x < X*2./3.) qs2[1]++;
         else qs2[2]++;
      }

      PointUnsign bar(0,0);
      //compute density around baricenter, with Manatthan distance.
      // Before move.
      for(auto& [k_, p_v] : posAndSpeedByRobot){
         auto& [p,v] = p_v;

         bar += PointUnsign(p.x,p.y);
      }
      bar /= posAndSpeedByRobot.size();
      if (bar.y > maxBar)
         maxBar = bar.y;

      std::vector<CoordUnsign> vDensity(X+Y,0U);
      for(const auto& [k_, p_v] : posAndSpeedByRobot){
         const auto& [p,v_] = p_v;

         auto d = manDistance(bar,p);
         vDensity[d]++;
      }

      GraphP connections = buildConnectionsFromField(posAndSpeedByRobot, fieldNew);

      for(auto& [k_, p_v] : posAndSpeedByRobot){
         auto [pCopy, v_] = p_v;

         PointUnsign pU = {CoordUnsign(pCopy.x), CoordUnsign(pCopy.y)};

         auto candidate = connections.countReachablesFrom(pU).first;

         if (candidate > maxCandidate){
            bestNumber = i;
            bestP = pU;
            maxCandidate = candidate;
            //bestField = fieldNew; rebuilt at the end
            foundAnyConnected = true;
         }
      }

      // move for next step
      for(auto& [k_, p_v] : posAndSpeedByRobot){
         auto& [p,v] = p_v;

         p += v;

         p.x %= X;
         p.y %= Y;
      }

      // print density scaled [so that it can be visuailzed].
      constexpr unsigned VisualY = 50;
      auto itMax = std::max_element(vDensity.begin(), vDensity.end());
      auto max = *itMax;
      auto distForMax = std::distance(vDensity.begin(),itMax);
      if (max > VisualY){
         for(auto &d : vDensity){
            d /= (max / VisualY);
         }
      }

      //if (std::min(qs[2],qs[3]) > 1.5 * std::min(qs[0],qs[1])) did not work
      //if (bar.y >= 55) did not work
      /*if (std::max(qs2[0],qs2[2]) < qs2[1]/2){ // worked ! 8159 !
         std::cout << std::string(10,'\n');
         for(std::size_t k = 0; k < max; ++k){
            std::string line(vDensity.size(), ' ');
            for(std::size_t j = 0; j < vDensity.size(); ++j)
               if (vDensity[j] >= max-k) line[j] = '|';
            std::cout << line << std::endl;;
         }

         std::cout << std::string(10,'\n');
         //std::cout << fieldNew.buildSubField(0, fieldNew.dimX(), 25, 75);
         std::cout << fieldNew;

         res = i;
      }*/
   }

   TResult collapsedNumber = 0U;
   TResult treeRegionSize = 0U;
   if (foundAnyConnected){
      Field2D bestField(X,Y);

      // print for the best candidate

      PointUnsign bar(0,0);
      for(auto& [k_, p_v] : posAndSpeedByRobot2){
         auto& [p,v] = p_v;
         p += (v*bestNumber);

         p.y %= Y;
         p.x %= X;

         bar += PointUnsign(p.x,p.y);

         if (bestField[PointUnsign(p.x,p.y)] != '*'){
            ++collapsedNumber;
         }

         bestField[PointUnsign(p.x,p.y)] = '*';
      }
      bar /= posAndSpeedByRobot.size();

      GraphP connections = buildConnectionsFromField(posAndSpeedByRobot2, bestField);

      GraphP::NodeSetRef connectedPoints = connections.getReachablesFrom(bestP).first;
      connectedPoints.insert(connections.refNode(bestP));
      treeRegionSize = connectedPoints.size();

      // find x-min, x-max, y-min, y-max from connectedPoints
      auto xMin = (*(std::min_element( connectedPoints.begin(), connectedPoints.end(),
                                       [](auto it1, auto it2){return it1->x < it2->x;})))->x;

      auto xMax = (*(std::max_element( connectedPoints.begin(), connectedPoints.end(),
                               [](auto it1, auto it2){return it1->x < it2->x;})))->x;

      auto yMin = (*(std::min_element( connectedPoints.begin(), connectedPoints.end(),
                               [](auto it1, auto it2){return it1->y < it2->y;})))->y;

      auto yMax = (*(std::max_element( connectedPoints.begin(), connectedPoints.end(),
                               [](auto it1, auto it2){return it1->y < it2->y;})))->y;

      std::vector<CoordUnsign> vDensity(X+Y,0U);
      for(const auto& [k_, p_v] : posAndSpeedByRobot){
         const auto& [p,v_] = p_v;

         auto d = manDistance(bar,p);
         vDensity[d]++;
      }

      constexpr unsigned VisualY = 50;
      auto itMax = std::max_element(vDensity.begin(), vDensity.end());
      auto max = *itMax;
      auto distForMax = std::distance(vDensity.begin(),itMax);
      if (max > VisualY){
         for(auto &d : vDensity){
            d /= (max / VisualY);
         }
      }

      {
         std::cout << bestField;
         std::cout << std::string(10,'\n');

         for(std::size_t k = 0; k < max; ++k){
            std::string line(vDensity.size(), ' ');
            for(std::size_t j = 0; j < vDensity.size(); ++j)
               if (vDensity[j] >= max-k) line[j] = '|';
            std::cout << line << std::endl;;
         }

         std::cout << std::string(10,'\n');
         std::cout << bestField.buildSubField(xMin-10, xMax+10, yMin-10, yMax+10);
      }
   }

   // TODO: check density computation... should be more with peaks within
   // low values -> ((xMax-xMin)+(yMax-yMin))/2

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "Number of robots: " << posAndSpeedByRobot.size() << std::endl;
   std::cout << "Number of collapsed positions when tree: " << collapsedNumber << std::endl;
   std::cout << "Number of robots for the tree: " << treeRegionSize << std::endl;
   std::cout << "\nResult P2: " << bestNumber << std::endl;
   std::cout << std::endl;

   return bestNumber;
}
