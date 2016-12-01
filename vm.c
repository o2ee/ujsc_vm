// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015

#include "vm.h"
#include "instruction.h"

error_t vmInit(MicroVM* vm)
{
	uint32_t addr;
	uint32_t len;
    uint8_t *pStr;
	
    //header
    addr = *(uint32_t*)(vm->pOpcodeAddr+VM_HEADER_OFFSET);
    if(addr != 0x756A7363)
    {
        return VM_INVALID_HEADER;
    }
	//get address index of opcode
	len  = *(uint32_t*)vm->pOpcodeAddr;
    //get offset address of string pool
	addr = *(uint32_t*)(vm->pOpcodeAddr+VM_STRPOOL_ADDR_OFFSET); 
	pStr = vm->pOpcodeAddr+addr;
	//return error when opcode size is zero
	if(vm->codeSize==0)
		return VM_BAD_CODE_SIZE_ERR;
	
//	//initialize string pool
//    printf("String pool length = %d\n", len);
//	if(len>0)
//	{
//		strPool = (Obj*)malloc(len*sizeof(Obj));
//		if (strPool == NULL)
//		{
//			printf("Cannot malloc string pool segments\r\n");
//			return VM_MALLOC_STRING_POOL_ERR;
//		}
//		for(i=0;i<len;)
//		{
//			 strLen = *(int32_t*)pStr;
//			 pStr+=4;
//			 tempStr = (strPool+i);
//             tempStr->var = malloc(2*sizeof(uint32_t));
//			 tempStr->classId = vm->stringClassId;
//             setInt(tempStr, 4, strLen);
//             array = malloc(sizeof(Array));
//             setRef(tempStr, 0, array);
//             array->addr = pStr;
//			 array->len = strLen;
//			 pStr=pStr+strLen;
//			 i++;
//		}
//	}
	//get size of static memory
	len  =   *(uint32_t*)(vm->pOpcodeAddr+VM_STATIC_LEN_OFFSET);
	vm->staticSize = len;
    printf("static memory size: %d bytes\r\n",len);
	if(len>0)
	{
        vm->pStaticAddr = (Var*)malloc(len*sizeof(Var));

		if(vm->pStaticAddr == NULL)
		{
			printf("Cannot malloc static memory segments\r\n");
			return VM_MALLOC_STATIC_MEM_ERR;
		}
        memset(vm->pStaticAddr,0,len);
	}
	return VM_OK;
}

Prop * findProp(Var * var, int hash)
{
    Prop * localProp = (Prop*)var->value.addr;
    if(localProp == NULL)
        return NULL;
    if(localProp->hash == hash)
        return localProp;
    while(localProp != NULL)
    {
        if(localProp->hash == hash)
            return localProp;
        localProp = (Prop*)localProp->next;
    }
    return localProp;
}

Prop *addProp(Var *var, int hash, Var value)
{
    Prop * tempProp;
    Prop * localProp = (Prop*)var->value.addr;

    if(localProp == NULL)
    {
        localProp = (Prop*)malloc(sizeof(Prop));
        memset((void*)localProp, 0, sizeof(Prop));
        localProp->hash = hash;
        localProp->value = value;
        var->value.addr = localProp;
        return localProp;
    }

    while(localProp != NULL)
    {
        if(localProp->next == NULL)
            break;

        localProp = (Prop*)localProp->next;
    }
    tempProp = (Prop*)malloc(sizeof(Prop));
    memset((void*)tempProp, 0, sizeof(Prop));
    tempProp->hash = hash;
    tempProp->value = value;
    localProp->next = tempProp;
    return tempProp;
}

void deleteProp(Var *var, int hash)
{
    Prop * tempProp;
    Prop * localProp = (Prop*)var->value.addr;
    if(localProp == NULL)
        return;
    if(localProp->hash == hash)
    {
        free(localProp);
        var->value.addr = NULL;
        return;
    }

    while(localProp != NULL)
    {
        tempProp = (Prop*)localProp->next;
        if(tempProp->hash == hash)
        {
            localProp->next = tempProp->next;
            free(tempProp);
            return;
        }
        localProp = (Prop*)localProp->next;
    }
}

