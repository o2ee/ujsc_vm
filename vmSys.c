// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015
#include "vm.h"

Var vm_print(MicroVM* vm, Var* params)
{
    double  pu8Buf  = params[0].value.fval;
    Var res;
    printf("%f",pu8Buf);
    return res;
}

Var vm_ledOn(MicroVM* vm, Var* params)
{
    Var res;
    printf("led on\n");
    return res;
}

Var vm_ledOff(MicroVM* vm, Var* params)
{
    Var res;
    printf("led off\n");
    return res;
}

Var vm_delay(MicroVM* vm, Var* params)
{
    double  pu8Buf  = params[0].value.fval;
    Var res;
    usleep(1000*pu8Buf);
    return res;
}
