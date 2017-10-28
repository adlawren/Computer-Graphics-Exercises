#pragma once

#include <cfloat>
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

  // returns the dimension boundary values which contain the anamation sequence
  // in the format: [min_x, max_x, min_y, max_y, min_z, max_z]
  std::array<float, 6> getAnimationDimensionBounds() {
    std::array<float, 6> animationDimensionBounds;
    float min_x = FLT_MAX, max_x = 0.0f, min_y = FLT_MAX, max_y = 0.0f,
          min_z = FLT_MAX, max_z = 0.0f;

    // iterate over the motion frame collection to find the dimesion boundaries
    for (MotionFrameCollection::Frame nextFrame :
         motionFrameCollection_.getFrames()) {
      float nextFrameRootTranslationX = nextFrame[0];
      float nextFrameRootTranslationY = nextFrame[1];
      float nextFrameRootTranslationZ = nextFrame[2];

      min_x =
          nextFrameRootTranslationX < min_x ? nextFrameRootTranslationX : min_x;
      max_x =
          nextFrameRootTranslationX > max_x ? nextFrameRootTranslationX : max_x;
      min_y =
          nextFrameRootTranslationY < min_y ? nextFrameRootTranslationY : min_y;
      max_y =
          nextFrameRootTranslationY > max_y ? nextFrameRootTranslationY : max_y;
      min_z =
          nextFrameRootTranslationZ < min_z ? nextFrameRootTranslationZ : min_z;
      max_z =
          nextFrameRootTranslationZ > max_z ? nextFrameRootTranslationZ : max_z;
    }

    // calculate the maximum dimesions of the skeleton in the neutral position
    std::array<float, 6> skeletonTreeDimensionBounds =
        skeletonTree_.getSkeletonTreeDimensionBounds();

    // pad the motion frame dimension boundaries by the maximum dimensions of
    // the neutal position
    min_x += skeletonTreeDimensionBounds[0];
    max_x += skeletonTreeDimensionBounds[1];
    min_y += skeletonTreeDimensionBounds[2];
    max_y += skeletonTreeDimensionBounds[3];
    min_z += skeletonTreeDimensionBounds[4];
    max_z += skeletonTreeDimensionBounds[5];

    animationDimensionBounds[0] = min_x;
    animationDimensionBounds[1] = max_x;
    animationDimensionBounds[2] = min_y;
    animationDimensionBounds[3] = max_y;
    animationDimensionBounds[4] = min_z;
    animationDimensionBounds[5] = max_z;

    return animationDimensionBounds;
  }

private:
  unsigned nextFrameIndex_;
  SkeletonTree skeletonTree_;
  MotionFrameCollection motionFrameCollection_;
};
