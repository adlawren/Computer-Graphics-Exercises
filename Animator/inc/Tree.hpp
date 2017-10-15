#include <array>
#include <exception>
#include <functional>
#include <vector>

class Tree {
public:
  class Node {
  public:
    typedef std::array<float, 3> Channel;
    typedef std::array<float, 3> Offset;

    Node() {}

    Node(const Channel &initialTranslationChannel,
         const Channel &initialAngleChannel, const Offset &initialOffset) {
      translationChannel_ = initialTranslationChannel;
      angleChannel_ = initialAngleChannel;
      offset_ = initialOffset;
    }

    Node(const Channel &initialAngleChannel, const Offset &initialOffset) {
      angleChannel_ = initialAngleChannel;
      offset_ = initialOffset;
    }

    // CC
    Node(const Node &otherNode) {
      translationChannel_ = otherNode.translationChannel_;
      angleChannel_ = otherNode.angleChannel_;
      offset_ = otherNode.offset_;
      childNodes_ = otherNode.childNodes_;
    }

    ~Node() {
      for (Node *nextChildNode : childNodes_) {
        delete nextChildNode;
      }
    }

    void addChildNode(const Node &newNode) {
      Node *newChildNode = new Node(newNode);
      childNodes_.push_back(newChildNode);
    }

    std::vector<Node *> getChildNodes() const { return childNodes_; }

  private:
    Channel translationChannel_; // note: only used in root node
    Channel angleChannel_;
    Offset offset_;

    std::vector<Node *> childNodes_;

    friend class Tree;
  };

  Tree(const Node &rootNode) { rootNode_ = rootNode; }

  Node *getRootNode() { return (Node *)&rootNode_; }

  Node *addNode(const Node &newNode, Node *parentNode) {
    // look for the node which matches the pointer, add the child using
    // breadth-first search, add new child node to found node
    std::vector<Node *> nextNodes, nextNextNodes;
    nextNodes.push_back((Node *)&rootNode_);
    while (nextNodes.size()) {
      nextNextNodes.clear();

      for (Node *nextNode : nextNodes) {
        if (nextNode == parentNode) {
          nextNode->addChildNode(newNode);
          return nextNode->childNodes_[nextNode->childNodes_.size() - 1];
        }

        for (Node *nextNextNode : nextNode->childNodes_) {
          nextNextNodes.push_back(nextNextNode);
        }
      }

      nextNodes = nextNextNodes;
    }

    throw std::runtime_error("Parent node not found");
  }

  void enumerateDepthFirst(
      std::function<void(Tree::Node *nextNode)> callback) const {
    std::vector<Node *> nextNodes;
    nextNodes.push_back((Node *)&rootNode_);
    while (nextNodes.size()) {
      Tree::Node *nextNode = nextNodes.back();
      nextNodes.pop_back();

      callback(nextNode);

      // todo: reverse iteration order to enumerate first children first instead
      // ... of last?
      for (Node *nextChildNode : nextNode->getChildNodes()) {
        nextNodes.push_back(nextChildNode);
      }
    }
  }

private:
  Node rootNode_;
};
