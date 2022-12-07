/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


#include <Core/Parser/ArrayMathFunctionCatalog.h>

namespace ArrayMathFunctions {

  using namespace SCIRun;

//--------------------------------------------------------------------------
// Add functions

// Add scalar + scalar
bool add_ss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = *data1 + *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Add vector + scalar
bool add_vs(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = *data1 + *data2; data0++; data1++;
    *data0 = *data1 + *data2; data0++; data1++;
    *data0 = *data1 + *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Add vector + vector
bool add_vv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + 3*(pc.get_size());

  while (data0 != data0_end)
  {
    *data0 = *data1 + *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Add tensor + scalar
bool add_ts(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = *data1 + *data2; data0++; data1++;
    *data0 = *data1 + *data2; data0++; data1++;
    *data0 = *data1 + *data2; data0++; data1++;
    *data0 = *data1 + *data2; data0++; data1++;
    *data0 = *data1 + *data2; data0++; data1++;
    *data0 = *data1 + *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Add tensor + tensor
bool add_tt(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + 6*(pc.get_size());

  while (data0 != data0_end)
  {
    *data0 = *data1 + *data2; data0++; data1++; data2++;
  }

  return (true);
}


//--------------------------------------------------------------------------
// Sub functions

// Sub scalar - scalar
bool sub_ss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = *data1 - *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Sub vector - scalar
bool sub_vs(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = *data1 - *data2; data0++; data1++;
    *data0 = *data1 - *data2; data0++; data1++;
    *data0 = *data1 - *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Sub scalar - vector
bool sub_sv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }
  double* data1_end = data1 + pc.get_size();

  while (data1 != data1_end)
  {
    *data0 = *data1 - *data2; data0++; data2++;
    *data0 = *data1 - *data2; data0++; data2++;
    *data0 = *data1 - *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Sub vector - vector
bool sub_vv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + 3*pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = *data1 - *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Sub tensor - scalar
bool sub_ts(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = *data1 - *data2; data0++; data1++;
    *data0 = *data1 - *data2; data0++; data1++;
    *data0 = *data1 - *data2; data0++; data1++;
    *data0 = *data1 - *data2; data0++; data1++;
    *data0 = *data1 - *data2; data0++; data1++;
    *data0 = *data1 - *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Sub scalar - tensor
bool sub_st(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data1_end = data1 + pc.get_size();

  while (data1 != data1_end)
  {
    *data0 = *data1 - *data2; data0++; data2++;
    *data0 = *data1 - *data2; data0++; data2++;
    *data0 = *data1 - *data2; data0++; data2++;
    *data0 = *data1 - *data2; data0++; data2++;
    *data0 = *data1 - *data2; data0++; data2++;
    *data0 = *data1 - *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Sub tensor - tensor
bool sub_tt(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }
  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    *data0 = *data1 - *data2; data0++; data1++; data2++;
  }

  return (true);
}

//--------------------------------------------------------------------------
// Neg functions

// Negate scalar
bool neg_s(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data1_end = data1 + pc.get_size();

  if (!data0 || !data1)
  {
    return false;
  }

  while(data1 != data1_end)
  {
    *data0 = -(*data1); data0++; data1++;
  }

  return (true);
}

// Negate vector
bool neg_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data1_end = data1 + 3*pc.get_size();

  while(data1 != data1_end)
  {
    *data0 = -(*data1); data0++; data1++;
  }

  return (true);
}

// Negate tensor
bool neg_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data1_end = data1 + 6*pc.get_size();

  while(data1 != data1_end)
  {
    *data0 = -(*data1); data0++; data1++;
  }

  return (true);
}

//--------------------------------------------------------------------------
// Mult functions

// Mult scalar * scalar
bool mult_ss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = *data1 * *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Mult vector * scalar
bool mult_vs(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = *data1 * *data2; data0++; data1++;
    *data0 = *data1 * *data2; data0++; data1++;
    *data0 = *data1 * *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Mult tensor * scalar
bool mult_ts(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = *data1 * *data2; data0++; data1++;
    *data0 = *data1 * *data2; data0++; data1++;
    *data0 = *data1 * *data2; data0++; data1++;
    *data0 = *data1 * *data2; data0++; data1++;
    *data0 = *data1 * *data2; data0++; data1++;
    *data0 = *data1 * *data2; data0++; data1++; data2++;
  }

  return (true);
}

namespace {
  const int VECTOR_LENGTH = 3;
  const int TENSOR_LENGTH = 6;
}

// Mult vector* tensor
bool mult_vt(SCIRun::ArrayMathProgramCode& pc)
{
  double* outputVector = pc.get_variable(0);
  double* inputVector = pc.get_variable(1);
  double* inputTensor = pc.get_variable(2);

  if (!outputVector || !inputVector || !inputTensor)
  {
    return false;
  }

  double* inputVectorEnd = inputVector + VECTOR_LENGTH*pc.get_size();

  while (inputVector != inputVectorEnd)
  {
    outputVector[0] = inputTensor[0]*inputVector[0] + inputTensor[1]*inputVector[1] + inputTensor[2]*inputVector[2];
    outputVector[1] = inputTensor[1]*inputVector[0] + inputTensor[3]*inputVector[1] + inputTensor[4]*inputVector[2];
    outputVector[2] = inputTensor[2]*inputVector[0] + inputTensor[4]*inputVector[1] + inputTensor[5]*inputVector[2];
    outputVector+=VECTOR_LENGTH; inputTensor+=TENSOR_LENGTH; inputVector+=VECTOR_LENGTH;
  }

  return (true);
}


// Mult tensor* tensor
bool mult_tt(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + TENSOR_LENGTH*pc.get_size();

  while (data2 != data2_end)
  {
    data0[0] = data1[0]*data2[0] + data1[1]*data2[1] + data1[2]*data2[2];
    data0[1] = data1[1]*data2[0] + data1[3]*data2[1] + data1[4]*data2[2];
    data0[2] = data1[2]*data2[0] + data1[4]*data2[1] + data1[5]*data2[2];
    data0[3] = data1[1]*data2[1] + data1[3]*data2[3] + data1[4]*data2[4];
    data0[4] = data1[2]*data2[1] + data1[4]*data2[3] + data1[5]*data2[4];
    data0[5] = data1[2]*data2[2] + data1[4]*data2[4] + data1[5]*data2[5];
    data0+=TENSOR_LENGTH; data1+=TENSOR_LENGTH; data2+=TENSOR_LENGTH;
  }

  return (true);
}


//--------------------------------------------------------------------------
// Div functions

// Div scalar / scalar
bool div_ss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = *data1 / *data2; data0++; data1++; data2++;
  }

  return (true);
}

// Div vector / scalar
bool div_vs(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    double val = 1.0/ *data2;
    *data0 = *data1 *val; data0++; data1++;
    *data0 = *data1 *val; data0++; data1++;
    *data0 = *data1 *val; data0++; data1++; data2++;
  }

  return (true);
}

// Div tensor / scalar
bool div_ts(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    double val = 1.0/ *data2;
    *data0 = *data1 *val; data0++; data1++;
    *data0 = *data1 *val; data0++; data1++;
    *data0 = *data1 *val; data0++; data1++;
    *data0 = *data1 *val; data0++; data1++;
    *data0 = *data1 *val; data0++; data1++;
    *data0 = *data1 *val; data0++; data1++; data2++;
  }

