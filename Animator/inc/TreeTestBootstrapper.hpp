#pragma once

#include <cassert>
#include <set>

#include "SkeletonTree.hpp"

#include <iostream> // todo: rm

class TreeTestBootstrapper {
public:
  static void runTests() {
    shouldConstructTree();
    shouldConstructTreeRootNode();
    shouldConstructTreeNode();
    shouldAddChildNodeToRoot();
    shouldAddChildrenNodesToRoot();
    shouldAddGrandchildNodeToRoot();
    shouldAddGreatgrandchildNodesToRoot();
    shouldEnumerateTree();
  }

private:
  static SkeletonTree::Node createRootTreeNode() {
    SkeletonTree::Node::Channel translationChannel;
    SkeletonTree::Node::Channel angleChannel;
    SkeletonTree::Node::Offset offset;

    return SkeletonTree::Node(translationChannel, angleChannel, offset);
  }

  static SkeletonTree::Node createTreeNode() {
    SkeletonTree::Node::Channel angleChannel;
    SkeletonTree::Node::Offset offset;

    return SkeletonTree::Node(angleChannel, offset);
  }

  static SkeletonTree createTree(std::set<SkeletonTree::Node *> &treeNodes) {
    treeNodes.clear();

    SkeletonTree::Node rootTreeNode = createRootTreeNode();

    SkeletonTree tree(rootTreeNode);

    SkeletonTree::Node *rootNode = tree.getRootNode();

    treeNodes.emplace(rootNode);

    SkeletonTree::Node newTreeNode = createTreeNode();

    const unsigned rootChildCount = 5;
    SkeletonTree::Node *childNode;
    for (unsigned i = 0; i < rootChildCount; ++i) {
      childNode = tree.addNode(newTreeNode, rootNode);
      treeNodes.emplace(childNode);

      // insert some sub-trees
      if (i == 1 || i == 3) {
        const unsigned tmp = 7;
        for (unsigned i = 0; i < tmp; ++i) {
          treeNodes.emplace(tree.addNode(newTreeNode, childNode));
        }
      }
    }

    const unsigned grandChildCount = 9;
    SkeletonTree::Node *grandChildNode;
    for (unsigned i = 0; i < grandChildCount; ++i) {
      grandChildNode = tree.addNode(newTreeNode, childNode);
      treeNodes.emplace(grandChildNode);

      // insert some sub-trees
      if (i == 4 || i == 6) {
        const unsigned tmp = 13;
        for (unsigned i = 0; i < tmp; ++i) {
          treeNodes.emplace(tree.addNode(newTreeNode, grandChildNode));
        }
      }
    }

    const unsigned greatGrandChildCount = 17;
    SkeletonTree::Node *greatGrandChildNode;
    for (unsigned i = 0; i < greatGrandChildCount; ++i) {
      greatGrandChildNode = tree.addNode(newTreeNode, grandChildNode);
      treeNodes.emplace(greatGrandChildNode);
    }

    const unsigned greatGreatGrandChildCount = 25;
    SkeletonTree::Node *greatGreatGrandChildNode;
    for (unsigned i = 0; i < greatGreatGrandChildCount; ++i) {
      greatGreatGrandChildNode = tree.addNode(newTreeNode, grandChildNode);
      treeNodes.emplace(greatGreatGrandChildNode);

      // insert some sub-trees
      if (i == 5 || i == 9 || i == 18) {
        const unsigned tmp = 16;
        for (unsigned i = 0; i < tmp; ++i) {
          treeNodes.emplace(
              tree.addNode(newTreeNode, greatGreatGrandChildNode));
        }
      }
    }

    const unsigned greatGreatGreatGrandChildCount = 31;
    SkeletonTree::Node *greatGreatGreatGrandChildNode;
    for (unsigned i = 0; i < greatGreatGreatGrandChildCount; ++i) {
      greatGreatGreatGrandChildNode =
          tree.addNode(newTreeNode, greatGreatGrandChildNode);
      treeNodes.emplace(greatGreatGreatGrandChildNode);
    }

    return tree;
  }

  static void shouldConstructTreeRootNode() {
    SkeletonTree::Node rootTreeNode = createRootTreeNode();
  }

  static void shouldConstructTree() {
    SkeletonTree::Node rootTreeNode = createRootTreeNode();

    SkeletonTree tree(rootTreeNode);
  }

  static void shouldConstructTreeNode() {
    SkeletonTree::Node node = createTreeNode();
  }

