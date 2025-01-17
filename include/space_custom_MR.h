#pragma once

#include <algorithm> // for std::swap before C++11
#include <array>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <utility> // for std::swap since C++11

#include "concept_custom_MR.h"

#include "enum_custom_MR.h"

#include "graph_custom_MR.h"

#include "hash_custom_MR.h"

namespace NSpace__MR{

    enum class Direction2 : unsigned { Front, Back };
    using Dir2Under = std::underlying_type_t<Direction2>;
    constexpr unsigned TwoDir2 = 2U;
    using namespace NEnum__MR;
    MR__ENUM_MAP_BEGIN_EXPAND(Direction2)(Front)MR__ENUM_MAP_END_EXPAND(Back);

    inline auto checkDir2ToUnder(Direction2 d, bool throwIfInconsistent){
        //using std::literals::string_literals::operator""s;
        return NEnum__MR::checkEnumToUnderline(d,TwoDir2, throwIfInconsistent? "Not-a-direciton" : nullptr);
    }

    inline std::string dir2ToStr(Direction2 d, bool throwIfInconsistent)
    {
        const auto [val, ok] = checkDir2ToUnder(d, throwIfInconsistent);
        return ok? std::string{MR__ENUM_GET_NAMEq(Direction2,d)} : "None";
    }


    enum class Direction4 : unsigned { Right, Down, Left, Up };
    using Dir4Under = std::underlying_type_t<Direction4>;
    constexpr Dir4Under FourDir4 = 4U;
    constexpr std::string Arrows4{">v<^"}; // same order as values of Direction4.
    using namespace NEnum__MR;
    MR__ENUM_MAP_BEGIN_EXPAND(Direction4)(Right)(Down)(Left)MR__ENUM_MAP_END_EXPAND(Up);

    constexpr std::string Mirrors{R"(-|/\)"};

    inline auto checkDir4ToUnder(Direction4 d, bool throwIfInconsistent){
        //using std::literals::string_literals::operator""s;
        return NEnum__MR::checkEnumToUnderline(d,FourDir4, throwIfInconsistent? "Not-a-direciton" : nullptr);
    }

    inline bool dir4IsVertical(Direction4 d, bool throwIfInconsistent = true){
        checkDir4ToUnder(d, throwIfInconsistent);
        using enum Direction4;
        return (d == Up) || (d == Down);
    }

    inline bool dir4IsHorizontal(Direction4 d, bool throwIfInconsistent = true){
        checkDir4ToUnder(d, throwIfInconsistent);
        using enum Direction4;
        return (d == Left) || (d == Right);
    }

    // false in case on non-consistent initial value of d.
    inline bool dir4rotate(Direction4 &d, bool clockwise, unsigned n = 1U, bool throwIfInconsistent = true){
        const auto [val, ok] = checkDir4ToUnder(d, throwIfInconsistent);
        if (!ok) return false;

        d = Direction4{(val + n*(clockwise? 1 : (FourDir4-1))) % FourDir4};
        return true;
    }

    // false in case on non-consistent initial value of d.
    inline bool dir4Invert(Direction4 &d, bool throwIfInconsistent = true){
        return dir4rotate(d,true,2);
    }

    // false in case on non-consistent value for 'd' or 'mirror'
    inline bool dir4mirror(Direction4 &d, char mirror, bool revertYfor2dField = true, bool throwIfInconsistent = true){
        const auto [val, ok] = checkDir4ToUnder(d, throwIfInconsistent);
        if (!ok) return false;

        if(auto pm = Mirrors.find(mirror); pm == std::string::npos){
            if (throwIfInconsistent){
                using namespace std::literals::string_literals;
                throw std::runtime_error("Inconsistent direction4-mirror: "s + std::string(1,mirror));
            } //else:
            return false;
        }

        bool clockwise;
        unsigned steps{1};
        using enum Direction4;
        switch(d){
        case Right :
        case Left :
            switch(mirror){
            case '-': return true; // direction unchanged
            case '|':
                steps = 2;
                break;
            case '/':
                clockwise = false;
                break;
            //case '\':
            default:
                clockwise = true;
            }
        // case Up :
        // case Down :
        default :
            switch(mirror){
            case '|': return true; // direction unchanged
            case '-':
                steps = 2;
                break;
            case '/':
                clockwise = true;
                break;
            //case '\':
            default:
                clockwise = false;
            }
        }

        if (revertYfor2dField) clockwise = !clockwise;

        static_cast<void>(dir4rotate(d, clockwise, steps, false)); // true for sure

        return true;
    }

    inline std::string dir4ToStr(Direction4 d, bool throwIfInconsistent = true)
    {
        const auto [val, ok] = checkDir4ToUnder(d, throwIfInconsistent);
        return ok ? std::string{MR__ENUM_GET_NAMEq(Direction4,d)} : "None";
    }

    // false if inconsistent char
    inline std::pair<char, bool> dir4ToArrow(Direction4 dir, bool revertYfor2dField = true, bool throwIfInconsistent = true)
    {
        using enum Direction4;
        if (revertYfor2dField && ((dir == Up) || (dir == Down))){
            dir = (dir == Up)? Down : Up;
        }
        const auto [val, ok] = checkDir4ToUnder(dir, throwIfInconsistent);
        
        return {ok ? Arrows4[val] : '*', ok};
    }