  return (true);
}

//--------------------------------------------------------------------------
// Rem functions

// Rem scalar / scalar
bool rem_ss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = fmod(*data1,*data2); data0++; data1++; data2++;
  }

  return (true);
}

// Div vector / scalar
bool rem_vs(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = fmod(*data1,*data2); data0++; data1++;
    *data0 = fmod(*data1,*data2); data0++; data1++;
    *data0 = fmod(*data1,*data2); data0++; data1++; data2++;
  }

  return (true);
}

// Div tensor / scalar
bool rem_ts(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data2_end = data2 + pc.get_size();

  while (data2 != data2_end)
  {
    *data0 = fmod(*data1,*data2); data0++; data1++;
    *data0 = fmod(*data1,*data2); data0++; data1++;
    *data0 = fmod(*data1,*data2); data0++; data1++;
    *data0 = fmod(*data1,*data2); data0++; data1++;
    *data0 = fmod(*data1,*data2); data0++; data1++;
    *data0 = fmod(*data1,*data2); data0++; data1++; data2++;
  }

  return (true);
}



//--------------------------------------------------------------------------
// Sequence functions (translation from arrays of size 1 into arrays of size n)

// Sequence scalar
bool seq_s(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  double val = data1[0];
  while (data0 != data0_end)
  {
    *data0 = val; data0++;
  }

  return (true);
}

// Sequence vector
bool seq_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + 3*pc.get_size();

  double val1 = data1[0];
  double val2 = data1[1];
  double val3 = data1[2];

  while (data0 != data0_end)
  {
    *data0 = val1; data0++;
    *data0 = val2; data0++;
    *data0 = val3; data0++;
  }

  return (true);
}

// Sequence vector
bool seq_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  if (!data0 || !data1)
  {
    return false;
  }

  double* data0_end = data0 + 6*pc.get_size();

  double val1 = data1[0];
  double val2 = data1[1];
  double val3 = data1[2];
  double val4 = data1[3];
  double val5 = data1[4];
  double val6 = data1[5];

  while (data0 != data0_end)
  {
    *data0 = val1; data0++;
    *data0 = val2; data0++;
    *data0 = val3; data0++;
    *data0 = val4; data0++;
    *data0 = val5; data0++;
    *data0 = val6; data0++;
  }

  return (true);
}



