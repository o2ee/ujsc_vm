// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015

#include "vm.h"

Var vm_print(MicroVM* vm, CELL* params);
Var vm_ledOn(MicroVM* vm, CELL* params);
Var vm_ledOff(MicroVM* vm, CELL* params);
Var vm_delay(MicroVM* vm, CELL* params);

NativeMethod Natives0[] = 
{
    vm_print,
    vm_ledOn,
    vm_ledOff,
    vm_delay,
};

NativeMethod* nativeTable[] = 
{
    Natives0,
};

//check if native id is valid
int isNativeValid(int id1, int id2)
{
  switch(id1)
  {
    case 0:
      if (id2 >= 4) return 0;
      else return Natives0[id2] != NULL;
    default:
       return 0;
  }
}

