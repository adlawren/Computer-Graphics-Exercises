#pragma once

#include <Eigen/Geometry>

#include "geometry.hpp"

class Quaternion {
public:
  Quaternion() { eigenQuaternion_ = Eigen::Quaternion<float>::Identity(); }

  Quaternion(const Eigen::Quaternion<float> &initialQuaternion) {
    eigenQuaternion_ = initialQuaternion;
  }

  // note: expected euler angle format is: [z_axis_angle, y_axis_angle,
  // x_axis_angle]
  Quaternion(const std::array<float, 3> &eulerAngle) {
    eigenQuaternion_ = Eigen::Quaternion<float>::Identity();

    Eigen::Quaternion<float> zAxisRotation(
        cos(degreesToRadians(eulerAngle[0]) / 2), 0.0f, 0.0f,
        sin(degreesToRadians(eulerAngle[0]) / 2));
    Eigen::Quaternion<float> yAxisRotation(
        cos(degreesToRadians(eulerAngle[1]) / 2), 0.0f,
        sin(degreesToRadians(eulerAngle[1]) / 2), 0.0f);
    Eigen::Quaternion<float> xAxisRotation(
        cos(degreesToRadians(eulerAngle[2]) / 2),
        sin(degreesToRadians(eulerAngle[2]) / 2), 0.0f, 0.0f);

    eigenQuaternion_ =
        zAxisRotation * yAxisRotation * xAxisRotation * eigenQuaternion_;
    eigenQuaternion_.normalize();
  }

  // CC
  Quaternion(const Quaternion &otherQuaterion) {
    eigenQuaternion_ = otherQuaterion.eigenQuaternion_;
  }

  // AO
  Quaternion &operator=(const Quaternion &otherQuaterion) {
    eigenQuaternion_ = otherQuaterion.eigenQuaternion_;
  }

  Eigen::Quaternion<float> getEigenQuaternion() const {
    return eigenQuaternion_;
  }

private:
  Eigen::Quaternion<float> eigenQuaternion_;
};