//--------------------------------------------------------------------------
// Assign functions

bool assign_sss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);

  if (!data0 || !data1 || !data2 || !data3)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    int idx = static_cast<int>(*data3+0.5);
    if (idx != 0) return (false);
    *data0 = *data2; data0++; data1++; data2++; data3++;
  }

  return (true);
}

bool assign_vss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);

  if (!data0 || !data1 || !data2 || !data3)
  {
    return false;
  }

  double* data3_end = data3 + pc.get_size();

  while (data3 != data3_end)
  {
    int idx = static_cast<int>(*data3+0.5);
    if (idx < 0 || idx > 2) return (false);
    data0[0] = data1[0];
    data0[1] = data1[1];
    data0[2] = data1[2];
    data0[idx] = *data2; data0+=3; data1+=3; data2++; data3++;
  }

  return (true);
}

bool assign_tss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);

  if (!data0 || !data1 || !data2 || !data3)
  {
    return false;
  }

  double* data3_end = data3 + pc.get_size();

  while (data3 != data3_end)
  {
    int idx = static_cast<int>(*data3+0.5);
    if (idx < 0 || idx > 5) return (false);
    data0[0] = data1[0];
    data0[1] = data1[1];
    data0[2] = data1[2];
    data0[3] = data1[3];
    data0[4] = data1[4];
    data0[5] = data1[5];
    data0[idx] = *data2; data0+=6; data1+=6; data2++; data3++;
  }

  return (true);
}

bool assign_tsss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);
  double* data4 = pc.get_variable(4);

  if (!data0 || !data1 || !data2 || !data3 || !data4)
  {
    return false;
  }

  double* data3_end = data0 + pc.get_size();

  const int ic[9] =  { 0, 1, 2, 1, 3, 4, 2, 4, 5 };


  while (data3 != data3_end)
  {
    int idx1 = static_cast<int>(*data3+0.5);
    int idx2 = static_cast<int>(*data4+0.5);
    if (idx1 < 0 || idx1 > 2) return (false);
    if (idx2 < 0 || idx2 > 2) return (false);
    int idx = idx1 + 3*idx2;

    data0[ic[idx]] = *data2; data0+=6; data1+=6; data2++; data3++; data4++;
  }

  return (true);
}

//--------------------------------------------------------------------------
// Subs functions

// Subs scalar scalar
bool subs_ss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    int idx = static_cast<int>(*data2+0.5);
    if (idx != 0) return (false);
    *data0 = *data1; data0++; data1++; data2++;
  }

  return (true);
}

bool subs_vs(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    int idx = static_cast<int>(*data2+0.5);
    if (idx < 0 || idx > 2) return (false);
    *data0 = data1[idx]; data0++; data1+=3; data2++;
  }

  return (true);
}

bool subs_ts(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);

  if (!data0 || !data1 || !data2)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    int idx = static_cast<int>(*data2+0.5);
    if (idx < 0 || idx > 5) return (false);
    *data0 = data1[idx]; data0++; data1+=6; data2++;
  }

  return (true);
}

bool subs_tss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);

  if (!data0 || !data1 || !data2 || !data3)
  {
    return false;
  }

  double* data0_end = data0 + pc.get_size();

  const int ic[9] =  { 0, 1, 2, 1, 3, 4, 2, 4, 5 };

  while (data0 != data0_end)
  {
    int idx1 = static_cast<int>(*data2+0.5);
    int idx2 = static_cast<int>(*data3+0.5);
    int idx = idx1 + 3*idx2;

    if (idx < 0 || idx1 > 8) return (false);

    *data0 = data1[ic[idx]]; data0++; data1+=6; data2++; data3++;
  }

  return (true);
}

// Select functions
bool select_sss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);

  if (!data0 || !data1 || !data2 || !data3)
  {
    return false;
  }

  double* data1_end = data1 + pc.get_size();

  while (data1 != data1_end)
  {
    if (*data1) { *data0 = *data2; } else { *data0 = *data3; }
    data0++; data1++; data2++; data3++;
  }

  return (true);
}

bool select_svv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);

  if (!data0 || !data1 || !data2 || !data3)
  {
    return false;
  }

  double* data1_end = data1 + pc.get_size();

  while (data1 != data1_end)
  {
    if (*data1)
    {
      data0[0] = data2[0];
      data0[1] = data2[1];
      data0[2] = data2[2];
    }
    else
    {
      data0[0] = data3[0];
      data0[1] = data3[1];
      data0[2] = data3[2];
    }
    data0+=3; data1++; data2+=3; data3+=3;
  }

  return (true);
}

