#include <stdio.h>
#include <stdint.h>
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/common.h"

#include "delegated_fpga.h"

#include "helper.h"



int main(void)
{
    uint8_t output[1024];


    uint8_t *pixels;
    int32 width;
    int32 height;
    int32 bytesPerPixel;
    ReadImageAlign("../test/grace_hopper.bmp", &pixels, &width, &height,&bytesPerPixel);
    printf("Loaded image, witdth=%d, height=%d; bytesPerPixel=%d \n", width, height, bytesPerPixel);


    TfLiteModel* model = TfLiteModelCreateFromFile("../model/mobilenet_v1_1.0_224_quant.tflite");
    TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
    TfLiteInterpreterOptionsSetNumThreads(options, 2);

    //Delegate
    //TfLiteDelegate * TFfpga = CreateMyDelegate();
    //TfLiteInterpreterOptionsAddDelegate(options, TFfpga );

    // Create the interpreter.
    TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);



    // Allocate tensors and populate the input tensor data.
    TfLiteInterpreterAllocateTensors(interpreter);
    TfLiteTensor* input_tensor =
        TfLiteInterpreterGetInputTensor(interpreter, 0);

    printf("Input Tensor type: %d\n", input_tensor->type);  

    printf("%d \n", TfLiteTensorCopyFromBuffer(input_tensor, pixels,
                                width*height*4));

    // Execute inference.
    TfLiteInterpreterInvoke(interpreter);

    // Extract the output tensor data.
    const TfLiteTensor* output_tensor =
          TfLiteInterpreterGetOutputTensor(interpreter, 0);

    TfLiteTensorCopyToBuffer(output_tensor, output, output_tensor->bytes);
    printf("OutData: Type=%d; Size=%ld \n", input_tensor->type, output_tensor->bytes);

    uint8_t max = 0;
    uint32_t index = 0;
    for(int i= 0; i < output_tensor->bytes; i++)
    {
        if(output[i] > max)
        {
            max = output[i];
            index = i;
        }
    }

    printf("Max at %i with %d \n", index, max);



    // Dispose of the model and interpreter objects.
    TfLiteInterpreterDelete(interpreter);
    TfLiteInterpreterOptionsDelete(options);
    TfLiteModelDelete(model);


    //free(pixels);
}
