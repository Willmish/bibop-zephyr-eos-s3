#include "model_functions.hpp"

#include "model.hpp"
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>

/* Globals, used for compatibility with Arduino-style sketches. */
namespace {
	tflite::ErrorReporter *error_reporter = nullptr;
	const tflite::Model *model = nullptr;
	tflite::MicroInterpreter *interpreter = nullptr;
	TfLiteTensor *input = nullptr;
	TfLiteTensor *output = nullptr;
	int inference_count = 0;

	constexpr int kTensorArenaSize = 2000;
	uint8_t tensor_arena[kTensorArenaSize];
}  /* namespace */

/* The name of this function is important for Arduino compatibility. */
void setup_model(void)
{
	/* Set up logging. Google style is to avoid globals or statics because of
	 * lifetime uncertainty, but since this has a trivial destructor it's okay.
	 * NOLINTNEXTLINE(runtime-global-variables)
	 */
	static tflite::MicroErrorReporter micro_error_reporter;

	error_reporter = &micro_error_reporter;

	/* Map the model into a usable data structure. This doesn't involve any
	 * copying or parsing, it's a very lightweight operation.
	 */
	model = tflite::GetModel(linear_bp_model_lighter_quant_tflite);
	if (model->version() != TFLITE_SCHEMA_VERSION) {
		TF_LITE_REPORT_ERROR(error_reporter,
						"Model provided is schema version %d not equal "
						"to supported version %d.",
						model->version(), TFLITE_SCHEMA_VERSION);
		return;
	}

	/* This pulls in all the operation implementations we need.
	 * NOLINTNEXTLINE(runtime-global-variables)
	 */
	static tflite::AllOpsResolver resolver;

	/* Build an interpreter to run the model with. */
	static tflite::MicroInterpreter static_interpreter(
		model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
	interpreter = &static_interpreter;

	/* Allocate memory from the tensor_arena for the model's tensors. */
	TfLiteStatus allocate_status = interpreter->AllocateTensors();
	if (allocate_status != kTfLiteOk) {
		TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
		return;
	}

	/* Obtain pointers to the model's input and output tensors. */
	input = interpreter->input(0);
	output = interpreter->output(0);

	/* Keep track of how many inferences we have performed. */
	inference_count = 0;
}

/* TODO: this needs to be supercharged to do the preprocessing of the data, probably should have reading from the main buffer */
/* Right now puts a dummy buffer of 6 features to the model */
Inferred loop_model(Features *ftrs)
{
	/* Calculate an x value to feed into the model. We compare the current
	 * inference_count to the number of inferences per cycle to determine
	 * our position within the range of possible x values the model was
	 * trained on, and use this to calculate a value.
	 */
	//float position = static_cast < float > (inference_count) /
	//		 static_cast < float > (kInferencesPerCycle);
	//float x = position * kXrange;

    static float buf[6] = { 0.34314155, -0.02509177, 0.43447335, 0.74105764, 0.32367285, 1.66480085 };
    static int8_t scaled_buf[6] = { };
	/* Quantize the input from floating-point to integer */
    for (int i = 0; i < 6; ++i)
        scaled_buf[i] = buf[i] / input->params.scale + input->params.zero_point;

	//int8_t x_quantized = x / input->params.scale + input->params.zero_point;
	///* Place the quantized input in the model's input tensor */
	input->data.int8[0] = scaled_buf[0];
	input->data.int8[1] = scaled_buf[1];
	input->data.int8[2] = scaled_buf[2];
	input->data.int8[3] = scaled_buf[3];
	input->data.int8[4] = scaled_buf[4];
	input->data.int8[5] = scaled_buf[5];

    TF_LITE_REPORT_ERROR(error_reporter, "scale: %f zp: %d\n", input->params.scale, input->params.zero_point);
    TF_LITE_REPORT_ERROR(error_reporter, "Model input: %x %x %x %x %x %x\n",
                        scaled_buf[0], scaled_buf[1], scaled_buf[2],
                        scaled_buf[3], scaled_buf[4], scaled_buf[5]);

	/* Run inference, and report any error */
	TfLiteStatus invoke_status = interpreter->Invoke();
	if (invoke_status != kTfLiteOk) {
		TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed\n");
		return { 0.f, 0.f };
	}

	/* Obtain the quantized output from model's output tensor */
	int8_t sbp_scaled = output->data.int8[0];
	int8_t dbp_scaled = output->data.int8[1];
	/* Dequantize the output from integer to floating-point */
	float sbp = (sbp_scaled - output->params.zero_point) * output->params.scale;
	float dbp = (dbp_scaled - output->params.zero_point) * output->params.scale;

	/* Output the results. A custom HandleOutput function can be implemented
	 * for each supported hardware target.
	 */

    TF_LITE_REPORT_ERROR(error_reporter, "Model outut: SBP: %f DBP: %f\n",
                         sbp, dbp);

	/* Increment the inference_counter, and reset it if we have reached
	 * the total number per cycle
	 */
	inference_count += 1;
	//if (inference_count >= kInferencesPerCycle) inference_count = 0;

    return { sbp, dbp };
}
