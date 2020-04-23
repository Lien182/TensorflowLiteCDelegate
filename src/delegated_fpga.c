//delegated_fpga.c
#include <stdio.h>
#include <stdlib.h>
#include "tensorflow/lite/c/c_api.h"

#include "tensorflow/lite/builtin_ops.h"
#include "tensorflow/lite/c/builtin_op_data.h"

#include "delegated_fpga.h"


int cnt = 0;
int cnt2 = 0;

typedef struct
{
    int32_t type; 
}
t_fpga_delegate;




  void* kernel_init(TfLiteContext* context, const char* buffer, size_t length)
{

    printf("Kernel Init %d; buffer=%x, length= %ld\n", cnt++, (uint32_t*)buffer, length);

    // In the node init phase, initialize MyDelegate instance
    TfLiteDelegateParams* delegate_params = (TfLiteDelegateParams*) buffer;        
    
    printf("Nodes to replace: %d; first node= %d \n", delegate_params->nodes_to_replace->size, delegate_params->nodes_to_replace->data[0]);
    printf("InputTensor to replace: %d; first node= %d \n", delegate_params->input_tensors->size, delegate_params->input_tensors->data[0]);
    printf("OutputTensor to replace: %d; first node= %d \n", delegate_params->output_tensors->size, delegate_params->output_tensors->data[0]);
    
    //printf("Padding:=%d,stride_width=%d, stride_height=%d \n", delegate_params->padding, delegate_params->stride_width, delegate_params->stride_height);
/*
    MyDelegate* my_delegate = new MyDelegate;
    if (!my_delegate->Init(context, params)) {
      return nullptr;
    }
    return my_delegate;
  }
  */
   return 0;

}

void kernel_free(TfLiteContext* context, void* buffer)
{
    printf("Kernel Free \n");
    return;
}

TfLiteStatus kernel_invoke(TfLiteContext* context, TfLiteNode* node)
{
    printf("Kernel Invoke \n");
    return kTfLiteOk;
}

TfLiteStatus kernel_prepare(TfLiteContext* context, TfLiteNode* node)
{
    printf("Kernel Prepare \n");
    return kTfLiteOk;
}


TfLiteRegistration GetMyDelegateNodeRegistration()
{
    TfLiteRegistration kernel_registration;
    //kernel_registration.builtin_code = kTfLiteBuiltinDelegate;
    
    kernel_registration.builtin_code = kTfLiteBuiltinConv2d;
    
    kernel_registration.custom_name = "MyDelegate";
    kernel_registration.free = &kernel_free;
  
    kernel_registration.init = &kernel_init;
    kernel_registration.invoke = &kernel_invoke;
    kernel_registration.prepare = &kernel_prepare;
    return kernel_registration;
}

static bool SupportedOp(const TfLiteRegistration* registration) {
    switch (registration->builtin_code) {
    case kTfLiteBuiltinDensify:
        printf("Yes, i support this operation: kTfLiteBuiltinFullyConnected \n");
        return false;

      case kTfLiteBuiltinConv2d:      
        printf("Yes, i support this operation: kTfLiteBuiltinConv2d %d\n", cnt2++);
        return true;
      case kTfLiteBuiltinDepthwiseConv2d:
        printf("Yes, i support this operation: kTfLiteBuiltinDepthwiseConv2d \n");
        return false;
      default:
        printf("No, i dont like this code %d \n", registration->builtin_code);
        return false;
    }
  }


TfLiteStatus DelegatePrepare(TfLiteContext* context, TfLiteDelegate* delegate) {

  // Claim all nodes that can be evaluated by the delegate and ask the
  // framework to update the graph with delegate kernel instead.
  // Reserve 1 element, since we need first element to be size.
  
    TfLiteIntArray* plan;

    TfLiteStatus ret_val;

    TF_LITE_ENSURE_STATUS(context->GetExecutionPlan(context, &plan));

    TfLiteIntArray* supported_nodes = TfLiteIntArrayCreate(plan->size);
    TfLiteNode* node;
    TfLiteRegistration* registration;

    int iterator = 0;

    for (int node_index = 0; node_index < plan->size; node_index++) 
    {
        TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(
            context, node_index, &node, &registration));
        if (SupportedOp(registration)) 
        {
            supported_nodes->data[++iterator] = node_index;
        }
    }

    // Set first element to the number of nodes to replace.
    supported_nodes->data[0] = iterator - 1;
    TfLiteRegistration my_delegate_kernel_registration =  GetMyDelegateNodeRegistration();

    // This call split the graphs into subgraphs, for subgraphs that can be
    // handled by the delegate, it will replace it with a
    // 'my_delegate_kernel_registration'

    printf("Build supgraphs \n");
    ret_val = context->ReplaceNodeSubsetsWithDelegateKernels(
      context, my_delegate_kernel_registration,
      supported_nodes, delegate);
  
    printf("Build supgraphs done \n");

    TfLiteIntArrayFree(supported_nodes);

    return ret_val;
}



void FreeBufferHandle(TfLiteContext* context, TfLiteDelegate* delegate,
                      TfLiteBufferHandle* handle) {
  // Do any cleanups.
}



TfLiteStatus CopyToBufferHandle(TfLiteContext* context,
                                TfLiteDelegate* delegate,
                                TfLiteBufferHandle buffer_handle,
                                TfLiteTensor* tensor) {
  // Copies data from tensor to delegate buffer if needed.
  printf("Copy data to Buffer \n");
  return kTfLiteOk;
}



TfLiteStatus CopyFromBufferHandle(TfLiteContext* context,
                                  TfLiteDelegate* delegate,
                                  TfLiteBufferHandle buffer_handle,
                                  TfLiteTensor* tensor) {
  // Copies the data from delegate buffer into the tensor raw memory.
  printf("Copy data from Buffer \n");
  return kTfLiteOk;
}



// Caller takes ownership of the returned pointer.
TfLiteDelegate* CreateMyDelegate(void) {

    TfLiteDelegate* delegate = (TfLiteDelegate*)malloc(sizeof(TfLiteDelegate));

    if (delegate == NULL)
      return 0;

    *delegate = TfLiteDelegateCreate();


    delegate->data_ = NULL;
    delegate->flags = kTfLiteDelegateFlagsNone;
    delegate->Prepare = &DelegatePrepare;
    // This cannot be null.
    delegate->CopyFromBufferHandle = &CopyFromBufferHandle;
    // This can be null.
    delegate->CopyToBufferHandle = &CopyToBufferHandle;
    // This can be null.
    delegate->FreeBufferHandle = &FreeBufferHandle;
    return delegate;
}
