#pragma once

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "concept_custom_MR.h"

namespace NContainer__MR{
    template<class SortedContainer>
    bool oneContainsAnother(const SortedContainer& c1, const SortedContainer& c2)
    {
        const auto sz1 = std::size(c1);
        const auto sz2 = std::size(c2);

        const auto &ref1 = (sz1 < sz2)? c1 : c2;
        const auto &ref2 = (sz1 < sz2)? c2 : c1;

        auto it2 = std::cbegin(ref2);
        return std::all_of( std::cbegin(ref1), std::cend(ref1),
                            [&it2, &ref2 = std::as_const(ref2)]( const auto& elem1){
                                it2 = std::find(it2, std::cend(ref2), elem1);
                                return (it2 != std::cend(ref2));
                            } );
    }

    template<NConcept__MR::Iterable Container>
    requires requires(Container c){c.clear(); c.contains(*std::begin(c)); c.clear(); c.erase(std::begin(c));}
    void reduceContainer(Container& cReduce, const Container& cTarget, bool keepNotRemoveIfInTarget = false)
    {
        if (&cReduce == &cTarget){
            if (!keepNotRemoveIfInTarget){
                cReduce.clear();
            }
        } else
        if (!keepNotRemoveIfInTarget){
            auto itReduce = std::begin(cReduce);
            auto itTarget = std::cbegin(cTarget);

            for(const auto& targetElem : cTarget){
                cReduce.erase(targetElem);
            }
        } else{
            // cReduce must become the intersection.
            for(auto it = std::begin(cReduce); it != std::end(cReduce); ){
                if (!cTarget.contains(*it)){
                    it = cReduce.erase(it);
                } else ++it;
            }
        }
    }

    template<NConcept__MR::Iterable Container>
    requires requires(Container c){c.clear(); c.contains(std::begin(c)->first); c.clear(); c.erase(std::begin(c)->first);}
    void reduceContainer(Container& cReduce, const Container& cTarget, bool keepNotRemoveIfInTarget = false)
    {
        if (&cReduce == &cTarget){
            if (!keepNotRemoveIfInTarget){
                cReduce.clear();
            }
        } else
        if (!keepNotRemoveIfInTarget){
            auto itReduce = std::begin(cReduce);
            auto itTarget = std::cbegin(cTarget);

            for(const auto& [keyElem, valueElem_] : cTarget){
                cReduce.erase(keyElem);
            }
        } else{
            // cReduce must become the intersection.
            for(auto it = std::begin(cReduce); it != std::end(cReduce); ){
                if (!cTarget.contains(it->first)){
                    it = cReduce.erase(it);
                } else ++it;
            }
        }
    }

    template<typename SortedContainer, typename KeyExtractor>
    concept ReduceableContainer =
            requires (SortedContainer sc, KeyExtractor&& keyExtractor){
                std::lower_bound(std::begin(sc), std::end(sc), keyExtractor(std::begin(sc)));
                sc.clear(); sc.erase(std::begin(sc)); sc.empty();
                keyExtractor(std::begin(sc)) < keyExtractor(std::begin(sc)); };

    // for keepNotRemoveIfInTarget==false and std::set container, it exploits
    // std::set_intersect / std::set_difference (and the modification remains in-place).
    template<NConcept__MR::Iterable SortedContainer, typename KeyExtractor>
    requires ReduceableContainer<SortedContainer, KeyExtractor>
    void reduceContainer( SortedContainer& cReduce, const SortedContainer& cTarget,
                          KeyExtractor&& keyExtractor, bool keepNotRemoveIfInTarget = false,
                          bool mayReallocateSurvivedSetElements = false )
    {
        if (cReduce.empty()) return;

        using KeyExtracted = std::invoke_result_t<KeyExtractor, decltype(std::begin(std::declval<SortedContainer>()))>;
        if constexpr(requires(SortedContainer sc){*std::begin(sc);}){
            if (std::is_same_v<KeyExtracted,decltype(*std::begin(std::declval<SortedContainer>()))>){
                SortedContainer cResult; // need default constructor...
                // Exploit make_move_iterator, see
                // https://stackoverflow.com/questions/16079362/how-to-compute-in-place-set-difference-of-two-multisets
                if (keepNotRemoveIfInTarget){
                    std::set_intersection(
                        std::make_move_iterator(cReduce.begin()), std::make_move_iterator(cReduce.end()),
                        cTarget.cbegin(), cTarget.cend(),
                        std::inserter(cResult, cResult.begin())
                    );
                } else{
                    std::set_difference(
                        std::make_move_iterator(cReduce.begin()), std::make_move_iterator(cReduce.end()),
                        cTarget.cbegin(), cTarget.cend(),
                        std::inserter(cResult, cResult.begin())
                    );
                }

                cReduce = std::move(cResult);

                return;
            }
        }

        if (&cReduce == &cTarget){
            if (!keepNotRemoveIfInTarget){
                cReduce.clear();
            }
        } else{
            auto itReduce = std::begin(cReduce);
            auto itTarget = std::cbegin(cTarget);

            while (itReduce != cReduce.end()){
                itTarget = std::lower_bound(itTarget, std::cend(cTarget), keyExtractor(itReduce));
                if ( keepNotRemoveIfInTarget !=
                    ((itTarget != std::cend(cTarget)) && (keyExtractor(itTarget) == keyExtractor(itReduce))) ){
                    itReduce = cReduce.erase(itReduce);
                } else{
                    itReduce++;
                }
            }
        }
    }

