#pragma once

#include <cfloat>
#include <Eigen/Geometry>

class Model {
 public:
  Model() {
    displacement_ = std::vector<float>{0.0, 0.0, 0.0};
    scale_ = std::vector<float>{1.0, 1.0, 1.0};
    orientation_ = Eigen::Quaternion<float>::Identity();
  }

  std::string getName() const {
    return modelName_;
  }

  void setName(const std::string& newModelName) {
    modelName_ = newModelName;
  }

  std::vector<float>& getVertices() {
    return vertices_;
  }

  void addVertex(const std::vector<float> newVertex) {
    if (newVertex.size() != 3)
      throw std::runtime_error("Vertices must contain 3 numbers");

    for (float vertexValue : newVertex) {
      vertices_.push_back(vertexValue);
    }

    // todo: do better
    // ... really, vertices should be a pair of lists;
    // ... coupling the vertex position and colors
    for (int i = 0; i < 3; ++i) {
      colors_.push_back(1.0f);
    }
  }

  std::vector<float>& getColors() {
    return colors_;
  }

  std::vector<std::vector<unsigned>>& getPolygons() {
    return polygons_;
  }

  void addPolygon(const std::vector<unsigned>& newPolygon) {
    for (unsigned vertexIndex : newPolygon) {
      if (vertexIndex >= vertices_.size()) {
        throw std::runtime_error("Invalid vertex index specified");
      }
    }

    polygons_.push_back(newPolygon);
  }

  std::vector<float> getCenter() const {
    std::vector<float> center(3);
    for (unsigned i = 0; i < vertices_.size(); ++i) {
      center[i % 3] += vertices_[i];
    }

    float tmp = float(vertices_.size()) / 3.0f;
    center[0] /= tmp;
    center[1] /= tmp;
    center[2] /= tmp;

    return center;
  }

  std::vector<float> getDimensions() const {
    float minX = FLT_MAX, maxX = 0.0, minY = FLT_MAX, maxY = 0.0,
          minZ = FLT_MAX, maxZ = 0.0;
    for (unsigned i = 0; i < vertices_.size(); i += 3) {
      float x = vertices_[i], y = vertices_[i + 1], z = vertices_[i + 2];
      if (x < minX) {
        minX = x;
      }
      if (x > maxX) {
        maxX = x;
      }

      if (y < minY) {
        minY = y;
      }
      if (y > maxY) {
        maxY = y;
      }

      if (z < minZ) {
        minZ = z;
      }
      if (z > maxZ) {
        maxZ = z;
      }
    }

    return std::vector<float>{maxX - minX, maxY - minY, maxZ - minZ};
  }

  std::vector<float> getScale() const {
    return scale_;
  }

  void scale(const std::vector<float>& proportions) {
    scale_[0] *= proportions[0];
    scale_[1] *= proportions[1];
    scale_[2] *= proportions[2];
  }

  std::vector<float> getDisplacement() const {
    return displacement_;
  }

  void translate(const std::vector<float>& delta) {
    if (delta.size() != 3)
      throw std::runtime_error(
          "Failed to translate model: the given delta must contain 3 "
          "dimensions");

    Eigen::Matrix<float, 3, 1> relativeDelta;
    relativeDelta << delta[0], delta[1], delta[2];
    Eigen::Matrix<float, 3, 3> mat = orientation_.toRotationMatrix();
    relativeDelta = mat * relativeDelta;

    displacement_[0] += relativeDelta[0];
    displacement_[1] += relativeDelta[1];
    displacement_[2] += relativeDelta[2];
  }

  Eigen::Quaternion<float> getOrientation() const {
    return orientation_;
  }

  void rotate(const Eigen::Quaternion<float>& delta) {
    orientation_ = delta * orientation_;
    orientation_.normalize();
  }

  // todo: figure out how to enforce a certain number of significant digits
  void writeToFile(const std::string& filePath) {
    std::ofstream outputFileStream(filePath);
    if (outputFileStream.is_open()) {
      outputFileStream << "o " << modelName_;

      // Write vertices
      for (unsigned i = 0; i < vertices_.size(); ++i) {
        if (i == (vertices_.size() - 1)) {
          outputFileStream << vertices_[i] << std::endl;
        } else if (i % 3 == 0) {
          outputFileStream << std::endl << "v " << vertices_[i] << " ";
        } else {
          outputFileStream << vertices_[i] << " ";
        }
      }

      // Write ***1-indexed*** faces
      for (std::vector<unsigned> polygon : polygons_) {
        outputFileStream << "f ";
        for (int i = 0; i < polygon.size(); ++i) {
          outputFileStream << polygon[i] + 1;
          if (i < polygon.size() - 1)
            outputFileStream << " ";
        }

        outputFileStream << std::endl;
      }

      outputFileStream.close();
    } else {
      throw std::runtime_error("Failed to open model output file");
    }
  }

 private:
  std::string modelName_;

  std::vector<float> vertices_;
  std::vector<float> colors_;

  std::vector<std::vector<unsigned>> polygons_;

  std::vector<float> displacement_;
  std::vector<float> scale_;

  Eigen::Quaternion<float> orientation_;

  friend class ModelFactory;
};