/* ============================================================================
* Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of Michael A. Jackson nor the names of its contributors may
* be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <math.h>

#include "CMUMutualInformation.h"

#define GET_INDEX(x, y, z, nbins)\
  ((nbins * nbins * z) + (nbins * y) + x)

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CMUMutualInformation::CMUMutualInformation()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CMUMutualInformation::~CMUMutualInformation()
{
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<typename T, typename K>
class NotEqual
{
  public:
    bool operator() (T value, K compare)
    {
      return (value != compare);
    }
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<class T, typename K>
std::vector<size_t> where(typename AIMArray<K>::Pointer array, T operation, K value, int &count)
{
  std::vector<size_t> ret;
  count = 0;
  for (size_t i = 0; i < array->getNumElements(); ++i)
  {
    if ( operation(array->getValue(i), value ) )
    {
      ret.push_back(i);
      ++count;
    }
  }
  return ret;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int64_t factorial(int64_t num)
{
  int64_t result=1;
  for (int64_t i=1; i<=num; ++i)
     result *= i;
  return result;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<typename T, typename K>
T getIndex(T* S, const int ndims, K* i)
{
  T ndx = i[0];
  std::vector<T> products(ndims);
  products[0] = 1;
  for (int d = 1; d < ndims; ++d)
  {
    products[d] = products[d-1] * S[d-1];
    ndx = ndx + products[d] * i[d];
    if (i[d] >= S[d]) { printf ("ERROR; i[d]:%lu >= S[d]:%lu\n", i[d], S[d]); }
  }
  return ndx;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CMUMutualInformation::printArray(size_t ndims, size_t* S, size_t* i, size_t curDimIdx, int* n, int* array)
{
  if (0 == curDimIdx)
  {
    printf(" printArray: \n");
    printf("   ndims: %03lu\n", ndims);
    printf("\tS\tn\n");
    for (size_t a=0; a<ndims; ++a)
    {
      printf("\t%03lu\t%03d\n", S[a], n[a]);
    }
  }
  size_t ndx = 0;
  for (size_t d = 0; d < S[n[curDimIdx]]; ++d)
  {
    i[n[curDimIdx]] = d;
    if (curDimIdx == ndims-1)
    {
      ndx = getIndex(S, ndims, i);
      printf("[%lu,%lu,%lu] %03d  ",i[0], i[1], i[2], array[ndx]);
      //printf("%03d  ", array[ndx]);
    }
    else
    {
      printArray(ndims, S, i, curDimIdx+1, n, array);
    }
  }
  if (curDimIdx == ndims-1)
    printf("\n");

  if (curDimIdx == ndims-2)
    printf("------------------------------\n");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<typename T>
void calc_sums(T* array,int actDim, int curDimIdx, size_t* dimsizes,
               int* n, size_t* S, int ndims, size_t* i,  T* totals, int &totalsIndex)
{
  size_t ndx = 0;
  for (size_t d = 0; d < dimsizes[curDimIdx]; ++d)
  {
    i[n[curDimIdx]] = d;
    if (curDimIdx == ndims - 2)
    {
      totals[totalsIndex] = 0;
    }
    if (actDim == n[curDimIdx])
    {
      ndx = getIndex<size_t, size_t>(S, ndims, i);
      totals[totalsIndex] += array[ndx];
    }
    else
    {
      calc_sums(array, actDim, curDimIdx+1, dimsizes, n, S, ndims, i, totals, totalsIndex);
    }
  }
  if (actDim == n[curDimIdx]) {
   // printf("%03d ", totals[totalsIndex]);
    totalsIndex++;
  }
 // if (curDimIdx == ndims - 2) printf("\n");
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<typename T>
void total(int actDim, typename AIMArray<T>::Pointer input,
                                             typename AIMArray<T>::Pointer output)
{
  std::vector<size_t>::size_type ndims = input->getDimensions().size();
  size_t* i = new size_t[ndims]; //static_cast<size_t*>(malloc(sizeof(size_t) * ndims));
 // size_t* dimsizes = new size_t[ndims]; // static_cast<size_t*>(malloc(sizeof(size_t) * ndims));
  std::vector<size_t> dimsizes(ndims);
  int* dimIndexLUT = new int[ndims]; //static_cast<int*>(malloc(sizeof(int) * ndims));
  std::vector<size_t> arrayDimSizes = input->getDimensions();
  size_t curIndex = 0;

  size_t tot = 1;
  for (int a = ndims - 1; a >= 0; --a)
  {
    if (a != actDim)
    {
      dimsizes[curIndex] = arrayDimSizes[a];
      tot *= arrayDimSizes[a];
      dimIndexLUT[curIndex] = a;
      ++curIndex;
    }
  }

  dimIndexLUT[curIndex] = (actDim);
  dimsizes[curIndex] = (arrayDimSizes[actDim]);
  output->allocateDataArray(tot, true);

  T* sums = output->getPointer(0); //static_cast<int*>(malloc(sizeof(int) * tot));
  int sumsNDims = ndims-1;
  int sumsArrayIndex = 0;

  calc_sums(input->getPointer(), actDim, 0, &(dimsizes.front()), dimIndexLUT, &(arrayDimSizes.front()), ndims, i, sums, sumsArrayIndex);

  // Print out the sums table

//  size_t* sumsDimSizes = new size_t[sumsNDims]; //static_cast<size_t*>(malloc(sizeof(size_t) * sumsNDims));
  std::vector<size_t> sumsDimSizes(sumsNDims);
  int* sumsDimLUT = new int[sumsNDims]; //static_cast<int*>(malloc(sizeof(int) * sumsNDims));

  for (int a = sumsNDims-1; a >= 0; --a)
  {
    sumsDimSizes[a] = dimsizes[sumsNDims-1-a];
    sumsDimLUT[a] = sumsNDims-1-a;
  }
//  printArray(sumsNDims, sumsDimSizes, i, 0, sumsDimLUT, sums);
  output->setDimensions(sumsDimSizes);


//  free(sums);
  delete i; //free(i);
//  delete dimsizes;
  delete dimIndexLUT; // free(dimIndexLUT);
//  free(sumsDimSizes);
  delete sumsDimLUT; // free(sumsDimLUT);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double CMUMutualInformation::mutualInfomation(AIMArray<double>::Pointer normalizedJointHistogram, int &level)
{

  level += 1;

  //get the dimensions to determine the sign of this contribution
  std::vector<size_t>::size_type szsz[1] = { normalizedJointHistogram->getDimensions().size() };

  int sign = 0;
  if (szsz[0] % 2 == 0)
  {
    sign = -1;
  }
  else
  {
    sign = +1;
  }

  // compute the entropy for this level of recursion
  int count;
  typedef NotEqual<double, double> NotEqualType;

  NotEqualType ne;
  double compare = 0.0f;
  std::vector<size_t> q = where(normalizedJointHistogram, ne, compare, count);
  //  q = where(ndhist ne 0.0,cnt)
  double H;
  if (count != 0)
  //  if (cnt ne 0)
  {
    double total = 0;
    int64_t f = factorial(level);
    for (std::vector<size_t>::iterator iter = q.begin(); iter != q.end(); ++iter )
    {
      total += normalizedJointHistogram->getValue(*iter) * log( static_cast<double>(normalizedJointHistogram->getValue(*iter)) );
    }
    H = -sign * total/f;

    //    H = -sign*total(ndhist[q] * alog(ndhist[q]))/factorial(level)
  }
  else
  {
    std::cout << "Warning: All joint histogram entries are zero" << std::endl;
    return -1;
  }
//  else begin
//    print,'Warning: All joint histogram entries are zero'
//    return,-1
//  endelse
  std::cout << "Entropy contribution for level " << level << "  -> " << H  << std::endl;

//
  AIMArray<double>::Pointer output = AIMArray<double>::New();
  if (szsz[0] > 1)
  {
    for (size_t i = 0; i < szsz[0]; ++i)
    {
       total<double>( (int)i, normalizedJointHistogram, output);
       // normalizedJointHistogram = output;
       H += mutualInfomation(output, level);
      //     H += mutualInfomation(reform(total(ndhist,i)), level);
       level -= 1;
     //  std::cout << "Mutual Infomation=" << H << std::endl;
    }
  }

  return H;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AIMArray<uint32_t>::Pointer CMUMutualInformation::jointHistogram(std::vector<ImageColumnType::Pointer> &inData,
                                                                 int binSize,
                                                                 bool normalize)
{
 // int err = NO_ERROR;

  size_t s[2] = {inData.size(), inData[0]->getNumElements() };
  // The check for a 2D array is implicit in the data structure that is passed in.


  int q = 0;
  int bsShifted = binSize >> q;
  while ( bsShifted != 1 )
  {
    q++;
    bsShifted = binSize >> q;
  }

  int32_t nbins = 256 >> q;

//  int32_t total_bins = pow (nbins, s[0]);
//  AIMArray<uint8_t>::Pointer image = inData[s[0]-1];
//  uint8_t* imagePtr = image->getPointer(0);

  AIMArray<uint32_t>::Pointer h = inData[s[0] -1]->bitShift<uint32_t>( -q);
  uint32_t* hPtr = h->getPointer(0);

  AIMArray<uint32_t>::Pointer temp = AIMArray<uint32_t>::NullPointer();

  for (int i = s[0] - 2; i >= 0; --i)
  {
    h = h->bitShift<uint32_t>( 8 - q);
    hPtr = h->getPointer(0);

    temp = inData[i]->bitShift<uint32_t>( -q);
    h = h->add<uint32_t, uint32_t>(temp);
    hPtr = h->getPointer(0);
    if (NULL == h.data())
    {
      return AIMArray<uint32_t>::NullPointer();
    }
  }

  std::cout << "nbins: " << nbins << std::endl;
  std::cout << "s[0]:  " << s[0] << std::endl;

  std::vector<size_t> dimensions;
  size_t total_elements = 1;
  for (size_t i = 0; i < s[0]; ++i)
  {
    total_elements *= nbins;
    dimensions.push_back(nbins);
  }
  std::cout << "total_elements: " << total_elements << std::endl;
  AIMArray<uint32_t>::Pointer ret = AIMArray<uint32_t>::New();
  ret->allocateDataArray(total_elements, 1);
  ret->zeroArrayData();
  ret->setDimensions(dimensions);

  // Histogram Generation
  uint32_t* retPtr = ret->getPointer(0);
  hPtr = h->getPointer(0);

  for(size_t i = 0; i < h->getNumElements(); ++i)
  {
    retPtr[ hPtr[i] ]++;
  }
  return ret;
}