#pragma once

#include <array>
#include <exception>
#include <fstream>
#include <functional>
#include <sstream>
#include <vector>

class SkeletonTree {
public:
  class Node {
  public:
    typedef std::array<float, 3> Channel;
    typedef std::array<float, 3> Offset;

    Node()
        : nodeLabel_(""), nodeName_(""), childNodes_(std::vector<Node *>(0)) {}

    Node(const Channel &initialTranslationChannel,
         const Channel &initialAngleChannel, const Offset &initialOffset)
        : nodeLabel_(""), nodeName_(""), childNodes_(std::vector<Node *>(0)) {
      translationChannel_ = initialTranslationChannel;
      angleChannel_ = initialAngleChannel;
      offset_ = initialOffset;
    }

    Node(const Channel &initialAngleChannel, const Offset &initialOffset)
        : nodeLabel_(""), nodeName_(""), childNodes_(std::vector<Node *>(0)) {
      angleChannel_ = initialAngleChannel;
      offset_ = initialOffset;
    }

    // CC
    Node(const Node &otherNode)
        : nodeLabel_(""), nodeName_(""), childNodes_(std::vector<Node *>(0)) {
      nodeLabel_ = otherNode.nodeLabel_;
      nodeName_ = otherNode.nodeName_;
      translationChannel_ = otherNode.translationChannel_;
      angleChannel_ = otherNode.angleChannel_;
      offset_ = otherNode.offset_;

      // recursively copy construct the child nodes on the heap
      for (Node *otherNodeChild : otherNode.childNodes_) {
        childNodes_.push_back(new Node(*otherNodeChild));
      }
    }

    // AO
    Node &operator=(const Node &otherNode) {
      nodeLabel_ = otherNode.nodeLabel_;
      nodeName_ = otherNode.nodeName_;
      translationChannel_ = otherNode.translationChannel_;
      angleChannel_ = otherNode.angleChannel_;
      offset_ = otherNode.offset_;

      // recursively copy construct the child nodes on the heap
      for (Node *otherNodeChild : otherNode.childNodes_) {
        childNodes_.push_back(new Node(*otherNodeChild));
      }

      return *this;
    }

    ~Node() {
      for (Node *nextChildNode : childNodes_) {
        delete nextChildNode;
      }
    }

    std::string getLabel() const { return nodeLabel_; }

    void setLabel(const std::string &newNodeLabel) {
      nodeLabel_ = newNodeLabel;
    }

    std::string getName() const { return nodeName_; }

    void setName(const std::string &newNodeName) { nodeName_ = newNodeName; }

    Channel getTranslationChannel() const { return translationChannel_; }

    void setTranslationChannel(const Channel &newTranslationChannel) {
      translationChannel_ = newTranslationChannel;
    }

    Channel getAngleChannel() const { return angleChannel_; }

    void setAngleChannel(const Channel &newAngleChannel) {
      angleChannel_ = newAngleChannel;
    }

    Offset getOffset() const { return offset_; }

    void setOffset(const Offset &newOffset) { offset_ = newOffset; }

    void addChildNode(const Node &newNode) {
      Node *newChildNode = new Node(newNode);
      childNodes_.push_back(newChildNode);
    }

    std::vector<Node *> getChildNodes() const { return childNodes_; }

  private:
    std::string nodeLabel_, nodeName_;

    Channel translationChannel_; // note: only used in root node
    Channel angleChannel_;
    Offset offset_;

    std::vector<Node *> childNodes_;

    friend class SkeletonTree;
  };

  SkeletonTree() {}

  SkeletonTree(const Node &rootNode) { rootNode_ = rootNode; }

  // CC
  SkeletonTree(const SkeletonTree &otherTree) {
    rootNode_ = otherTree.rootNode_;
  }

  // AO
  SkeletonTree &operator=(const SkeletonTree &otherTree) {
    rootNode_ = otherTree.rootNode_;
    return *this;
  }

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
      std::function<void(SkeletonTree::Node *nextNode)> callback) const {
    std::vector<Node *> nextNodes;
    nextNodes.push_back((Node *)&rootNode_);
    while (nextNodes.size()) {
      SkeletonTree::Node *nextNode = nextNodes.back();
      nextNodes.pop_back();

      callback(nextNode);

      // todo: reverse iteration order to enumerate the first children first
      // ... instead of last?
      for (Node *nextChildNode : nextNode->getChildNodes()) {
        nextNodes.push_back(nextChildNode);
      }
    }
  }

  void writeToFileStream(std::ofstream &outputFileStream) const {
    if (outputFileStream.is_open()) {
      writeNodeToFile((Node *)&rootNode_, outputFileStream);
    } else {
      throw std::runtime_error("Failed to open skeleton output file");
    }
  }

private:
  Node rootNode_;

  // recursive function to write nodes
  void writeNodeToFile(Node *node, std::ofstream &outputFileStream,
                       bool isRoot = true, int tabDepth = 0) const {
    // pad output by tab depth
    std::stringstream tabOffsetStream;
    for (int i = 0; i < tabDepth; ++i)
      tabOffsetStream << '\t';

    // write node name
    outputFileStream << tabOffsetStream.str() << node->getLabel() << " "
                     << node->getName() << std::endl;

    outputFileStream << tabOffsetStream.str() << "{" << std::endl;

    std::stringstream indentedTabOffsetStream;
    for (int i = 0; i < tabDepth + 1; ++i)
      indentedTabOffsetStream << '\t';

    // write node offset
    Node::Offset offset = node->getOffset();
    outputFileStream << indentedTabOffsetStream.str() << "OFFSET " << offset[0]
                     << " " << offset[1] << " " << offset[2] << std::endl;

    // write node channels if the node is not an endsite
    if (node->getChildNodes().size()) {
      if (isRoot) {
        outputFileStream
            << indentedTabOffsetStream.str()
            << "CHANNELS 6 Xposition Yposition Zposition Zrotation "
               "Yrotation Xrotation "
            << std::endl;
      } else {
        outputFileStream << indentedTabOffsetStream.str()
                         << "CHANNELS 3 Zrotation Yrotation Xrotation "
                         << std::endl;
      }
    }

    // write node children
    for (Node *nextChildNode : node->getChildNodes()) {
      writeNodeToFile(nextChildNode, outputFileStream, false, tabDepth + 1);
    }

    outputFileStream << tabOffsetStream.str() << "}" << std::endl;
  }
};