    // false if inconsistent direction
    inline std::pair<Direction4, bool> dir4FromArrow(char dc, bool revertYfor2dField = true, bool throwIfInconsistent = true)
    {
        using enum Direction4;
        if (auto pos = Arrows4.find(dc); pos != std::string::npos){
            auto dir{Direction4{static_cast<std::underlying_type_t<Direction4>>(pos)}};
            if (revertYfor2dField && ((dir == Up) || (dir == Down))){
                dir = (dir == Up)? Down : Up;
            }
            return {dir,true};
        }
        
        if (throwIfInconsistent){
            using namespace std::literals::string_literals;
            throw std::runtime_error("Inconsistent direction4-arrow: "s + std::string(1,dc));
        } // else:

        return {Direction4{0},false};
    }

    using CoordUnsign = std::size_t;
    using CoordSign = long int;

    template<typename Coord = CoordUnsign>
    struct Point {
        Coord x; Coord y;

        Point& operator+=(const Point p2){x += p2.x; y += p2.y; return *this;}

        Point& operator-=(const Point p2){x -= p2.x; y -= p2.y; return *this;}

        Point& operator*=(const Coord c){x *= c; y *= c; return *this;}

        Point& operator/=(const Coord c){x /= c; y /= c; return *this;}

        Coord operator[](unsigned idx) const {return (idx == 0)? x : y;}

        Coord& operator[](unsigned idx) {return (idx == 0)? x : y;}

        constexpr std::size_t size() const {return 2U;}

        Point() = default;

        template<std::convertible_to<Coord> T1, std::convertible_to<Coord> T2>
        explicit Point(const T1 &vx, const T2 &vy) : x(vx), y(vy){}

        /* ICE on gcc for the expression *(std::begin(iter)) in requires clause.
           -> https://gcc.gnu.org/bugzilla/show_bug.cgi?id=118060
           only in problem 6, whereas it would work (and be correctly used) in problem 13 */
        /*template<NConcept__MR::Iterable Iter>
        requires requires (Iter iter){ {*(std::begin(iter))} -> std::convertible_to<Coord>; }
        Point(const Iter& iter) : Point(){
            auto it = std::begin(iter);
            if (it != std::end(iter)){
                x = *it;
                it = std::next(it);
                if (it != std::end(iter)){
                    y = *it;
                }
            }
        }*/

        //template<std::convertible_to<Coord> T>
        //Point(std::initializer_list<T> l) : Point(){
        // TODO: understand why templatic ruins stuff.
        Point(std::initializer_list<Coord> l) : Point(){
            auto it = std::begin(l);
            if (it != std::end(l)){
                x = *it;
                it = std::next(it);
                if (it != std::end(l)){
                    y = *it;
                }
            }
        }

        friend void swap(Point& lhs, Point& rhs) {
            std::swap(lhs.x,rhs.x);
            std::swap(lhs.y,rhs.y);
        }
    };
    // TODO: manage spaceship operator

    template<typename Coord = CoordUnsign>
    using Vector = Point<Coord>;

    using PointUnsign = Point<CoordUnsign>;
    using VectorUnsign = PointUnsign;

    using PointSign = Point<CoordSign>;
    using VectorSign = PointSign;

    template<typename Coord>
    bool operator==(const Point<Coord>& p1, const Point<Coord>& p2)
    { return (p1.x == p2.x) && (p1.y == p2.y); }

    template<typename Coord>
    bool operator<(const Point<Coord>& p1, const Point<Coord>& p2)
    {
        if (p1.y != p2.y) { return (p1.y < p2.y); }
        return (p1.x < p2.x);
    }

    template<typename Coord>
    bool operator<=(const Point<Coord>& p1, const Point<Coord>& p2)
    { return (p1==p2) || (p1 < p2); }

    template<typename Coord>
    bool operator!=(const Point<Coord>& v1, const Point<Coord>& v2)
    { return !(v1 == v2); }

    template<typename Coord>
    std::string pointToStr(Point<Coord> p) {
        using std::literals::string_literals::operator""s;
        return "("s + std::to_string(p.x) + ", " + std::to_string(p.y) + ')';
    }


    class Field2D final{
    public:
        static constexpr char FreeTile = '.';
        static constexpr char WallTile = '#';
        static constexpr char StoneTile = 'O';
        static constexpr char StartTile = 'S';
        static constexpr char EndTile = 'E';
        inline static std::string allKnownTiles = NString__MR::getStringWithChars({FreeTile,WallTile,StoneTile,StartTile,EndTile});

        using Field2DException = std::string;

        Field2D(std::size_t X, std::size_t Y, char ch = ' '){
            lines.resize(Y, std::string(X, ch));
        }

        Field2D(std::vector<std::string> lines_){
            if (lines_.empty()){
                // dimX would be undefined.
                throw Field2DException{"no lines"};
            }

            if (!std::all_of(++lines_.cbegin(), lines_.cend(),
                            [size0 = lines_[0].size()] (const auto &line){
                return line.size() == size0;
            })){
                throw Field2DException{"lines with different size"};
            }

            lines = std::move(lines_);
        }

