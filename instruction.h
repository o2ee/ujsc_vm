// Copyright (c)  Zhe Wang
// wzedmund@gmail.com

#ifndef INSTRUCTION_H
#define INSTRUCTION_H


enum Opcode
{
  Nop,
  LoadVarLocal,
  StoreVarLocal,

  LoadVarParam,
  StoreVarParam,

  LoadVarStatic,
  StoreVarStatic,

  LoadVarLocalWide,
  StoreVarLocalWide,

  LoadProp,
  StoreProp,
  DeleteProp,

  LoadBool,
  LoadDouble,

  VarAdd,
  VarSub,
  VarMul,
  VarDiv,
  VarAnd,
  VarOr,
  VarXor,
  VarNot,
  VarRem,
  VarShiftLeft,
  VarShiftRight,
  VarInc,
  VarDec,
  VarNeg,

  Jump,
  JumpZero,
  JumpNotZero,
  JumpEq,
  JumpNotEq,
  JumpGt,
  JumpLt,
  JumpGtEq,
  JumpLtEq,

  SJump,
  SJumpZero,
  SJumpNotZero,
  SJumpEq,
  SJumpNotEq,
  SJumpGt,
  SJumpLt,
  SJumpGtEq,
  SJumpLtEq,

  Call,
  CallNative,
  CallNativeVoid,

  Return,
  ReturnVar,

  VarEq,
  VarNotEq,
  VarGt,
  VarGtEq,
  VarLt,
  VarLtEq,

  New,
  Free,

  Pop,
  Dup,
  Sup,
  Sdn,
  Sup2,
  Sdn2,

  LoadStr,
  LoadFunc,

  PushObject,
  PopObject,

  CallI,
};



#endif
