#pragma once

#include "Model.hpp"

class ModelFactory {
 public:
  ModelFactory(const std::string& modelDataFilePath) {
    // Load the data from the file into a data structure
    std::ifstream modelDataFileStream(modelDataFilePath);
    model_ = loadModel(modelDataFileStream);
  }

  Model getModel() const {
    return model_;
  }

  Model getNormalizedModel() const {
    Model normalizedModel(model_);

    // Translate model to the origin and scale vertices
    std::vector<float> modelCenter = model_.getCenter();

    std::vector<float> modelDimensions = model_.getDimensions();
    float maxDimension = std::max(
        std::max(modelDimensions[0], modelDimensions[1]), modelDimensions[2]);

    std::vector<float> scale = std::vector<float>{
        1 / maxDimension, 1 / maxDimension, 1 / maxDimension};
    scale[0] *= 1.25;
    scale[1] *= 1.25;
    scale[2] *= 1.25;

    // The ModelFactory class is a 'friend' class of Model
    // ... so as to access the private variable vertices
    for (std::vector<float>& vertex : normalizedModel.vertices_) {
      vertex[0] -= modelCenter[0];
      vertex[0] *= scale[0];

      vertex[1] -= modelCenter[1];
      vertex[1] *= scale[1];

      vertex[2] -= modelCenter[2];
      vertex[2] *= scale[2];
    }

    return normalizedModel;
  }

 private:
  Model model_;

  Model loadModel(std::ifstream& fileStream) {
    Model modelData;

    if (!fileStream.good()) {
      throw std::runtime_error(
          "The given model specification file path is invalid");
    }

    const std::string fileFormatErrorMessage =
        "invalid model specification file format";

    unsigned currentLineNumber = 0, firstPolygonLineNumber = -1;
    std::string nextLine;
    while (std::getline(fileStream, nextLine)) {
      ++currentLineNumber;

      std::stringstream ss(nextLine);
      char c = nextLine[0];
      switch (c) {
        case 'o': {
          if (currentLineNumber != 1)
            throw std::runtime_error(fileFormatErrorMessage);

          modelData.setName(&nextLine[2]);
          break;
        }
        case 'v': {
          if (currentLineNumber > firstPolygonLineNumber)
            throw std::runtime_error(fileFormatErrorMessage);

          std::stringstream ss(nextLine);
          char c;
          float f1, f2, f3;
          if (!(ss >> c >> f1 >> f2 >> f3)) {
            throw std::runtime_error(fileFormatErrorMessage);
          }

          modelData.addVertex(std::vector<float>{f1, f2, f3});
          break;
        }
        // todo: add support for faces with vertex counts > 4
        case 'f': {
          if (firstPolygonLineNumber == -1)
            firstPolygonLineNumber = currentLineNumber;

          std::stringstream ss(nextLine);
          char c;
          unsigned u1, u2, u3, u4;

          //// Check if the face is a polygon, if so, convert to two triangles
          if ((ss >> c >> u1 >> u2 >> u3 >> u4)) {
            // Subtract by 1; the data is 1-indexed
            modelData.addPolygon(std::vector<unsigned>{u1 - 1, u2 - 1, u3 - 1});
            modelData.addPolygon(std::vector<unsigned>{u1 - 1, u3 - 1, u4 - 1});
            break;
          }

          // Reset string stream
          ss.str(nextLine);
          ss.clear();

          if ((ss >> c >> u1 >> u2 >> u3)) {
            // Subtract by 1; the data is 1-indexed
            modelData.addPolygon(std::vector<unsigned>{u1 - 1, u2 - 1, u3 - 1});
            break;
          }

          throw std::runtime_error(fileFormatErrorMessage);
        }
        default:
          throw std::runtime_error(fileFormatErrorMessage);
      }
    }

    return modelData;
  }
};