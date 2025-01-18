#pragma once

#include <charconv>
#include <concepts>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "concept_custom_MR.h"

namespace NString__MR{
    inline void trim(std::string& s){
        if ( size_t pos = s.find_last_not_of(' ');
             (pos+1U < s.size()) && (pos != std::string::npos) ){
            s.resize(pos+1U);
        }

        if ( size_t pos = s.find_first_not_of(' ');
             (pos > 0U) && (pos != std::string::npos) ){
            s = s.substr(pos,s.size());
        }

        if ((!s.empty()) && (s[0] == ' ')){
            s.clear();
        }
    }

    inline std::string trimCopy(const std::string& s){
        auto sCopy{s};
        trim(sCopy);
        return sCopy;
    }


    inline std::string sstream_to_string(std::stringstream& ss){
        std::string res;
        bool firstPart = true;
        do{
            std::string part;
            std::getline(ss, part);
            if (ss && !firstPart){
                res += '\n';
            }
            res += part;
            firstPart = false;
        } while (ss);
        return res;
    }


    template<typename First, typename... Args>
    requires requires(First&& f, Args&&... args){ {std::cout << std::forward<First>(f)};
                                                  {((std::cout << ' ' << std::forward<Args>(args)), ...)}; }
    void print(First&& first, Args&&... args)
    {
        printOnStream(std::cout, ' ', std::forward<First>(first), std::forward<Args>(args)... );
        /*
        std::cout << std::forward<First>(first);
        ((std::cout << ' ' << std::forward<Args>(args)), ...);
        std::cout << std::endl;
        */
    }

    template<typename First, typename... Args>
    requires requires(std::ostream ostream, First&& f, Args&&... args){ {ostream << std::forward<First>(f)};
                                                                        {((ostream << ' ' << std::forward<Args>(args)), ...)}; }
    void printOnStream(std::ostream& ostream, std::string_view separator, First&& first, Args&&... args)
    requires requires(std::ostream ostream, decltype(separator) sep) {ostream << sep;}
    {
        const auto lPrintOnStream = [&ostream, &separator = std::as_const(separator)](auto&& arg){
            using Arg = std::remove_reference_t<decltype(arg)>;

            ostream << separator;
            if constexpr(NConcept__MR::Iterable<Arg> && !requires{ostream << arg;}){
                ostream << iterableToStr(std::forward<Arg>(arg), separator);
            } else{
                ostream << std::forward<Arg>(arg);
            }
        };

        /* by Vincenzo Simone @ Luxoft [except the extension with the the ternay operator for iterables]
        original by @Vincenzo
        // check print in C++23.
        ostream << std::forward<First>(first);
        // ( (ostream << separator << std::forward<Args>(args)), ...  ); */

        lPrintOnStream(std::forward<First>(first));
        ( lPrintOnStream(std::forward<Args>(args)), ...  );
    }

    template<typename First, typename... Args>
    std::string getString(std::string_view separator, First&& first, Args&&... args)
    //same as buildString.
    requires requires(std::stringstream sstream, decltype(separator) sep) {sstream << sep;}
    {
        std::stringstream ss;
        printOnStream(ss, separator, std::forward<First>(first), std::forward<Args>(args)...);

        return sstream_to_string(ss);
    }
    template<typename First, typename... Args>
    std::string buildString(std::string_view separator, First&& first, Args&&... args)
    requires requires(std::stringstream sstream, decltype(separator) sep) {sstream << sep;}
    {return getString(separator,std::forward<First>(first),std::forward<Args>(args)...);}

    template<NConcept__MR::Iterable Iter>
    std::string getString(const Iter& iter, std::string_view separator)
    requires requires(std::stringstream sstream, Iter iter, decltype(separator) sep) {sstream << sep << *(std::cbegin(iter));}
    {
        std::stringstream ss;

        bool firstPart = true;
        for(auto it = std::cbegin(iter); it != std::cend(iter); ++it){
            if (!firstPart){
                ss << separator;
            }
            ss << *it;
            firstPart = false;
        }

        return sstream_to_string(ss);
    }

    inline std::string getStringWithChars(std::initializer_list<char> l){
        std::string res;
        for(auto ch : l) res += std::string(1,ch);
        return res;
    }

    // this does not use rgx, therefore begin position automatically indicates also end positions.
    inline std::vector<std::size_t> getSubStrPositions(const std::string& str, const std::string& subStr){
        std::vector<std::size_t> res;

        std::size_t acc = 0U;
        while(true){
            auto p = str.find(subStr, acc);
            if (p == std::string::npos) break;

            res.push_back(acc += p);
            acc += subStr.size();
        }

        return res;
    }

