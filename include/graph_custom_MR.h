//(_14_)
#pragma once

#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string_view>
#include <utility>

#include "concept_custom_MR.h"
#include "container_custom_MR.h"
#include "hash_custom_MR.h" // for the hash on pairs / iterators
#include "string_custom_MR.h" // for the hash on pairs

namespace NGraph__MR{
  using NodeOrder = std::size_t;

  enum class GraphKind {Direct, Indirect};

  template<typename UnaryPredicate, typename Graph>
  concept IsUnaryPredForNode =
          NConcept__MR::IsUnaryPredFor< UnaryPredicate, typename Graph::Node > ||
          NConcept__MR::IsUnaryPredFor< UnaryPredicate, typename Graph::NodeIt >;

  template< typename Graph, typename > // might use IsUnaryPredForNode<Graph> for second parameter
  struct UnaryPredicateInfo;

  template<typename Graph, NConcept__MR::IsUnaryPredFor<typename Graph::Node> UnaryPredicate>
  struct UnaryPredicateInfo<Graph, UnaryPredicate>{
    using NodeTypeParam = typename Graph::Node;
  };

  template<typename Graph, NConcept__MR::IsUnaryPredFor<typename Graph::NodeIt> UnaryPredicate>
  struct UnaryPredicateInfo<Graph, UnaryPredicate>{
    using NodeTypeParam = typename Graph::NodeIt;
  };

  template<typename Graph, typename T>
  using UnaryPredicateParamType = typename UnaryPredicateInfo<Graph,T>::NodeTypeParam;  


  //template<typename BinaryPredicate, typename Type>
  //concept IsBinaryPredFor = std::convertible_to<std::invoke_result_t<BinaryPredicate,Type,Type>, bool>;

  template<typename BinaryPredicate, typename Graph>
  concept IsBinaryPredForNode =
          NConcept__MR::IsBinaryPredFor< BinaryPredicate, typename Graph::Node > ||
          NConcept__MR::IsBinaryPredFor< BinaryPredicate, typename Graph::NodeIt >;

  template< typename Graph, typename > // might use IsBinaryPredForNode<Graph> for second parameter
  struct BinaryPredicateInfo;

  template<typename Graph, NConcept__MR::IsBinaryPredFor<typename Graph::Node> UnaryPredicate>
  struct BinaryPredicateInfo<Graph, UnaryPredicate>{
    using NodeTypeParam = typename Graph::Node;
  };

  template<typename Graph, NConcept__MR::IsBinaryPredFor<typename Graph::NodeIt> UnaryPredicate>
  struct BinaryPredicateInfo<Graph, UnaryPredicate>{
    using NodeTypeParam = typename Graph::NodeIt;
  };

  template<typename Graph, typename T>
  using BinaryPredicateParamType = typename BinaryPredicateInfo<Graph,T>::NodeTypeParam;  


  template<typename NodeParamType, typename Graph>
  concept IsNodeType = (std::convertible_to<NodeParamType,typename Graph::Node> || std::convertible_to<NodeParamType,typename Graph::NodeIt>);

  template<typename EdgeParamType, typename Graph>
  concept IsEdgeType = (std::convertible_to<EdgeParamType,typename Graph::Edge> || std::convertible_to<EdgeParamType,typename Graph::EdgeIt>);

  // TODO: tried to check speed -> transform in Graph template parameters.
  template<typename... Ts>
  using GraphSetImpl = std::unordered_set<Ts...>; //std::set<Ts...>;
  template<typename... Ts>
  using GraphMapImpl = std::unordered_map<Ts...>; //std::map<Ts...>;
  
  template< NConcept__MR::Hashable NodeType,
            GraphKind Kind,
            NConcept__MR::SummAndOrdinable EdgeWeight = std::size_t,
            EdgeWeight defaultWeight = 1U,
            bool AncestorAndDescending = false >
  requires NConcept__MR::Hashable<typename GraphSetImpl<NodeType>::const_iterator>
  class Graph final{
  public:
    using Node = NodeType;
    using NodeSet = GraphSetImpl<Node>;
    using NodeIt = NodeSet::const_iterator;
    using NodeSetRef = GraphSetImpl<NodeIt>;

    template<typename Eval>
    using MapEvalAndLeadingSources = GraphMapImpl<NodeIt, std::pair<Eval, NodeSetRef> >;
    // 'leading sources' -> the ones that led to the evaluation.

    using Edge = std::pair<Node,Node>;
    using EdgeIt = std::pair<NodeIt,NodeIt>;

    Graph(){
      if constexpr(isDirect()){
        withPredecessors = std::make_unique<ConnectionMap>(); 
      }

      if constexpr(AncestorAndDescending){
        allDescending = std::make_unique<ConnectionMapUnweighted>();
        if constexpr(isDirect()){
          allAncestors = std::make_unique<ConnectionMapUnweighted>();          
        }
      }
    }

    Graph(const Graph& other){
      *this = other;
    }

    //cannot really move, as iterators must persist during operations.
    //Graph(Graph&& other) ...;

    Graph& operator=(const Graph& other){ // TODO: still to test.
      if (this == &other) return *this;

      nodes = other.nodes;

      auto connectedWRef = std::ref(withSuccessors);
      auto otherConnectedWRef = std::cref(other.withSuccessors);

      for(int k = 0; k < 2; ++k){
        auto& connectedRef = connectedWRef.get();
        const auto& otherConnectedRef = otherConnectedWRef.get();

        connectedRef.clear();

        for( const auto& [otherNodeIt, nextMap] : otherConnectedRef){
          auto myNodeIt = refNode(*otherNodeIt);

          auto& myConnectedMap = connectedRef[myNodeIt];

          for(auto [otherNearNodeIt, edgeWeight] : nextMap){
            auto myNearNodeIt = refNode(*otherNearNodeIt);
            myConnectedMap[myNearNodeIt] = edgeWeight;
          }
        }

        // otherConnectedRef.clear(); would be done in a move operation.

        if constexpr(isDirect()){
          connectedWRef = std::ref(*withPredecessors);
          otherConnectedWRef = std::cref(*(other.withPredecessors));
        } else break;
      }

      if constexpr(AncestorAndDescending){
        auto allConnectedWRef = std::ref(*allDescending);
        auto otherAllConnectedWRef = std::cref(*(other.allDescending));

        allConnectedWRef.clear();

        for(int k = 0; k < 2; ++k){
          auto& allConnectedRef = allConnectedWRef.get();
          const auto& otherAllConnectedRef = otherAllConnectedWRef.get();

          for( const auto& [otherNodeIt, allSet] : otherAllConnectedWRef){
            auto myNodeIt = refNode(*otherNodeIt);

            auto& myAllSet = allConnectedRef[myNodeIt];

            for(auto otherFarNodeIt : allSet){
              auto myFarNodeIt = refNode(*otherFarNodeIt);
              myAllSet.insert(myFarNodeIt);
            }
          }

          //otherAllConnectedWRef.clear(); would be done in a move operation.

          if constexpr(isDirect()){//cannot really move, as iterators mustpersist during operations.
            auto allConnectedWRef = std::ref(*allAncestors);
            auto otherAllConnectedWRef = std::cref(*(other.allAncestors));
          } else break;
        }
      }

      // other nodes.clear(); would be done in a move operation.

      return *this;
    }

    //cannot really move, as iterators mustpersist during operations.
    //Graph& operator=(Graph&& other) = delete;

    constexpr static bool isDirect() {return Kind == GraphKind::Direct;}

    constexpr static bool isIndirect() {return Kind != GraphKind::Direct;}

    decltype(auto) operator<<(const Node& node){
      static_cast<void>(insertNode(Node{node}));
      // no problem if node already exists.

      return (*this);
    }

    void addNode(const Node& node){
      static_cast<void>((*this) << node);
    }

    bool removeNode(const Node& node){
      return deleteNode(node, true);
    }

    NodeIt begin() const{
      return nodes.cbegin();
    }

    NodeIt cbegin() const{
      return nodes.cbegin();
    }

    NodeIt end() const{
      return nodes.cend();
    }

    NodeIt cend() const{
      return nodes.cend();
    }

    NodeIt refNode(const Node& node) const{
      return nodes.find(node);
    }

    // to compare with refNode().
    NodeIt endNodes() const{
      return nodes.cend();
    }

    Graph& operator<<(Edge edge){
      if (!addEdge(std::move(edge))){
        throw std::invalid_argument("edge already present!");
        // the weight has to be updated with resetEdgeWeight.
      }
      return *this;
    }

    // false if already present.
    bool addEdge(Edge edge){
      return addEdge(std::move(edge), defaultWeight);
    }

    // false if already present.
    bool addEdge(Edge edge, const EdgeWeight& edgeWeight){
      return addEdge(std::move(edge.first), std::move(edge.second), edgeWeight);
    }
    // false if not present.
    bool resetEdgeWeight(const Edge& edge, const EdgeWeight& edgeWeight){
      return resetEdgeWeight(edge.first, edge.second, edgeWeight);
    }

    // false if already present.
    bool addEdge(EdgeIt edge, const EdgeWeight& edgeWeight){
      return addEdge(edge.first, edge.second, edgeWeight);
    }
    // false if not present.
    bool resetEdgeWeight(EdgeIt edge, const EdgeWeight& edgeWeight){
      return resetEdgeWeight(edge.first, edge.second, edgeWeight);
    }

    // This method allows to use references for Nodes, in case they are heavy objects.
    // Users may exploit instead of Edge.
    template<typename Node1, typename Node2>
    requires std::same_as<std::decay_t<Node1>,Node> &&
             std::same_as<std::decay_t<Node2>,Node>
    // false if already present.
    bool addEdge(Node1 node1, Node2 node2, const EdgeWeight& edgeWeight = defaultWeight){
      if (node1 == node2){
        throw std::invalid_argument("Self-edge are not allowed");
      }

      if (!insertEdge(std::forward<Node>(node1), std::forward<Node>(node2), edgeWeight)) return false;
      // insertEdge takes care of adding also the reverse in case of isIndirect().

      return true;
    }
    // false if not present.
    bool resetEdgeWeight(const Node& node1, const Node& node2, const EdgeWeight& edgeWeight = defaultWeight){
      return resetEdgeWeight(refNode(node1),refNode(node2));
    }

    // false if already present.
    bool addEdge(NodeIt itSource, NodeIt itDest, const EdgeWeight& edgeWeight){
      if (itSource != refNode(*itSource)) return false; // in case it is an iterator to another graph
      if (itDest != refNode(*itDest)) return false;

      if (itSource == itDest){
        throw std::invalid_argument("Self-edge are not allowed");
      }

      return insertEdge(itSource, itDest, edgeWeight);
    }
    template< IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 >
    // false if not present.
    bool resetEdgeWeight(NodeParamType1 sourceParam, NodeParamType2 destParam, const EdgeWeight& edgeWeight){
      auto [itSource, existsSource] = internalGetItNode<NodeParamType1>(sourceParam);
      auto [itDest, existsDest] = internalGetItNode<NodeParamType2>(destParam);

      if (!(existsSource && existsDest)) return false;

      if ( auto it = withSuccessors.find(itSource);
           it != withSuccessors.end() ){
        auto& succMap = it->second;
        if ( auto itSucc = succMap.find(itDest); // in case it is an iterator to another graph
             itSucc != succMap.end() ){
          itSucc->second = edgeWeight;

          auto& mapPrev = isDirect() ? *withPredecessors : withSuccessors;

          mapPrev[itDest][itSource] = edgeWeight;

          return true;
        }
      }

      return false;
    }

