//To be compiled with "g++ -std=c++17" or "g++ -std=c++2a" (gcc 9.4.0 does not contain yet __cpp_using_enum)
//to see expansion -> g++ -std=c++2a your_source_file_using_it.cpp -E -P -o your_expand_result.cpp

#pragma once

#include <array>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string_view>
#include <utility>

#include "concept_custom_MR.h"

// const char* may be replaced by std::string_view
// easy modification operations on code lines are remarked by ^^^

#define _CONCAT2(a, b) a ## b
#define CONCAT2(a, b) _CONCAT2(a, b)

namespace NEnum__MR{

  template<typename E>
  constexpr auto toUnderlying(const E e) noexcept
  {
    return static_cast<std::underlying_type_t<E>>(e);
  }

  using MR__stringType = std::string_view; // const char*; // ^^^ change type if you whish so (and include what you need)

  template<typename EnumType>
  constexpr std::pair<EnumType,MR__stringType>
  MR__ENUM_MAP_BEGIN_EXPAND_1(EnumType v, const NEnum__MR::MR__stringType vs)
  {return std::make_pair(v,vs);}

  template<typename EnumType>
  constexpr std::pair<EnumType,MR__stringType>
  MR__ENUM_MAP_BEGIN_EXPAND_2(EnumType v, const NEnum__MR::MR__stringType vs)
  {return std::make_pair(v,vs);}

  #define MR__INTERNAL_ENUM_TYPE_NAME type_enum

  #ifdef __cpp_using_enum
    #define MR__EXPLOIT_USING_DIR_ENUM
    // ^^^ switch to undef in order to force not using 'using enum directive'
  #else
    #undef MR__EXPLOIT_USING_DIR_ENUM
  #endif

  #ifdef MR__EXPLOIT_USING_DIR_ENUM
    //exploit 'using enum' (C++20)
    #define MR__QUALIFY_BY_INTERNAL_ENUM_TYPE_NAME
    #define MR__USING_DIR_INTERNAL_ENUM_TYPE using enum MR__INTERNAL_ENUM_TYPE_NAME
  #else
    #define MR__QUALIFY_BY_INTERNAL_ENUM_TYPE_NAME MR__INTERNAL_ENUM_TYPE_NAME::
    #define MR__USING_DIR_INTERNAL_ENUM_TYPE
  #endif

  #define MR__ENUM_MAP_BEGIN_EXPAND_1(V) std::make_pair(MR__QUALIFY_BY_INTERNAL_ENUM_TYPE_NAME V, #V), MR__ENUM_MAP_BEGIN_EXPAND_2
  #define MR__ENUM_MAP_BEGIN_EXPAND_2(V) std::make_pair(MR__QUALIFY_BY_INTERNAL_ENUM_TYPE_NAME V, #V), MR__ENUM_MAP_BEGIN_EXPAND_1

  // cannot mix with templates, because macro expansions will happen in different
  // namespaces from the original template.

  // should not be used directly
  #define MR__ENUM_MAP_EXPAND_NAME___(EnumType) CONCAT2(Expand_,EnumType)

  // should not be used directly
  #define MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM___(EnumType, InternalContainerDefinition) \
    struct MR__ENUM_MAP_EXPAND_NAME___(EnumType) { \
      using type_enum = EnumType; \
      \
      constexpr MR__ENUM_MAP_EXPAND_NAME___(EnumType)() = default; \
      \
      MR__USING_DIR_INTERNAL_ENUM_TYPE; \
      \
      inline static const InternalContainerDefinition mapping \
      {  MR__ENUM_MAP_BEGIN_EXPAND_1

  // should not be used directly
  #define MR__BUILD_CONTAINER_P_NAME___(EnumType, ContainerOfPairs) \
    ContainerOfPairs<std::pair<EnumType, NEnum__MR::MR__stringType>>
  // can be used directly
  #define MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_P(EnumType, ContainerOfPairs) \
    MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM___(EnumType, MR__BUILD_CONTAINER_P_NAME___(EnumType,ContainerOfPairs))

  // should not be used directly
  #define MR__BUILD_CONTAINER_PS_NAME___(EnumType, ContainerSizedOfPairs, nValues) \
    ContainerSizedOfPairs<std::pair<EnumType, NEnum__MR::MR__stringType>, nValues>
  // can be used directly
  #define MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_P_SIZED(EnumType, ContainerSizedOfPairs, nValues) \
    MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM___(EnumType, MR__BUILD_CONTAINER_PS_NAME___(EnumType,ContainerSizedOfPairs,nValues))

  // should not be used directly
  #define MR__BUILD_CONTAINER_KV_NAME__(EnumType, ContainerWithKeys) \
    ContainerWithKeys<EnumType, NEnum__MR::MR__stringType>
  // can be used directly
  #define MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_KV(EnumType, ContainerWithKeys) \
    MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM___(EnumType, MR__BUILD_CONTAINER_KV_NAME__(EnumType,ContainerWithKeys))

  #define MR__ENUM_MAP_BEGIN_EXPAND(EnumType) \
    MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_KV(EnumType, std::map)
    // possible alternatives: any container constructable with an initializer-list
    // and iterable with std::cbegin() on.
    // However, if no (unordered) map, getMappedString needs find_if
    // MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_KV(EnumType, std::map)
    // MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_KV(EnumType, std::unordered_map)
    // MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_P(EnumType, std::list)
    // MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_P(EnumType, std::vector)

  #define MR__ENUM_MAP_BEGIN_EXPAND_SIZED(EnumType, nValues) \
    MR__ENUM_MAP_BEGIN_EXPAND_CUSTOM_P_SIZED(EnumType, std::array, nValues)

  // last occurrence of macro name will invoke the function template
  #define MR__ENUM_MAP_END_EXPAND(V) <MR__INTERNAL_ENUM_TYPE_NAME>(MR__QUALIFY_BY_INTERNAL_ENUM_TYPE_NAME V, #V) }; \
    };