    inline std::vector<std::string_view> getSubStrBoundaries(std::string_view source, const std::string& rgxSubStr){
        std::vector<std::string_view> res;

        std::match_results<std::string_view::const_iterator> matchInfo{};
        auto itLastEnd = source.cbegin();

        while (std::regex_search(itLastEnd, source.cend(), matchInfo, std::regex{rgxSubStr})){
            itLastEnd = matchInfo[0].second;

            std::string_view sv{matchInfo[0].first, matchInfo[0].second};

            res.push_back(sv);
        }

        return res;
    }

    inline std::vector<std::string> getSubStrOccurrences(std::string_view source, const std::string& rgxSubStr){
        std::vector<std::string> res;

        auto s_views = getSubStrBoundaries( source, rgxSubStr );

        for(auto sv : s_views){
            res.push_back(std::string{sv});
        }

        return res;
    }

    template<typename ValueType> requires std::is_arithmetic_v<ValueType>
    inline std::vector<ValueType> getAllValues(std::string_view source){
        
        std::vector<ValueType> res;

        auto s_views = getSubStrBoundaries(source, R"__((-?[\d]+))__");

        for(auto sv : s_views){
            auto first = sv.begin();
            if constexpr (std::numeric_limits<ValueType>::lowest() == 0){
                if (*first == '-')
                    first++; // jump initial '-'
            }    

            ValueType v;
            switch (std::from_chars(first, sv.end(), v).ec){
            case std::errc::invalid_argument:
                throw std::runtime_error("Fix code of getAllValues");
            case std::errc::result_out_of_range:
                using std::literals::string_literals::operator""s;                        
                throw std::runtime_error("Type ValueType="s + typeid(ValueType).name() + " to be wider");
            }

            res.push_back(v);
        }

        return res;
    }

    // Second element of each pair is the occurrence of the separator rgx-expression (empty string after last occurrence).
    inline std::vector<std::pair<std::string, std::string>> split2(const std::string& source, const std::string& rgxSeparator, bool doTrim){
        using Pair = std::pair<std::string, std::string>;
        std::vector<Pair> res;

        std::smatch matchInfo{};
        auto itLastEnd = source.cbegin();

        while (std::regex_search(itLastEnd, source.cend(), matchInfo, std::regex{rgxSeparator})){
            std::string s{itLastEnd, matchInfo[0].first};
            itLastEnd = matchInfo[0].second;

            if (doTrim){
                trim(s);
            }

            res.push_back(Pair{std::move(s), std::string{matchInfo[0].first, matchInfo[0].second}});
        }

        if (itLastEnd != source.cend()){
            std::string s{itLastEnd, source.cend()};
            if (doTrim){
                trim(s);
            }

            res.push_back(Pair{std::move(s), std::string{}});
        }

        return res;
    }

    inline std::vector<std::string> split(const std::string& source, const std::string& rgxSeparator, bool doTrim){
        auto res2 = split2(source, rgxSeparator, doTrim);

        std::vector<std::string> res;

        for(auto& [s, sep_] : res2){
            res.push_back(std::move(s));
        }

        return res;
    }

    template<NConcept__MR::Iterable Iter, typename Translator = std::function< NConcept__MR::IterValueType<Iter>(
                                                                               NConcept__MR::IterValueType<Iter>) >>
    requires requires(Iter iter, Translator&& tr){{tr(*std::begin(iter))} -> std::convertible_to<std::string>;}
    inline std::string join( const Iter& iter, const std::string& separator,
                             Translator&& translator = NConcept__MR::idemExtractor,
                             bool doPreTrim = false){
        auto res = std::accumulate( iter.begin(), iter.end(), std::string{},
                                    [doPreTrim, &separator, translator](auto acc, const auto& next){
                                        std::string nextStr = translator(next);
                                        if (doPreTrim){
                                            trim(nextStr);
                                        }
                                        acc += std::move(nextStr);
                                        acc += separator;
                                        return acc;
                                    });

        if (!res.empty()){
            res.resize(res.size()-1U);
        }

        return res;
    }

    // Remember to escap chars that are special for regex.
    inline void stringReplaceRgx(std::string &str, const std::string& removeRgx, const std::string& addClassic){
        //from: https://stackoverflow.com/questions/75742782/how-to-use-c-stdranges-to-replace-substring-in-string
        // TODO: use simpler solution there in C++23
        str = std::regex_replace(str, std::regex(removeRgx), addClassic);
    }

    inline auto& getSharedNestingLevelForIterablePrint(){
        // TODO: should be a private function. It's here to be independent from
        // the template specializations.
        thread_local unsigned nestingLevel = 0;
        return nestingLevel;
    }

    inline auto& getSharedNestingLevelWhenSepForIterablePrint(){
        // TODO: should be a private function. It's here to be independent from
        // the template specializations.
        thread_local unsigned nestingLevelWhenComma = 0;
        return nestingLevelWhenComma;
    }
} // namespace NPrint__MR