    // TODO: check for possible bug of gcc: this overload is not good alone due to lower_bound using comparisons,
    // but it also generates overloads if both are defined ! This is true when in the requires clause
    // we miss the direct comparison. Not sure whether the standard requires just to match the signature...
    // ... but otherwise it would mean that std::lower_bound might get a require.
    template<NConcept__MR::Iterable SortedContainer>
    requires requires(SortedContainer sc){*std::begin(sc);}
             && ReduceableContainer<SortedContainer, decltype([](auto itCont){return *itCont;})>
    void reduceContainer(SortedContainer& cReduce, const SortedContainer& cTarget, bool keepNotRemoveIfInTarget = false)
    {
        return reduceContainer(cReduce, cTarget, [](auto itCont){return *itCont;}, keepNotRemoveIfInTarget);

        /*if (&cReduce == &cTarget){
            if (!keepNotRemoveIfInTarget){
                cReduce.clear();
            }
        } else{
            auto itReduce = std::begin(cReduce);
            auto itTarget = std::cbegin(cTarget);

            while (itReduce != cReduce.end()){
                itTarget = std::lower_bound(itTarget, std::cend(cTarget), *itReduce);
                if ( keepNotRemoveIfInTarget !=
                    ((itTarget != std::cend(cTarget)) && (*itTarget == *itReduce)) ){
                    itReduce = cReduce.erase(itReduce);
                } else{
                    itReduce++;
                }
            }
        }*/
    }

    template<NConcept__MR::Iterable SortedContainer>
    requires requires(SortedContainer sc){std::begin(sc)->first;}
             && ReduceableContainer<SortedContainer, decltype([](auto itCont){return itCont->first;})>
    void reduceContainer(SortedContainer& cReduce, const SortedContainer& cTarget, bool keepNotRemoveIfInTarget = false)
    {
        return reduceContainer(cReduce, cTarget, [](auto itCont){return itCont->first;}, keepNotRemoveIfInTarget);

        /*if (&cReduce == &cTarget){
            if (!keepNotRemoveIfInTarget){
                cReduce.clear();
            }
        } else{
            auto itReduce = std::begin(cReduce);
            auto itTarget = std::cbegin(cTarget);

            while (itReduce != cReduce.end()){
                itTarget = std::lower_bound(itTarget, std::cend(cTarget), itReduce->first);
                if ( keepNotRemoveIfInTarget !=
                    ((itTarget != std::cend(cTarget)) && (itTarget->first == itReduce->first)) ){
                    itReduce = cReduce.erase(itReduce);
                } else{
                    itReduce++;
                }
            }
        }*/
    }

    template<template<typename, typename> class ContTempl, class Key, class Value, typename EvalCombiner>
    requires NConcept__MR::Callable<EvalCombiner, bool, Value&, Value&&>
    void mergeMapCombining( ContTempl<Key,Value>& mapDest, ContTempl<Key,Value>&& mapSource, EvalCombiner&& combiner )
    {
        mapDest.merge(mapSource);

        for(auto itSource = mapSource.begin(); itSource != mapSource.end(); ){
            if (combiner(mapDest[itSource->first],std::move(itSource->second))){
                itSource = mapSource.erase(itSource);
            } else{
                ++itSource; // all the unused remains.
            }
        }
    }

    template< typename TResult, template<typename, typename> class ContTempl, class Key, class Value, typename EvalCombiner >
    requires requires (TResult res, std::pair<const Key, Value> p){ {std::declval<EvalCombiner>()(res,p)} -> std::convertible_to<TResult>; }
    TResult accumulateMapValues( const ContTempl<Key,Value>& map, TResult seed, EvalCombiner&& comb )
    {
        return std::accumulate(std::cbegin(map), std::cend(map), seed, std::forward<EvalCombiner>(comb));
    }


    template< template<typename, typename> class ContTempl, class Key, class Value,
              typename ValueTransformer = std::function<Value(Value)> >
    requires requires(ContTempl<Key,Value> map){std::cbegin(map)->second; std::next(std::cbegin(map)); std::cend(map);}
             && requires(std::invoke_result_t<ValueTransformer, Value> acc, Value v){acc += std::declval<ValueTransformer>()(v);}
    // TResult = std::invoke_result_t<ValueTransformer, Value>
    auto sumMapValues( const ContTempl<Key,Value>& map, ValueTransformer&& valueTransformer = NConcept__MR::idemExtractor,
                       std::invoke_result_t<ValueTransformer, Value> seed = 0U )
    {
        return accumulateMapValues<std::invoke_result_t<ValueTransformer, Value>, ContTempl>
               (map, seed, [&valueTransformer = std::as_const(valueTransformer)](auto acc, const auto& p){return acc += valueTransformer(p.second);});
    }

    template< NConcept__MR::Iterable ContDest, NConcept__MR::Iterable ContSource,
              typename Transformer = std::function< NConcept__MR::IterValueType<ContSource>(
                                                    NConcept__MR::IterValueType<ContSource>) > >
    requires requires(ContSource cSource, Transformer&& tr){ {tr(*(std::begin(cSource)))} -> std::convertible_to< NConcept__MR::IterValueType<ContDest> >; }
    void transformContainer(ContDest& contDest, const ContSource& contSource, Transformer&& transformer = NConcept__MR::idemExtractor){
        contDest.clear();

        if constexpr(requires(ContDest cDest, ContSource cSource, Transformer&& tr)
                             {cDest.push_back(tr(*(std::begin(cSource))));}){
            std::transform( std::begin(contSource), std::end(contSource),
                            std::back_insert_iterator(contDest),
                            std::forward<Transformer>(transformer) );
        } else
        if constexpr(requires(ContDest cDest){std::inserter(cDest, contDest.begin());}){
            std::transform( std::begin(contSource), std::end(contSource),
                            std::inserter(contDest, contDest.begin()),
                            std::forward<Transformer>(transformer) );
        } else{
            static_assert(false);
        }
    }
} // namespace NContainer__MR
