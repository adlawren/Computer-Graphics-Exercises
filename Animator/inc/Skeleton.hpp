#pragma once

#include <vector>

#include "MotionFrameCollection.hpp"
#include "SkeletonTree.hpp"

class Skeleton {
public:
  Skeleton() : nextFrameIndex_(0) {}

  Skeleton(const SkeletonTree &skeletonTree,
           const MotionFrameCollection &motionFrameCollection)
      : nextFrameIndex_(0), skeletonTree_(skeletonTree),
        motionFrameCollection_(motionFrameCollection) {}

  SkeletonTree &getSkeletonTree() { return skeletonTree_; }

  MotionFrameCollection &getMotionFrameCollection() {
    return motionFrameCollection_;
  }

  // CC
  Skeleton(const Skeleton &otherSkeleton) {
    motionFrameCollection_ = otherSkeleton.motionFrameCollection_;
    skeletonTree_ = otherSkeleton.skeletonTree_;
  }

  // AO
  Skeleton &operator=(const Skeleton &otherSkeleton) {
    motionFrameCollection_ = otherSkeleton.motionFrameCollection_;
    skeletonTree_ = otherSkeleton.skeletonTree_;

    return *this;
  }

  void writeToFile(const std::string &filePath) const {
    std::ofstream outputFileStream(filePath);

    outputFileStream << "HIERARCHY" << std::endl;
    skeletonTree_.writeToFileStream(outputFileStream);

    outputFileStream << "MOTION" << std::endl;
    motionFrameCollection_.writeToFileStream(outputFileStream);

    outputFileStream.close();
  }

  void applyNextFrame() {
    std::vector<MotionFrameCollection::Frame> frames =
        motionFrameCollection_.getFrames();

    MotionFrameCollection::Frame nextFrame =
        frames[nextFrameIndex_++ % frames.size()];

    skeletonTree_.updateChannels(nextFrame);

    // avoid integer overflow
    if (nextFrameIndex_ == frames.size()) {
      nextFrameIndex_ = 0;
    }
  }

  void reset() {
    nextFrameIndex_ = 0;

    // zero channels
    MotionFrameCollection::Frame zeroFrame = std::vector<float>(
        motionFrameCollection_.getFrames()[nextFrameIndex_].size(), 0);

    skeletonTree_.updateChannels(zeroFrame);
  }

private:
  unsigned nextFrameIndex_;
  SkeletonTree skeletonTree_;
  MotionFrameCollection motionFrameCollection_;
};
