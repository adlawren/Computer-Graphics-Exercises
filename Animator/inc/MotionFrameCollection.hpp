#pragma once

#include <exception>
#include <fstream>
#include <vector>

class MotionFrameCollection {
public:
  typedef std::vector<float> Frame;

  MotionFrameCollection() : currentFrameCount_(0) {}

  MotionFrameCollection(unsigned frameCount, float frameTime)
      : currentFrameCount_(0), frameCount_(frameCount), frameTime_(frameTime) {
    frames_ = std::vector<std::vector<float>>(frameCount_);
  }

  unsigned getFrameCount() const { return frameCount_; }

  void setFrameCount(unsigned newFrameCount) { frameCount_ = newFrameCount; }

  unsigned getFrameTime() const { return frameTime_; }

  void setFrameTime(float newFrameTime) { frameTime_ = newFrameTime; }

  void addFrame(const Frame &newFrame) {
    if (currentFrameCount_ == frameCount_)
      throw std::runtime_error(
          "error adding motion frame: frame buffer exceeded");

    // todo insert by index, and validate length against max
    frames_[currentFrameCount_] = newFrame;

    ++currentFrameCount_;
  }

  std::vector<Frame> getFrames() const { return frames_; }

  void writeToFileStream(std::ofstream &outputFileStream) const {
    if (outputFileStream.is_open()) {
      outputFileStream << "Frames: " << frameCount_ << std::endl;

      outputFileStream << "Frame Time: " << frameTime_ << std::endl;

      for (Frame frame : frames_) {
        for (int i = 0; i < frame.size() - 1; ++i) {
          outputFileStream << frame[i] << " ";
        }

        outputFileStream << frame[frame.size() - 1] << std::endl;
      }
    } else {
      throw std::runtime_error("Failed to open skeleton output file");
    }
  }

private:
  unsigned currentFrameCount_, frameCount_;
  float frameTime_;
  std::vector<Frame> frames_;
};
