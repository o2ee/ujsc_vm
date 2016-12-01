// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015

#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include "errcode.h"

// includes
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <endian.h>
#include <sys/stat.h>
#include <math.h>

//VM Marco
#define STACK_SIZE 2048

#define VM_HEADER_OFFSET 0
#define VM_STRPOOL_LEN_OFFSET 4
#define VM_STRPOOL_ADDR_OFFSET 8
#define VM_STATIC_LEN_OFFSET 12
#define VM_NUM_OBJECTS_OFFSET 16
#define VM_NUM_ARRAYS_OFFSET 18
#define VM_ENDIAN_OFFSET 20
#define VM_DEBUG_OFFSET 21
#define VM_STACK_OFFSET 22
#define VM_STRING_CLASS_TYPE_OFFSET 26

#define VM_HEAD_SIZE 30

#define CAST_BIT8     0x000000FF
#define CAST_BIT16    0x0000FFFF

#define TYPE_NULL_UNDEFINED 0
#define TYPE_BOOLEAN 1
#define TYPE_NUMBER 2
#define TYPE_STRING 3
#define TYPE_ARRAY  4
#define TYPE_OBJECT 5
#define TYPE_FUNCTION 6


//basic memory unit
typedef union
{
    long ival;
    double  fval;
	void*   addr;
}CELL;

typedef struct VAR
{
    char type;
    CELL value;
}Var;

typedef struct PROP
{
    int hash;
    Var value;
    void * next;
}Prop;

typedef int32_t	 error_t;
typedef Var (*NativeMethod)(void* vm, Var* params);

//object structure
typedef struct OBJ
{
    CELL* parent; //memory allocated for parent classes
    void* var; //member variables
    int32_t classId; //class id
    CELL* data; //data pointer of inherited class
}Obj;

//array structure
typedef struct ARRAY
{
	void* addr; //array content
	uint32_t len; //length of the content
}Array;

//vm structure
typedef struct MICROVM
{
    uint8_t* pOpcodeAddr;  //opcode address
    uint32_t pcOffset;     //pc offset
    uint32_t codeSize;	   //opcode size
    uint32_t stackSize;    //local stack size
    uint16_t stackOffset;  //stack offset address
    uint32_t staticSize;   //static memory size
    Var* pParamAddr;      //parameter pointer
    Var* pStackAddr;      //local stack pointer
    Var* pLocalAddr;	   //local memory pointer
    void* pStaticAddr;     //static memory pointer
    NativeMethod** nativeTable; //native map pointer
    uint8_t* pc;           //program counter
    uint32_t stringClassId;//string class id
    Var* sp;
    void* self;
}MicroVM;

//check if the native method is valid
int isNativeValid(int id1, int id2);

//load codes and initialize the vm
error_t vmStart(MicroVM* vm, char *filename);
//start running the vm
error_t vmRun(MicroVM* vm);
//initialize the vm
error_t vmInit(MicroVM* vm);
//load codes
int vmLoadFile(MicroVM* vm, char *filename);


Prop * findProp(Var *var, int hash);
Prop * addProp(Var *var, int hash, Var value);
void deleteProp(Var *var, int hash);
#endif
