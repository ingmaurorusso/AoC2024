
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

using TCoordD = long double; // would work also with double (modfl -> modf)

using Equation = std::array<TCoordD,3>; //TODO: VectorN
struct System{
   Equation eq1;
   Equation eq2;
};

// prize multipliers
constexpr Equation eqMin{3,1,0}; // last 0 does not matter.

#define AGAINST_BUG

NAoc__MR::TResult day13Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult count{0U};
   TResult nTooManyPressures = 0U;
   TResult nChangedPrizeForTooManyPressures = 0U;
   TResult nPrizes = 0U;

   std::list<System> systems;
   System currSys;
   TResult nSystems = 0U;

   constexpr TCoordD Eps = 0.0000001;

   bool buttonA = false, buttonB = false;

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines) break;

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         if (buttonA || buttonB){
            std::cout << msgLine << "WARNING: empty line\n";
         }
         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---
      //  --- END LINE CHECKS ---

      //std::match_results<decltype(it)> match{}; same as smatch
      std::smatch matchLine{};
      
      if (std::regex_search(line.cbegin(), line.cend(), matchLine, std::regex{R"__((^.*?)\:(.*?)X.([\d]+)(.*?)Y.([\d]+).*)__"})){
         if (matchLine.size() < 6){
            throw std::invalid_argument(msgLine + "lack of title (before column ':') or numbers");
         }

         std::string_view title{matchLine[1].first, matchLine[1].second};

         std::string_view X{matchLine[3].first, matchLine[3].second};

         std::string_view Y{matchLine[5].first, matchLine[5].second};

         unsigned moveX;
         unsigned moveY;

         switch(std::from_chars(X.cbegin(), X.cend(), moveX).ec){
         case std::errc::result_out_of_range:
         case std::errc::invalid_argument:
            throw std::runtime_error(msgLine + "fix code");
         }

         switch(std::from_chars(Y.cbegin(), Y.cend(), moveY).ec){
         case std::errc::result_out_of_range:
         case std::errc::invalid_argument:
            throw std::runtime_error(msgLine + "fix code");
         }

         if (title == "Button A"){
            currSys.eq1[0] = moveX;
            currSys.eq2[0] = moveY;

            if (buttonA){
               throw std::invalid_argument(msgLine + "repeated button A part");
            }

            if (buttonB){
               std::cout << msgLine << "WARNING: button A after button B\n";
            }
            buttonA = true;
         } else
         if (title == "Button B"){
            currSys.eq1[1] = moveX;
            currSys.eq2[1] = moveY;

            if (buttonB){
               throw std::invalid_argument(msgLine + "repeated button B part");
            }
            buttonB = true;
         } else{
            constexpr TCoordD ADD_PRIZE_COORD = 10000000000000.;

            if (!(buttonA && buttonB)){
               throw std::invalid_argument(msgLine + "some button part missing");
            }
            buttonA = false;
            buttonB = false;

            currSys.eq1[2] = moveX + ADD_PRIZE_COORD;
            currSys.eq2[2] = moveY + ADD_PRIZE_COORD;

#ifdef AGAINST_BUG
            auto sys = currSys;
#endif

#ifndef AGAINST_BUG
            // buggy if no-move with long double
            // 1) calls its own initilizer when construct_at
            //    in stl_construct.h has the instruction
            //    with the new placement. Even if we define
            //    our own constructors for System.
            // 2) copies only eq1 with 'long double', whereas
            //    with 'double' it works correctly.
            systems.push_back(std::move(currSys));
            // 3) even with move, some other manifestation
            // of problems (in other isntructions) during
            // the memory management (corrupted list of memory areas).
         }
      }
   }

   for(auto sys : systems){
#endif

   constexpr unsigned MAX_PRESSURES = 0U; //unlimited
   constexpr TCoordD ROUND = 0.0001;

   using PointD = Point<TCoordD>;

   const auto checkPressNumber = [](PointD p){
      if constexpr(MAX_PRESSURES > 0){
         return (p.x <= MAX_PRESSURES + ROUND) && (p.y <= MAX_PRESSURES + ROUND);
      } //else:
         return true;
   };

      ++nSystems;
      SquareMatrix<2> matrix2 { {sys.eq1[0], sys.eq1[1]}, {sys.eq2[0], sys.eq2[1]} };
      Matrix<2,1> prize {{sys.eq1[2]}, {sys.eq2[2]}};

      auto prizeRow = prize.buildTranspost()[0];

      TResult maxSys = std::max(sys.eq1[2], sys.eq2[2]);

      if ( (sys.eq1[2] < std::max(sys.eq1[0],sys.eq1[1])) ||
           (sys.eq2[2] < std::max(sys.eq2[0],sys.eq2[1])) ){
         continue;
      }

      PointD px0{0.,0}, py0{0, 0.}, pInters;

      auto y1 = (sys.eq1[1] == 0)? maxSys : sys.eq1[2]/sys.eq1[1];
      auto y2 = (sys.eq2[1] == 0)? maxSys : sys.eq2[2]/sys.eq2[1];

      auto x1 = (sys.eq1[0] == 0)? maxSys : sys.eq1[2]/sys.eq1[0];
      auto x2 = (sys.eq2[0] == 0)? maxSys : sys.eq2[2]/sys.eq2[0];

      px0.y = std::min(y1,y2);

      py0.x = std::min(x1,x2);

      TCoordD matrix[2][2] = {{sys.eq1[0], sys.eq1[1]}, {sys.eq2[0], sys.eq2[1]}};
      TCoordD C[2] = {sys.eq1[2], sys.eq2[2]};

      auto [inv2, det2] = matrix2.buildInverse();

      TResult Q;

      bool foundPoint = false;
      PointD bestPoint;

      TCoordD det = matrix[0][0]*matrix[1][1] - matrix[1][0]*matrix[0][1];

      if (std::fabs(det-det2) > 0.5){
         throw std::runtime_error("Error!");
      }

      if (std::abs(det) > 0.5){
         auto solSys2 = (inv2 * prize).buildTranspost()[0];

         //pInters = PointD(solSys2); -> uses a method that is ICE for gcc on problem 6
         pInters = PointD(solSys2[0],solSys2[1]);

         // pInters is the only point that solves the system.
         // If it is integer and positive, well.

         TCoordD integerPart;
         if ( (pInters.x >= 0) && (pInters.y >= 0) &&
              (modfl(pInters.x+ROUND/2.,&integerPart) < ROUND) && (modfl(pInters.y+ROUND/2.,&integerPart) < ROUND) ){
            if (checkPressNumber(pInters)){
               bestPoint = pInters;
               foundPoint = true;
            } else{
               ++nTooManyPressures;
            }
         }
      } else{
         // the two boundaries do not intersect each other: do not
         // consider pInters and just consider the edge between Px0 and Py0.
         // However, if the two constraints-edge are only parallel but
         // not corresponding, then there is no solution.

         // included in the lcm check
         //if (det > std::numeric_limits<TResult>::max()){
         //   throw std::runtime_error("Cannot compute lcm for too high numbers");
         //}

         // check the system range
         TCoordD det1 = C[0]*matrix[1][1] - C[1]*matrix[0][1];
         TCoordD det2 = matrix[0][0]*C[1] - matrix[1][0]*C[0];

         if ((std::abs(det1) > 0.5) || (std::abs(det2) > 0.5)){
            continue; // no solution.
         }

         // the two equations are equivalent.
         // Let's look on any integer point along the edge.

         // In general, solve a simplex.

         {
            auto d = TResult(det+ROUND);

            Q = d;
            for(int i = 0; i < 2; ++i)
               for(int j = 0; j < 2; ++j)
                  Q = std::lcm(Q, TResult(matrix[i][j]+ROUND));

            bool bad = (Q%d != 0);
            for(int i = 0; i < 2; ++i)
               for(int j = 0; j < 2; ++j)
                  bad |= (Q % TResult(matrix[i][j]+ROUND) != 0);
            
            if (bad){
               throw std::runtime_error("Cannot compute lcm for too high numbers");
            }
         }

         /* unused
         TCoordD lb = std::min( scalarProd(eqMin,px0), scalarProd(eqMin,py0) );
         if (useInter){
            lb = std::min( lb, scalarProd(eqMin,pInters) );
         }*/
      
         auto ph = px0; // highest
         auto pr = py0; // rightest

         // search on the edge between ph and pr
         auto diffX = pr.x - ph.x;
         auto diffY = ph.y - pr.y;

         if (diffX+diffY < ROUND){
            continue; // (0,0) is the only point.
         }

         bool StartX0 = (diffX > ROUND);

         // search for the first integer point.
         // Start from one of the extremes that is on an axis.
         PointD p_integ = StartX0 ? ph : pr;

         // The move along dX, -dY (or -dX, dY)
         // needs, for example, dX integer if start from ph,
         // therefore |dY| = dX*f (f = diffY/diffX) min must
         // make the other coordinate C also integer:
         // that is, C - dX*f must be integer: now,
         // C and f are multiple of the same minimum 1/Q
         // where Q is lcm('det',matrix_coefficients).
         // Therefore, after multiplying by Q the fractional
         // parts of C and f, they become integer and we can
         // find the gcd of the two numbers, let's say
         // C0 and f0 -> now, within Q/gcd times f0 is equivalent
         // to C0 modulo-(Q/gcd) TODO: look for a known algotihm
         // to know C1, I have used one (see the code).
         // After C1, the periodicy is by N where N is the
         // minimum that makes N*f integer.
         // Given f = P/Q, it is enough to compute
         // gcd(P,Q) and consider f = [P/gcd] / [Q/gcd] -> N = Q/gcd.
         TCoordD c = StartX0 ? ph.y : pr.x;
         TCoordD f = StartX0 ? diffY/diffX : (diffX/diffY);

         TCoordD intC;
         auto fracC = modfl(c, &intC);

         TCoordD intF;
         auto fracF = modfl(f, &intF);

         TResult P = TResult(fracF*Q + ROUND);
         auto gcdPQ = std::gcd(P,Q);
         auto N = Q/gcdPQ;

         TResult cI = TResult(fracC*Q + ROUND);
         TResult fI = TResult(fracF*Q + ROUND);

         TResult C1{};
         if (fI == 0){
            if (cI > 0){
               // continue: no first point exists... maybe not possible
               // ... or maybe possible as need some mixed (positive-negative)
               // couple of values...
               continue;
            }
            // p_integ has already integer coordinates.
            C1 = 0U;
         } else{
            auto gcdCF = std::gcd(std::gcd(cI, fI), Q);
            auto Q_over_gcd = Q/gcdCF;

            if ((cI%gcdCF != 0) || (fI%gcdCF != 0) || (Q%gcdCF != 0)){
               throw std::runtime_error("Fix the code!");
            }

            cI /= gcdCF;
            fI /= gcdCF;

            auto howMany = Q_over_gcd/fI;

            auto r = Q_over_gcd - fI*howMany;

            auto check = fI;
            TResult steps = 0U;
            while (check != cI){
               // first try to reach or overcome cI.
               if (check < cI){
                  auto deltaSteps = 1 + (cI - check -1)/fI;
                  steps += deltaSteps;

                  check = (check + deltaSteps*fI) % Q_over_gcd;
               }
               if (check > cI){
                  // apply Q_over_gcd/fI steps and go back by r.
                  steps += howMany;
                  check -= r;
                  //TODO: speed-up with a certain number of (howMany+1)
                  //to advance that number of (check-r).
               }
            }

            C1 = steps+1U;
         }

         if (StartX0){
            p_integ.y -= (C1*f);
            p_integ.x += C1;
         } else{
            p_integ.x -= (C1*f);
            p_integ.y += C1;
         }

         // p_integ is now the first point.
         TResult nTokens = maxSys+1U; // as upper-bound.
         TResult nTokensBest = maxSys+1U; // including discarded ones.
         
         bool bestDiscardedByTooManyPressures = false;

         while ( StartX0 ? (p_integ.y >= pr.y-ROUND)
                           : (p_integ.x >= ph.x-ROUND) ){
            TResult nTokensCand = TResult(scalarProd(p_integ,eqMin) + ROUND);
            auto okNumPress = !checkPressNumber(p_integ);
            if ((nTokensCand < nTokens) && okNumPress){
               nTokens = nTokensCand;
               bestPoint = p_integ;
               foundPoint = true;
            }
            if (nTokensCand < nTokensBest){
               nTokensBest = nTokensCand;
               bestDiscardedByTooManyPressures = !okNumPress;
            }

            if (StartX0){
               p_integ.y -= (N*diffY/diffX);
               p_integ.x += N;
            } else{
               p_integ.x -= (N*diffX/diffY);
               p_integ.y += N;
            }
         }

         if (bestDiscardedByTooManyPressures){
            ++nChangedPrizeForTooManyPressures;
         }
      }

      if (foundPoint){
         auto deltaPrice = scalarProd(bestPoint,eqMin) + ROUND;
         if (deltaPrice > std::numeric_limits<TResult>::max()){
            throw std::runtime_error("TResult to be higher");
         }

         if (count > std::numeric_limits<TResult>::max() - deltaPrice){
            throw std::runtime_error("TResult to be higher");
         }

         count += TResult(deltaPrice);
         ++nPrizes;
      }
   }

#ifdef AGAINST_BUG
         }
      }
#endif

   if (buttonA || buttonB){
      throw std::invalid_argument("Missing prize part at the end");
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "Number of systems: " << nSystems << std::endl;
   std::cout << "Number of prizes: " << nPrizes << std::endl;
   std::cout << "Number of prizes discarded for too many pressues: " << nTooManyPressures << std::endl;
   std::cout << "Number of prizes changed for too many pressues: " << nChangedPrizeForTooManyPressures << std::endl;
   std::cout << "\nResult P2: " << count << std::endl;
   std::cout << std::endl;

   return count;
}
