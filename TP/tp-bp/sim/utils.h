///////////////////////////////////////////////////////////////////////
//  Copyright 2015 Samsung Austin Semiconductor, LLC.                //
///////////////////////////////////////////////////////////////////////


#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;

#define UINT32      unsigned int
#define INT32       int
#define UINT64      unsigned long long
#define COUNTER     unsigned long long


#define NOT_TAKEN 0
#define TAKEN 1

#define FAILURE 0
#define SUCCESS 1

typedef enum {
  BIMODAL_PREDICTOR = 0,
  GSHARE_PREDICTOR = 1,
  LOCAL_PREDICTOR = 2,
  META_PREDICTOR = 3,
  PERCEPTRON = 4,
}PredictorType;

typedef enum {
   G1 = 0,
   G0 = 1,
   L0 = 2,
   L1 = 3
} State;

//JD2_2_2016
//typedef enum {
//  OPTYPE_OP               =2,
//  OPTYPE_BRANCH_COND      =3,
//  OPTYPE_RET              =4,
//  OPTYPE_BRANCH           =6,
//  OPTYPE_INDIRECT         =7,
//  OPTYPE_MAX              =8
//}OpType;

//JD2_17_2016 break down types into COND/UNCOND
typedef enum {
   OPTYPE_OP = 2,

   OPTYPE_RET_UNCOND,
   OPTYPE_JMP_DIRECT_UNCOND,
   OPTYPE_JMP_INDIRECT_UNCOND,
   OPTYPE_CALL_DIRECT_UNCOND,
   OPTYPE_CALL_INDIRECT_UNCOND,

   OPTYPE_RET_COND,
   OPTYPE_JMP_DIRECT_COND,
   OPTYPE_JMP_INDIRECT_COND,
   OPTYPE_CALL_DIRECT_COND,
   OPTYPE_CALL_INDIRECT_COND,

   OPTYPE_ERROR,

   OPTYPE_MAX
} OpType;

static inline UINT32 SatIncrement(UINT32 x, UINT32 max)
{
   if (x < max)
      return x + 1;
   return x;
}

static inline UINT32 SatDecrement(UINT32 x)
{
   if (x > 0)
      return x - 1;
   return x;
}

#endif
