// header file to use mshadow
#include "mshadow/tensor.h"
#include <cassert>
#include <iostream>

using namespace std;
// this namespace contains all data structures, functions
using namespace mshadow;
// this namespace contains all operator overloads
using namespace mshadow::expr;

template<typename xpu>
void UpdateSGD(Tensor<xpu, 2, double> & weight, const Tensor<xpu, 2, double> &grad,
               double eta, double lambda) {

}

void printTensor2D(const Tensor<cpu, 2, double> & tensor) {
	for (index_t i = 0; i < tensor.size(0); ++i) {
		for (index_t j = 0; j < tensor.size(1); ++j) {
			printf("%f ", tensor[i][j]);
		}
		printf("\n");
	}
}

void test() {
	static int data[2] = {0};
	data[0]++;
	cout<<data[0]<<endl;
}



int main(void) {
  // intialize tensor engine before using tensor operation, needed for CuBLAS
  InitTensorEngine<cpu>();


  Tensor<cpu, 2, double> ts3 = NewTensor<cpu>(Shape2(2,1), 1.0);

	double data1[4] = {2,2};
	Tensor<cpu, 2, double> ts4(data1, Shape2(1,2));

	double data5[4] = {0, 0};
	Tensor<cpu, 2, double> ts5(data5, Shape2(2,2));

	ts4 = ts3.T();

	double size = ts4.size(0);

	//ts5 = dot(ts3, ts4)+dot(ts3, ts4);

	//cout<<output.T().size(0)<<endl;

	printTensor2D(ts4);



	FreeSpace(&ts3);


  return 0;

  {
	  float data[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	  Tensor<cpu, 2> ts;
	  ts.dptr_ = data;
	  ts.shape_ = mshadow::Shape2(3, 2);
	  ts.stride_ = 3;
	  // now: ts[0][0] == 0, ts[0][1] == 1 , ts[1][0] == 3, ts[1][1] == 4
	  for (index_t i = 0; i < ts.size(0); ++i) {
		for (index_t j = 0; j < ts.size(1); ++j) {
		  printf("ts[%u][%u]=%f\n", i, j, ts[i][j]);
		}
	  }
  }
  //return 0;

  // assume we have a float space
  float data[20];
  // create a 2 x 5 x 2 tensor, from existing space
  Tensor<cpu, 3> ts(data, Shape3(2,5,2));
    // take first subscript of the tensor
  Tensor<cpu, 2> mat = ts[0];
  // Tensor object is only a handle, assignment means they have same data content
  // we can specify content type of a Tensor, if not specified, it is float bydefault
  Tensor<cpu, 2, float> mat2 = mat;
  Tensor<cpu, 1> mat1 = Tensor<cpu, 1>(data, Shape1(10));
  mat = mat1.FlatTo2D();

  // shaape of matrix, note size order is same as numpy
  printf("%u X %u matrix\n", mat.size(0), mat.size(1));
  // return 0;
  // initialize all element to zero
  mat = 0.0f;
  // assign some values
  mat[0][1] = 1.0f; mat[1][0] = 2.0f;
  // elementwise operations
  mat += (mat + 10.0f) / 10.0f + 2.0f;
  
  // print out matrix, note: mat2 and mat1 are handles(pointers)
  for (index_t i = 0; i < mat.size(0); ++i) {
    for (index_t j = 0; j < mat.size(1); ++j) {
      printf("%.2f ", mat2[i][j]);
    }
    printf("\n");
  }



  // shutdown tensor enigne after usage
  ShutdownTensorEngine<cpu>();
  return 0;
}


