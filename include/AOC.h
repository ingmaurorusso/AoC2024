
#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <charconv> // for std::from_chars
#include <cmath>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


#include "algebra_custom__MR.h"
using namespace NAlgebra__MR;

#include "concept_custom_MR.h"
using namespace NConcept__MR;

#include "container_custom_MR.h"
using namespace NContainer__MR;

#include "enum_custom_MR.h"
using namespace NEnum__MR;

#include "generators_custom_MR.h"
using namespace NGen__MR;

#include "graph_custom_MR.h"
using namespace NGraph__MR;

#include "hash_custom_MR.h"
using namespace NHash__MR;

#include "space_custom_MR.h"
using namespace NSpace__MR;

#include "string_custom_MR.h"
using namespace NString__MR;

// MUST BE AT THE END, due to its operator<< related
// to iterables not already printable.
#include "z_print_custom_MR.h"
using namespace NPrint__MR;

#define HEAD_FILE Day

#define _CONCAT4(a, b, c, d) a ## b ## c ## d
#define CONCAT4(a, b, c, d) _CONCAT4(a, b, c, d)

#define DAY_FILE(name) CONCAT4(HEAD_FILE, DAY, _solution_part, name)

#define _QUOTE(x) #x
#define QUOTE(x) _QUOTE(x)

#define HEADER_PART_1 DAY_FILE(_1.h) //cannot start with numbers/dot
#define HEADER_PART_2 DAY_FILE(_2.h)

#define INCLUDE_FILE(x) QUOTE(x)


namespace std_fs = std::filesystem;

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

namespace NAoc__MR{
    inline void callDebug(){
        std::string s;
        s.append("");
    }

    inline bool IsShortTest = false;

    using TResult = unsigned long long;

    template<typename T1 = TResult, typename T2 = T1> requires std::is_arithmetic_v<T1>
    inline bool checkSumResult(T1 add1, T2 add2, bool doThrow = true){

        bool bad = false;
        if ((add2 < 0) && (add2 <= std::numeric_limits<T1>::lowest())){ // first clause as, in second one, add2 might be converted
            bad = (add1 < 0); // (_1_)
        }
        if ((add2 > 0) && (add2 > std::numeric_limits<T2>::max())){ // first clause as, in second one, add2 might be converted
            bad = (add1 >= 0); // (_2_)
        }

        if constexpr (std::numeric_limits<T2>::lowest() < 0){
            bad |= ( (add2 >= 0)
                     ? (add1 > std::numeric_limits<T1>::max() - add2)
                     // - if T1 is unsigned but smaller than T2, then it is converted to signed-T2
                     //   and there is no overflow problems in computing T1_max-add2 (as add2 >= 0),
                     //   since even add1 is converted in the wider signed-T2.
                     // - if T1 is signed, all is even better, all converted into the wider type (T1
                     //   and/or T2), again with T1_max-add2 that cannot generate overflow (too negative).
                     // - if T1 is unsigned and not smaller than T2-signed, then add2 can never reach
                     //   T1_max, and no problem arises.
                     : (add1 < std::numeric_limits<T1>::lowest() - add2) );
                     // - if T1 is unsigned -> T1_lowest==0 and then, since add2 < 0, add1 must be >= 0
                     //   otherwise there is already (_1_). As a consequence, no possible problem in
                     //   eventually converting add1 (this happens only if T2-signed has more bits than
                     //   T1-unsigned). Eventaully converting add2 into unsigned does still
                     //   produce a correct effect about -add2.
                     // - if T1 is signed, then both types are signed and there are no problems
                     //   of conversions -> the widest type is used, and no overflow is possible
                     //   because T1_lowest and -add2 have different sign.
        } else // <-> T2 is unsigned -> only overcoming T1_max needs to be checked.
        if constexpr (std::numeric_limits<T1>::lowest() == 0){
            // both unsigned -> eventual conversions into the wider type.
            bad |= (add1 > std::numeric_limits<T1>::max() - add2); // all right -> same as in (_3_).
        } else{ // <-> T1 signed and T2 unsigned
            /* translated, due to (_4_)
            bad |= ( (add1 >= 0)
                     ? (add1 > std::numeric_limits<T1>::max() - add2)
                     // (_3_)
                     // - if add2 > T1_max, the correct check add1 >= 0 is already in (_2_),
                     //   therefore here either add2 <= T1_max or add1 < 0 (or both).
                     //   But, here, is also the case ("?" part of the ternary) where add1 is >= 0.
                     //   Therefore, the only interesting case is: add2 <= T1_max, from which the
                     //   second member results positive, with no problems on conversions,
                     //   and the same for first member, where add1 >= 0.
                     : ( (sizeof(T2) < sizeof(T1))
                         ? (add1+add2 > std::numeric_limits<T1>::max())
                     // (_4_)
                     // - if T2 is smaller than T1, then add2 is converted into T1-signed
                     //   and add1+add2 is for sure a representable value for T1 because
                     //   add1 is negative and add2 is positive. Moreover, the result for
                     //   sure does not lead bad to be true.
                         : (add2 > std::numeric_limits<T1>::max() - T2{add1}) ) );
                     // - if T2 is not smaller than T1, then T1_max and add1 are converted
                     //   into T2-unsigned and their difference represents the gap between
                     //   -|add1| and T1_max (correctly computed), finally correctly compared
                     //   with add2 with no problems. */
            if (add1 >= 0){
                bad |= (add1 > std::numeric_limits<T1>::max() - add2);
            } else
            if constexpr (sizeof(T2) >= sizeof(T1)){
                bad |= (add2 > std::numeric_limits<T1>::max() - T2{add1});
            }
        }

        if (bad && doThrow){
            throw std::runtime_error("TResult to be wider: sum "s + std::to_string(add1) + " + " + std::to_string(add2));
        }
        return bad;
    }