error_t vmRun(MicroVM* vm)
{
    register Var*  sp;
    register Var*  param;
    register Var*  local;
    register uint8_t*  pc;

    Prop * tempProp;
    int hash;
    int tempInt1, tempInt2;
    Var tempVar;
    Var * pTempVar;
    Var * pThis;

    uint8_t*  pcBaseAddr;
    Var* pStatic;
    Var* inv;

    Var* spHigh;
    Var* spLow;
    Var cell;
    Var* temp_param;
    Var* temp_local;
	
	int64_t temp64_value;
	int32_t temp32_value;
	int16_t temp16_value;
	int8_t  temp8_value;
	int32_t error;

	NativeMethod** nativeTable;
	NativeMethod native;
  
    Var* pStackAddr;
	
	error = VM_OK;
	pStackAddr = vm->pStackAddr;
	
	pStatic = vm->pStaticAddr;
    sp = vm->pStackAddr;
    param = vm->pParamAddr;
	pc=vm->pOpcodeAddr+vm->pcOffset;
	pcBaseAddr=vm->pOpcodeAddr;

	sp = sp + vm->stackOffset;
    local = sp;

	nativeTable = vm->nativeTable;
    spHigh = pStackAddr+(vm->stackSize/sizeof(Var))-1;
	spLow  = pStackAddr;
	inv = spHigh;
	while(1)
	{ 	
		//check if stack overflows
		if((sp>=(inv)||sp<spLow))
		{
			error = VM_STACK_OVERFLOW_ERR;
			goto VM_ERROR;
		}
		switch(*pc)
	 	{
			case Nop:pc++;break;
			//locals
            case LoadVarLocal:++sp; *sp = *(local+*(uint8_t*)(pc+1));pc=pc+2;break;
            case LoadVarLocalWide:++sp; *sp = *(local+*(uint16_t*)(pc+1));pc=pc+3;break;

            case StoreVarLocal:*(local+*(uint8_t*)(pc+1)) = *sp;sp--;pc=pc+2;break;
            case StoreVarLocalWide:*(local+*(uint16_t*)(pc+1))= *sp;sp--;pc=pc+3;break;

            case LoadVarParam:++sp; *sp = *(param+*(uint8_t*)(pc+1));pc=pc+2;break;
            case StoreVarParam:*(param+*(uint8_t*)(pc+1)) = *sp;sp--;pc=pc+2;break;


            case LoadVarStatic:++sp; *sp = *(pStatic+*(uint32_t*)(pc+1));pc=pc+5;break;
            case StoreVarStatic:*(pStatic+*(uint32_t*)(pc+1)) = *sp;sp--;pc=pc+5;break;

            case LoadProp:
                ++sp;
                pTempVar = pThis;
                hash = *(uint32_t*)(pc+1);
                tempProp = findProp(pTempVar, hash);
                *sp = tempProp->value;
                pc=pc+5;
                break;
            case StoreProp:
                hash = *(uint32_t*)(pc+1);
                pTempVar = pThis;
                tempProp = findProp(pTempVar, hash);
                if(tempProp == NULL)
                    tempProp = addProp(pTempVar, hash, *sp);
                sp--;
                pc=pc+5;
                break;
            case DeleteProp:
                hash = *(uint32_t*)(pc+1);
                tempVar = *(sp-1);
                deleteProp(&tempVar, hash);
                pc=pc+5;
                break;
            case LoadBool:
                ++sp;
                sp->value.fval = *(uint8_t*)(pc+1);
                sp->type = TYPE_BOOLEAN;
                pc += 5;
                break;
            case LoadDouble:
                ++sp;
                sp->value.fval = *(double*)(pc+1);
                sp->type = TYPE_NUMBER;
                pc += 9;
                break;
			//arithmetic of 64-bit
            case VarAdd:(sp-1)->value.fval = (sp-1)->value.fval + sp->value.fval;pc++;break;
            case VarSub:(sp-1)->value.fval = (sp-1)->value.fval - sp->value.fval;pc++;break;
            case VarMul:(sp-1)->value.fval = (sp-1)->value.fval * sp->value.fval;pc++;break;
            case VarDiv:(sp-1)->value.fval = (sp-1)->value.fval / sp->value.fval;pc++;break;
            case VarAnd:
                tempInt1 = (sp-1)->value.fval;
                tempInt2 = sp->value.fval;
                tempInt1 = tempInt1 & tempInt2;
                (sp-1)->value.fval = tempInt1;
                pc++;
                break;
            case VarOr:
                tempInt1 = (sp-1)->value.fval;
                tempInt2 = sp->value.fval;
                tempInt1 = tempInt1 | tempInt2;
                (sp-1)->value.fval = tempInt1;
                pc++;
                break;
            case VarXor:
                tempInt1 = (sp-1)->value.fval;
                tempInt2 = sp->value.fval;
                tempInt1 = tempInt1 ^ tempInt2;
                (sp-1)->value.fval = tempInt1;
                pc++;
                break;
            case VarNot:
                tempInt1 = sp->value.fval;
                tempInt1 = ~tempInt1;
                sp->value.fval = tempInt1;
                pc++;
                break;
            case VarRem:
                tempInt1 = (sp-1)->value.fval;
                tempInt2 = sp->value.fval;
                tempInt1 = tempInt1 % tempInt2;
                (sp-1)->value.fval = tempInt1;
                pc++;
                break;
            case VarShiftLeft:
                tempInt1 = (sp-1)->value.fval;
                tempInt2 = sp->value.fval;
                tempInt1 = tempInt1 << tempInt2;
                (sp-1)->value.fval = tempInt1;
                sp--;
                pc++;
                break;
            case VarShiftRight:
                tempInt1 = (sp-1)->value.fval;
                tempInt2 = sp->value.fval;
                tempInt1 = tempInt1 >> tempInt2;
                (sp-1)->value.fval = tempInt1;
                sp--;
                pc++;
                break;
            case VarInc:sp->value.fval++;pc++;break;
            case VarDec:sp->value.fval--;pc++;break;
            case VarNeg:sp->value.fval=-sp->value.fval;pc++;break;

			//jump
			case Jump:pc=pcBaseAddr+(*(uint32_t*)(pc+1));break;
            case JumpZero:    if(sp->value.fval==0) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-1;break;
            case JumpNotZero: if(sp->value.fval!=0) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-1;break;
            case JumpEq:   if((sp-1)->value.fval==sp->value.fval) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
            case JumpNotEq:if((sp-1)->value.fval!=sp->value.fval) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
            case JumpGt:   if((sp-1)->value.fval>sp->value.fval)  pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
            case JumpLt:   if((sp-1)->value.fval<sp->value.fval)  pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
            case JumpGtEq: if((sp-1)->value.fval>=sp->value.fval) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
            case JumpLtEq: if((sp-1)->value.fval<=sp->value.fval) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
			//short jump
			case SJump:        pc=pc+(*(int8_t*)(pc+1));break;
            case SJumpZero:    if(sp->value.fval==0) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-1;break;
            case SJumpNotZero: if(sp->value.fval!=0) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-1;break;
            case SJumpEq:   if((sp-1)->value.fval==sp->value.fval) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
            case SJumpNotEq:if((sp-1)->value.fval!=sp->value.fval) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
            case SJumpGt:   if((sp-1)->value.fval>sp->value.fval)  pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
            case SJumpLt:   if((sp-1)->value.fval<sp->value.fval)  pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
            case SJumpGtEq: if((sp-1)->value.fval>=sp->value.fval) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
            case SJumpLtEq: if((sp-1)->value.fval<=sp->value.fval) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
			//method call
			case Call:
					  temp_param = param;
					  temp_local = local;
					  param = (sp-(*(pc+5))+1);
					  sp++;
					  local=sp;
					  sp=sp+(*(int16_t*)(pc+6));
                      sp->value.addr=pc+8;
					  sp++;
                      sp->value.ival=*(pc+5);
					  sp++;
                      sp->value.ival=*(int16_t*)(pc+6);
					  sp++;
                      sp->value.addr = temp_param;
					  sp++;
                      sp->value.addr = temp_local;
                      temp32_value = *(uint32_t*)(pc+1);
					  pc=pcBaseAddr+*(uint32_t*)(pc+1);
					  break;
			//native call
			case CallNative:
							temp16_value = *(pc+3);
							if(!isNativeValid(*(pc+1),*(pc+2)))
							{
								error = VM_INVALID_NATIVE_ERR;
								goto VM_ERROR;
							}
							native = nativeTable[*(pc+1)][*(pc+2)];
							sp-=temp16_value-1;
							cell = native(vm,sp);
							*sp=cell;
							pc+=4;
							break;
			case CallNativeVoid:
							temp16_value = *(pc+3);
							if(!isNativeValid(*(pc+1),*(pc+2)))
							{
								error = VM_INVALID_NATIVE_ERR;
								goto VM_ERROR;
							}
							native = nativeTable[*(pc+1)][*(pc+2)];
							sp-=temp16_value-1;
							native(vm,sp);
							sp--;
							pc+=4;
							break;
			//method return
			case Return:
                        local = sp->value.addr;
						sp--;
                        param = sp->value.addr;
						sp--;
                        pc=(uint8_t*)((sp-2)->value.addr);
                        sp=sp-sp->value.ival-(sp-1)->value.ival-3;
						break;

            case ReturnVar:
                            tempVar = *sp;
							sp--;
                            local = (Var*)sp->value.addr;
							sp--;
                            param = (Var*)sp->value.addr;
							sp--;
                            pc=(uint8_t*)((sp-2)->value.addr);
                            sp=sp-sp->value.ival-(sp-1)->value.ival-2;
                            *sp = tempVar;
							break;
	   	        //comparsion of 32-bit
            case VarEq:--sp;sp->value.fval = sp->value.fval == (sp+1)->value.fval;pc++;break;
            case VarNotEq:--sp;sp->value.fval=sp->value.fval != (sp+1)->value.fval;pc++;break;
            case VarGt:--sp;sp->value.fval = sp->value.fval > (sp+1)->value.fval;pc++;break;
            case VarGtEq:--sp;sp->value.fval = sp->value.fval >= (sp+1)->value.fval;pc++;break;
            case VarLt:--sp;sp->value.fval = sp->value.fval < (sp+1)->value.fval;pc++;break;
            case VarLtEq:--sp;sp->value.fval = sp->value.fval <= (sp+1)->value.fval;pc++;break;


			//object instantiation
			case New:   
                sp++;
                sp->type = TYPE_OBJECT;
                sp->value.addr = NULL;
                pc=pc+1;
                break;
			//object deletion
			case Free:  
                hash = *(uint32_t*)(pc+1);
                deleteProp(sp, hash);
                pc+=5;
                sp--;
                break;
			//string pool
            case LoadStr:
                pc=pc+1;
                break;
			case Pop: *(sp-1) = *sp;sp--;pc++;break;
            case Dup: *(sp+1) = *sp;sp++;pc++;break;
			case Sup:sp++;pc++;break;
			case Sdn:
				sp-=1;
				pc++;
				break;
			case Sup2:sp+=2;pc++;break;
			case Sdn2:sp-=2;pc++;break;

            case LoadFunc:
                sp++;sp->value.ival = *(uint64_t*)(pc+1);pc=pc+9;break;
            case PushObject:
                *inv = *sp;
                inv--;
                pThis = sp;
                pc++;
                break;
            case PopObject:
                inv++;
                pThis = inv;
                pc++;
                break;
            case CallI:
                temp64_value = sp->value.ival;
                sp--;
                //num of parameters
                temp8_value = (temp64_value >> 32) & 0xff;
                //num of locals
                temp16_value = (temp64_value >> 40) & 0xffff;
                //address
                temp32_value = (temp64_value) & 0xffff;
                temp_param = param;
                temp_local = local;
                param = (sp-temp8_value+1);
                sp++;
                local=sp;
                sp = sp + temp16_value;
                sp->value.addr = pc+1;
                sp++;
                sp->value.ival = temp8_value;
                sp++;
                sp->value.ival = temp16_value;
                sp++;
                sp->value.addr = temp_param;
                sp++;
                sp->value.addr = temp_local;
                pc=pcBaseAddr+temp32_value;
                break;
			default:
				error = VM_UNKNOWN_OPCODE_ERR;
				goto VM_ERROR;
	   	}
	}
	VM_ERROR:
	printf("Error code = %d\n",error);
	printf("pc = %d\n",*pc);
	if(vm->stackSize)
		free(vm->pStackAddr);
	free(vm);
	return error;
}