bool select_stt(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);

  if (!data0 || !data1 || !data2 || !data3)
  {
    return false;
  }

  double* data1_end = data1 + pc.get_size();

  while (data1 != data1_end)
  {
    if (*data1)
    {
      data0[0] = data2[0];
      data0[1] = data2[1];
      data0[2] = data2[2];
      data0[3] = data2[3];
      data0[4] = data2[4];
      data0[5] = data2[5];
    }
    else
    {
      data0[0] = data3[0];
      data0[1] = data3[1];
      data0[2] = data3[2];
      data0[3] = data3[3];
      data0[4] = data3[4];
      data0[5] = data3[5];
    }
    data0+=6; data1++; data2+=6; data3+=6;
  }

  return (true);
}



} // end namsespace

namespace SCIRun {

void
InsertBasicArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog)
{
  // Add add functions to database
  catalog->add_sym_function(ArrayMathFunctions::add_ss,"add$S:S","S");
  catalog->add_sym_function(ArrayMathFunctions::add_vs,"add$V:S","V");
  catalog->add_sym_function(ArrayMathFunctions::add_ts,"add$T:S","T");
  catalog->add_sym_function(ArrayMathFunctions::add_vv,"add$V:V","V");
  catalog->add_sym_function(ArrayMathFunctions::add_tt,"add$T:T","T");

  // Add sub functions to database
  catalog->add_function(ArrayMathFunctions::sub_ss,"sub$S:S","S");
  catalog->add_function(ArrayMathFunctions::sub_vs,"sub$V:S","V");
  catalog->add_function(ArrayMathFunctions::sub_sv,"sub$S:V","V");
  catalog->add_function(ArrayMathFunctions::sub_ts,"sub$T:S","T");
  catalog->add_function(ArrayMathFunctions::sub_st,"sub$S:T","T");
  catalog->add_function(ArrayMathFunctions::sub_vv,"sub$V:V","V");
  catalog->add_function(ArrayMathFunctions::sub_tt,"sub$T:T","T");

  // Add neg function to database
  catalog->add_function(ArrayMathFunctions::neg_s,"neg$S","S");
  catalog->add_function(ArrayMathFunctions::neg_v,"neg$V","V");
  catalog->add_function(ArrayMathFunctions::neg_t,"neg$T","T");

  // Add mult functions to database
  catalog->add_sym_function(ArrayMathFunctions::mult_ss,"mult$S:S","S");
  catalog->add_sym_function(ArrayMathFunctions::mult_vs,"mult$V:S","V");
  catalog->add_sym_function(ArrayMathFunctions::mult_ts,"mult$T:S","T");
  catalog->add_sym_function(ArrayMathFunctions::mult_vt,"mult$V:T","V");
  catalog->add_sym_function(ArrayMathFunctions::mult_tt,"mult$T:T","T");

  // Add div functions to database
  catalog->add_function(ArrayMathFunctions::div_ss,"div$S:S","S");
  catalog->add_function(ArrayMathFunctions::div_vs,"div$V:S","V");
  catalog->add_function(ArrayMathFunctions::div_ts,"div$T:S","T");

  // Add rem functions to database
  catalog->add_function(ArrayMathFunctions::rem_ss,"rem$S:S","S");
  catalog->add_function(ArrayMathFunctions::rem_vs,"rem$V:S","V");
  catalog->add_function(ArrayMathFunctions::rem_ts,"rem$T:S","T");

  // Add assign functions to database
  catalog->add_function(ArrayMathFunctions::assign_sss, "assign$S:S:S","S");
  catalog->add_function(ArrayMathFunctions::assign_vss, "assign$V:S:S","V");
  catalog->add_function(ArrayMathFunctions::assign_tss, "assign$T:S:S","T");
  catalog->add_function(ArrayMathFunctions::assign_tsss,"assign$T:S:S:S","T");

  // Add subs functions to database
  catalog->add_function(ArrayMathFunctions::subs_ss, "subs$S:S","S");
  catalog->add_function(ArrayMathFunctions::subs_vs, "subs$V:S","S");
  catalog->add_function(ArrayMathFunctions::subs_ts, "subs$T:S","S");
  catalog->add_function(ArrayMathFunctions::subs_tss,"subs$T:S:S","S");

  // Add sequencer code: translate from single to sequence of data
  catalog->add_function(ArrayMathFunctions::seq_s,"seq$S","S");
  catalog->add_function(ArrayMathFunctions::seq_v,"seq$V","V");
  catalog->add_function(ArrayMathFunctions::seq_t,"seq$T","T");

  catalog->add_function(ArrayMathFunctions::select_sss,"select$S:S:S","S");
  catalog->add_function(ArrayMathFunctions::select_svv,"select$S:V:V","V");
  catalog->add_function(ArrayMathFunctions::select_stt,"select$S:T:T","T");
}

} // end namespace
