#pragma once

#include <cfloat>
#include <chrono>
#include <cmath>
#include <vector>

#include "MotionFrameCollection.hpp"
#include "SkeletonTree.hpp"

class Skeleton {
public:
  Skeleton()
      : defaultFramesPerSecond_(120), framesPerSecond_(defaultFramesPerSecond_),
        lastInterpolatedFrameIndex_(0) {}

  Skeleton(const SkeletonTree &skeletonTree,
           const MotionFrameCollection &motionFrameCollection)
      : skeletonTree_(skeletonTree),
        motionFrameCollection_(motionFrameCollection),
        defaultFramesPerSecond_(120), framesPerSecond_(defaultFramesPerSecond_),
        lastInterpolatedFrameIndex_(0) {}

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

  void updateFPS(int fpsDelta) { framesPerSecond_ += fpsDelta; }

  void applyNextFrame() {
    // if displaying initial frame, reset the animation start time
    if (!isAnimate_) {
      animationStartTime_ = std::chrono::system_clock::now();
      timeLastFrameRendered_ = animationStartTime_;

      skeletonTree_.updateChannels(motionFrameCollection_.getFrames()[0]);

      isAnimate_ = true;
    } else {
      // if the current time is not past the standard (30 fps) frame period,
      // prematurely return
      std::chrono::system_clock::time_point currentTime =
          std::chrono::system_clock::now();

      std::chrono::duration<double> defaultFramePeriod(1 /
                                                       defaultFramesPerSecond_);
      std::chrono::duration<double> currentFramePeriod =
          currentTime - timeLastFrameRendered_;
      if (currentFramePeriod < defaultFramePeriod) {
        return;
      }

      //// determine frames to interpolate between
      std::chrono::duration<double> framePeriod(1 / framesPerSecond_);

      double tmp =
          std::chrono::duration<double>(currentTime - animationStartTime_)
              .count() /
          framePeriod.count();

      int firstFrameIndex = int(floor(tmp)) + lastInterpolatedFrameIndex_;
      int secondFrameIndex = firstFrameIndex + 1;
      double interpolationParameter = tmp - floor(tmp);

      std::vector<MotionFrameCollection::Frame> frames =
          motionFrameCollection_.getFrames();

      // reset the animation to the LAST frame if FPS is negative
      if (framesPerSecond_ < 0 && lastInterpolatedFrameIndex_ == 0) {

        lastInterpolatedFrameIndex_ =
            motionFrameCollection_.getFrames().size() - 2;
      }
      // reset animation if frame collection bounds exceeded
      else if (firstFrameIndex >= frames.size() - 1) {
        // reset animation
        animationStartTime_ = currentTime;
        skeletonTree_.updateChannels(motionFrameCollection_.getFrames()[0]);
        lastInterpolatedFrameIndex_ = 0;
      } else {
        // for each pair of node rotations from both frames, convert the
        // rotations to quaternions, interpolate, and assign the interpolated
        // quaternions to the appropriate nodes
        skeletonTree_.updateChannels(frames[firstFrameIndex],
                                     frames[secondFrameIndex],
                                     interpolationParameter);

        if (firstFrameIndex != lastInterpolatedFrameIndex_) {
          double delta = (firstFrameIndex - lastInterpolatedFrameIndex_) *
                         (1 / framesPerSecond_);
          delta *= 1000000000;

          std::chrono::nanoseconds nanosecondDelta((int)delta);
          animationStartTime_ += nanosecondDelta;

          lastInterpolatedFrameIndex_ = firstFrameIndex;
        }
      }

      timeLastFrameRendered_ = currentTime;
    }
  }

  bool isPaused() const { return isPaused_; }

  void pauseAnimation() { isPaused_ = true; }

  void unpauseAnimation() {
    if (isPaused_) {
      animationStartTime_ = std::chrono::system_clock::now();

      isPaused_ = false;
    }
  }

  void reset() {
    // reset animation parameters
    isAnimate_ = false;
    animationStartTime_ = std::chrono::system_clock::now();
    timeLastFrameRendered_ = animationStartTime_;
    framesPerSecond_ = defaultFramesPerSecond_;
    lastInterpolatedFrameIndex_ = 0;

    // zero channels
    MotionFrameCollection::Frame zeroFrame =
        std::vector<float>(motionFrameCollection_.getFrames()[0].size(), 0);

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
  SkeletonTree skeletonTree_;
  MotionFrameCollection motionFrameCollection_;

  // animation parameters
  bool isAnimate_, isPaused_;
  std::chrono::system_clock::time_point animationStartTime_,
      timeLastFrameRendered_;
  double defaultFramesPerSecond_, framesPerSecond_;

  int lastInterpolatedFrameIndex_;
};
