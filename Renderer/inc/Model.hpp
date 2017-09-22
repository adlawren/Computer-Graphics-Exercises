#pragma once

#include <cfloat>

class Model {
 public:
  Model() {
    displacement_ = std::vector<float>{0.0, 0.0, 0.0};
    scale_ = std::vector<float>{1.0, 1.0, 1.0};
  }

  std::string getName() const {
    return modelName_;
  }

  void setName(const std::string& newModelName) {
    modelName_ = newModelName;
  }

  std::vector<std::vector<float>> getVertices() const {
    return vertices_;
  }
  void addVertex(const std::vector<float> newVertex) {
    if (newVertex.size() != 3)
      throw std::runtime_error("Vertices must contain 3 numbers");

    vertices_.push_back(newVertex);

    // todo: do better
    // ... really, vertices should be a list of pairs;
    // ... coupling the vertex position and colors
    for (int i = 0; i < 3; ++i) {
      colors_.push_back(0.0f);
    }
  }

  void printVertices() const {
    for (auto vertex : vertices_) {
      std::cout << std::endl << "v ";
      for (float vertexValue : vertex) {
        std::cout << vertexValue << " ";
      }
    }

    std::cout << std::endl;
  }

  std::vector<float> getColors() const {
    return colors_;
  }

  std::vector<std::vector<unsigned>> getPolygons() {
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

  void printPolygons() const {
    for (auto polygon : polygons_) {
      std::cout << "f ";

      for (auto vertexIndex : polygon) {
        std::cout << vertexIndex << " ";
      }

      std::cout << std::endl;
    }
  }

  std::vector<float> getCenter() const {
    std::vector<float> center(3);
    for (auto vertex : vertices_) {
      center[0] += vertex[0];
      center[1] += vertex[1];
      center[2] += vertex[2];
    }

    center[0] /= vertices_.size();
    center[1] /= vertices_.size();
    center[2] /= vertices_.size();

    return center;
  }

  std::vector<float> getDimensions() const {
    float minX = FLT_MAX, maxX = 0.0, minY = FLT_MAX, maxY = 0.0,
          minZ = FLT_MAX, maxZ = 0.0;
    for (auto vertex : vertices_) {
      float x = vertex[0], y = vertex[1], z = vertex[2];
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

  // todo: do this another way
  void normalizeVertices() {
    // Translate model to the origin and scale vertices
    std::vector<float> modelCenter = getCenter();

    std::vector<float> modelDimensions = getDimensions();
    float maxDimension = std::max(
        std::max(modelDimensions[0], modelDimensions[1]), modelDimensions[2]);

    std::vector<float> scale = std::vector<float>{
        1 / maxDimension, 1 / maxDimension, 1 / maxDimension};
    scale[0] *= 1.25;
    scale[1] *= 1.25;
    scale[2] *= 1.25;

    for (std::vector<float>& vertex : vertices_) {
      vertex[0] -= modelCenter[0];
      vertex[0] *= scale[0];

      vertex[1] -= modelCenter[1];
      vertex[1] *= scale[1];

      vertex[2] -= modelCenter[2];
      vertex[2] *= scale[2];
    }
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
    displacement_[0] += delta[0];
    displacement_[1] += delta[1];
    displacement_[2] += delta[2];
  }

  // todo: figure out how to enforce a certain number of significant digit
  void writeToFile(const std::string& filePath) {
    std::ofstream outputFileStream(filePath);
    if (outputFileStream.is_open()) {
      outputFileStream << "o " << modelName_ << std::endl;

      // write vertices
      for (auto vertex : vertices_) {
        outputFileStream << "v ";
        for (int i = 0; i < vertex.size(); ++i) {
          outputFileStream << vertex[i];
          if (i < vertex.size() - 1)
            outputFileStream << " ";
        }

        outputFileStream << std::endl;
      }

      // write ***1-indexed*** faces
      for (auto polygon : polygons_) {
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

  std::vector<std::vector<float>> vertices_;
  std::vector<float> colors_;  // todo: do better

  std::vector<std::vector<unsigned>> polygons_;

  std::vector<float> displacement_;
  std::vector<float> scale_;

  friend class ModelFactory;
};