#include "bdd.h"

#include <algorithm>
#include <iostream>
#include <stack>
#include <unordered_map>

BinaryDecisionDiagram* BinaryDecisionDiagramManager::makeBinaryDecisionDiagram(std::size_t index, BinaryDecisionDiagram* high, BinaryDecisionDiagram* low)
{
    if ((index != 0 && index != 1) && high == low) {
        return high;
    }
    // lambda to actually create the node
    auto addNode = [this, index, high, low]() {
        auto newNode = std::make_unique<BinaryDecisionDiagram>(index, high, low);
        indexToNode.insert({ index, newNode.get() });
        nodes.push_back(std::move(newNode));
        return nodes.back().get();
    };

    // search for an index to a node
    auto [indexRangeBegin, indexRangeEnd] = indexToNode.equal_range(index);

    // this block is entered if no node with the same index is found
    if (indexRangeBegin == std::end(indexToNode) && indexRangeEnd == std::end(indexToNode)) {
        return addNode();
    }

    // attempt to see if the given node parameters already exist
    // this is a node where the index, high, and low are all the same
    auto nodeExistsIterator = std::find_if(indexRangeBegin, indexRangeEnd, [high, low](const auto& nodeItPtr) {
        return (nodeItPtr.second->high_ == high) && (nodeItPtr.second->low_ == low);
    });

    // if the node does not exist then add the node
    if (nodeExistsIterator == indexRangeEnd) {
        return addNode();
    }

    // if this specific node exists, then return it
    return nodeExistsIterator->second;
}

BinaryDecisionDiagram::BinaryDecisionDiagram(std::size_t index, BinaryDecisionDiagram* high, BinaryDecisionDiagram* low) : index_(index), high_(high), low_(low) {}

BinaryDecisionDiagramManager::BinaryDecisionDiagramManager(std::size_t estimatedNumberVariables)
{
    nodes.reserve(estimatedNumberVariables);
    indexToNode.reserve(estimatedNumberVariables);

    // add the 0/false and 1/true nodes
    makeBinaryDecisionDiagram(0, nullptr, nullptr);
    makeBinaryDecisionDiagram(1, nullptr, nullptr);
}

BinaryDecisionDiagram* BinaryDecisionDiagramManager::addNthIndex(std::size_t n)
{
    return makeBinaryDecisionDiagram(n, nodes[1].get(), nodes[0].get());
}

// The if then else operation computers and returns the node that is the result of applying the if-then-else operator
// to three nodes that serve as the if, then, and else clauses
BinaryDecisionDiagram* BinaryDecisionDiagramManager::ifThenElse(BinaryDecisionDiagram* ifNode, BinaryDecisionDiagram* thenNode, BinaryDecisionDiagram* elseNode)
{
    if (ifNode == one()) { return thenNode; }
    if (ifNode == zero()) { return elseNode; }
    if (thenNode == elseNode) { return thenNode; }
    if (thenNode == one() && elseNode == zero()) { return ifNode; }

    // General cases
    // Splitting variable must be the topmost root
    std::size_t splitVar = ifNode->index_;
    if (splitVar < thenNode->index_) { splitVar = thenNode->index_; }
    if (splitVar < elseNode->index_) { splitVar = elseNode->index_; }

    auto* ifTrue = restrict(ifNode, splitVar, true);
    auto* thenTrue = restrict(thenNode, splitVar, true);
    auto* elseTrue = restrict(elseNode, splitVar, true);
    auto* positiveFactor = ifThenElse(ifTrue, thenTrue, elseTrue);

    BinaryDecisionDiagram* ifFalse = restrict(ifNode, splitVar, false);
    BinaryDecisionDiagram* thenFalse = restrict(thenNode, splitVar, false);
    BinaryDecisionDiagram* elseFalse = restrict(elseNode, splitVar, false);
    BinaryDecisionDiagram* negativeFactor = ifThenElse(ifFalse, thenFalse, elseFalse);

    return makeBinaryDecisionDiagram(splitVar, positiveFactor, negativeFactor);
}

BinaryDecisionDiagram* BinaryDecisionDiagramManager::restrict(BinaryDecisionDiagram* root, std::size_t index, bool val)
{
    if (root->index_ < index) {
        return root;
    }

    if (root->index_ > index) {
        auto* high = restrict(root->high_, index, val);
        auto* low = restrict(root->low_, index, val);
        return makeBinaryDecisionDiagram(root->index_, high, low);
    }
    // subtree->index_ == index
    return val ? restrict(root->high_, index, val) : restrict(root->low_, index, val);
}

