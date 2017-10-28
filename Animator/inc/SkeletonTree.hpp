#pragma once

#include <array>
#include <cfloat>
#include <exception>
#include <fstream>
#include <functional>
#include <sstream>
#include <vector>

#include "MotionFrameCollection.hpp"

class SkeletonTree {
public:
  class Node {
  public:
    typedef std::array<float, 3> Channel;
    typedef std::array<float, 3> Offset;

    Node()
        : nodeLabel_(""), nodeName_(""), childNodes_(std::vector<Node *>(0)) {
      translationChannel_[0] = 0.0f;
      translationChannel_[1] = 0.0f;
      translationChannel_[2] = 0.0f;

      angleChannel_[0] = 0.0f;
      angleChannel_[1] = 0.0f;
      angleChannel_[2] = 0.0f;

      offset_[0] = 0.0f;
      offset_[1] = 0.0f;
      offset_[2] = 0.0f;
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

  void enumerateDepthFirst(std::function<void(Node *nextNode)> callback) const {
    std::vector<Node *> nextNodes;
    nextNodes.push_back((Node *)&rootNode_);
    while (nextNodes.size()) {
      Node *nextNode = nextNodes.back();
      nextNodes.pop_back();

      callback(nextNode);

      std::vector<Node *> childNodes = nextNode->getChildNodes();
      for (auto reverseIterator = childNodes.rbegin();
           reverseIterator != childNodes.rend(); ++reverseIterator) {
        nextNodes.push_back(*reverseIterator);
      }
    }
  }

  void updateChannels(const MotionFrameCollection::Frame &motionFrame) {
    unsigned nextChannelIndex = 0;
    bool isRoot = true;
    enumerateDepthFirst(
        [&motionFrame, &nextChannelIndex, &isRoot](Node *nextNode) {
          // skip endsites
          if (nextNode->getLabel() == "End")
            return;

          if (isRoot) {
            //// set translation channel
            Node::Channel translationChannel;
            for (int i = 0; i < 3; ++i) {
              translationChannel[i] = motionFrame[3 * nextChannelIndex + i];
            }

            ++nextChannelIndex;

            nextNode->setTranslationChannel(translationChannel);

            //// set rotation channel
            Node::Channel rotationChannel;
            for (int i = 0; i < 3; ++i) {
              rotationChannel[i] = motionFrame[3 * nextChannelIndex + i];
            }

            ++nextChannelIndex;

            nextNode->setAngleChannel(rotationChannel);

            isRoot = false;
          } else {
            //// set rotation channel
            Node::Channel rotationChannel;
            for (int i = 0; i < 3; ++i) {
              rotationChannel[i] = motionFrame[3 * nextChannelIndex + i];
            }

            ++nextChannelIndex;

            nextNode->setAngleChannel(rotationChannel);
          }
        });
  }

  void writeToFileStream(std::ofstream &outputFileStream) const {
    if (outputFileStream.is_open()) {
      writeNodeToFile((Node *)&rootNode_, outputFileStream);
    } else {
      throw std::runtime_error("Failed to open skeleton output file");
    }
  }

  // returns the dimension boundary values which contain the skeleton tree in
  // the neutral position, in the format: [min_x, max_x, min_y, max_y, min_z,
  // max_z]
  std::array<float, 6> getSkeletonTreeDimensionBounds() {
    std::array<float, 6> skeletonTreeDimensionBounds;
    skeletonTreeDimensionBounds[0] = FLT_MAX; // initial min_x
    skeletonTreeDimensionBounds[1] = 0.0f;    // initial max_x
    skeletonTreeDimensionBounds[2] = FLT_MAX; // initial min_y
    skeletonTreeDimensionBounds[3] = 0.0f;    // initial max_y
    skeletonTreeDimensionBounds[4] = FLT_MAX; // initial min_z
    skeletonTreeDimensionBounds[5] = 0.0f;    // initial max_z

    std::vector<float> accumulatedOffset(3, 0);

    calculateSkeletonTreeDimesionBounds(
        (Node *)&rootNode_, skeletonTreeDimensionBounds, accumulatedOffset);

    return skeletonTreeDimensionBounds;
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

  // recursive method to compute the maximum dimension bounds of the skeleton
  // tree, in the neutral position
  void calculateSkeletonTreeDimesionBounds(
      Node *node, std::array<float, 6> &dimensionBounds,
      std::vector<float> accumulatedOffset) const {
    Node::Offset nodeOffset = node->getOffset();

    std::vector<float> newAccumulatedOffset = accumulatedOffset;
    newAccumulatedOffset[0] += nodeOffset[0];
    newAccumulatedOffset[1] += nodeOffset[1];
    newAccumulatedOffset[2] += nodeOffset[2];

    // compare accumulated offset with dimensions
    dimensionBounds[0] = newAccumulatedOffset[0] < dimensionBounds[0]
                             ? newAccumulatedOffset[0]
                             : dimensionBounds[0];
    dimensionBounds[1] = newAccumulatedOffset[0] > dimensionBounds[1]
                             ? newAccumulatedOffset[0]
                             : dimensionBounds[1];
    dimensionBounds[2] = newAccumulatedOffset[1] < dimensionBounds[2]
                             ? newAccumulatedOffset[1]
                             : dimensionBounds[2];
    dimensionBounds[3] = newAccumulatedOffset[1] > dimensionBounds[3]
                             ? newAccumulatedOffset[1]
                             : dimensionBounds[3];
    dimensionBounds[4] = newAccumulatedOffset[2] < dimensionBounds[4]
                             ? newAccumulatedOffset[2]
                             : dimensionBounds[4];
    dimensionBounds[5] = newAccumulatedOffset[2] > dimensionBounds[5]
                             ? newAccumulatedOffset[2]
                             : dimensionBounds[5];

    for (Node *nextChildNode : node->getChildNodes()) {
      calculateSkeletonTreeDimesionBounds(nextChildNode, dimensionBounds,
                                          newAccumulatedOffset);
    }
  }
};
