#pragma once

#include <fstream>
#include <sstream>

#include "MotionFrameCollection.hpp"
#include "Skeleton.hpp"
#include "SkeletonTree.hpp"

class SkeletonFactory {
public:
  SkeletonFactory(const std::string &motionCaptureDataFilePath) {
    // Load the data from the file into a data structure
    std::ifstream skeletonDataFileStream(motionCaptureDataFilePath);

    skeleton_ = loadSkeleton(skeletonDataFileStream);
  }

  Skeleton getSkeleton() const { return skeleton_; }

private:
  Skeleton skeleton_;

  void getNextLineFromFileStream(std::ifstream &fileStream,
                                 std::stringstream &ss) const {
    std::string nextLine;
    std::getline(fileStream, nextLine);

    ss.str(nextLine);
    ss.clear();
  }

  void parseSkeletonHierarchy(SkeletonTree &skeletonTree,
                              std::ifstream &fileStream,
                              SkeletonTree::Node *parentNode) {
    //// recursively parse the skeleton hierarchy in the file
    std::stringstream ss;
    getNextLineFromFileStream(fileStream, ss);

    std::string openingCurlyBrace;
    ss >> openingCurlyBrace;
    if (openingCurlyBrace != "{")
      throw std::runtime_error("parsing error: expected opening curly brace");

    // update the parent's offset
    getNextLineFromFileStream(fileStream, ss);

    std::string offsetLabel;
    float offsetValue1, offsetValue2, offsetValue3;
    ss >> offsetLabel >> offsetValue1 >> offsetValue2 >> offsetValue3;
    if (offsetLabel != "OFFSET")
      throw std::runtime_error("parsing: expected offset");

    SkeletonTree::Node::Offset parsedOffset;
    parsedOffset[0] = offsetValue1;
    parsedOffset[1] = offsetValue2;
    parsedOffset[2] = offsetValue3;
    parentNode->setOffset(parsedOffset);

    // verify parent channel format
    if (parentNode->getLabel() != "End") {
      getNextLineFromFileStream(fileStream, ss);
      std::string nextLine = ss.str();

      std::string channelsLabel, channel1Name, channel2Name, channel3Name,
          channel4Name, channel5Name, channel6Name;
      unsigned channelCount;
      if (ss >> channelsLabel >> channelCount >> channel1Name >> channel2Name >>
          channel3Name >> channel4Name >> channel5Name >> channel6Name) {
        if (channelsLabel != "CHANNELS")
          throw std::runtime_error("parsing error: invalid channels label");
        if (channelCount != 6)
          throw std::runtime_error(
              "parsing error: root node should have 6 channels");
        if (channel1Name != "Xposition")
          throw std::runtime_error("parsing error: invalid channel 1 name");
        if (channel2Name != "Yposition")
          throw std::runtime_error("parsing error: invalid channel 2 name");
        if (channel3Name != "Zposition")
          throw std::runtime_error("parsing error: invalid channel 3 name");
        if (channel4Name != "Zrotation")
          throw std::runtime_error("parsing error: invalid channel 4 name");
        if (channel5Name != "Yrotation")
          throw std::runtime_error("parsing error: invalid channel 5 name");
        if (channel6Name != "Xrotation")
          throw std::runtime_error("parsing error: invalid channel 6 name");
      } else {
        ss.str(nextLine);
        ss.clear();
        if (ss >> channelsLabel >> channelCount >> channel1Name >>
            channel2Name >> channel3Name) {
          if (channelsLabel != "CHANNELS")
            throw std::runtime_error("parsing error: invalid channels label");
          if (channelCount != 3)
            throw std::runtime_error(
                "parsing error: node should have 3 channels");
          if (channel1Name != "Zrotation")
            throw std::runtime_error("parsing error: invalid channel 1 name");
          if (channel2Name != "Yrotation")
            throw std::runtime_error("parsing error: invalid channel 2 name");
          if (channel3Name != "Xrotation")
            throw std::runtime_error("parsing error: invalid channel 3 name");
        } else {
          throw std::runtime_error(
              "parsing error: node channels' line is improperly formatted");
        }
      }
    }

    std::string firstWhitespaceDelimitedSubstring;

    getNextLineFromFileStream(fileStream, ss);
    std::string nextLine = ss.str();

    ss >> firstWhitespaceDelimitedSubstring;
    while (firstWhitespaceDelimitedSubstring != "}") {
      //// add next child node

      // get node label and name
      ss.str(nextLine);
      ss.clear();

      std::string nextNodeLabel, nextNodeName;
      if (!(ss >> nextNodeLabel >> nextNodeName))
        throw std::runtime_error("parsing error: expected node label and name");

      SkeletonTree::Node *nextNode =
          skeletonTree.addNode(SkeletonTree::Node(), parentNode);
      nextNode->setLabel(nextNodeLabel);
      nextNode->setName(nextNodeName);

      parseSkeletonHierarchy(skeletonTree, fileStream, nextNode);

      // get next line
      getNextLineFromFileStream(fileStream, ss);
      nextLine = ss.str();
      ss >> firstWhitespaceDelimitedSubstring;
    }
  }

  Skeleton loadSkeleton(std::ifstream &fileStream) {
    SkeletonTree skeletonTree;

    if (!fileStream.good()) {
      throw std::runtime_error(
          "The given motion capture data file path is invalid");
    }

    std::stringstream ss;
    getNextLineFromFileStream(fileStream, ss);

    if (ss.str() != "HIERARCHY")
      throw std::runtime_error(
          "parsing error: expected hierarchy specification");

    // get, and set, root node label and name
    getNextLineFromFileStream(fileStream, ss);

    std::string rootNodeLabel, rootNodeName;
    if (!(ss >> rootNodeLabel >> rootNodeName))
      throw std::runtime_error(
          "parsing error: expected root node label and name");

    SkeletonTree::Node *rootNode = skeletonTree.getRootNode();
    rootNode->setLabel(rootNodeLabel);
    rootNode->setName(rootNodeName);

    parseSkeletonHierarchy(skeletonTree, fileStream, rootNode);

    getNextLineFromFileStream(fileStream, ss);
    if (ss.str() != "MOTION")
      throw std::runtime_error("parsing error: expected motion specification");

    getNextLineFromFileStream(fileStream, ss);

    std::string framesLabel;
    unsigned frameCount;
    if (!(ss >> framesLabel >> frameCount)) {
      throw std::runtime_error("parsing error: expected frame count");
    } else if (framesLabel != "Frames:") {
      throw std::runtime_error("parsing error: expected frame count");
    }

    getNextLineFromFileStream(fileStream, ss);

    std::string frameTimeLabel1, frameTimeLabel2;
    float frameTime;
    if (!(ss >> frameTimeLabel1 >> frameTimeLabel2 >> frameTime)) {
      throw std::runtime_error("parsing error: expected frame time");
    } else if (frameTimeLabel1 != "Frame" || frameTimeLabel2 != "Time:") {
      throw std::runtime_error("parsing error: expected frame time");
    }

    MotionFrameCollection motionFrameCollection(frameCount, frameTime);

    for (int i = 0; i < frameCount; ++i) {
      getNextLineFromFileStream(fileStream, ss);

      MotionFrameCollection::Frame nextFrame;

      float tmp;
      while (ss >> tmp) {
        nextFrame.push_back(tmp);
      }

      motionFrameCollection.addFrame(nextFrame);
    }

    return Skeleton(skeletonTree, motionFrameCollection);
  }
};