    bool removeEdge(NodeIt itSource, NodeIt itDest){
      return internalDeleteEdge(itSource, itDest);
    }

    bool removeEdge(const Node& source, const Node& dest){
      return internalDeleteEdge(refNode(source), refNode(dest));
    }

    std::size_t nodeNumber() const noexcept{return nodes.size();}

    bool empty() const {return !nodeNumber();};

    std::size_t edgeNumber() const noexcept{
      return sumMapValues<GraphMapImpl>(withSuccessors, [](const auto& nodeSet){return nodeSet.size();} );
    }

    auto getNodesRef() const {
      GraphSetImpl<NodeIt> nodeSet;

      for(auto it = nodes.begin(); it != nodes.end(); ++it){
        nodeSet.insert(it);
      }

      return nodeSet;
    }

    auto isolatedNodeSet() const {
      GraphSetImpl<NodeIt> nodeSet;

      for(auto it = nodes.begin(); it != nodes.end(); ++it){
        if (!withSuccessors.contains(it)){
          nodeSet.insert(it);
        }
      }

      return nodeSet;
    }

    auto edgeSet() const {
      GraphSetImpl<EdgeIt> edgeSet;

      for( auto& [nodeIt, succSet] : withSuccessors){
        for( auto [succIt, edgeW_] : succSet){
          edgeSet.insert({nodeIt, succIt});
        }
      }

      return edgeSet;
    }

    bool edgeExists(NodeIt source, NodeIt dest, bool alsoOppositeConnection = false) const noexcept{
      if (auto itS = withSuccessors.find(source); itS != withSuccessors.end()){
        return itS->second.contains(dest);
      }
      if (alsoOppositeConnection){
        if constexpr (isDirect()){ // else: useless to check.
          if (auto itS = withSuccessors.find(dest); itS != withSuccessors.end()){
            return itS->second.contains(source);
          }
        }
      }
      return false;
    }

    auto edgeSetFrom(NodeIt itSource) const {
      return internalEdgeSet(itSource,true); // forward
    }

    auto edgeSetTo(NodeIt itDest) const {
      return internalEdgeSet(itDest,false); // backward
    }

    auto edgeSetFrom(const Node& source) const {
      return internalEdgeSet(refNode(source),true); // forward
    }

    auto edgeSetTo(const Node& dest) const {
      return internalEdgeSet(refNode(dest),false); // backward
    }

    bool hasEdge(const Node& source, const Node& dest) const{
      if(auto it = withSuccessors.find(refNode(source)); it != withSuccessors.end()){
        return it->second.contains(refNode(dest));
      }
      return false;
    }


    template<IsNodeType<Graph> NodeParamType = NodeIt>
    void implode(const GraphSetImpl<NodeParamType>& forceKeepNodes) {
      // erase nodes with 1 predecessor and 1 successor (2 in case of Indirect), and connect them.
      for( auto itSuccMap = withSuccessors.begin(); itSuccMap != withSuccessors.end(); ){
        const auto& succSet = itSuccMap->second;
        const auto nodeIt = itSuccMap->first;

        bool remove = false;
        const auto [itSucc, edgeSuccWeight] = *succSet.begin(); // for sure not empty (otherwise key always removed)

        if ( (succSet.size() == 1U + !!isIndirect()) && !checkNodeSetContains(forceKeepNodes,nodeIt) ){
          if constexpr(isDirect()){ //else: predecessors not created at all.
            if( const auto itPrevMap = withPredecessors->find(nodeIt);
                itPrevMap != withPredecessors->end() ){
              const auto& prevSet = itPrevMap->second;
              if (prevSet.size() == 1U){
                // may remove if the two connected nodes are also not the same node,
                // as otherwise there would be a self-link edge.

                if ( const auto itPrev = prevSet.begin()->first;
                     itPrev != itSucc ){
                  remove = true;

                  // create connection:
                  addEdge( itPrev, itSucc,
                           prevSet.begin()->second + edgeSuccWeight );
                }
              }
            }
          } else{ // isIndirect
            // may remove if the two connected nodes are also not the same node,
            // nor already connected each other.
            const auto itPrev = succSet->rbegin()->first;

            if ( (itPrev != itSucc) && //(_8_)
                 !withSuccessors[itPrev].contains(itSucc) ){
              remove = true;

              addEdge( itPrev, itSucc,
                       succSet->rbegin()->second + edgeSuccWeight );
            }
          }
        }

        ++itSuccMap; // before removing the node.
        if (remove){
          deleteNode(nodeIt,false); // false: don't clear known loops.
        }
      }
    }


    template<IsNodeType<Graph> T>
    using NodeIdemExtractor = std::function<T(T)>;

    template< NConcept__MR::Iterable NodeInfoContainer,
              typename NodeExtractor = NodeIdemExtractor<NConcept__MR::IterValueType<NodeInfoContainer>> >
    requires IsNodeType< std::invoke_result_t< NodeExtractor, NConcept__MR::IterValueType<NodeInfoContainer> >, Graph >
    //requires(NodeInfoContainer cont, NodeExtractor extr){ {extr(*(cont.cbegin()))} -> IsNodeType;}
    Graph buildSubGraph( const NodeInfoContainer& nodeInfoContainer,
                         NodeExtractor&& nodeExtractor =
                                         NodeIdemExtractor<NConcept__MR::IterValueType<NodeInfoContainer>>(NConcept__MR::idemExtractor) ) const{
      Graph subGraph;

      /* cannot ignore NodeExtractor, it might transform the node-info
      if constexpr(std::same_as<NodeSet,NodeInfoContainer>){ // speed-up
        // ignores NodeExtractor
        if (nodes == this->nodes){
          subGraph = *this;
          return subGraph;
        }
      } */

      NodeSetRef nodeItSubset;
      for(const auto& nodeInfo: nodeInfoContainer){
        auto [nodeIt, exists] = internalGetItNode(nodeExtractor(nodeInfo));

        if (exists){ // else: not a node of this Graph.
          nodeItSubset.insert(nodeIt);
        }
      }

      for(auto nodeIt : nodeItSubset){
        subGraph << *nodeIt;

        if( const auto itSuccSet = withSuccessors.find(nodeIt);
            itSuccSet != withSuccessors.end() ){
          for (const auto& [itSucc, weightedEdge] : itSuccSet->second){
            if (nodeItSubset.contains(itSucc)){
              // need to use Node type, as nodeIt is an iterator to *this, not
              // to subGraph, and *itSucc might not yet be part of subGraph.
              subGraph.addEdge(*nodeIt, *itSucc, weightedEdge);
              // will generate a couple of edges for subGraph in case of Indirect.
              // Therefore, uselessly added twice for indirect.
            }
          }
        }

        if constexpr(isDirect()){ //else: predecessors not created at all.
          if( const auto itPrevSet = withPredecessors->find(nodeIt);
              itPrevSet != withPredecessors->end() ){
            for (const auto& [itPrev, weightedEdge] : itPrevSet->second){
              if (nodeItSubset.contains(itPrev)){
                subGraph.addEdge(*itPrev, *nodeIt, weightedEdge);
              }
            }
          }
        }
      }

      return subGraph;
    }

    /* Graph buildSubGraph(const NodeSetRef& nodesRef) const{
      return buildSubGraph(nodesRef, [](const auto nodeIt){return *nodeIt;});
    } */

    inline static const auto predAnyNode = [](const Node& node){return true;};
    inline static const auto predAnyNodeIt = [](const NodeIt& nodeIt){return true;};
    inline static const auto predAnyEdge = [](const Node& node1, const Node& node2){return true;};
    inline static const auto predAnyEdgeIt = [](const NodeIt& nodeIt1, const NodeIt& nodeIt2){return true;};