    template<typename T1 = TResult, typename T2 = T1> requires std::is_arithmetic_v<T1>
    inline bool checkProdResult(T1 factor1, T2 factor2__, bool doThrow = true){
        bool bad;
        T1 factor2 = factor2__;

        if ((factor1 < 1) || (factor2 < 1)){
            if constexpr (std::numeric_limits<T1>::epsilon() < 1){
                if (std::fabs(factor1) < std::fabs(factor2)){
                    std::swap(factor1, factor2); // factor2-module lowest
                }

                bad = (factor1 < std::numeric_limits<T1>::epsilon() / factor2); // underflow check
            } else bad = false; // at least one factor is 0.
        } else
        if constexpr (std::numeric_limits<T1>::lowest() < 0){
            const bool n1 = (factor1 < 0);
            const bool n2 = (factor2 < 0);

            bad = (n1 == n2)
                  ? (std::fabs(factor1) > std::numeric_limits<T1>::max() / std::fabs(factor1))
                  : (std::fabs(factor1) > std::numeric_limits<T1>::lowest() / (-std::fabs(factor2)) );
        } else{
            bad = (factor1 > std::numeric_limits<T1>::max() / factor2);
        }

        if (bad && doThrow){
            throw std::runtime_error("TResult to be wider: prod "s + std::to_string(factor1) + " * " + std::to_string(factor2));
        }
        return bad;
    }

    template<typename T = TResult> requires std::is_arithmetic_v<T>
    inline bool checkResultExtension(std::size_t bitsNumber, bool doThrow = true){
        bool bad = (bitsNumber > sizeof(TResult)*CHAR_BIT);

        if (bad && doThrow){
            throw std::runtime_error( "TResult to be wider: bit number "s +
                                      std::to_string(sizeof(TResult)*CHAR_BIT) + " < " + std::to_string(bitsNumber) );
        }
        return bad;
    }

    constexpr auto MaxLineLength = 10000;

    struct LineResult
    {
        bool endedLines, emptyLine, tooLongLine;
    };
    using TExtractionRes = std::tuple<std::string, std::stringstream, LineResult>;
    

