#include <stdio.h>
#include "vm.h"

int main(int argc, char *argv[])
{
    MicroVM* vm = (MicroVM*)malloc(sizeof(MicroVM));
    int error = vmStart(vm, "/home/zhe/project/uol_lib/output.bin");
	error = vmRun(vm);
    printf("error code = %d", error);
    return 0;
}