    template<IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt))>
    Graph buildSubGraph(UnaryPredicate&& predNode = std::function(predAnyNodeIt)) const{
      NodeSetRef filteredNodes;
      for(auto nodeIt = nodes.cbegin(); nodeIt != nodes.cend(); ++nodeIt){
        if (predNode(internalGetNodeFromIt<UnaryPredicateParamType<Graph, UnaryPredicate>>(nodeIt))){
          filteredNodes.insert(nodeIt);
        }
      }

      return buildSubGraph(filteredNodes);
    }

    // The case keepAllNodes==true is the only one able to keep isolated nodes.
    template<IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdge))>
    Graph buildSubGraph( BinaryPredicate&& predFilterEdges = std::function(predAnyEdge),
                         bool keepAllNodes = false ) const{
      Graph subGraph;

      if (keepAllNodes){
        subGraph.nodes = nodes;
      }

      for(const auto& [nodeIt, itSuccSet] : withSuccessors){
        for(const auto& [nodeIt2, edgeWeight] : itSuccSet){
          if (checkPredEdge(std::forward<BinaryPredicate>(predFilterEdges), nodeIt, nodeIt2)){
            subGraph.addEdge(*nodeIt, *nodeIt2, edgeWeight); // nodes and edge
          }
        }
      }

      return subGraph;
    }


    template<IsNodeType<Graph> NodeParamType = NodeIt, IsEdgeType<Graph> EdgeParamType = EdgeIt>
    auto topologicalOrderForDirect( const GraphSetImpl<NodeParamType>& filterNodes,
                                    bool edgesAreToKeep = false,
                                    const GraphSetImpl<EdgeParamType>& filterEdges = {} ) {
      using NodeTypeInEdge = typename EdgeParamType::first_type;
      return topologicalOrderForDirect(
        [&filterNodes = std::as_const(filterNodes)](const NodeParamType& nodeParam){return filterNodes.contains(nodeParam);},
        [&filterEdges = std::as_const(filterEdges), edgesAreToKeep]
          (const NodeTypeInEdge& source, const NodeTypeInEdge& dest)
          {return edgesAreToKeep == filterEdges.contains(EdgeParamType{source,dest});} );
    }

    // resulting references are invalid in case the graph is removed a node,
    // whereas the order is invalidated in case nodes are added or removed.
    // Filters determine a sub-graph on which the order is computed.
    template< IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    // Second element false: there is a loop.
    // Third element: true if the overall node ordering is unique. If false, the position
    //                of any node may be unique (if it may reach/observe any other node).
    std::tuple<std::vector<NodeIt>, bool, bool >
    topologicalOrderForDirect( UnaryPredicate &&predFilterNodes = predAnyNodeIt,
                               BinaryPredicate &&predFilterEdges = predAnyEdgeIt ){
      if constexpr(isIndirect()){
        throw std::runtime_error("topologicalOrderForDirect invoked on an indirect graph");
      }

      return internalTopologicalOrder( nodes.cend(), // (_14_)
                                       std::forward<UnaryPredicate>(predFilterNodes),
                                       std::forward<BinaryPredicate>(predFilterEdges) );
    }

    //(_12_)
    // Differently from topologicalOrderForDirect, the starting node is considered the
    // first and the graph must be fully connected otherwise the bool is false as if
    // there is a loop (for an indirect this means a couple of nodes with
    // multiple paths to reach each other). Indeed, in case of non-connection, there is
    // no way to choose the first node in each connected region (it would be possible
    // only in case of isolated nodes, and for simplicity this is not managed).
    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    std::pair<std::vector<NodeIt>, bool>
    topologicalOrderForIndirect( const NodeParamType& startNodeParam, UnaryPredicate &&predFilterNodes = predAnyNodeIt,
                                 BinaryPredicate &&predFilterEdges = predAnyEdgeIt ) {
      if constexpr(isDirect()){
        throw std::runtime_error("topologicalOrderForIndirect invoked on a direct graph");
      }

      auto [startNodeIt, exists] = internalGetItNode<NodeParamType>(startNodeParam);

      if (!exists){
        return { {}, false, false };
      }

      auto [ordering, noLoop, unique_] =
        // for indirect, unique_ cannot be false when noLoop = true.
        internalTopologicalOrder( startNodeIt,
                                  std::forward<UnaryPredicate>(predFilterNodes),
                                  std::forward<BinaryPredicate>(predFilterEdges) );

      return {std::move(ordering), noLoop }; // here noLoop is <-> noLoop && fullyConnected -> read (__) ??? [TODO: refer the point]
    }

    // For direct graphs, need to have couples of edges.
    NodeSetRef getMaxClique() const{
      return *internalMaxCliques(false).begin();
    }

    std::list<NodeSetRef> getMaxCliques(){
      return internalMaxCliques(true);
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 = NodeIt>
    auto countReachablesFrom( const NodeParamType1& startNodeParam, const GraphSetImpl<NodeParamType2>& destNodes ){
      return countReachablesFrom( startNodeParam, [&destNodes = std::as_const(destNodes), this](const NodeIt& nodeIt)
                                                  {return destNodes.contains(internalGetNodeFromIt<NodeParamType2>(nodeIt));} );
                                                  // no need to check nodeIt, as it comes from private methods, scrolling existing nodes.
    }

    // In case of predicates with Node, the default argument does not work.
    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    std::pair<std::size_t,bool> countReachablesFrom( const NodeParamType& startNodeParam,
                                                     UnaryPredicate &&predDestNodes = std::function(predAnyNodeIt),
                                                     BinaryPredicate &&predFilterEdges = std::function(predAnyEdgeIt) ){
      auto res = getReachablesFrom( startNodeParam, std::forward<UnaryPredicate>(predDestNodes),
                                    std::forward<BinaryPredicate>(predFilterEdges) );

      return {res.first.size(), res.second}; // not that bad to uselessly get first the set of reachables.
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 = NodeIt>
    auto countObservablesTo( const NodeParamType1& endNodeParam, const GraphSetImpl<NodeParamType2>& sourceNodes ){
      return countObservablesTo( endNodeParam, [&sourceNodes = std::as_const(sourceNodes), this](NodeIt nodeIt)
                                               {return sourceNodes.contains(internalGetNodeFromIt<NodeParamType2>(nodeIt));} );
                                               // no need to check nodeIt, as it comes from private methods, scrolling existing nodes.
    }

    // In case of predicates with Node, the default argument does not work.
    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    std::pair<std::size_t,bool> countObservablesTo( const NodeParamType& endNodeParam,
                                                    UnaryPredicate &&predSourceNodes = std::function(predAnyNodeIt),
                                                    BinaryPredicate &&predFilterEdges = std::function(predAnyEdgeIt) ){
      auto res = getObservablesTo( endNodeParam, std::forward<UnaryPredicate>(predSourceNodes),
                                   std::forward<BinaryPredicate>(predFilterEdges) );

      return {res.first.size(), res.second}; // not that bad to uselessly get first the set of reachables.
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2>
    auto getReachablesFrom( const NodeParamType1& startNodeParam, const GraphSetImpl<NodeParamType2>& destNodes ){
      return getReachablesFrom( startNodeParam, [&destNodes = std::as_const(destNodes), this](NodeIt nodeIt)
                                                {return destNodes.contains(internalGetNodeFromIt<NodeParamType2>(nodeIt));} );
                                                // no need to check nodeIt, as it comes from private methods, scrolling existing nodes.
    }

    // if second false, there is a loop (the opposite is not true),
    // but in any case it returns the number of reachable nodes (satisfying the predicate).
    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    std::pair<NodeSetRef,bool> getReachablesFrom( const NodeParamType& startNodeParam,
                                                  UnaryPredicate &&predDestNodes = std::function(predAnyNodeIt),
                                                  BinaryPredicate &&predFilterEdges = std::function(predAnyEdgeIt) ){
      return internalReachables( startNodeParam, std::forward<UnaryPredicate>(predDestNodes),
                                 std::forward<BinaryPredicate>(predFilterEdges), true ); // forward
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 = NodeIt>
    auto getObservablesTo( const NodeParamType1& endNodeParam, const GraphSetImpl<NodeParamType2>& sourceNodes ){
      return getObservablesTo( endNodeParam, [&sourceNodes = std::as_const(sourceNodes), this](NodeIt nodeIt)
                                             {return sourceNodes.contains(internalGetNodeFromIt<NodeParamType2>(nodeIt));} );
                                             // no need to check nodeIt, as it comes from private methods, scrolling existing nodes.
    }

    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    std::pair<NodeSetRef,bool> getObservablesTo( const NodeParamType& endNodeParam,
                                                 UnaryPredicate &&predSourceNodes = std::function(predAnyNodeIt),
                                                 BinaryPredicate &&predFilterEdges = std::function(predAnyEdgeIt) ){
      return internalReachables( endNodeParam, std::forward<UnaryPredicate>(predSourceNodes),
                                 std::forward<BinaryPredicate>(predFilterEdges), false ); // backward
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 = NodeIt>
    auto getConnectedNodes( const NodeParamType1& nodeParam, const GraphSetImpl<NodeParamType2>& connNodes ){
      return getConnectedNodes( nodeParam, [&connNodes = std::as_const(connNodes), this](NodeIt nodeIt)
                                           {return connNodes.contains(internalGetNodeFromIt<NodeParamType2>(nodeIt));} );
                                           // no need to check nodeIt, as it comes from private methods, scrolling existing nodes.
    }

    // reachables and observables.
    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    std::pair<NodeSetRef,bool> getConnectedNodes( const NodeParamType& nodeParam,
                                                  UnaryPredicate &&predConnNodes = std::function(predAnyNodeIt),
                                                  BinaryPredicate &&predFilterEdges = std::function(predAnyEdgeIt) ){
      auto [reach, noLoop1] = getReachablesFrom(nodeParam, std::forward<UnaryPredicate>(predConnNodes), std::forward<BinaryPredicate>(predFilterEdges) );
      auto [observ, noLoop2] = getReachablesFrom(nodeParam, std::forward<UnaryPredicate>(predConnNodes), std::forward<BinaryPredicate>(predFilterEdges));
      reach.merge(observ);
      return std::make_pair( std::move(reach), noLoop1 && noLoop2 );
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 = NodeIt>
    auto distancesFrom( const NodeParamType1& endNodeParam, const GraphSetImpl<NodeParamType2>& sourceNodes ){
      return distancesFrom( endNodeParam, [&sourceNodes = std::as_const(sourceNodes), this](NodeIt nodeIt)
                                          {return sourceNodes.contains(internalGetNodeFromIt<NodeParamType2>(nodeIt));} );
    }

    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    GraphMapImpl<NodeIt,EdgeWeight> distancesFrom(
                                    const NodeParamType& startNodeParam, UnaryPredicate &&predDestNodes = std::function(predAnyNodeIt),
                                    BinaryPredicate &&predFilterEdges = std::function(predAnyEdgeIt) ){
      return internalDistances( startNodeParam,
                                std::forward<UnaryPredicate>(predDestNodes),
                                std::forward<BinaryPredicate>(predFilterEdges), true ); // forward
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 = NodeIt>
    auto distancesTo( const NodeParamType1& endNodeParam, const GraphSetImpl<NodeParamType2>& destNodes ){
      return distancesTo( endNodeParam, [&destNodes = std::as_const(destNodes), this](NodeIt nodeIt)
                                        {return destNodes.contains(internalGetNodeFromIt<NodeParamType2>(nodeIt));} );
    }

    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    GraphMapImpl<NodeIt,EdgeWeight> distancesTo(
                                    const NodeParamType& endNodeParam,
                                    UnaryPredicate &&predSourceNodes = std::function(predAnyNodeIt),
                                    BinaryPredicate &&predFilterEdges = std::function(predAnyEdgeIt) ){
      return internalDistances( endNodeParam,
                                std::forward<UnaryPredicate>(predSourceNodes),
                                std::forward<BinaryPredicate>(predFilterEdges), false ); // backward
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 = NodeIt>
    auto countPathsFrom( const NodeParamType1& startNodeParam, const GraphSetImpl<NodeParamType2>& destNodes, bool excludeLoops = false ){
      return countPathsFrom( startNodeParam, [&destNodes = std::as_const(destNodes), this](const NodeIt& nodeIt)
                                             {return destNodes.contains(internalGetNodeFromIt<NodeParamType2>(nodeIt).first);},
                                             // no need to check nodeIt, as it comes from private methods, scrolling existing nodes.
                             predAnyEdge, excludeLoops );
    }

    // if second false, there is a loop (the opposite is also true), and in this case the couple {0, false} is returned
    // unless excludeLoop is true (then, paths with any loop are not counted... they would be infinite).
    // However, second is true (even in case of existing loops) if the passed node does not exists.
    template< IsNodeType<Graph> NodeParamType,
              typename UnaryPredicate = decltype(std::function(predAnyNode)),
              typename BinaryPredicate = decltype(std::function(predAnyEdge)) >
    std::pair<std::size_t,bool> countPathsFrom( const NodeParamType& startNodeParam, UnaryPredicate &&predDestNodes = predAnyNode,
                                                BinaryPredicate &&predFilterEdges = std::function(predAnyEdge),
                                                bool excludeLoops = false ){
      auto [startNodeIt, exists] = internalGetItNode<NodeParamType>(startNodeParam);
      if (!exists){
        return {0U, true};
      }

      auto [map_, val, noLoop] =
           internalVisit<std::size_t>( startNodeIt,
                                       std::forward<UnaryPredicate>(predDestNodes),
                                       std::forward<BinaryPredicate>(predFilterEdges),
                                       1U, // startNodeValue
                                       [](const std::size_t sourceEval, const EdgeWeight&){return sourceEval;}, // eval-computer
                                       [](std::size_t& v1, const std::size_t v2 ) { // eval-combiner
                                         if (v1 > std::numeric_limits<std::size_t>::max() - v2){
                                           throw std::runtime_error(NString__MR::getString("", "Overflow evaluation: ", v1, ", ", v2));
                                         }
                                         v1 += v2;
                                         return std::make_pair(true,false); // propagate and don't reset contributers
                                       }, 0U, sumEval2, // final-seed, final-combiner
                                       !excludeLoops, // stopOnLoops = !excludeLoops
                                       false ); // => don't get paths

      if (excludeLoops && !noLoop){
        return {0U, false};
      } // else:

      return {std::size_t(val), noLoop};
    }


    template<IsNodeType<Graph> NodeParamType1, IsNodeType<Graph> NodeParamType2 = NodeIt, IsEdgeType<Graph> EdgeParamType = EdgeIt>
    auto findMinPath( const NodeParamType1& startNodeParam, bool getPaths,
                      const GraphSetImpl<NodeParamType2> &destNodes, bool nodeSetToKeep = true,
                      const GraphSetImpl<EdgeParamType> &edges = {}, bool edgeSetToAvoid = true ){

      GraphSetImpl<NodeIt> destNodesItLocal;
      auto refDestNodesToUse = std::cref(destNodesItLocal);
      if constexpr(std::convertible_to<NodeParamType2,Node>){
        for(const NodeParamType2& node : destNodes){
          if (auto [nodeIt, exists] = internalGetItNode<NodeParamType2>(node,true); exists){
            destNodesItLocal.insert(nodeIt);
          }
        }
      } else{
        refDestNodesToUse = std::cref(destNodes);
      }

      GraphSetImpl<EdgeIt> destEdgesItLocal;
      auto refEdgesToUse = std::cref(destEdgesItLocal);
      if constexpr(std::convertible_to<EdgeParamType,Edge>){  
        for(const Edge& edge : edges){
          if ( auto nodeIt1 = this->refNode(edge.first), nodeIt2 = this->refNode(edge.second);
               (nodeIt1 != this->end()) && (nodeIt2 != this->end()) ){
            destEdgesItLocal.insert(EdgeIt{nodeIt1,nodeIt2});
          }
        }
      } else{
        refEdgesToUse = std::cref(edges);
      }

      return findMinPath( startNodeParam, getPaths,
                          [&refDestNodesToUse = std::as_const(refDestNodesToUse), nodeSetToKeep]
                            (NodeIt nodeIt)
                            {return refDestNodesToUse.get().contains(nodeIt) == nodeSetToKeep;},
                          [&refEdgesToUse = std::as_const(refEdgesToUse), edgeSetToAvoid]
                            (NodeIt itSource, NodeIt itDest)
                            {return refEdgesToUse.get().contains(EdgeIt{itSource,itDest}) != edgeSetToAvoid;} );
    }

    // false if no path.
    // NodeSetRef -> all possible nodes on best paths (for each of the ends [exclude any 'end' which is not a node]).
    template< IsNodeType<Graph> NodeParamType,
              IsUnaryPredForNode<Graph> UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              IsBinaryPredForNode<Graph> BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    std::tuple<EdgeWeight, GraphMapImpl<NodeIt,NodeSetRef>, bool>
    findMinPath( const NodeParamType& startNodeParam, bool getPaths = true,
                 UnaryPredicate &&predDestNodes = std::function(predAnyNodeIt),
                 BinaryPredicate &&predFilterEdges = std::function(predAnyEdgeIt) ){
      constexpr bool useDjikstra = true; // faster compared to the shared method.

      auto [startNodeIt, exists] = internalGetItNode<NodeParamType>(startNodeParam);
      if (!exists){
        return std::make_tuple( EdgeWeight{}, GraphMapImpl<NodeIt,NodeSetRef>{}, false );
      }

      bool mapHasAllReachableNodes;

      MapEvalAndLeadingSources<EdgeWeight> mapMinPaths;

      if constexpr(useDjikstra){ // Directly Djikstra.
        using NodeIdx = std::size_t;

        std::vector<NodeIt> its;
        GraphMapImpl<NodeIt, NodeIdx> idxs;
        
        const auto N = nodes.size();

        std::vector<EdgeWeight> dist(N, EdgeWeight{});
        std::vector<NodeSetRef> path(N, NodeSetRef{});
        std::vector<bool> visited(N, false);
        
        its.reserve(N);
        NodeIdx i = 0;
        for(auto it = nodes.begin(); it != nodes.end(); ++it ){
          idxs[it] = i;
          its[i++] = it;
        }

        const auto distCmp = [&dist = std::as_const(dist)](const auto idx1, const auto idx2)
        {return (dist[idx1] < dist[idx2]) || ((dist[idx1] == dist[idx2]) && (idx1 < idx2));};

        //std::set<NodeIdx, std::function<bool(NodeIdx,NodeIdx)>> propagated{
        //  std::function<bool(NodeIdx,NodeIdx)>(distCmp)};
        
        std::priority_queue<NodeIdx, std::vector<NodeIdx>, std::function<bool(NodeIdx,NodeIdx)>>
          // any order is ok, but better by increasing distances.
          propagated{std::function<bool(NodeIdx,NodeIdx)>(distCmp)};

        auto id_start = idxs[startNodeIt];
        visited[id_start] = true;
        //propagated.insert(id_start);
        propagated.push(id_start);

        while(!propagated.empty()){
          GraphSetImpl<NodeIdx> nextPropagated;

          //for(auto idxSource : propagated){
          while(!propagated.empty()){
            auto idxSource = propagated.top();
            propagated.pop();
          
            if ( auto itSuccMap = withSuccessors.find(its[idxSource]);
                 itSuccMap != withSuccessors.end() ){
              for(const auto& [itDest, edgeWeight] : itSuccMap->second){
                auto idxDest = idxs[itDest];
                auto newDist = dist[idxSource] + edgeWeight;

                if ((newDist < dist[idxDest]) || !visited[idxDest]){
                  if (getPaths){
                    path[idxDest].clear();
                    path[idxDest].insert(its[idxSource]);
                  }
                  dist[idxDest] = newDist;
                  visited[idxDest] = true;

                  nextPropagated.insert(idxDest);
                } else
                if (getPaths && (newDist == dist[idxDest])){
                  path[idxDest].insert(its[idxSource]);
                }
              }
            }
          }

          //propagated.clear();
          for(auto idx : nextPropagated){ // translate qhile distances are not changed.
            //propagated.insert(idx);
            propagated.push(idx);
          }
        }

        for (NodeIdx idx = 0; idx < visited.size(); ++idx){
          if (visited[idx]){
            auto it = its[idx];
            mapMinPaths[it] = std::make_pair( dist[idx], std::move(path[idx]) );
          }
        }

        mapHasAllReachableNodes = true;
      } else{

        auto [startNodeIt, exists] = internalGetItNode<NodeParamType>(startNodeParam);

        if (!exists){
          return {EdgeWeight{}, {}, false}; // false <-> no-path
        }

        mapHasAllReachableNodes = getPaths;
        auto [mapMinPaths2, maxMinPath_, noLoop_] =
          internalVisit<EdgeWeight>( startNodeIt,
                                     getPaths
                                     ? std::function(predAnyNodeIt) // In case of getPaths, need all paths in this first call, and then filter later.
                                     : std::forward<UnaryPredicate>(predDestNodes),
                                     std::forward<BinaryPredicate>(predFilterEdges),
                                     0U, // startNodeValue
                                     [](const EdgeWeight &sourceV, const EdgeWeight& edgeW){  // eval-computer
                                         auto res = sourceV + edgeW; return res; },
                                     [](EdgeWeight &v, const EdgeWeight &newV){ // eval-combiner
                                       if (newV >= v){
                                         return std::make_pair( false, // don't propagate
                                                                newV == v ); // update contributors if ==
                                       }
                                       v = newV;
                                       return std::make_pair( true, // propagate
                                                              true ); }, // pre-clear contributors
                                     std::numeric_limits<EdgeWeight>::max(), // final-seed
                                     [](const EdgeWeight &v1, const EdgeWeight &v2){ // final-combiner
                                       return std::min(v1,v2); },
                                     false, // => don't stop on loops
                                     getPaths );


        mapMinPaths = std::move(mapMinPaths2);
      }

      /*
      EdgeWeight minDistDifferent;
      EdgeWeight minDistAlg2;
      std::size_t iMin;
      bool any = false;
      if (getPaths){
        for(std::size_t i = 0; i < N; ++i)
          if (visited[i] && ( dist[i] !=
                              ( mapMinPaths.contains(its[i])
                                ? mapMinPaths[its[i]].first.first
                                : 0 ) )){
            if ((dist[i] < minDistDifferent) || !any){
              minDistDifferent = dist[i];
              minDistAlg2 = mapMinPaths[its[i]].first.first;
              iMin = i;
              any = true;
            }
          }
      }
      */

      GraphSetImpl<NodeIt> bestEnds;
      EdgeWeight minPathToAnyEnd{};
      bool noPathYetToEnd = true;
      
      for (auto& [itEnd, evalAndLeading] : mapMinPaths){
        if ( (!mapHasAllReachableNodes) || // speed-up
             checkPredNode(predDestNodes, itEnd) ){
          auto minPathEnd = evalAndLeading.first;
          if (noPathYetToEnd || (minPathEnd < minPathToAnyEnd)){
            bestEnds.clear();
            minPathToAnyEnd = minPathEnd;
            noPathYetToEnd = false;
          } else
          if (minPathEnd > minPathToAnyEnd){
            continue;
          }

          bestEnds.insert(itEnd); // shared.
        }
      }

      GraphMapImpl<NodeIt,NodeSetRef> mapPaths;

      if (getPaths){
        // in order to build NodeSetRef, use another graph with successors corresponding to paths.
        // In this way, all nodes involved in the best path are stored.
        using GraphPath = Graph<NodeIt, GraphKind::Direct>;
        GraphPath graphBestPaths;
        for( const auto& [itReached, evalAndSources] : mapMinPaths){
          for (auto itSource : evalAndSources.second){
            graphBestPaths << typename GraphPath::Edge{itSource,itReached};
          }
        }

        for(auto itEnd : bestEnds){
          const auto& [onPath,noLoop_] = graphBestPaths.getObservablesTo(*(graphBestPaths.refNode(itEnd)));

          auto& setResForEnd = mapPaths[itEnd];
          
          for (auto itToIt : onPath){
            setResForEnd.insert(*itToIt);
          }
        }
      }

      return {minPathToAnyEnd, mapPaths, !noPathYetToEnd};
    }

  private:
    using EdgeMapRef = GraphMapImpl<NodeIt, EdgeWeight>;
    using ConnectedSet = NodeSetRef;
    using ConnectionMap = GraphMapImpl<NodeIt,EdgeMapRef>;
    using ConnectionMapUnweighted = GraphMapImpl<NodeIt,NodeSetRef>;

    NodeSet nodes;
    ConnectionMap withSuccessors;
    std::unique_ptr<ConnectionMap> withPredecessors{nullptr}; // not created in case Kind != GraphKind::Direct.
    std::unique_ptr<ConnectionMapUnweighted> allAncestors{nullptr}, allDescending{nullptr}; // to support noLoopNodes and loopNodes

    NodeSetRef noLoopNodes; // nodes for which it is already known there is no loop involving each.
    NodeSetRef loopNodes; // nodes for which it is already known there is a loop involving each.

    //same result of nodes.insert -> iterator and bool
    std::pair<NodeIt, bool> insertNode(Node&& node){
      const auto pair = nodes.emplace(std::forward<Node>(node));
      if constexpr(AncestorAndDescending){
        if (pair.second){
          (*allDescending)[pair.first];
          if constexpr(isDirect()){
            (*allAncestors)[pair.first];
          }
        }
      }

      return pair;
    }


    template<typename NodeParamType>
    bool deleteNode(NodeParamType nodeParam, bool clearKnownLoops){

      auto [nodeIt, exists] = internalGetItNode(nodeParam,true); // include check

      if (!exists) return false;

      auto reachedWRef = std::ref(withSuccessors);

      for(auto i = 0; i < 2; ++i){
        auto& reachedRef = reachedWRef.get();

        if (auto it = reachedRef.find(nodeIt); it != reachedRef.end()){
          const auto& reachSet = it->second;
          for(const auto [itDest, edgeWeight_] : reachSet){

            auto& reachedBackRef = (isDirect() && (i == 0)) //when i==1 reachedRef is on *withPredecessors
                                   ? *withPredecessors
                                   : withSuccessors;

            auto itPrevOfDest = reachedBackRef.find(itDest);
            if (itPrevOfDest == reachedBackRef.end()){
              throw std::runtime_error("error!");
            }
            auto& prevSetFromDest = itPrevOfDest->second;

            prevSetFromDest.erase(nodeIt);

            if (prevSetFromDest.empty()){
              reachedBackRef.erase(itPrevOfDest);
            }
          }
          
          reachedRef.erase(it);
        }

        if constexpr(isIndirect()){
          break;
        } else{
          reachedWRef = std::ref(*withPredecessors); // invert roles
        }
      }

      if constexpr(AncestorAndDescending){
        allDescending->erase(nodeIt);
        if constexpr(isDirect()){
          allAncestors->erase(nodeIt);
        }
      }

      if (clearKnownLoops){
        loopNodes.clear(); // simple decision
      } else{
        loopNodes.erase(nodeIt); // !clearKnownLoops used by implode() method
      }
      noLoopNodes.erase(nodeIt);

      nodes.erase(nodeIt);

      return true;
    }


    template<std::convertible_to<Node> Node1, std::convertible_to<Node> Node2>
    // requires repeated to distinguish from insertEdge with NodeIt.
    bool insertEdge(Node1 node1, Node2 node2, const EdgeWeight& edgeWeight){
      // This method hast to work also for const Node&, therefore
      // remove_reference_t is applied on Node.
      const auto [itF,_] = insertNode(std::move(node1));
      const auto [itS,__] = insertNode(std::move(node2));
      // no problem if nodes already exist.

      return insertEdge(itF, itS, edgeWeight);
    }


    bool insertEdge(NodeIt itSource, NodeIt itDest, const EdgeWeight& edgeWeight, bool myNestedCall = false){
      // edgeWeight not forwarded: still useful later for withPredecessors.
      if ( !withSuccessors[itSource].insert({itDest, edgeWeight}).second ){
        return false;
      }

      ConnectedSet ancF_old;
      ConnectedSet descS_old;

      if constexpr(isDirect()){ //else: predecessors not created at all.
        (*withPredecessors)[itDest].insert({itSource, edgeWeight});
      }

      if constexpr(AncestorAndDescending){
        descS_old = (*allDescending)[itDest];
        if constexpr(isDirect()){
          ancF_old = (*allAncestors)[itSource];
        }

        auto &descF = (*allDescending)[itSource];
        descF.insert(itDest);
        descF.merge(ConnectedSet{descS_old});

        if constexpr(isDirect()){
          auto &ancS = (*allAncestors)[itDest];
          ancS.insert(itSource);
          ancS.merge(ConnectedSet{ancF_old});
        }

        // all ancestors of itSource need to get itDest (and its descending) as descending, and vice-versa
        auto& connectedMap = (isDirect()) ? allAncestors : allDescending;
        // in case of !Direct, only descending exist (as they are logically equivalent)
        for(auto& ancOfF : (*connectedMap)[itSource]){
          auto& descOfAnc = (*allDescending)[ancOfF];
          descOfAnc.insert(itDest);
          descOfAnc.merge(ConnectedSet{descS_old});
          // it would be the same to use directly descF, but
          // this may contain further values already stored
          // as descending of the ancestors.
        }

        //... and vice-versa
        for(auto& descOfS : (*allDescending)[itDest]){
          auto& connectedMap = (isDirect()) ? allAncestors : allDescending;
          // in case of !Direct, only descending exist (as they are logically equivalent)
          auto& ancOfDesc = (*connectedMap)[descOfS];
          ancOfDesc.insert(itSource);
          ancOfDesc.merge(ConnectedSet{ancF_old});
        }
      }

      bool unchangeNoLoop = false;

      if constexpr(AncestorAndDescending){
        if (descS_old.contains(itSource)){ // both are loop, as well as their ancestors
          loopNodes.insert(itDest);
          // loopNodes.insert(itSource); already in descS_old

          loopNodes.merge(ConnectedSet{descS_old});
          //(_1_)a.
          // It is enough to add ancestors of the first,
          // that for sure now contain all the nodes
          // that can reach it, including the second and
          // all ancestors of second as well as of all
          // its descending. Moreover, ancF_old already
          // container all needed nodes.
        } else{
          unchangeNoLoop = true;
        }
      }

      if (!unchangeNoLoop){
        if constexpr(AncestorAndDescending){
          noLoopNodes.erase(itDest);
          //(_1_)b : enough to use descS_old, as in (_1_)a.
          reduceContainer(noLoopNodes, descS_old);
        } else{
          // easiest choice: remove for all. It should have been done
          // only for itSource and itDest ancestors.
          noLoopNodes.clear();
        }
      }

      if constexpr(isIndirect()){
        if (!myNestedCall){
          static_cast<void>(insertEdge(itDest, itSource, edgeWeight, true)); // invert roles
          //returns true.
        }
      }

      return true;
    }


    auto internalEdgeSet(NodeIt itBegin, bool forward) const{
      GraphSetImpl<EdgeIt> edgeSet;

      if (itBegin != endNodes()){
        const auto& connectedRef = (isDirect() && !forward)
                                   ? *withPredecessors
                                   : withSuccessors;

        if (auto itConn = connectedRef.find(itBegin); itConn != connectedRef.end()){
          for( auto [itOther, edgeW_] : itConn->second){
            if (forward){
              edgeSet.insert({itBegin, itOther});
            } else{
              edgeSet.insert({itOther, itBegin});
            }
          }
        }
      }

      return edgeSet;
    }


    bool internalDeleteEdge(NodeIt itSource, NodeIt itDest){

      auto itSuccMap = withSuccessors.find(itSource);
      if (itSuccMap == withSuccessors.end()) return false;

      auto& succMap = itSuccMap->second;
      auto itSucc = succMap.find(itDest);
      if (itSucc == succMap.end()) return false;

      succMap.erase(itSucc);
      if (succMap.empty()){
        withSuccessors.erase(itSuccMap);
      }

      {
        auto predecessorWRef = std::ref(withSuccessors);
        if constexpr(isDirect()){
          predecessorWRef = std::ref(*withPredecessors);
        }
        auto& predecessorRef = predecessorWRef.get();

        auto itPrevMap = predecessorRef.find(itDest);
        auto& prevMap = itPrevMap->second;

        prevMap.erase(itSource); // directly with the key.
        if (prevMap.empty()){
          predecessorRef.erase(itPrevMap);
        }
      }

      if constexpr(AncestorAndDescending){
        (*allDescending)[itSource].erase(itDest);
        if constexpr(isDirect()){
          (*allAncestors)[itDest].erase(itSource);
        }
      }

      loopNodes.clear(); // simple decision

      return true;
    }


    template<typename UnaryPredicate>
    static bool checkPredNode(UnaryPredicate&& predFilterNodes, NodeIt nodeIt){
      if constexpr(NConcept__MR::IsUnaryPredFor<UnaryPredicate,NodeIt>){
        return predFilterNodes(nodeIt);
      } else{ // (_3_)a: cannot use ternary operator... it would require to compile both options.
        return predFilterNodes(*nodeIt);
      }
    }

    template<typename BinaryPredicate>
    static bool checkPredEdge(BinaryPredicate&& predFilterEdges, NodeIt itSource, NodeIt itDest){
      if constexpr(NConcept__MR::IsBinaryPredFor<BinaryPredicate,NodeIt>){
        return predFilterEdges(itSource,itDest);
      } else{ // (_3_)b: cannot use ternary operator... it would require to compile both options.
        return predFilterEdges(*itSource,*itDest);
      }
    }


    template<typename NodeParamType>
    // false if not existing (provided that checkNode == true)
    std::pair<NodeIt,bool> internalGetItNode(const NodeParamType& nodeParam, bool checkNode = true) const{
      NodeIt nodeIt;
      if constexpr(std::convertible_to<NodeParamType,NodeIt>){
        if (checkNode){
          if (nodeParam == nodes.end()){
            nodeIt = nodes.end();
          } else{
            nodeIt = refNode(*nodeParam);
          }
        } else{
          nodeIt = nodeParam; // will return anyway true as second.
        }
      } else{
        nodeIt = refNode(nodeParam);
        // despite no cost to check,... prefer following checkNode, even if false
      }

      return {nodeIt, (!checkNode || (nodeIt != endNodes()))};
    }

    
    template<std::convertible_to<NodeIt> NodeResultType>
    NodeIt internalGetNodeFromIt(NodeIt nodeIt) const{return nodeIt;}

    template<std::convertible_to<Node> NodeResultType>
    const Node& internalGetNodeFromIt(NodeIt nodeIt) const{return *nodeIt;}


    template<typename NodeParamType>
    static bool checkNodeSetContains(const GraphSetImpl<NodeParamType>& nodeSet, NodeIt nodeIt){
      if constexpr(std::convertible_to<NodeParamType,NodeIt>){
        return nodeSet.contains(nodeIt);
      } else{ // (_3_)c: cannot use ternary operator... it would require to compile both options.
        return nodeSet.contains(*nodeIt);
      }
    }

    template<typename EdgeType>
    static bool checkEdgeSetContains(const GraphSetImpl<EdgeType>& edgeSet, NodeIt itSource, NodeIt itDest){
      if constexpr(std::convertible_to<EdgeType,EdgeIt>){
        return edgeSet.contains(EdgeIt{itSource,itDest});
      } else{ // (_3_)d: cannot use ternary operator... it would require to compile both options.
        return edgeSet.contains(Edge{*itSource,*itDest});
      }
    }

    // all==false -> only one maximum-size. TODO: weighted clicques serch.
    std::list<NodeSetRef> internalMaxCliques(bool all) const{
      constexpr bool UseBronKerbosch = true;

      std::list<NodeSetRef> maxCliques;

      if constexpr(UseBronKerbosch){ // useful in case of 'all maximal cliques'.
        if (all){
          // TODO: implement
          // Used version 3 from https://en.wikipedia.org/wiki/Bron%E2%80%93Kerbosch_algorithm

          /*
          algorithm BronKerbosch1(R, P, X) is
          if P and X are both empty then
              report R as a maximal clique
          for each vertex v in P do
              BronKerbosch1(R ⋃ {v}, P ⋂ N(v), X ⋂ N(v))
              P := P \ {v}
              X := X ⋃ {v}
          -> then call BronKerbosch1({}, allNodes, {})

          algorithm BronKerbosch2(R, P, X) is
          if P and X are both empty then
              report R as a maximal clique
          choose a pivot vertex u in P ⋃ X
          for each vertex v in P \ N(u) do
              BronKerbosch2(R ⋃ {v}, P ⋂ N(v), X ⋂ N(v))
              P := P \ {v}
              X := X ⋃ {v}
          -> then call BronKerbosch2({}, allNodes, {})

          algorithm BronKerbosch3(G) is
          P = V(G)
          R = X = empty
          for each vertex v in a degeneracy ordering of G do
              BronKerbosch2({v}, P ⋂ N(v), X ⋂ N(v))
              P := P \ {v}
              X := X ⋃ {v}
          -> then call BronKerbosch3(Graph)
          */

          // Compute set of bidirectional connections (for direct graphs).
          ConnectionMap intersectBidir;

          if constexpr(isDirect()){
            intersectBidir = withSuccessors;

            for(auto itSucc = intersectBidir.begin(); itSucc != intersectBidir.end(); ){
              if ( auto itPrev = withPredecessors->find(itSucc->first);
                   itPrev != withPredecessors->end() ){
                reduceContainer( itSucc->second, itPrev->second, true ); // intersection
              } else itSucc->second.clear(); // empty intersection

              if (itSucc->second.empty()){
                itSucc = intersectBidir.erase(itSucc);
              } else ++itSucc;
            }
          }

          const auto& connectedEdgesRef = isDirect() ? intersectBidir : withSuccessors;

          // use EdgeMapRef instead of NodeSetRef, in order to re-use edge maps.
          std::function<void(EdgeMapRef,EdgeMapRef)> f_BronKerbosch2;

          NodeSetRef currentClique;

          f_BronKerbosch2 = [&f_BronKerbosch2, &maxCliques, &currentClique, &connectedEdgesRef]
                            ( auto residualReachablesFromAllInClique, // 'P' at the web page
                              auto visitedReachablesFromAllInClique ) // 'X' at the web page
          {
            if (residualReachablesFromAllInClique.empty() && visitedReachablesFromAllInClique.empty()){
              maxCliques.push_back(currentClique);
            } else{
              // pivot 'u' at the web page
              auto pivotIt = ( residualReachablesFromAllInClique.empty()
                               ? visitedReachablesFromAllInClique.begin()
                               : residualReachablesFromAllInClique.begin() )->first;

              // compute 'P - N(u)' at the web page
              EdgeMapRef reducedResidualByPivot = residualReachablesFromAllInClique;
              if (auto it = connectedEdgesRef.find(pivotIt); it != connectedEdgesRef.end()){
                NContainer__MR::reduceContainer( reducedResidualByPivot, it->second, false );
                // don't scroll the nodes that are reachable from the pivot node.
              }

              // 'for each v in P - N(u)' at the web page
              while(!reducedResidualByPivot.empty()){
                EdgeMapRef residualIntersected;
                EdgeMapRef visitedIntersected;

                auto nodeIt = reducedResidualByPivot.begin()->first;
                reducedResidualByPivot.erase(reducedResidualByPivot.begin());

                if (auto it = connectedEdgesRef.find(nodeIt); it != connectedEdgesRef.end()){
                  residualIntersected = residualReachablesFromAllInClique;
                  visitedIntersected = residualReachablesFromAllInClique;

                  NContainer__MR::reduceContainer( residualIntersected, it->second, true );
                  NContainer__MR::reduceContainer( visitedIntersected, it->second, true );
                } // else: intersection with empty set is empty set -> will
                  // end a maximal clique in the direct recursive call.

                currentClique.insert(nodeIt);
                f_BronKerbosch2(residualIntersected,visitedIntersected);
                currentClique.erase(nodeIt);

                residualReachablesFromAllInClique.erase(nodeIt); // 'remove v from P'
                visitedReachablesFromAllInClique[nodeIt]; // 'add v into X' (use default constructor of EdgeWeight)
              }
            }
          }; // lambda

          // Compute degeneracy order (https://en.wikipedia.org/wiki/Degeneracy_(graph_theory))
          // in linear time.

          EdgeMapRef allNodesIt;
          std::list<NodeIt> degeneracyOrder;
          {
            std::unordered_map<NodeIt,std::size_t> notYetInTheOrder;

            // add nodes one by one by minimum number of residual neighboors.

            const auto& connectedEdgesRef = isDirect() ? intersectBidir : withSuccessors;

            std::unordered_map<std::size_t, std::unordered_set<NodeIt> > mapDegrees;
            for(auto nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt){
              allNodesIt[nodeIt]; // hope EdgeWeight has default constructor...

              std::size_t d = 0;
              if ( auto itConn = connectedEdgesRef.find(nodeIt); itConn != connectedEdgesRef.end() ){
                d = itConn->second.size();
              }

              mapDegrees[d].insert(nodeIt);
              notYetInTheOrder[nodeIt] = d;
            }

            //std::size_t k = 0; // this wil be the degeneracy order of the graph.

            while(!mapDegrees.empty()){
              std::size_t d = 0;
              typename decltype(mapDegrees)::iterator itSet;
              do{
                itSet = mapDegrees.find(d);
                ++d;
              }while(itSet == mapDegrees.end());
              --d;

              //if (d > k) k = d;

              auto& nodesWithD = mapDegrees[d];

              auto nextNodeItInOrder = *nodesWithD.begin();
              degeneracyOrder.push_back(nextNodeItInOrder);
              notYetInTheOrder.erase(nextNodeItInOrder);

              nodesWithD.erase(nodesWithD.begin());
              if (nodesWithD.empty()){
                mapDegrees.erase(itSet);
              }

              // lower the degree for all neighboors of nextNodeItInOrder.
              if (d > 0){ // => there are (residual) neighboors
                for( auto [neighboorIt, edgeWeight_] : connectedEdgesRef.find(nextNodeItInOrder)->second ){
                  if (auto it = notYetInTheOrder.find(neighboorIt); it != notYetInTheOrder.end()){
                    d = --(it->second); // for sure it was positive.
                    auto itOldSet = mapDegrees.find(d+1U);
                    itOldSet->second.erase(neighboorIt);
                    if (itOldSet->second.empty()){
                      mapDegrees.erase(itOldSet);
                    }
                    mapDegrees[d].insert(neighboorIt);
                  }
                }
              }
            }
          }

          EdgeMapRef residualNodesIt = std::move(allNodesIt);
          EdgeMapRef visitedNodesIt; // starts empty.
          for (auto nodeIt : degeneracyOrder){
            EdgeMapRef residualIntersected;
            EdgeMapRef visitedIntersected;

            if (auto it = connectedEdgesRef.find(nodeIt); it != connectedEdgesRef.end()){
              residualIntersected = residualNodesIt;
              visitedIntersected = visitedNodesIt;

              NContainer__MR::reduceContainer( residualIntersected, it->second, true );
              NContainer__MR::reduceContainer( visitedIntersected, it->second, true );
            } // else: intersection with empty set is empty set -> will
              // end a maximal clique in the direct recursive call.

            currentClique.insert(nodeIt);
            f_BronKerbosch2(residualIntersected,visitedIntersected);
            currentClique.clear();

            residualNodesIt.erase(nodeIt);
            visitedNodesIt[nodeIt];
          }

          return maxCliques;
        } // if all
      }

      //TODO: implement with the optimization as https://www.sciencedirect.com/science/article/pii/S0166218X01002906
      // in case of !all .

      std::size_t best = 0U;
      NodeSetRef bestClique;

      NodeSetRef alreadyVisited;

      for(auto nodeIt = begin(); nodeIt != end(); ++nodeIt){
        const auto& predMap = (isDirect() ? *withPredecessors : withSuccessors);

        if (auto itSucc = withSuccessors.find(nodeIt); itSucc != withSuccessors.end()){
          const auto& connectedRef = itSucc->second;

          EdgeMapRef intersect;
          if constexpr(isDirect()){
            if ( auto itPred = withPredecessors->find(nodeIt); itPred != withPredecessors->end()){
              const auto& predSet = itPred->second;

              intersect = connectedRef;

              reduceContainer( intersect, predSet, true ); // that's a kind of map_intersect (true <-> keep).           
            }
          }

          const auto& connectedRefToUse = isDirect() ? intersect : connectedRef;

          std::size_t ub = connectedRefToUse.size()+1U;

          NodeSetRef bestCliqueForNode;

          for(const auto edgeIt : connectedRefToUse){

            // speed-up not to consider the nodes already visited, because
            // then the couple *nodeIt & *edgeIt.first has been already considered
            // when starting from *edgeIt.first . However, excluding *edgeIt.first,
            // may still determine a different clique including *nodeIt .
            if (alreadyVisited.contains(edgeIt.first)) continue;

            if (ub <= best) break; // speed-up, useless to continue

            if constexpr(isDirect()){
            }

            bool ok = true;
            for(auto cliqueNodeIt : bestCliqueForNode){
                if (!edgeExists(edgeIt.first, cliqueNodeIt)){
                  ok = false;
                  --ub;
                  break;
                }
            }

            if (ok){
                bestCliqueForNode.insert(edgeIt.first);
            }
          }

          bestCliqueForNode.insert(nodeIt); // can be done at the end, it's faster not to check for it
          alreadyVisited.insert(nodeIt);

          if (bestCliqueForNode.size() > best){
            best = bestCliqueForNode.size();

            for(auto pcIt : bestCliqueForNode){
                bestClique.insert(pcIt);
            }

            if (!all){
              maxCliques.clear();
            }

            maxCliques.emplace_back(std::move(bestClique));
            bestClique.clear(); // TODO: likely not needed.

            if constexpr(!UseBronKerbosch){
              if (all){
                best = 0U;
              }
            }
          }
        }
      }

      return maxCliques;
    }


    static inline auto sumEval2 = []<typename Eval>(Eval v1, Eval v2 ){return v1+v2;};

    template<typename BinaryPredicate>
    bool isAnyEdgePredicate(BinaryPredicate&& predFilterEdges){
      for(const auto& [nodeIt, itSuccSet] : withSuccessors){
        for(const auto& [nodeIt2, eval2] : itSuccSet){
          if (!checkPredEdge(std::forward<BinaryPredicate>(predFilterEdges), nodeIt, nodeIt2)) return false;
        }
      }
      return true;
    }

    /*template< typename Eval, typename NodeCollection, typename NodeItExtractor, typename EvalExtractor,
              typename UnaryPredicate, typename FinalCombiner = std::function<Eval(Eval,Eval)> >
    requires NConcept__MR::Iterable<NodeCollection>
    static std::pair<NodeSetRef, Eval>
           filterAndComputeNodes( const NodeCollection& nodeCollection, UnaryPredicate&& predFilterNodes,
                                  NodeItExtractor&& nodeItExtractor, EvalExtractor&& evalExtractor,
                                  Eval finalSeed, FinalCombiner&& finalCombiner ){
      NodeSetRef resSet;

      auto resEval = finalSeed;
      for(const auto& nodeInfo : nodeCollection){
        auto nodeIt = nodeItExtractor(nodeInfo);

        if (checkPredNode(predFilterNodes, nodeIt)){
          resEval = finalCombiner(resEval, evalExtractor(nodeInfo)); // overflow check should be inside finalComb
          resSet.insert(nodeIt);
        }
      }

      return {resSet,resEval};
    }*/

    // No constraints: checked by the public methods [and no overloads for this private method]
    template< typename Eval, typename UnaryPredicate, typename BinaryPredicate,
              typename EvalComputerFromSource, typename EvalCombiner, typename FinalCombiner = std::function<Eval(Eval,Eval)> >
    requires (NConcept__MR::computerCombiner2<EvalCombiner, Eval, std::pair<bool,bool>> || NConcept__MR::voidCombiner2<EvalCombiner, Eval>)
             && ( NConcept__MR::Callable<EvalComputerFromSource, Eval, Eval, EdgeWeight>
                  || std::convertible_to<EvalComputerFromSource, Eval> )
    std::tuple<MapEvalAndLeadingSources<Eval>, Eval, bool>
    // Remember that a voidcombiner is not fair if getPaths==true -> read (_11_)
    // TODO: switch to node-indexes during the search.
    internalVisit( NodeIt startNodeIt,
                   UnaryPredicate&& predDestNodes, BinaryPredicate &&predFilterEdges,
                   Eval startNodeValue, EvalComputerFromSource&& evalComputer, EvalCombiner&& evalCombiner,
                   Eval finalSeed, FinalCombiner&& finalCombiner,
                   bool stopOnLoops, bool getPaths,
                   bool dontInvertToPredecessors = true){
      
      auto successorWRef = std::cref(withSuccessors);
      if constexpr(isDirect()){
        if (!dontInvertToPredecessors){
          successorWRef = std::cref(*withPredecessors);
        }
      }
      auto& successorRef = successorWRef.get();

      using EvalAndLeadingSources = typename MapEvalAndLeadingSources<Eval>::mapped_type;

      if (startNodeIt == endNodes()){
        return {MapEvalAndLeadingSources<Eval>{}, Eval{}, false};
      }

      bool filterExcludedAnyEdge = false;
      const bool dontCareMultipleVisits =
                 NConcept__MR::voidCombiner2<EvalCombiner, Eval> && !getPaths; 
      const bool checkLoop = !( noLoopNodes.contains(startNodeIt) || dontCareMultipleVisits );
                          // In case of dontCareMultipleVisits, the algorithm never cares
                          // about loops -> see (_2_)a, therefore they are not found.

      bool newLoopFound = false;

      MapEvalAndLeadingSources<Eval> nextReached;
      nextReached[startNodeIt].first = std::move(startNodeValue);

      MapEvalAndLeadingSources<Eval> allReached = nextReached;
      // startNodeIt is then removed at the end.

      using SourceNodesFor = GraphMapImpl<NodeIt,NodeSetRef>;
      std::unique_ptr<SourceNodesFor> sourceNodesFor;
      std::unique_ptr<SourceNodesFor> directSourceNodesFor;

      if (checkLoop){
        sourceNodesFor = std::make_unique<SourceNodesFor>();
      }
      if constexpr(isIndirect()){
        directSourceNodesFor = std::make_unique<SourceNodesFor>();
      }

      GraphSetImpl<NodeIt> dummyNodeItSet;
      while(!nextReached.empty()){
        MapEvalAndLeadingSources<Eval> oldReached{std::move(nextReached)};

        GraphMapImpl<NodeIt, NodeSetRef> loopNodesNow;

        // sourceEval may be needed to be not const -> see (_13_) (TODO: should use a mutable pair there ?)
        // More generally, some visiting algorithm might need to change something on the source node.
        for(auto& [sourceIt, sourceEval] : oldReached){
          auto sourceNodesForSource = std::cref(
            checkLoop ? (*sourceNodesFor)[sourceIt] : dummyNodeItSet );

          if(auto itSucc = successorRef.find(sourceIt); itSucc != successorRef.end()){
            for(auto [newReachIt, edgeWeight] : itSucc->second){
              if (!checkPredEdge(std::forward<BinaryPredicate>(predFilterEdges), sourceIt, newReachIt)){
                filterExcludedAnyEdge = true;
              } else{
                bool stopPropagation = false;
                bool updateContributers; // = true; -> shared in (_7_)
                if (dontCareMultipleVisits){
                  stopPropagation = allReached.contains(newReachIt);
                  //(_2_)a
                  // since the combiner is void, it is useless to pass more
                  // times for the same nodes.
                  // This does not mean for sure a loop: there might be multiple
                  // paths to reach the same node. However, this leads to fail
                  // finding loops, therefore checkLoop was set to false.

                  //updateContributers = !stopPropagation; -> shared in (_7_)
                  // not really needed, because !getPaths -> see (_6_)
                }

                bool loopFoundNow = false;
                if (checkLoop){
                  if (!noLoopNodes.contains(newReachIt)){

                    loopFoundNow = sourceNodesForSource.get().contains(newReachIt);
                    // that's normal for direct source in case of Indirect
                    // graph -> see (_2_)b to avoid considering loop.
                    // Moreover, even in case of imploding, it's not allowed to
                    // duplicate a connection between 2 nodes -> see (_8_).
                    
                    if (!loopFoundNow){
                      //else: useless to keep sourceNodesFor from newReachIt on.
                      // This information is not propagated to descending for the
                      // moment, but at the end of the method (if no loop is found),
                      // all reached nodes will be marked as no-loop.
                      auto& sourceNodesForDest = (*sourceNodesFor)[newReachIt];
                      sourceNodesForDest.insert(sourceIt);
                      sourceNodesForDest.merge(NodeSetRef{sourceNodesForSource.get()});
                    }
                  }
                }

                // This to be done even if !checkLoop.
                if constexpr(isIndirect()){
                  // avoiding go-back on the sources, is needed even in
                  // case checkLoop==false, as for example 'no loop' finding
                  // in any previous call was supported by directSourceNodesFor
                  // to avoid cycle forever.
                  (*directSourceNodesFor)[newReachIt].insert(sourceIt);
                  if ((*directSourceNodesFor)[sourceIt].contains(newReachIt)){
                    loopFoundNow = false;
                    //(_2_)b
                    stopPropagation = true; // for whatever kind of combiner.
                    //updateContributers = false; -> shared in (_7_)
                  }
                }

                if (checkLoop){
                  if (loopFoundNow){
                    // loop found.
                    newLoopFound = true;

                    loopNodes.insert(startNodeIt);
                    loopNodes.insert(sourceIt);
                    loopNodes.insert(newReachIt);
                    // ... might add also predecessors.

                    if (stopOnLoops){
                      return {MapEvalAndLeadingSources<Eval>{}, Eval{},false};
                    }

                    /* (_10_)
                       need to be postponed [in (_11_)], for the case when the new node evaluation
                       is anyway better, despite the loop detection that is contemporary but
                       does not affect the fgood path leading to the better evaluation.
                    stopPropagation = true;
                    //updateContributers = false; -> shared in (_7_)
                    */
                    if constexpr(NConcept__MR::voidCombiner2<EvalCombiner, Eval>){
                      loopNodesNow[newReachIt].insert(sourceIt);
                    } // else: the evaluation system will already be able to avoid propagating.
                  }
                }

                // (_7_) Until now, stopPropagation true only for loops or avoid-go-back for indirect.
                updateContributers = !stopPropagation;
                if (!stopPropagation){
                  Eval newV;
                  if constexpr(NConcept__MR::Callable<EvalComputerFromSource, Eval, Eval, EdgeWeight>){
                    newV = evalComputer(sourceEval.first, edgeWeight);
                  } else{
                    newV = evalComputer; // used as a constant.
                  }

                  bool resetContributors = false;
                  auto [it, ok] = nextReached.insert({newReachIt,{newV,NodeSetRef{}}});
                  auto& vEval = it->second;

                  if (!ok){ // else: just added newV, resetContributors would be useless.
                    //(_9_)
                    if constexpr(NConcept__MR::voidCombiner2<EvalCombiner, Eval>){
                      evalCombiner(vEval.first, newV);
                      // resetContributors remains false. All predecessors that lead here are ok.
                      // If !getPaths, there is dontCareMultipleVisits==true that applies in (_2_)a
                      // to stop as soon as possible the propagation.
                    } else{ // computerCombiner
                      const auto oldEval = vEval.first; // (_5_)a

                      const auto propagateAndAffect = evalCombiner(vEval.first, newV);  // overflow check should be inside finalComb
                      // if this node is already in allReached, no problem, as the
                      // further propagation will consider only the new partial evaluation.
                      // This private method is used only for 'sums' (count nodes or paths)

                      // stopPropagation = !propagateAndAffect.first; useless, as the node
                      // was anyway already in nextReached and has not to be removed.
                      
                      if (vEval.first != oldEval){ // value is changed -> 'second' means whether clear.
                        resetContributors = propagateAndAffect.second;
                        // updateContributers remains true
                      } else{ // values is not changed -> 'second' means whether contributed
                        updateContributers = propagateAndAffect.second;
                      }
                    }
                  }

                  if (getPaths){ //(_6_)
                    if (resetContributors){
                      vEval.second.clear();
                    }

                    if (updateContributers){
                      vEval.second.insert(sourceIt);
                    }
                  }
                }
              }
            }
          }
        }

        auto nextReachedCopy = nextReached;
        NContainer__MR::mergeMapCombining<GraphMapImpl>( //clang before version 19 refuses the template template argument.
          allReached, std::move(nextReachedCopy),
          // TODO: adding && on EvalAndLeadingSources does not satisfy NConcept__MR::Callable<EvalCombiner, void, Value&, Value&&>
          [&evalCombiner = std::as_const(evalCombiner), getPaths]
          (EvalAndLeadingSources& v1, EvalAndLeadingSources v2){
            bool updateContributers = true;

            if constexpr(NConcept__MR::voidCombiner2<EvalCombiner, Eval>){
              evalCombiner(v1.first, v2.first);
            } else{
              const auto oldEval = v1.first; // (_5_)b

              const auto propagateAndAffect = evalCombiner(v1.first, v2.first);
              
              if (v1.first != oldEval){ // value is changed -> 'second' means whether clear.
                if (getPaths && propagateAndAffect.second){
                  v1.second.clear();
                }
              } else{ // values is not changed -> 'second' means whether contributed
                updateContributers = propagateAndAffect.second;
              }
            }

            if (getPaths && updateContributers)
              v1.second.merge(v2.second);
            
            return updateContributers;
                   //&& !dontCareMultipleVisits
                   // -> useless here: in (_2_)a already avoided propagation
                   //    if the node was already in allReached.
          } );

        // The elements still in nextReachedCopy, have not been merged, therefore
        // they are useless and do not need to generate propagation. Indeed, checks
        // within (_9_) were just for the set nextReached, not yet with allReached.
        NContainer__MR::reduceContainer(nextReached, nextReachedCopy);

        if constexpr(NConcept__MR::voidCombiner2<EvalCombiner, Eval>){
          //(_11_)
          //need to avoid now to really propagate if a loop was just found -> from (_10_).
          //This means a void combiner cannot be used if paths are interesting, as there is risk
          //to lose the case when from Start there are two paths toward another node A, but also
          //a loop involving A with some nodes of one of these two paths -> in case of getPaths,
          //the involved predecessor of A will not be considered despite being also part of the
          //alternative way to move from Start to A.
          for(const auto& [itLoopNode, loopDirectSources] : loopNodesNow){
            nextReached.erase(itLoopNode);
            NContainer__MR::reduceContainer(allReached[itLoopNode].second, loopDirectSources);
          }
        }
      }

      bool mightUpdateNoLoop = !(filterExcludedAnyEdge || AncestorAndDescending);
      // In case of AncestorAndDescending, loops are alread known.
      // In case of filterExcludesEdges (known by the caller), absence of loops
      // cannot determine a general absence of loops.

      if (mightUpdateNoLoop && checkLoop && !newLoopFound){
        // no loop was found: all nodes in allReached are no-loop
        noLoopNodes.insert(startNodeIt);
        auto itSpeedUp = noLoopNodes.begin();
        for (auto [itReached, eval_] : allReached){
          noLoopNodes.insert(itReached);
        }
      }

      allReached.erase(startNodeIt);

      for(auto it = allReached.begin(); it != allReached.end(); ){
        if (!checkPredNode(predDestNodes, it->first)){
          it = allReached.erase(it);
        } else{
          ++it;
        }
      }

      auto resEval = finalSeed;
      for(const auto& [itReached, eval] : allReached){
        resEval = finalCombiner(resEval, eval.first);
      }

      return {std::move(allReached), resEval, !loopNodes.contains(startNodeIt)};
      // read loopNodes for the case of void combiner.
    }


    // if second false, there is a loop (the opposite is not true),
    // but in any case it returns the number of reachable nodes (satisfying the predicate).
    template< typename NodeParamType, typename UnaryPredicate, typename BinaryPredicate >
    std::pair<NodeSetRef,bool> internalReachables( const NodeParamType& beginNodeParam,
                                                   UnaryPredicate &&predDestNodes,
                                                   BinaryPredicate &&predFilterEdges,
                                                   bool forward ){
      // if (predFilterEdges.template target<bool(Node,Node)> == &(predAnyEdge.operator()))
      // TODO: refine, if possible [https://stackoverflow.com/questions/20833453/comparing-stdfunctions-for-equality]
      //       Maybe in C++23/26 we may use static for the operator() of the lambda.... however, likely not
      //       possible to know the parameter type of predFilterEdge, unless reflection.
      NodeSetRef resSet;
      bool noLoop = false;

      if (auto [itBeginNode, exists] = internalGetItNode<NodeParamType>(beginNodeParam,true); exists){
        bool callVisit = true;
        if constexpr(AncestorAndDescending){ //speed-up
          if (isAnyEdgePredicate(std::forward<BinaryPredicate>(predFilterEdges))){
          //else: ancestors/descending cannot be exploited, because of the filter.
            
            noLoop = !loopNodes.contains(itBeginNode);
            // for sure well set, as it corresponds to belonging to its own descending/ancestors.
            // might be also used in the evalExtractor to avoid nodeIt == nodeIt and return soon 1 in any case.

            auto descendingWRef = std::cref(*allDescending);
            if constexpr(isDirect()){
              if (!forward){
                descendingWRef = std::cref(*allAncestors);
              }
            }
            auto& descendingRef = descendingWRef.get();

            for( auto nodeIt : descendingRef[itBeginNode]){
              if (checkPredNode(predDestNodes, nodeIt)){
                resSet.insert(nodeIt);
              }
            }

            callVisit = false;
          }
        }
        
        if (callVisit){
          bool dontCareAnyPredicate = true;
          auto [map, val_, noLoop_] =
               internalVisit<std::size_t>( itBeginNode,
                                           std::forward<UnaryPredicate>(predDestNodes),
                                           std::forward<BinaryPredicate>(predFilterEdges),
                                           1U, // startNodeValue
                                           1U, // eval-computer
                                           [](const std::size_t, const std::size_t ) {}, // void eval-combiner
                                           // a void-combiner is ok when getPaths==false -> read (_11_)
                                           0U, sumEval2, // final-seed / final-combiner
                                           false, // => don't stop on loops
                                           false, // => don't get paths
                                           forward );
          for(auto [nodeIt, one_] : map ){
            resSet.insert(nodeIt);
          }
          noLoop = noLoop_;
        
          // val_ is set.size()
        }
      }

      return {std::move(resSet), noLoop};
    }


    template< typename NodeParamType,
              typename UnaryPredicate = decltype(std::function(predAnyNodeIt)),
              typename BinaryPredicate = decltype(std::function(predAnyEdgeIt)) >
    GraphMapImpl<NodeIt,EdgeWeight> internalDistances(
                                    const NodeParamType& nodeParamBegin, UnaryPredicate &&predDestNodes,
                                    BinaryPredicate &&predFilterEdges,
                                    bool forward ){

      auto [nodeIt, exists] = internalGetItNode<NodeParamType>(nodeParamBegin);
      if (!exists){
        return {}; // empty map
      }

      auto [map, val_, noLoop_] =
        internalVisit<std::size_t>( nodeIt,
                                    std::forward<UnaryPredicate>(predDestNodes),
                                    std::forward<BinaryPredicate>(predFilterEdges),
                                    0U, // startNodeValue
                                    [](EdgeWeight sourceDist, const EdgeWeight edgeW) { // eval-computer
                                      return sourceDist + edgeW;
                                    },
                                    [](EdgeWeight& currDist, const EdgeWeight distNew) {  // eval-combiner
                                      if (distNew < currDist){
                                        currDist = distNew;

                                        return std::make_pair(true,true); // second-true -> reset contributers
                                      }

                                      return std::make_pair(false,(distNew==currDist)); // second-true -> update contributers if equivalent
                                    },
                                    0U, [](const EdgeWeight& v1, const EdgeWeight& v2){return std::min(v1,v2);}, // final-seed / final-combiner -> max-min Dist
                                    false, // => don't stop on loops
                                    false, // => don't get paths
                                    forward );

      // map has type GraphMapImpl<NodeIt, std::pair<EdgeWeight, NodeSetRef> >
      GraphMapImpl<NodeIt,EdgeWeight> res;
      for(const auto& [nodeIt2, distAndReachedSet] : map){
        res[nodeIt2] = distAndReachedSet.first;
      }

      res[nodeIt] = EdgeWeight{};

      return res;
    }


    // Here: always NodeIt, as in (_14_) nodes.cend() is used.
    template< typename UnaryPredicate, typename BinaryPredicate >
    std::tuple<std::vector<NodeIt>, bool, bool>
    internalTopologicalOrder( NodeIt startNodeItForIndirect,
                              UnaryPredicate &&predFilterNodes, BinaryPredicate &&predFilterEdges ) {
      // Using a subGraph is a simple way to deal with edge-filtering while
      // searching for nodes without predecessors.
      auto subGraph = buildSubGraph( std::forward<UnaryPredicate>(predFilterNodes) )
                      .buildSubGraph( std::forward<BinaryPredicate>(predFilterEdges), true );

      NodeIt startNodeIt;

      // TODO: exploit AncestorAndDescending
      if (startNodeItForIndirect != endNodes()){
        startNodeIt = subGraph.nodes.find(*startNodeItForIndirect);
      } else startNodeIt = subGraph.endNodes();

      bool graphFullyConnected = true;

      using EvalPair = std::pair<std::size_t, std::size_t>;

      MapEvalAndLeadingSources<EvalPair> fullMapEvaluations;

      bool uniqueOrdering = true;
      std::size_t nextIdx = 0U;
      do{
        if (startNodeIt == subGraph.endNodes()){
          // find any new node with no predecessors (touched ones are keys in fullMapEvaluations).

          const auto& preds = isDirect() ? *(subGraph.withPredecessors) : subGraph.withSuccessors;

          bool found = false;
          for(auto nodeIt = subGraph.nodes.begin(); (nodeIt != subGraph.nodes.end()) && !found; ++nodeIt){
            if (!(preds.contains(nodeIt) || fullMapEvaluations.contains(nodeIt))){
              found = true;
              startNodeIt = nodeIt;
              *nodeIt;
            }
          }

          if (!found){
            return { {}, false, false }; // for sure loops. No order is possible.
          }
        }

        auto [mapEvaluations, finalComb, noLoop] =
          subGraph.template internalVisit<EvalPair>(
            startNodeIt, predAnyNodeIt, predAnyEdgeIt,
            {nextIdx, 0U}, // start
            //(_13_)
            [](EvalPair& vSource, const EdgeWeight& edgeWeight) { // eval-computer
              return std::make_pair(vSource.first + (++vSource.second), 0U );
              // will be propagated (eval-combiner always updates) again and its children
              // need to be re-counted when v1 will get the role of vSource.
            },
            [&uniqueOrdering](EvalPair& vOld, const EvalPair vNew) { // eval-combiner
              if (vNew.first > vOld.first){
                vOld.first = vNew.first; // vNew computed in the eval-computer, based on vSource.
              } // '<' may happen (if no ordering is followed for 'next-reached' scrolling)
              vOld.second = 0;
              uniqueOrdering = false; // here means that a node has more than a single predecessor.
              return std::make_pair(true, false); // propagate, and then also clear because
                                                  // the leading sources do not represent useful
                                                  // information for the topological order.
            },
            {0U, 0U}, // final-seed, useless
            [](EvalPair v1, EvalPair v2){return EvalPair{std::max(v1.first,v2.first), 0U};},
            true, // stopOnLoops -> no order in cse of loops
            true // getPaths -> supports the ability to know unique ordering
          );

        if (!noLoop){
          return { {}, false, false };
        }

        // type of mapEvaluations : GraphMapImpl<NodeIt, std::pair<Eval, NodeSetRef> >
        mapEvaluations[startNodeIt] = std::pair{EvalPair{nextIdx,0U}, NodeSetRef{}};

        if (mapEvaluations.size() < subGraph.nodes.size()){
          uniqueOrdering = false;
          if constexpr(isIndirect()){
            return { {}, false, std::unordered_set<std::size_t>{} }; // like a loop -> read (_12_)
          }
        }

        fullMapEvaluations.merge(mapEvaluations);
        nextIdx = finalComb.first+1U;

        startNodeIt = subGraph.endNodes();
      } while(fullMapEvaluations.size() < subGraph.nodes.size());

      // let's sort the nodes by increasing order index.
      std::vector<std::pair<NodeIt, std::size_t> > indexesByIt;
      for(const auto& [nodeIt, rankAndContributors] : fullMapEvaluations){
        indexesByIt.emplace_back(this->refNode(*nodeIt), rankAndContributors.first.first); // EvalPair is a pair
        // translate from nodeIt (referring to a node-copy in sub-graph) to this->someNodeIt.
      }

      std::sort( indexesByIt.begin(), indexesByIt.end(),
                 [](auto itAndRank1, auto itAndRank2){
                  return itAndRank1.second < itAndRank2.second;
                 } );

      std::vector<NodeIt> result( subGraph.nodes.size(), endNodes() );

      for(std::size_t i = 0; i < indexesByIt.size(); ++i){
        result[i] = indexesByIt[i].first; // the iterator is toward the subgraph.
      }

      return {std::move(result), true, uniqueOrdering};
    }
  };
} // namespace NGraph__MR