extern NativeMethod* nativeTable[];

//customize your way to load executable file
int vmLoadFile(MicroVM* vm, char *filename)
{
	FILE *file;
	size_t result;
	uint32_t lSize;
	uint8_t *buffer;
	
	//load opcodes
	file = fopen(filename,"rb");
	if (file == NULL) 
	{
		printf("can't open file\r\n");
		return VM_FILE_OPEN_ERR;
	}

	//get opcode size
	fseek (file , 0 , SEEK_END);
	lSize = ftell (file);
	rewind (file);
	buffer = (uint8_t*) malloc (sizeof(uint8_t)*lSize);
	printf("File size is: %d\r\n",lSize);

	if(buffer == NULL)
	{
		return VM_MALLOC_CODE_ERR;
	}

	// read file into memory
	result = fread (buffer,1,lSize,file);
	printf("code size = %d bytes\n",lSize);
	vm->codeSize = lSize;
	vm->pcOffset = VM_HEAD_SIZE;
	if (!result) 
	{
		return VM_FILE_READ_ERR;
	}
	//close file
	fclose(file);
	//initialize opcode address
	vm->pOpcodeAddr = buffer;
	return VM_OK;
}

int vmStart(MicroVM* vm, char *filename)
{
	uint32_t vm_err;
	uint32_t vmStackSize;
	//load executable file
	printf("%s\n",filename);
	vm_err = vmLoadFile(vm,filename);
	//exit if error occurs
	if(vm_err!=VM_OK)
	{
		return vm_err;
	}
	//get stack size
	vmStackSize = *(uint32_t*)(vm->pOpcodeAddr+VM_STACK_OFFSET);

	//allocate memory for stack
	if(vmStackSize==0)
	{
        vm->pStackAddr = (Var*)malloc(STACK_SIZE);
		vm->stackSize = STACK_SIZE;
		printf("stack size = %d bytes\r\n",STACK_SIZE);
	}
	else
	{
        vm->pStackAddr = (Var*)malloc(vmStackSize);
		vm->stackSize = vmStackSize;
		printf("stack size = %d bytes\r\n",vmStackSize);
	}
	vm->stackOffset = 0;
	vm->stringClassId= *(uint32_t*)(vm->pOpcodeAddr+VM_STRING_CLASS_TYPE_OFFSET);
	printf("String class id: %d\n",vm->stringClassId);
	//return stack allocation error
	if (vm->pStackAddr == NULL)
	{
		return VM_MALLOC_STACK_ERR;
	}
	//get native table
	vm->nativeTable = nativeTable;

	//initialize vm
	vm_err = vmInit(vm);
	if(vm_err!=VM_OK)
	{
		return vm_err;
	}
	return vm_err;
}







