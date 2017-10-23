#pragma once

#include "MotionFrameCollection.hpp"
#include "SkeletonTree.hpp"

class Skeleton {
public:
  Skeleton() {}

  Skeleton(const SkeletonTree &skeletonTree,
           const MotionFrameCollection &motionFrameCollection)
      : skeletonTree_(skeletonTree),
        motionFrameCollection_(motionFrameCollection) {}

  const SkeletonTree &getSkeletonTree() const { return skeletonTree_; }

  const MotionFrameCollection &getMotionFrameCollection() const {
    return motionFrameCollection_;
  }

  // todo
  // CC
  Skeleton(const Skeleton &otherSkeleton) {
    motionFrameCollection_ = otherSkeleton.motionFrameCollection_;
    skeletonTree_ = otherSkeleton.skeletonTree_;
  }

  // todo
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

private:
  SkeletonTree skeletonTree_;
  MotionFrameCollection motionFrameCollection_;
};