        Field2D buildSubField( CoordUnsign beginX, CoordUnsign endX,
                               CoordUnsign beginY, CoordUnsign endY ){
            raiseIfOutOfBoundary({beginX,beginY});
            if ((endX <= beginX) || (endY <= beginY)){ // => ends are positive
                throw std::runtime_error("Empty sub-build");
            }
            raiseIfOutOfBoundary({endX-1U,endY-1U});

            Field2D res(endX - beginX, endY - beginY);

            CoordUnsign j = 0;
            while(beginY < endY){
                res.lines[j++] = lines[beginY].substr(beginX,endX);
                ++beginY;
            }

            return res;
        }

        char& operator[](PointUnsign p) {
            raiseIfOutOfBoundary(p);
            return lines[p.y][p.x];
        }
        char operator[](PointUnsign p) const{
            raiseIfOutOfBoundary(p);
            return lines[p.y][p.x];
        }

        std::string& operator[](CoordUnsign y) {
            raiseIfOutOfBoundary({0,y});
            return lines[y];
        }
        std::string operator[](CoordUnsign y) const{
            raiseIfOutOfBoundary({0,y});
            return lines[y];
        }

        // need C++23 to use []
        char& operator()(CoordUnsign x, CoordUnsign y){return lines[y][x];}
        char operator()(CoordUnsign x, CoordUnsign y) const{return lines[y][x];}

        CoordUnsign dimX() const noexcept {return lines[0].size();}

        CoordUnsign dimY() const noexcept {return lines.size();}

        bool inBoundary(PointUnsign p) const noexcept{
            return (p.x < dimX()) && (p.y < dimY());
        }

        // true in case of steps==0U
        // false if only a part of the steps may be done -> then doAlsoPartial
        // regulates whether doing partial or nothing.
        bool movePoint(PointUnsign &p, Direction4 d, std::size_t steps = 1U, bool doAlsoPartial = false) const{
            if (toUnderlying(d) >= FourDir4){
                using namespace std::literals::string_literals;
                throw Field2DException{"unexpected direction: "s + std::to_string(toUnderlying(d))};
            }

            if (steps == 0U) return true;

            using enum Direction4;
            switch(d){
            case Down :
                if (p.y < steps){
                    if (doAlsoPartial) p.y = 0U;
                    return false;
                }
                p.y -= steps; break;
            case Up :
                if (steps > dimY()) steps = dimY();
                if (p.y >= dimY()-steps){
                    if (doAlsoPartial) p.y = dimY()-1U;
                    return false;
                }
                p.y += steps; break;
            case Left :
                if (p.x < steps){
                    if (doAlsoPartial) p.x = 0U;
                    return false;
                }
                p.x -= steps; break;
            // case Right :
            default :
                if (steps > dimX()) steps = dimX();
                if (p.x >= dimX()-steps){
                    if (doAlsoPartial) p.x = dimX()-1U;
                    return false;
                }
                p.x += steps; break;
            }

            return true;
        }

        std::size_t moveShapes( PointUnsign &startPoint, Direction4 d, 
                                std::vector<Field2D> shapesToMove, const std::string& walls,
                                std::size_t steps = 1U, bool doAlsoPartial = false ) const{
            // TODO: implement and use in problem AOC-2024-15-P2
            // Consider the cases with multiple matches due to adjacent shapes.
            return 0U;
        }

        std::pair<PointUnsign,bool> find_first(char ch){
            for(std::size_t y = 0; y < dimY(); ++y){
                if(auto x = lines[y].find(ch); x != std::string::npos){
                    return {{x,y}, true};
                }
            }
            return {{0,0}, false};
        }

        using PointAndDir = std::pair<PointUnsign,Direction4>;

        template<NConcept__MR::SummAndOrdinable EdgeWeight>
        using GraphPDdirect = NGraph__MR::Graph<PointAndDir, NGraph__MR::GraphKind::Direct, EdgeWeight>;

        template<NConcept__MR::SummAndOrdinable EdgeWeight>
        using GraphPDedge = typename GraphPDdirect<EdgeWeight>::Edge;

        template<NConcept__MR::SummAndOrdinable EdgeWeight>
        using GraphPindirect = NGraph__MR::Graph<PointUnsign, NGraph__MR::GraphKind::Indirect, EdgeWeight>;

        template<NConcept__MR::SummAndOrdinable EdgeWeight>
        using GraphPedge = typename GraphPindirect<EdgeWeight>::Edge;

        // works also for indirect, an edge is still a couple {point, point}
        inline static auto constOneForAnyEdge = []<NConcept__MR::SummAndOrdinable EdgeWeight>
                                                (const GraphPDedge<EdgeWeight>& edge)
                                                {return 1U;};