    template<auto MaxLineLength>
    inline TExtractionRes lineExtraction(const std::shared_ptr<std::istream>& inputStream, bool doTrim = true, bool anyLength = true){
        static_assert(MaxLineLength > 0);

        thread_local std::array<char, MaxLineLength + 1> cc{};
        std::string line;

        bool ok;
        // if from input file, automatically filters an empty line at the end

        bool tooLongLine{false};
        bool emptyLine{false};

        std::size_t c;

        do{
            ok = static_cast<bool>(inputStream->getline(cc.data(), MaxLineLength/*, '\n'*/));
            c = static_cast<size_t>(inputStream->gcount());
            if (ok){            
                // 'c' includes the delimiter, which is replaced by '\0',
                // unless end of stream.

                // std::cout << "length = " << c << '\n';
                if (cc.at(c - 1U) == '\0') { // 'c' is positive as line has been extracted.
                    --c; // excludes '\0' that replaced the delimiter '\n'.
                } //else: end of stream reached

                line += std::string_view(cc.data(), c);
                // std::cout << "line: " << line << '\n';
            } else{
                // May fail either for too long input line, or for no character.
                if (c > 0){
                    if (!anyLength){
                        tooLongLine = true;
                        break;
                    } else{
                        line += std::string_view(cc.data(), c);
                        inputStream->clear();
                    }
                } else{
                    break;
                }
            }
        } while(!ok);

        if (doTrim){
            trim(line);
        }
        if (ok){ // after trimming
            emptyLine = (c == 0);
        }
        
        std::stringstream ssCopyRes;
        ssCopyRes << line;

        return std::make_tuple( std::move(line),
                                std::move(ssCopyRes),
                                LineResult{.endedLines = !(ok || tooLongLine), .emptyLine = emptyLine, .tooLongLine = tooLongLine});
    };

    inline bool toSigned(const std::string& s, signed long& res) {
        try {
            if (s.empty()) {
                return false;
            }
            if (!std::all_of(std::next(s.cbegin()), s.cend(), [](const char ch) {
                    return std::isdigit(ch);
                })) {
                // stol would ignore tailing non-digits.
                // Here ignore first char, that is, allowing '-' (specific check later for future use).
                return false;
            }
            res = std::stol(s);

            return errno != ERANGE;
        } catch (...) {
            // std::invalid_argument, eventually std::out_of_range
            return false;
        }
    }

    inline bool toUnsigned(const std::string& s, unsigned long& res) {
        try {
            if (s.empty()) {
                return false;
            }
            if (!std::all_of(std::next(s.cbegin()), s.cend(), [](const char ch) {
                    return std::isdigit(ch); // avoid first, with '-' in order to allow '-0'.
                })) {
                // stoul would ignore tailing non-digits.
                // Here ignore first char, that is, allowing '-' (specific check later for future use).
                return false;
            }
            res = std::stoul(s);
            // Note also that stoul would have converted into unsigned a value starting with minus
            // ('-').

            if (s.find('-') != std::string::npos) {
                // stoul converts negative values with a reinterpret cast.
                if (s.substr(0U, 2U) != "-0") {
                    return false;
                }
            }

            return errno != ERANGE;
        } catch (...) {
            // std::invalid_argument, eventually std::out_of_range
            return false;
        }
    }

    inline bool hexToUnsigned(const std::string& s, unsigned long& res) {
        try {
            if (s.empty()) {
                return false;
            }
            if (!std::all_of(s.cbegin(), s.cend(), [](const char ch) {
                    return (std::isdigit(ch) || ((ch >= 'a') && (ch <= 'f'))
                            || ((ch >= 'A') && (ch <= 'F')));
                })) {
                // operator>> would ignore tailing non-digits (TODO: check if so).
                return false;
            }

            std::stringstream ss;
            ss << std::hex << s;
            if (ss >> res) {
                return true;
            }

            return true;
        } catch (...) {
        }

        return false;
    }
} // namespace NAoc__MR

/*
unable to make it working for std::_Rb_tree_const_iterator
// outside namespaces
template<typename It>
requires //requires(It it){std::next(it);} &&
         requires(It it){*it;} && (!requires(It it){it < it;}) // when not already defined
         && requires(It it){*it < *it;}
inline bool operator<(const It& it1, const It& it2){
    return *it1 < *it2;
    // just experimental, as iterators should be only partially sorted.
    // see: https://stackoverflow.com/questions/43996394/how-to-implement-less-than-operator-for-random-access-iterator
}
*/