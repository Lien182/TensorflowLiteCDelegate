//delegated_fpga.h

#include "tensorflow/lite/c/common.h"


TfLiteDelegate* CreateFPGADelegate(void);
void DeleteFPGADelegate(TfLiteDelegate * delegate);