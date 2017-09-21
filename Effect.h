
#ifndef __EFFECT_H
#define __EFFECT_H

#include <entry.h>



extern "C"
DllExport 
PF_Err EntryPointFunc(
    PF_Cmd          cmd,
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output );


#endif
