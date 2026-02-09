#include "model_obmc_ctb.h"
#include <string>
#include <vector>
#include <cmath>

/* Model data */
static const struct CatboostModel {
  CatboostModel() = default;
  unsigned int FloatFeatureCount = 3;
  unsigned int CatFeatureCount = 0;
  unsigned int BinaryFeatureCount = 3;
  unsigned int TreeCount = 1;
  std::vector<std::vector<float>> FloatFeatureBorders = {
    {753},
    {2242.5},
    {0.5}
  };
  unsigned int TreeDepth[1] = {3};
  unsigned int TreeSplits[3] = {1, 2, 0};
  unsigned int BorderCounts[3] = {1, 1, 1};
  float Borders[3] = {753.f, 2242.5f, 0.5f};

  /* Aggregated array of leaf values for trees. Each tree is represented by a separate line: */
  double LeafValues[8][1] = {
    {0.01045667414677607}, {0.02120536159113863}, {0.0190146002940848}, {0.02385745267229985}, {-0.02288147599538634}, {-0.02308620335576534}, {0}, {0}
  };
  double Scale = 1;
  double Biases[1] = {0};
  unsigned int Dimension = 1;
} CatboostModelStatic;

/* Model applicator */
std::vector<double> ApplyCatboostModelMulti(
    const std::vector<float>& floatFeatures
) {
  const struct CatboostModel& model = CatboostModelStatic;

  /* Binarize features */
  std::vector<unsigned char> binaryFeatures(model.BinaryFeatureCount);
  unsigned int binFeatureIndex = 0;
  for (unsigned int i = 0; i < model.FloatFeatureCount; ++i) {
    for(unsigned int j = 0; j < model.BorderCounts[i]; ++j) {
      binaryFeatures[binFeatureIndex] = (unsigned char)(floatFeatures[i] > model.Borders[binFeatureIndex]);
      ++binFeatureIndex;
    }
  }

  /* Extract and sum values from trees */
  std::vector<double> results(model.Dimension, 0.0);
  const unsigned int* treeSplitsPtr = model.TreeSplits;
  const auto* leafValuesForCurrentTreePtr = model.LeafValues;
  for (unsigned int treeId = 0; treeId < model.TreeCount; ++treeId) {
    const unsigned int currentTreeDepth = model.TreeDepth[treeId];
    unsigned int index = 0;
    for (unsigned int depth = 0; depth < currentTreeDepth; ++depth) {
      index |= (binaryFeatures[treeSplitsPtr[depth]] << depth);
    }

    for (unsigned int resultIndex = 0; resultIndex < model.Dimension; resultIndex++) {
      results[resultIndex] += leafValuesForCurrentTreePtr[index][resultIndex];
    }

    treeSplitsPtr += currentTreeDepth;
    leafValuesForCurrentTreePtr += 1 << currentTreeDepth;
  }

  std::vector<double> finalResults(model.Dimension);
  for (unsigned int resultId = 0; resultId < model.Dimension; resultId++) {
    finalResults[resultId] = model.Scale * results[resultId] + model.Biases[resultId];
  }
  return finalResults;
}

double ApplyCatboostModel(
    const std::vector<float>& floatFeatures
) {
  return ApplyCatboostModelMulti(floatFeatures)[0];
}

// Also emit the API with catFeatures, for uniformity
std::vector<double> ApplyCatboostModelMulti(
    const std::vector<float>& floatFeatures,
    const std::vector<std::string>&
) {
  return ApplyCatboostModelMulti(floatFeatures);
}

double ApplyCatboostModel(
    const std::vector<float>& floatFeatures,
    const std::vector<std::string>&
) {
  return ApplyCatboostModelMulti(floatFeatures)[0];
}

double Sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

double ApplyCatboostModelWrapper(
    const float* floatFeatures, int floatFeaturesSize
) {
    std::vector<float> floatFeaturesVec(floatFeatures, floatFeatures + floatFeaturesSize);

    double prediction = ApplyCatboostModel(floatFeaturesVec);
    return std::round(Sigmoid(prediction));
}