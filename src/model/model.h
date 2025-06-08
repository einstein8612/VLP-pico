#ifndef MODEL_H
#define MODEL_H

#include "tensorflow/lite/core/c/common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    TfLiteStatus load_model(void);
    TfLiteStatus predict(float leds[36], float *x, float *y);

#ifdef __cplusplus
}
#endif

#endif // MODEL_H