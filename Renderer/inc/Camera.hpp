#pragma once

class Camera {
 public:
  enum CAMERA_PROJECTION_MODE { ORTHOGRAPHIC, PERSPECTIVE };

  Camera() {
    cameraProjectionMode = ORTHOGRAPHIC;
    displacement_ = std::vector<float>(3, 0);
  }

  CAMERA_PROJECTION_MODE getCameraProjectionMode() const {
    return cameraProjectionMode;
  }

  void setCameraProjectionMode(CAMERA_PROJECTION_MODE newCameraProjectionMode) {
    cameraProjectionMode = newCameraProjectionMode;
  }

  std::vector<float> getDisplacement() const {
    return displacement_;
  }

  void translate(const std::vector<float>& delta) {
    if (delta.size() != 3)
      throw std::runtime_error(
          "Failed to translate camera: the given delta must contain 3 "
          "dimensions");

    displacement_[0] += delta[0];
    displacement_[1] += delta[1];
    displacement_[2] += delta[2];
  }

 private:
  CAMERA_PROJECTION_MODE cameraProjectionMode;
  std::vector<float> displacement_;
};