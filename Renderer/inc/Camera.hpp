#pragma once

class Camera {
 public:
  enum CAMERA_PROJECTION_MODE { ORTHOGRAPHIC, PERSPECTIVE };

  Camera() {
    cameraProjectionMode = ORTHOGRAPHIC;
  }

  CAMERA_PROJECTION_MODE getCameraProjectionMode() const {
    return cameraProjectionMode;
  }

  void setCameraProjectionMode(CAMERA_PROJECTION_MODE newCameraProjectionMode) {
    cameraProjectionMode = newCameraProjectionMode;
  }

 private:
  CAMERA_PROJECTION_MODE cameraProjectionMode;
};