  static void shouldAddChildNodeToRoot() {
    SkeletonTree::Node rootTreeNode = createRootTreeNode();

    SkeletonTree tree(rootTreeNode);

    SkeletonTree::Node *rootNode = tree.getRootNode();

    SkeletonTree::Node newTreeNode = createTreeNode();
    tree.addNode(newTreeNode, rootNode);

    std::vector<SkeletonTree::Node *> rootNodechildren =
        rootNode->getChildNodes();
    assert(rootNodechildren.size() == 1);
  }

  static void shouldAddChildrenNodesToRoot() {
    SkeletonTree::Node rootTreeNode = createRootTreeNode();

    SkeletonTree tree(rootTreeNode);

    SkeletonTree::Node *rootNode = tree.getRootNode();

    SkeletonTree::Node newTreeNode = createTreeNode();
    tree.addNode(newTreeNode, rootNode);

    {
      std::vector<SkeletonTree::Node *> rootNodeChildren =
          rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 1);
    }

    tree.addNode(newTreeNode, rootNode);

    {
      std::vector<SkeletonTree::Node *> rootNodeChildren =
          rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 2);
    }
  }

  static void shouldAddGrandchildNodeToRoot() {
    SkeletonTree::Node rootTreeNode = createRootTreeNode();

    SkeletonTree tree(rootTreeNode);

    SkeletonTree::Node *rootNode = tree.getRootNode();

    SkeletonTree::Node newTreeNode = createTreeNode();

    SkeletonTree::Node *newChildNode = tree.addNode(newTreeNode, rootNode);

    std::vector<SkeletonTree::Node *> rootNodeChildren =
        rootNode->getChildNodes();
    assert(rootNodeChildren.size() == 1);

    tree.addNode(newTreeNode, newChildNode);

    std::vector<SkeletonTree::Node *> childNodeChildren =
        newChildNode->getChildNodes();
    assert(childNodeChildren.size() == 1);
  }

  static void shouldAddGreatgrandchildNodesToRoot() {
    SkeletonTree::Node rootTreeNode = createRootTreeNode();

    SkeletonTree tree(rootTreeNode);

    SkeletonTree::Node *rootNode = tree.getRootNode();

    SkeletonTree::Node newTreeNode = createTreeNode();

    SkeletonTree::Node *newChildNode = tree.addNode(newTreeNode, rootNode);

    {
      std::vector<SkeletonTree::Node *> rootNodeChildren =
          rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 1);
    }

    tree.addNode(newTreeNode, rootNode);

    {
      std::vector<SkeletonTree::Node *> rootNodeChildren =
          rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 2);
    }

    tree.addNode(newTreeNode, newChildNode);

    {
      std::vector<SkeletonTree::Node *> childNodeChildren =
          newChildNode->getChildNodes();
      assert(childNodeChildren.size() == 1);
    }

    SkeletonTree::Node *grandchildNode =
        tree.addNode(newTreeNode, newChildNode);

    {
      std::vector<SkeletonTree::Node *> rootNodeChildren =
          rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 2);
    }

    tree.addNode(newTreeNode, grandchildNode);

    {
      std::vector<SkeletonTree::Node *> grandchildNodeChildren =
          grandchildNode->getChildNodes();
      assert(grandchildNodeChildren.size() == 1);
    }

    tree.addNode(newTreeNode, grandchildNode);

    {
      std::vector<SkeletonTree::Node *> grandchildNodeChildren =
          grandchildNode->getChildNodes();
      assert(grandchildNodeChildren.size() == 2);
    }
  }

  static void shouldEnumerateTree() {
    std::set<SkeletonTree::Node *> treeNodes;
    SkeletonTree tree = createTree(treeNodes);

    // todo: rm; debugging
    // std::cout << "node count: " << treeNodes.size() << std::endl;
    // std::cout << "----- nodes -----" << std::endl;
    // for (SkeletonTree::Node *nextNode : treeNodes) {
    //   std::cout << nextNode << std::endl;
    // }
    // std::cout << "----------" << std::endl;

    tree.enumerateDepthFirst([&treeNodes](SkeletonTree::Node *nextNode) {
      // todo: rm; debugging
      // std::cout << "next node: " << nextNode << std::endl;

      // check that all nodes exist in the list
      // ... and remove each node from the list
      auto it = treeNodes.find(nextNode);
      if (it == treeNodes.end())
        throw std::runtime_error("unrecognized node");

      treeNodes.erase(nextNode);
    });

    // verify that all nodes were found
    assert(treeNodes.size() == 0);
  }
};