std::vector<BinaryDecisionDiagram*> BinaryDecisionDiagramManager::getNodes(std::size_t variableSubscript)
{
    std::vector<BinaryDecisionDiagram*> retNodes;
    for (const auto& nodePtr : nodes) {
        if (nodePtr->index_ == variableSubscript) {
            retNodes.push_back(nodePtr.get());
        }
    }
    return retNodes;
}

std::string toDot(BinaryDecisionDiagram* root, BinaryDecisionDiagram* one, BinaryDecisionDiagram* zero, const std::string& graphName)
{
    // keep track of nodes that have been visited
    std::unordered_map<BinaryDecisionDiagram*, bool> visited{ { zero, true }, { one, true } };

    // map unique graph id to label
    std::unordered_map<std::size_t, std::string> idToLabel{ { 0, "0" }, { 1, "1" } };

    // map a node pointer to an id
    std::unordered_map<BinaryDecisionDiagram*, std::size_t> nodeToId = { { zero, 0 }, { one, 1 } };

    // used for dfs algorithm
    std::stack<BinaryDecisionDiagram*> nodeStack;
    nodeStack.push(root);

    std::string dotInner;
    std::size_t uniqueGraphId = 2;
    while (!nodeStack.empty()) {
        auto* node = nodeStack.top();
        nodeStack.pop();

        // check to see if we've visited
        if (visited.count(node) != 0) {
            visited[node] = true;
            continue;
        }

        const std::size_t nodeID = nodeToId.count(node) == 0 ? uniqueGraphId : nodeToId[node];

        if (nodeID == uniqueGraphId) {
            nodeToId[node] = uniqueGraphId;
            idToLabel[uniqueGraphId++] = std::to_string(node->index_);
        }

        visited[node] = true;

        if (node->high_ != nullptr) {
            if (nodeToId.count(node->high_) == 0) {
                nodeToId[node->high_] = uniqueGraphId;
                idToLabel[uniqueGraphId++] = std::to_string(node->high_->index_);
            }

            // need to map from graphiz node to graphiz node
            dotInner += "\t" + std::to_string(nodeID) + " -- " + std::to_string(nodeToId[node->high_]) + ";\n";
            if (visited.count(node->high_) == 0) {
                nodeStack.push(node->high_);
            }
        }

        if (node->low_ != nullptr) {
            if (nodeToId.count(node->low_) == 0) {
                nodeToId[node->low_] = uniqueGraphId;
                idToLabel[uniqueGraphId++] = std::to_string(node->low_->index_);
            }

            dotInner += "\t" + std::to_string(nodeID) + " -- " + std::to_string(nodeToId[node->low_]) + " [style=dashed];\n";
            if (visited.count(node->low_) == 0) {
                nodeStack.push(node->low_);
            }
        }
    }

    std::string graphLabels;
    for (auto& [id, label] : idToLabel) {
        graphLabels += "\t" + std::to_string(id) + " [label=\"" + label + "\"];\n";
    }

    return "graph " + graphName + " {\n" + graphLabels + dotInner + "}";
}

std::string toDot(BinaryDecisionDiagramManager& bdd, const std::string& name)
{
    std::string dotInner;
    std::size_t graphId = 0;
    std::unordered_map<std::size_t, std::string> idToLabel;
    for (auto& nodePtr : bdd) {
        idToLabel[graphId] = std::to_string(nodePtr->index_);
        const std::string graphIdStr = std::to_string(graphId);
        if (nodePtr->low_ != nullptr) {
            dotInner += graphIdStr + " -- " + std::to_string(nodePtr->low_->index_) + " [style=dashed];\n";
        }

        if (nodePtr->high_ != nullptr) {
            dotInner += graphIdStr + " -- " + std::to_string(nodePtr->high_->index_) + ";\n";
        }
        ++graphId;
    }

    // build the label string to be at the top
    std::string labelStr;
    for (auto& [id, label] : idToLabel) {
        labelStr += std::to_string(id) + " [label=\"" + label + "\"];\n";
    }
    return "graph " + name + "{\n" + labelStr + dotInner + "\n}";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace std {
template<>
struct hash<BinaryDecisionDiagram>
{
    std::size_t operator()(BinaryDecisionDiagram const& node) const
    {
        const std::size_t indexHash = std::hash<std::size_t>{}(node.index_);
        const std::size_t highHash = std::hash<BinaryDecisionDiagram*>{}(node.high_);
        const std::size_t lowHash = std::hash<BinaryDecisionDiagram*>{}(node.low_) << 1;
        const std::size_t lowHighHash = (highHash ^ lowHash) << 1;
        const std::size_t finalHash = (indexHash ^ lowHighHash) << 1;
        return finalHash;
    }
};
}// namespace std
