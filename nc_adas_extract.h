#ifndef NC_ADAS_EXTRACT_H
#define NC_ADAS_EXTRACT_H
#include "nc_cnn_aiware_runtime.h"
#include "nc_adas_types.h"
int adas_extract(const pp_result_buf *det,
                 const pp_result_buf *seg,
                 const pp_result_buf *lane,
                 int overlay_w,
                 int overlay_h,
                 AdasResult *out);
void adas_extract_debug_print(const AdasResult *r);
#endif