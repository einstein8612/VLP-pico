#include "model.h"

#include "../degradation_model/degradation_model.h"
#include "model_data.h"

#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/recording_micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace
{
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;

    constexpr int kTensorArenaSize = 128 * 1024;
    alignas(16) uint8_t tensor_arena[kTensorArenaSize];
} // namespace

static inline float fastInvSqrt(float x) {
    float xhalf = 0.5f * x;
    int i = *(int*)&x;              // treat float bits as int
    i = 0x5f3759df - (i >> 1);     // initial magic guess
    x = *(float*)&i;
    x = x * (1.5f - xhalf * x * x); // 1st Newton iteration
    // Optionally do a 2nd iteration for better accuracy:
    // x = x * (1.5f - xhalf * x * x);
    return x;
}


// Normalize a vector of length 'len' in-place using Euclidean norm
static inline void normalize(float *vec, int len)
{
    float norm = 0.0f;
    for (int i = 0; i < len; i++)
    {
        norm += vec[i] * vec[i];
    }
    float inv_norm = fastInvSqrt(norm);

    if (norm > 0.0f)
    {
        for (int i = 0; i < len; i++)
        {
            vec[i] *= inv_norm;
        }
    }
}

TfLiteStatus load_model(void)
{
    tflite::InitializeTarget();

    model = tflite::GetModel(model_int8_tflite);
    TFLITE_CHECK_EQ(model->version(), TFLITE_SCHEMA_VERSION);

    // This pulls in all the operation implementations we need.
    static tflite::MicroMutableOpResolver<3> op_resolver;
    TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
    TF_LITE_ENSURE_STATUS(op_resolver.AddRelu());
    TF_LITE_ENSURE_STATUS(op_resolver.AddAdd());

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, op_resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    TF_LITE_ENSURE_STATUS(allocate_status);

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    return kTfLiteOk;
}

// Predict function that takes an array of 36 LED values and outputs the predicted x and y coordinates
// Will scale the input values using the scalars from the degradation model, normalize them, and then run inference
// This replaces the LED input with the scaled values from the degradation model as a side effect
TfLiteStatus predict(float leds[36], float *x, float *y)
{
    if (!interpreter)
        return kTfLiteError;

    float *scalars = get_scalars();

    float temp_input[36];
    // Copy the input data to the temp input tensor
    for (int i = 0; i < 36; i++)
    {
        temp_input[i] = leds[i] * scalars[i];
        leds[i] = temp_input[i];
    }
    // Normalize the input data with Euclidean normalization
    normalize(temp_input, 36);

    // Place the quantized input in the model's input tensor
    for (int i = 0; i < 36; i++)
    {
        input->data.int8[i] = static_cast<int8_t>(
            (temp_input[i] / input->params.scale) + input->params.zero_point);
    }

    // Run inference
    TF_LITE_ENSURE_STATUS(interpreter->Invoke());

    // Copy output data
    int8_t x_quantized = output->data.int8[0];
    int8_t y_quantized = output->data.int8[1];

    // Dequantize the output from integer to floating-point
    *x = (x_quantized - output->params.zero_point) * output->params.scale;
    *y = (y_quantized - output->params.zero_point) * output->params.scale;

    return kTfLiteOk;
}