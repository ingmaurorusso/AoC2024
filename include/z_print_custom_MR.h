#pragma once

#include <charconv>
#include <concepts>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "concept_custom_MR.h"

#include "string_custom_MR.h"

// forward declaration, aimed at nest any level of iterators and iterables.
template<NConcept__MR::Iterable Iter>
requires (!requires(std::ostream os, Iter iter){os << iter;})
std::ostream& operator<<(std::ostream &os, const Iter& iter) noexcept;

#define GENERAL_SOLUTION
//#undef GENERAL_SOLUTION

#ifndef GENERAL_SOLUTION

 // forward declaration, aimed at nest any level of iterators and iterables.
 // This will be able to 'see' the operator above, in case here there is an
 // iterator referring to objects that, in turn, are iterable containers.
 // This solution only supports iterables of iterables, so that iterators for
 // each iterable refer to iterables, but it does not work if an iterable contains
 // iterators as elements so that the iterator (e.g. begin()) refers to another
 // iterator.
 template<typename ReferSomething> requires
 requires(std::ostream os, ReferSomething rf){os << *rf;}
 && (!requires(std::ostream os, ReferSomething rf){os << rf;}) // if not already printable
 std::ostream& operator<<(std::ostream &os, const ReferSomething& rf) noexcept;*

#else

 // forward declaration, aimed at nest any level of iterators and iterables.
 // This will be able to 'see' the operator above, in case here there is an
 // iterator referring to objects that, in turn, are iterable containers.
 template<typename ReferSomething> requires
 NConcept__MR::CPrintRefAny<ReferSomething>
 && (!NConcept__MR::CPrintRefN<0,ReferSomething>) // if not already printable
 std::ostream& operator<<(std::ostream &os, const ReferSomething& rf) noexcept;
#endif

namespace NPrint__MR{
  #ifdef GENERAL_SOLUTION
    template<typename ReferSomething>
    requires requires(ReferSomething rf){requires NConcept__MR::CPrintRefAny<ReferSomething>;}
    auto& printDerefOnStream(std::ostream& ostream, const ReferSomething& ref){
        if constexpr(requires(std::ostream os, ReferSomething rf){os << rf;}){
            return ostream << ref;
        } else{
            return printDerefOnStream(ostream,*ref);
        }
    }
  #endif

    template<NConcept__MR::Iterable Iter, typename ElemPrinter>
    requires requires(std::ostream ostream, Iter iter, ElemPrinter&& elemPrinter)
             {elemPrinter( ostream, *(std::begin(iter)) );}
    void printIterableOnStream( std::ostream& ostream, const Iter &iter,
                                ElemPrinter &&elemPrinter,
                                std::string_view beginner, std::string_view separator, std::string_view ender,
                                std::string_view finalPrint = "" )
    {
        auto& nestingLevel = ++getSharedNestingLevelForIterablePrint();
        auto& nestingLevelWhenSep = getSharedNestingLevelWhenSepForIterablePrint();
        const auto whenSepStillToSet = [&nestingLevelWhenSep]() -> bool {return nestingLevelWhenSep == 0;};

        ostream << beginner;

        bool first = true;
        for(const auto& elem : iter){
            if (!first){
                if (whenSepStillToSet() || (nestingLevel >= nestingLevelWhenSep) || (beginner.size() + ender.size() > 0)){
                    ostream << separator;
                    nestingLevelWhenSep = nestingLevel;
                } else{
                    // space super-separator used in case of iterables whose elements
                    // are iterables. This mechanism still 'merges' in case the
                    // first comma is reached earlier, due to empty iterable elements.
                    // To avoid any problem, 'beginner' and 'ender' to be used.
                    ostream << std::string(nestingLevelWhenSep - nestingLevel,' ');
                }
            }
            elemPrinter( ostream, elem );
            first = false;
        }

        if (first && whenSepStillToSet()){ // for the case of singleton iterable.
            nestingLevelWhenSep = nestingLevel;
        }

        ostream << ender;

        ostream << finalPrint;

        if (0U == --nestingLevel) nestingLevelWhenSep = 0U;
    }