        template< NConcept__MR::SummAndOrdinable EdgeWeight = std::size_t,
                  NConcept__MR::Callable< EdgeWeight, GraphPDedge<EdgeWeight> >
                    EdgeEvaluator = std::function<EdgeWeight(GraphPDedge<EdgeWeight>)> >
        // destinations that match endsFilterTake are ok.
        // destination that match endsFilterCondition, instead, are taken only if the further move on the
        // associated direction matches afterEndsFilter.
        // PointAndDir in the graph represents, for each point, the 'exit' direction. Destination node may have
        // a rotated direction with respect to the starting node, but the destination-point is the one reached
        // from the starting-point according to the starting-direction.
        GraphPDdirect<EdgeWeight> getConnectionGraphWithExitDir(
                                    std::string_view startsFilterTake,
                                    std::string_view startsFilterConditional, std::string_view beforeStartsFilter,
                                    std::string_view endsFilterTake,
                                    std::string_view endsFilterConditional, std::string_view afterEndsFilter,
                                    EdgeEvaluator&& edgeEvaluator = EdgeEvaluator(constOneForAnyEdge) ) const{
            GraphPDdirect<EdgeWeight> graph;

            for(std::size_t i = 0; i < dimX(); ++i){
                for(std::size_t j = 0; j < dimY(); ++j){
                    const auto p = PointUnsign{i,j};

                    const bool okStartSoon = (startsFilterTake.find((*this)[p]) != std::string_view::npos);
                    bool okStartIfCondition = (startsFilterConditional.find((*this)[p]) != std::string_view::npos);

                    if (okStartSoon || okStartIfCondition){
                        for(Dir4Under d2 = 0; d2 < FourDir4; ++d2){
                            const auto dir2 = Direction4{d2};
                            auto p2 = p;

                            if (movePoint(p2,dir2)){ // dir2 is the direction that leads to p2.
                                const bool okEndSoon = (endsFilterTake.find((*this)[p2]) != std::string_view::npos);
                                bool okEndIfCondition = (endsFilterConditional.find((*this)[p2]) != std::string_view::npos);
                                if (okEndSoon || okEndIfCondition){
                                    for(Dir4Under d1 = 0; d1 < FourDir4; ++d1){
                                        auto dir1 = Direction4(d1);
                                        bool sameAxis = (dir4IsVertical(dir1) == dir4IsVertical(dir2));
                                        if ((d1 == d2) || !sameAxis){ // => sameAxis == (d1 == d2)
                                            auto edge = typename GraphPDdirect<EdgeWeight>::Edge{PointAndDir{p,dir1}, PointAndDir{p2,dir2}};

                                            bool okEdgeStart = okStartSoon;
                                            if (okStartIfCondition && !okStartSoon){
                                                // try to move further, along dir2.
                                                auto p0 = p;
                                                auto dir0 = dir1;
                                                dir4rotate(dir0, true, 2U);
                                                if (movePoint(p0,dir0)){
                                                    okEdgeStart = (beforeStartsFilter.find((*this)[p0]) != std::string_view::npos);
                                                }
                                            }

                                            bool okEdgeEnd = okEndSoon;
                                            if (okEndIfCondition && !okEndSoon){
                                                // try to move further, along dir2.
                                                auto p3 = p2;
                                                if (movePoint(p3,dir2)){
                                                    okEdgeEnd = (afterEndsFilter.find((*this)[p3]) != std::string_view::npos);
                                                }
                                            }

                                            if (okEdgeStart && okEdgeEnd){
                                                graph.addEdge(edge, edgeEvaluator(edge));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return graph;
        }

        template< NConcept__MR::SummAndOrdinable EdgeWeight = std::size_t,
                   NConcept__MR::Callable< EdgeWeight, GraphPedge<EdgeWeight> > EdgeEvaluator
                   = std::function<EdgeWeight(GraphPedge<EdgeWeight>)> >
        GraphPindirect<EdgeWeight> getConnectionGraphByPoint(
                                    std::string_view nodeFilter,
                                    EdgeEvaluator&& edgeEvaluator = EdgeEvaluator(constOneForAnyEdge) ) const{
            GraphPindirect<EdgeWeight> graph;

            for(std::size_t i = 0; i < dimX(); ++i){
                for(std::size_t j = 0; j < dimY(); ++j){
                    const auto p = PointUnsign{i,j};

                    if (nodeFilter.find((*this)[p]) != std::string_view::npos){
                        for(Dir4Under d = 0; d < FourDir4/2U; ++d){
                            auto p2 = p;
                            if (movePoint(p2,Direction4{d})){
                                if (nodeFilter.find((*this)[p]) != std::string_view::npos){
                                    const auto edge = GraphPedge<EdgeWeight>{p,p2};
                                    graph.addEdge(edge, edgeEvaluator(edge));
                                }
                            }
                        }
                    }
                }
            }

            return graph;
        }

        // works also for direct, an edge is still a couple {point, point}
        inline static auto OkOneForAnyEdge = []<NConcept__MR::SummAndOrdinable EdgeWeight>
                                             (const PointUnsign& pSource, const PointUnsign& pDest)
                                             {return std::make_pair(true,1U);};

        
        inline static auto OkAnyNode = [](const PointUnsign& point){return true;};

        template< NGraph__MR::GraphKind Kind, NConcept__MR::SummAndOrdinable EdgeWeight = std::size_t,
                  NConcept__MR::Callable< std::pair<bool,EdgeWeight>, PointUnsign, PointUnsign > 
                    EdgeFilterAndEvaluator = std::function<std::pair<bool,EdgeWeight>(PointUnsign,PointUnsign)>,
                  NConcept__MR::Callable< bool, PointUnsign > IsolatedNodeFilter = std::function<bool(PointUnsign)> >
        NGraph__MR::Graph<PointUnsign, Kind, EdgeWeight> getConnectionGraphByEdge(
            EdgeFilterAndEvaluator&& edgeFilterAndEvaluator = EdgeFilterAndEvaluator(OkOneForAnyEdge),
            IsolatedNodeFilter&& isolatedNodeFilter = IsolatedNodeFilter(OkAnyNode) ) const
        {
            NGraph__MR::Graph<PointUnsign, Kind, EdgeWeight> graph;

            for(std::size_t i = 0; i < dimX(); ++i){
                for(std::size_t j = 0; j < dimY(); ++j){
                    const auto p = PointUnsign{i,j};

                    bool isolated = true;
                    constexpr auto FactorDir = (Kind == NGraph__MR::GraphKind::Indirect) ? 2U : 1U;
                    for(Dir4Under d = 0; d < FourDir4/FactorDir; ++d){
                        auto p2 = p;
                        if (movePoint(p2,Direction4{d})){
                            auto [ok, edgeWeight] = edgeFilterAndEvaluator(p,p2);
                            if (ok){
                                graph.addEdge(p, p2, edgeWeight);
                                isolated = false;
                            }
                        }
                    }

                    if (isolated && isolatedNodeFilter(p)){
                        graph << p;
                    }
                }
            }

            return graph;
        }

    private:
        std::vector<std::string> lines;

        void raiseIfOutOfBoundary(PointUnsign p) const{
            if (!inBoundary(p)){
                throw std::runtime_error("Point out of boundaries");
            }
        }

        friend class Field3D;
    };  
        
    static void moveUnlimitedPoint(PointSign &p, Direction4 d){
        using Field2DException = std::string;
        if (toUnderlying(d) >= FourDir4){
            using namespace std::literals::string_literals;
            throw Field2DException{"unexpected direction: "s + std::to_string(toUnderlying(d))};
        }

        using enum Direction4;
        switch(d){
        case Down :
            --p.y; break;
        case Up :
            ++p.y; break;
        case Left :
            --p.x; break;
        // case Right :
        default :
            ++p.x;
        }
    }


    class Field2DUnlimited final{
    public:
        using Field2DException = std::string;

        Field2DUnlimited(std::vector<std::string> lines_){
            if (lines_.empty()){
                // dimX would be undefined.
                throw Field2DException{"no lines"};
            }

            if (lines_.size() > 1U){
                if (!std::all_of(++lines_.cbegin(), lines_.cend(),
                                [size0 = lines_[0].size()] (const auto &line){
                    return line.size() == size0;
                })){
                    throw Field2DException{"lines with different size"};
                }
            }

            lines = std::move(lines_);
        }

        char& operator[](PointSign p) {
            raiseIfOutOfBoundary(p);
            return lines[p.y][p.x];
        }
        char operator[](PointSign p) const{
            raiseIfOutOfBoundary(p);
            return lines[p.y][p.x];
        }

        // need C++23 to use []
        char& operator()(CoordSign x, CoordSign y){return operator[]({x,y});}
        char operator()(CoordSign x, CoordSign y) const{return operator[]({x,y});}

        CoordSign dimX() const noexcept {return lines[0].size();}

        CoordSign dimY() const noexcept {return lines.size();}

        bool inBoundary(PointSign p) const noexcept{
            return (p.x >= 0) && (p.x < dimX()) &&
                   (p.y >= 0) && (p.y < dimY());
        }

        bool movePoint(PointSign &p, Direction4 d) const{
            if (toUnderlying(d) >= FourDir4){
                using namespace std::literals::string_literals;
                throw Field2DException{"unexpected direction: "s + std::to_string(toUnderlying(d))};
            }

            using enum Direction4;
            switch(d){
            case Down :
                if (p.y == 0U) return false;
                --p.y; break;
            case Up :
                if (p.y+1U >= dimY()) return false;
                ++p.y; break;
            case Left :
                if (p.x == 0U) return false;
                --p.x; break;
            // case Right :
            default :
                if (p.x+1U >= dimX()) return false;
                ++p.x;
            }

            return true;
        }

    private:
        std::vector<std::string> lines;

        void raiseIfOutOfBoundary(PointSign p) const{
            if (!inBoundary(p)){
                throw std::runtime_error("Point out of boundaries");
            }
        }
    };

    template<typename Coord = CoordUnsign>
    struct Point3 final {
        Coord x; Coord y; Coord z;

        Point3& operator+=(const Point3 p2){x += p2.x; y += p2.y; z += p2.z; return *this;}

        Point3& operator-=(const Point3 p2){x -= p2.x; y -= p2.y; z -= p2.z; return *this;}

        Point3& operator*=(const Coord c){x *= c; y *= c; z *= c; return *this;}

        Point3& operator/=(const Coord c){x /= c; y /= c; z /= c; return *this;}

        Coord operator[](unsigned idx) const {return (idx == 0)? x : ((idx == 1)? y : z);}

        Coord& operator[](unsigned idx) {return (idx == 0)? x : ((idx == 1)? y : z);}

        constexpr std::size_t size() const {return 3U;}
    };
    // TODO: manage spaceship operator

    using Point3Unsign = Point3<CoordUnsign>;
    using Point3Sign = Point3<CoordSign>;

    template<typename Coord>
    bool operator==(const Point3<Coord>& p1, const Point3<Coord>& p2)
    { return (p1.x == p2.x) && (p1.y == p2.y) && (p1.z == p2.z); }

    template<typename Coord>
    bool operator<(const Point3<Coord>& p1, const Point3<Coord>& p2)
    {
        if (p1.z != p2.z) { return (p1.z < p2.z); }
        if (p1.y != p2.y) { return (p1.y < p2.y); }
        return (p1.x < p2.x);
    }

    template<typename Coord>
    bool operator!=(const Point3<Coord>& p1, const Point3<Coord>& p2)
    { return !(p1 == p2); }

    template<typename Coord>
    std::string point3ToStr(const Point3<Coord>& p) {
        using std::literals::string_literals::operator""s;
        return "("s + std::to_string(p.x) + ", " + std::to_string(p.y) + ", " + std::to_string(p.z) + ')';
    }


    enum class Direction6 : unsigned { Right, Down, Below, Left, Up, Above };
    using Dir6Under = std::underlying_type_t<Direction6>;
    constexpr Dir6Under SixDir6 = 6U;
    MR__ENUM_MAP_BEGIN_EXPAND(Direction6)(Right)(Down)(Left)(Up)(Above)MR__ENUM_MAP_END_EXPAND(Below);

    inline auto checkDir6ToUnder(Direction6 d, bool throwIfInconsistent){
        //using std::literals::string_literals::operator""s;
        return NEnum__MR::checkEnumToUnderline(d, SixDir6, throwIfInconsistent? "Not-a-direciton3" : nullptr);
    }

    inline bool dir6IsOnX(Direction6 d, bool throwIfInconsistent = true){
        checkDir6ToUnder(d, throwIfInconsistent);
        using enum Direction6;
        return (d == Left) || (d == Right);
    }

    inline bool dir6IsOnY(Direction6 d, bool throwIfInconsistent = true){
        checkDir6ToUnder(d, throwIfInconsistent);
        using enum Direction6;
        return (d == Up) || (d == Down);
    }

    inline bool dir6IsOnZ(Direction6 d, bool throwIfInconsistent = true){
        checkDir6ToUnder(d, throwIfInconsistent);
        using enum Direction6;
        return (d == Above) || (d == Below);
    }

    // false in case on non-consistent initial value of d.
    inline bool dir6Invert(Direction6 &d, bool throwIfInconsistent = true){
        const auto [val, ok] = checkDir6ToUnder(d, throwIfInconsistent);
        if (!ok) return false;

        d = Direction6{ (toUnderlying(d) +3) % 6 };
        return true;
    }

    inline std::string dir6ToStr(Direction6 d, bool throwIfInconsistent = true)
    {
        const auto [val, ok] = checkDir6ToUnder(d, throwIfInconsistent);
        return ok ? std::string{MR__ENUM_GET_NAMEq(Direction6,d)} : "None";
    }

    class Field3D final{
    public:
        using TLayer = std::vector<std::string>;
        using TLayers = std::vector<TLayer>;

        using Field3DException = std::string;

        Field3D(std::size_t X, std::size_t Y, std::size_t Z, char ch = ' '){
            TLayer layer(Y,std::string{});
            layers.resize(Z, layer);
        }

        Field3D(TLayers layers_){
            constrCheck(layers_);
            layers = std::move(layers_);
        }

        Field3D(const std::vector<Field2D>& fields){
            TLayers layers_;
            layers_.reserve(fields.size());
            for(const auto& field : fields){
                layers_.push_back(field.lines);
            }

            constrCheck(layers_);
            layers = std::move(layers_);
        }

        Field3D buildSubField( CoordUnsign beginX, CoordUnsign endX,
                               CoordUnsign beginY, CoordUnsign endY,
                               CoordUnsign beginZ, CoordUnsign endZ ){
            raiseIfOutOfBoundary({beginX,beginY,beginZ});
            if ((endX <= beginX) || (endY <= beginY) || (endZ <= beginZ)){ // => ends are positive
                throw std::runtime_error("Empty sub-build");
            }
            raiseIfOutOfBoundary({endX-1U,endY-1U, endZ-1U});

            Field3D res(endX - beginX, endY - beginY, endZ - beginZ);

            CoordUnsign k = 0;
            while(beginZ < endZ){
                Field2D fk(layers[beginZ]);

                res.layers[k++] = fk.buildSubField(beginX,endX,beginY,endY).lines;
                ++beginZ;
            }

            return res;
        }

        char& operator[](Point3Unsign p) {
            raiseIfOutOfBoundary(p);
            return layers[p.z][p.y][p.x];
        }
        char operator[](Point3Unsign p) const{
            raiseIfOutOfBoundary(p);
            return layers[p.z][p.y][p.x];
        }

        // need C++23 to use []
        char& operator()(CoordUnsign x, CoordUnsign y, CoordUnsign z){return layers[z][y][x];}
        char operator()(CoordUnsign x, CoordUnsign y, CoordUnsign z) const{return layers[z][y][x];}

        CoordUnsign dimX() const noexcept {return layers[0][0].size();}

        CoordUnsign dimY() const noexcept {return layers[0].size();}

        CoordUnsign dimZ() const noexcept {return layers[0].size();}

        bool inBoundary(Point3Unsign p) const noexcept{
            return (p.x < dimX()) && (p.y < dimY()) && (p.z < dimZ());
        }

        // true in case of steps==0U
        // false if only a part of the steps may be done -> then doAlsoPartial
        // regulates whether doing partial or nothing.
        bool movePoint(Point3Unsign &p, Direction6 d, std::size_t steps = 1U, bool doAlsoPartial = false) const{
            if (toUnderlying(d) >= SixDir6){
                using namespace std::literals::string_literals;
                throw Field3DException{"unexpected direction: "s + std::to_string(toUnderlying(d))};
            }

            if (steps == 0U) return true;

            using enum Direction6;
            switch(d){
            case Below :
                if (p.z < steps){
                    if (doAlsoPartial) p.z = 0U;
                    return false;
                }
                p.z -= steps; break;
            case Above :
                if (steps > dimZ()) steps = dimZ();
                if (p.z >= dimZ()-steps){
                    if (doAlsoPartial) p.z = dimZ()-1U;
                    return false;
                }
                p.z += steps; break;
            case Down :
                if (p.y < steps){
                    if (doAlsoPartial) p.y = 0U;
                    return false;
                }
                p.y -= steps; break;
            case Up :
                if (steps > dimY()) steps = dimY();
                if (p.y >= dimY()-steps){
                    if (doAlsoPartial) p.y = dimY()-1U;
                    return false;
                }
                p.y += steps; break;
            case Left :
                if (p.x < steps){
                    if (doAlsoPartial) p.x = 0U;
                    return false;
                }
                p.x -= steps; break;
            // case Right :
            default :
                if (steps > dimX()) steps = dimX();
                if (p.x >= dimX()-steps){
                    if (doAlsoPartial) p.x = dimX()-1U;
                    return false;
                }
                p.x += steps; break;
            }

            return true;
        }

    private:
        std::vector<std::vector<std::string>> layers;

        void raiseIfOutOfBoundary(Point3Unsign p) const{
            if (!inBoundary(p)){
                throw std::runtime_error("Point out of boundaries");
            }
        }

        void constrCheck(const TLayers& layers_){
            if (layers_.empty()){
                // dimX and dimY would be undefined.
                throw Field3DException{"no layers"};
            }

            Field2D f0Temp = layers_[0];
            if (!std::all_of(++layers_.cbegin(), layers_.cend(),
                            [&f0Temp = std::as_const(f0Temp)] (const auto &layer){
                    try{
                        Field2D fTemp(layer);
                        return (fTemp.dimX() == f0Temp.dimX()) && (fTemp.dimY() == f0Temp.dimY());
                    }catch(Field2D::Field2DException){
                        throw Field3DException{"some layer with non-uniform or zero lines!"};
                    }
                })){
                throw Field3DException{"layers with different size"};
            }
        }
    };


    // N as last, to support using Specializes.
    template<typename Coord, std::size_t N = 1>
    requires (N > 0U)
    struct PointN {
        std::array<Coord,N> x_s;

        auto begin(){return x_s.begin();}
        auto begin() const{return x_s.begin();}

        auto end(){return x_s.end();}
        auto end() const{return x_s.end();}

        PointN& operator+=(const PointN p2)
        {for(std::size_t i = 0; i < N; ++i) x_s[i] += p2.x_s[i]; return *this;}

        PointN& operator-=(const PointN p2)
        {for(std::size_t i = 0; i < N; ++i) x_s[i] -= p2.x_s[i]; return *this;}

        PointN& operator*=(const Coord c)
        {for(std::size_t i = 0; i < N; ++i) x_s[i] *= c; return *this;}

        PointN& operator/=(const Coord c)
        {for(std::size_t i = 0; i < N; ++i) x_s[i] /= c; return *this;}

        Coord operator[](std::size_t idx) const {return x_s[idx];}

        Coord& operator[](std::size_t idx) {return x_s[idx];}

        constexpr std::size_t size() const {return N;}

        friend void swap(PointN& lhs, PointN& rhs) {
            for(auto i = 0U; i < lhs.size(); ++i){
                std::swap(lhs[i],rhs[i]);
            }
        }
    };
    // TODO: manage spaceship operator

    template<typename Coord = CoordUnsign>
    using VectorN = PointN<Coord>;

    template<std::size_t N>
    using PointNunsign = PointN<CoordUnsign, N>;

    template<std::size_t N>
    using VectorNUnsign = PointNunsign<N>;

    template<std::size_t N>
    using PointNsign = PointN<CoordSign, N>;

    template<std::size_t N>
    using VectorNSign = PointNsign<N>;

    template<typename Coord, std::size_t N>
    bool operator==(const PointN<Coord, N>& p1, const PointN<Coord, N>& p2)
    { return std::all_of( p1.x_s.begin(), p1.x_s.end(),
                          [&p1, &p2](const Coord& c1){return c1 == p2[std::distance(p1.x_s.begin(),&c1)];} ); }

    template<typename Coord, std::size_t N>
    bool operator<(const PointN<Coord, N>& p1, const PointN<Coord, N>& p2)
    {
        auto it1 = std::find_if( p1.x_s.begin(), p1.x_s.end(),
                                 [&p1, &p2](const Coord& c1){return c1 != p2[std::distance(p1.x_s.begin(),&c1)];} );
        if (it1 == p1.x_s.end()) return false; // equivalent
        return *it1 < p2[std::distance(p1.x_s.begin(),it1)];
    }

    template<typename Coord, std::size_t N>
    bool operator!=(const PointN<Coord, N>& p1, const PointN<Coord, N>& p2)
    { return !(p1 == p2); }

    template< typename T, std::size_t N >
    requires (N > 0U)
    std::string arrayToStr(const std::array<T,N>& a){
        using std::literals::string_literals::operator""s;

        return std::accumulate( std::next(a.begin()), a.end(), "("s + std::to_string(a[0]),
                                [](std::string acc, const auto& tv){return acc + ", " + std::to_string(tv);} ) + ")";
    }

    template<typename Coord, std::size_t N>
    requires (N > 0U)
    std::string pointNtoStr(const PointN<Coord, N>& p) {
        return arrayToStr(p.x_s);
    }


    template<typename Vector>
    concept IsSomePoint = NConcept__MR::Specializes<Vector, Point> ||
                          NConcept__MR::Specializes<Vector, Point3> ||
                          NConcept__MR::Specializes_1<Vector, PointN>;

    template<IsSomePoint Vector>
    //requires requires(Vector v1, Vector v2){v1 += v2;}
    Vector operator+(const Vector& v1, const Vector& v2)
    { return Vector{v1}+=v2; }

    template<IsSomePoint Vector>
    //requires requires(Vector v1, Vector v2){v1 -= v2;}
    Vector operator-(const Vector& v1, const Vector& v2)
    { return Vector{v1}-=v2; }

    template<typename Vector, typename Coord>
    requires requires(Vector v, Coord c){v *= c;}
    Vector operator*(const Vector& v, const Coord& c)
    { return Vector{v} *= c; }

    template<typename Vector, typename Coord>
    requires requires(Vector v, Coord c){v /= c;}
    Vector operator/(const Vector& v, const Coord& c)
    { return Vector{v} /= c; }


    template<typename T1, typename T2>
    requires requires(T1 v1, T2 v2){v1.size(); v2.size(); v1[0]; v2[0];}
    // sizeReduced == 0 <-> use Size.
    auto scalarProd(const T1& v1, const T2 &v2, std::size_t sizeReduced = 0U){
        std::size_t size = (sizeReduced > 0U)? sizeReduced : std::min(v1.size(), v2.size());

        using V1 = std::decay_t<decltype(std::declval<T1>()[0])>;
        using V2 = std::decay_t<decltype(std::declval<T2>()[0])>;
        using V = decltype(std::declval<V1>() * std::declval<V2>());

        if (size == 0){
            return V{0U};
        }

        V res = 0U;
        for(auto i = 0U; i < size; ++i){
            auto f1 = std::fabs(v1[i]);
            auto f2 = std::fabs(v2[i]);

            // checkProdResult(v1[i], v2[i]);
            // checkSumResult(res, v1[i]*v2[i]);
            // TODO: defined in AOC.h ....

            res += (v1[i]*v2[i]);
        }
        return res;
    };

    template<typename T1, typename T2>
    requires requires(T1 v1, T2 v2){v1.size(); v2.size(); v1[0]; v2[0];}
    // sizeReduced == 0 <-> use Size.
    auto manDistance(const T1& v1, const T2 &v2, std::size_t sizeReduced = 0U){
        std::size_t size = (sizeReduced > 0U)? sizeReduced : std::min(v1.size(), v2.size());

        using V1 = std::decay_t<decltype(std::declval<T1>()[0])>;
        using V2 = std::decay_t<decltype(std::declval<T2>()[0])>;
        using V = decltype(std::declval<V1>() + std::declval<V2>());

        if (size == 0){
            return V{0U};
        }

        V res = 0U;
        for(auto i = 0U; i < size; ++i){
            res += ((v1[i] > v2[i]) ? (v1[i] - v2[i]) : (v2[i] - v1[i]));
        }
        return res;
    };

} // namespace NSpace__MR

template<typename Coord>
struct std::hash<NSpace__MR::Point<Coord>> final
{
    inline std::size_t operator()(const NSpace__MR::Point<Coord>& p) const noexcept
    { return NHash__MR::CombineHash{}(p.x, p.y); }
};

template<typename Coord> // outside namespace NSpace__MR
struct std::hash<NSpace__MR::Point3<Coord>> final
{
    inline std::size_t operator()(const NSpace__MR::Point3<Coord>& p) const noexcept
    { return NHash__MR::CombineHash{}(p.x, p.y, p.z); }
};

template<typename Coord, std::size_t N> // outside namespace NSpace__MR
struct std::hash<NSpace__MR::PointN<Coord, N>> final
{
    inline std::size_t operator()(const NSpace__MR::PointN<Coord, N>& p) const noexcept
    {
        return NHash__MR::CombineHash{}(p);
    }
};

inline auto& operator<<(std::ostream &os, NSpace__MR::Direction2 d) noexcept{
    return os << NSpace__MR::dir2ToStr(d,false);
}

inline auto& operator<<(std::ostream &os, NSpace__MR::Direction4 d) noexcept{
    return os << NSpace__MR::dir4ToStr(d,false);
}

inline auto& operator<<(std::ostream &os, NSpace__MR::Direction6 d) noexcept{
    return os << NSpace__MR::dir6ToStr(d,false);
}

template<NSpace__MR::IsSomePoint Vector>
auto& operator<<(std::ostream &os, const Vector& vec) noexcept{
    if constexpr(NConcept__MR::Specializes<Vector, NSpace__MR::Point>){
        return os << NSpace__MR::pointToStr(vec);
    } else
    if constexpr(NConcept__MR::Specializes<Vector, NSpace__MR::Point3>){
        return os << NSpace__MR::point3ToStr(vec);
    } else
    if constexpr(NConcept__MR::Specializes_1<Vector, NSpace__MR::PointN>){
        return os << NSpace__MR::pointNtoStr(vec);
    }
}

inline auto& operator<<(std::ostream &os, NSpace__MR::Field2D f) noexcept{
    for(std::size_t y = 0; y < f.dimY(); ++y)
        os << f[y] << std::endl;
    return os;
}