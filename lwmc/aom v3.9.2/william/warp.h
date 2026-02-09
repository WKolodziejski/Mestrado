#ifndef AOM_WARP_H
#define AOM_WARP_H

#ifdef __cplusplus
extern "C" {
#endif

double ApplyCatboostModelWrapper(
    const float* floatFeatures, int floatFeaturesSize,
    const char** catFeatures, int catFeaturesSize
);

#ifdef __cplusplus
}
#endif

#endif  // AOM_WARP_H
