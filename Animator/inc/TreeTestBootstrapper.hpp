#include <cassert>
#include <set>

#include "Tree.hpp"

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
  static Tree::Node createRootTreeNode() {
    Tree::Node::Channel translationChannel;
    Tree::Node::Channel angleChannel;
    Tree::Node::Offset offset;

    return Tree::Node(Tree::Node(translationChannel, angleChannel, offset));
  }

  static Tree::Node createTreeNode() {
    Tree::Node::Channel angleChannel;
    Tree::Node::Offset offset;

    return Tree::Node(Tree::Node(angleChannel, offset));
  }

  static Tree createTree(std::set<Tree::Node *> &treeNodes) {
    treeNodes.clear();

    Tree::Node rootTreeNode = createRootTreeNode();

    Tree tree(rootTreeNode);

    Tree::Node *rootNode = tree.getRootNode();

    treeNodes.emplace(rootNode);

    Tree::Node newTreeNode = createTreeNode();

    const unsigned rootChildCount = 5;
    Tree::Node *childNode;
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
    Tree::Node *grandChildNode;
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
    Tree::Node *greatGrandChildNode;
    for (unsigned i = 0; i < greatGrandChildCount; ++i) {
      greatGrandChildNode = tree.addNode(newTreeNode, grandChildNode);
      treeNodes.emplace(greatGrandChildNode);
    }

    const unsigned greatGreatGrandChildCount = 25;
    Tree::Node *greatGreatGrandChildNode;
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
    Tree::Node *greatGreatGreatGrandChildNode;
    for (unsigned i = 0; i < greatGreatGreatGrandChildCount; ++i) {
      greatGreatGreatGrandChildNode =
          tree.addNode(newTreeNode, greatGreatGrandChildNode);
      treeNodes.emplace(greatGreatGreatGrandChildNode);
    }

    return tree;
  }

  static void shouldConstructTreeRootNode() {
    Tree::Node rootTreeNode = createRootTreeNode();
  }

  static void shouldConstructTree() {
    Tree::Node rootTreeNode = createRootTreeNode();

    Tree tree(rootTreeNode);
  }

  static void shouldConstructTreeNode() { Tree::Node node = createTreeNode(); }

  static void shouldAddChildNodeToRoot() {
    Tree::Node rootTreeNode = createRootTreeNode();

    Tree tree(rootTreeNode);

    Tree::Node *rootNode = tree.getRootNode();

    Tree::Node newTreeNode = createTreeNode();
    tree.addNode(newTreeNode, rootNode);

    std::vector<Tree::Node *> rootNodechildren = rootNode->getChildNodes();
    assert(rootNodechildren.size() == 1);
  }

  static void shouldAddChildrenNodesToRoot() {
    Tree::Node rootTreeNode = createRootTreeNode();

    Tree tree(rootTreeNode);

    Tree::Node *rootNode = tree.getRootNode();

    Tree::Node newTreeNode = createTreeNode();
    tree.addNode(newTreeNode, rootNode);

    {
      std::vector<Tree::Node *> rootNodeChildren = rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 1);
    }

    tree.addNode(newTreeNode, rootNode);

    {
      std::vector<Tree::Node *> rootNodeChildren = rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 2);
    }
  }

  static void shouldAddGrandchildNodeToRoot() {
    Tree::Node rootTreeNode = createRootTreeNode();

    Tree tree(rootTreeNode);

    Tree::Node *rootNode = tree.getRootNode();

    Tree::Node newTreeNode = createTreeNode();

    Tree::Node *newChildNode = tree.addNode(newTreeNode, rootNode);

    std::vector<Tree::Node *> rootNodeChildren = rootNode->getChildNodes();
    assert(rootNodeChildren.size() == 1);

    tree.addNode(newTreeNode, newChildNode);

    std::vector<Tree::Node *> childNodeChildren = newChildNode->getChildNodes();
    assert(childNodeChildren.size() == 1);
  }

  static void shouldAddGreatgrandchildNodesToRoot() {
    Tree::Node rootTreeNode = createRootTreeNode();

    Tree tree(rootTreeNode);

    Tree::Node *rootNode = tree.getRootNode();

    Tree::Node newTreeNode = createTreeNode();

    Tree::Node *newChildNode = tree.addNode(newTreeNode, rootNode);

    {
      std::vector<Tree::Node *> rootNodeChildren = rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 1);
    }

    tree.addNode(newTreeNode, rootNode);

    {
      std::vector<Tree::Node *> rootNodeChildren = rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 2);
    }

    tree.addNode(newTreeNode, newChildNode);

    {
      std::vector<Tree::Node *> childNodeChildren =
          newChildNode->getChildNodes();
      assert(childNodeChildren.size() == 1);
    }

    Tree::Node *grandchildNode = tree.addNode(newTreeNode, newChildNode);

    {
      std::vector<Tree::Node *> rootNodeChildren = rootNode->getChildNodes();
      assert(rootNodeChildren.size() == 2);
    }

    tree.addNode(newTreeNode, grandchildNode);

    {
      std::vector<Tree::Node *> grandchildNodeChildren =
          grandchildNode->getChildNodes();
      assert(grandchildNodeChildren.size() == 1);
    }

    tree.addNode(newTreeNode, grandchildNode);

    {
      std::vector<Tree::Node *> grandchildNodeChildren =
          grandchildNode->getChildNodes();
      assert(grandchildNodeChildren.size() == 2);
    }
  }

  static void shouldEnumerateTree() {
    std::set<Tree::Node *> treeNodes;
    Tree tree = createTree(treeNodes);

    // todo: rm; debugging
    // std::cout << "node count: " << treeNodes.size() << std::endl;
    // std::cout << "----- nodes -----" << std::endl;
    // for (Tree::Node *nextNode : treeNodes) {
    //   std::cout << nextNode << std::endl;
    // }
    // std::cout << "----------" << std::endl;

    tree.enumerateDepthFirst([&treeNodes](Tree::Node *nextNode) {
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