    template<NConcept__MR::Iterable Iter>
    requires (!NConcept__MR::IsStdMapIter<typename Iter::iterator>) &&
             requires(std::ostream ostream, Iter iter) {ostream << *(std::begin(iter));}
    void printIterableOnStream( std::ostream& ostream, const Iter &iter,
                                std::string_view beginner, std::string_view separator, std::string_view ender,
                                std::string_view finalPrint = "" )
    { printIterableOnStream( ostream, iter,
                             [](std::ostream& ostream, const auto& elem){ostream << elem;},
                             beginner, separator, ender, finalPrint ); }

    template<NConcept__MR::Iterable Iter>
    requires (!NConcept__MR::IsStdMapIter<typename Iter::iterator>) &&
             requires(Iter iter) {std::cout << *(std::begin(iter));}
    void printIterable( const Iter &iter,
                        std::string_view beginner, std::string_view separator, std::string_view ender,
                        std::string_view finalPrint = "" )
    { printIterableOnStream(std::cout, iter, beginner, separator, ender, finalPrint); }


    template<NConcept__MR::Iterable Iter>
    requires NConcept__MR::IsStdMapIter<typename Iter::iterator> &&
             requires(std::ostream ostream, Iter iter){ostream << std::begin(iter)->first << std::begin(iter)->second;}
    void printIterableOnStream( std::ostream& ostream, const Iter &iter,
                                std::string_view beginner, std::string_view separator, std::string_view ender,
                                std::string_view finalPrint = "" )
    { printIterableOnStream( ostream, iter,
                             [](std::ostream& ostream, const auto& elem){
                                ostream << '[' << elem.first << ',' << elem.second << ']';
                             },
                             beginner, separator, ender, finalPrint ); }

    template<NConcept__MR::Iterable Iter>
    requires NConcept__MR::IsStdMapIter<typename Iter::iterator> &&
             requires(Iter iter){std::cout << std::begin(iter)->first << std::begin(iter)->second;}
    void printIterable( std::ostream& ostream, const Iter &iter,
                        std::string_view beginner, std::string_view separator, std::string_view ender,
                        std::string_view finalPrint = "" )
    { printIterableOnStream(std::cout, iter, beginner, separator, ender, finalPrint); }



    template<NConcept__MR::Iterable Iter>
    requires ( (!NConcept__MR::IsStdMapIter<typename Iter::iterator>) &&
               requires(Iter iter) {std::cout << *(std::begin(iter));} )
             ||
             ( NConcept__MR::IsStdMapIter<typename Iter::iterator> &&
               requires(Iter iter){std::cout << std::begin(iter)->first << std::begin(iter)->second;} )
    std::string iterableToStr( const Iter &iter,
                               std::string_view beginner, std::string_view separator, std::string_view ender,
                               std::string_view finalPrint = "" )
    requires requires( std::stringstream ss,
                       typename NConcept__MR::IterValueInfo<Iter>::type value) {ss << value;}
    {
        std::stringstream ss;
        printIterableOnStream(ss,iter,beginner,separator,ender,finalPrint);

        return NString__MR::sstream_to_string(ss);
    }

} // namespace NPrint__MR

// only iterables not already printable -> that's the motivation to be at the end of AOC.h that
// includes all other header files.
template<NConcept__MR::Iterable Iter>
requires (!requires(std::ostream os, Iter iter){os << iter;})
std::ostream& operator<<(std::ostream &os, const Iter& iter) noexcept{
    NPrint__MR::printIterableOnStream(os, iter, "(", ",", ")");

    return os;
}

#ifndef GENERAL_SOLUTION

 template<typename ReferSomething> requires
 requires(std::ostream os, ReferSomething rf){os << *rf;}
 && (!requires(std::ostream os, ReferSomething rf){os << rf;}) // if not already printable
 std::ostream& operator<<(std::ostream &os, const ReferSomething& rf) noexcept{
     return os << *rf;
 }

#else

 template<typename ReferSomething> requires
 NConcept__MR::CPrintRefAny<ReferSomething>
 && (!NConcept__MR::CPrintRefN<0,ReferSomething>) // if not already printable
 std::ostream& operator<<(std::ostream &os, const ReferSomething& rf) noexcept{
     return NPrint__MR::printDerefOnStream(os,*rf);
 }

#endif