  // implementation for generic (iterable) container
  template<typename EnumType, typename Expand>
  constexpr NEnum__MR::MR__stringType getMappedString(EnumType v)
  requires NConcept__MR::Iterable<decltype(Expand::mapping)> &&
           requires(decltype(Expand::mapping) map){ {std::begin(map)->first} -> std::convertible_to<EnumType>;
                                                    {std::begin(map)->second} -> std::convertible_to<MR__stringType>; } &&
           (! // TODO: study why had to specify that the other overload does not match... it should be preferred as more specialized, isn't it ?
              (
                std::same_as<typename decltype(Expand::mapping)::key_type, EnumType> &&
                std::convertible_to<typename decltype(Expand::mapping)::mapped_type, MR__stringType>
              )
           ) 
  /*requires requires{ std::find_if( std::cbegin(Expand::mapping),
                                   std::cend(Expand::mapping),
                                   [](const auto& i) { return i.first == EnumType{}; })->second; }*/
  {
    if (const auto it = std::find_if( std::cbegin(Expand::mapping),
                                      std::cend(Expand::mapping),
                                      [v](const auto& i) { return i.first == v; });
        it != std::cend(Expand::mapping)) {
      return it->second;
    }else{
      return "";
    }
  }

  // implementation for maps
  template<typename EnumType, typename Expand>
  constexpr NEnum__MR::MR__stringType getMappedString(EnumType v)
  requires NConcept__MR::Iterable<decltype(Expand::mapping)> &&
           requires(decltype(Expand::mapping) map){ {std::begin(map)->first} -> std::convertible_to<EnumType>;
                                                    {std::begin(map)->second} -> std::convertible_to<MR__stringType>; } &&
           std::same_as<typename decltype(Expand::mapping)::key_type, EnumType> &&
           std::convertible_to<typename decltype(Expand::mapping)::mapped_type, MR__stringType>
  /*requires requires{ std::find_if( std::cbegin(Expand::mapping),
                                   std::cend(Expand::mapping),
                                   [](const auto& i) { return i.first == EnumType{}; })->second; }*/
  {
    if (const auto it = Expand::mapping.find(v); it != Expand::mapping.end()){
      return it->second;
    }else{
      return "";
    }
  }


  template<typename> class DEBUG;

  template<typename EnumType, typename Expand>
  std::pair<EnumType, bool> getValue(NEnum__MR::MR__stringType s){
    static std::map<NEnum__MR::MR__stringType, EnumType> invertMap;

    if (invertMap.empty()){
      for(const auto& pair : Expand::mapping){
        invertMap[NEnum__MR::MR__stringType{pair.second}] = pair.first;
      }
    }

    if (auto it = invertMap.find(s); it != invertMap.end()){
      return {it->second, true};
    } else{
      return {EnumType{}, false};
    }
  }

  #define MR__ENUM_GET_NAMEq(EnumType,E_V) \
    getMappedString< EnumType, MR__ENUM_MAP_EXPAND_NAME___(EnumType) >(E_V)
  #define MR__ENUM_GET_NAME(EnumType,V) MR__ENUM_GET_NAMEq(EnumType,EnumType::V)

  #define MR__ENUM_GET_VALUE(EnumType,s) \
    getValue< EnumType, MR__ENUM_MAP_EXPAND_NAME___(EnumType) >(NEnum__MR::MR__stringType{s})

  /*
  #define ENUM_GET_NAMEnoPrefix(EnumType,E_V,prefixToRemove) \
    std::string( ENUM_GET_NAMEq(EnumType,E_V) ) \
    .erase( 0, std::size(prefixToRemove)-!!std::is_array_v<std::remove_reference_t<decltype(prefixToRemove)>> )
      //avoid counting '\0' in case of char array.
      //std::string::erase is constexpr since C++20
  */

  template<typename EnumType>
  inline auto checkEnumToUnderline(EnumType e, std::underlying_type_t<EnumType> max_, const char* msgThrow = nullptr){
      using std::literals::string_literals::operator""s;
      bool ok = toUnderlying(e) < max_;
      if (!ok && msgThrow){
          throw std::string(msgThrow) + ": " + std::to_string(toUnderlying(e));
      }
      return std::make_pair(toUnderlying(e), ok);
  }
} // namespace NEnum__MR