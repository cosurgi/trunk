//#define DEBUG_NDIMTABLE

//#define YADE_FFTW3
#include <iostream>
#include <complex>
#include <boost/lexical_cast.hpp>
//#define DEBUG_NDIMTABLE
#include "lib/base/NDimTable.hpp"

int main(void){
	std::cout << "./NDimTable_Fill_Test > test3.tmp 2>&1\ndiff -s test3.tmp NDimTable_Fill_Test_correct_output.txt\n";
	std::cout << "\n";
	std::cout.precision(10);
	std::cout.setf(std::ios::fixed,std::ios::floatfield);

	std::vector<std::size_t> dim_A({2,6,4});
	std::vector<std::size_t> dim_B({4,4});
	std::vector<std::size_t> dim_C({6});
	std::vector<std::size_t> dim_D({6,2});

	std::cout << "\nAllocating A, B, C, D\n";
	NDimTable<double> A(dim_A),B(dim_B),C(dim_C),D(dim_D);

	A.fill1WithFunction(A.dim().size()
		, [](double i,int d)->double{ return i;}
		, [](Eigen::Matrix<double,3,1> r)->double{ return r.x()+10*r.y()+100*r.z()+0.1;}
	);

	std::cout << "\nassert A.fill1WithFunction ... ";
	for(int A_  =0 ; A_  < dim_A[0] ; A_  ++)
	for(int A__ =0 ; A__ < dim_A[1] ; A__ ++)
	for(int A___=0 ; A___< dim_A[2] ; A___++)
		assert(A.at(A_,A__,A___) == A_+10*A__+100*A___+0.1);
	std::cout << "OK\n";

	std::cout << "dim_A sizes = " << dim_A << " \n";
	std::cout << "\nPrint A = \n";
	A.print("Aᵦᵧᵥ",12);
	A.shiftByHalf();
	std::cout << "\nPrint A.shiftByHalf() = \n";
	A.print("A...",12);
	
	std::cout << "\nassert A.shiftByHalf().fill1WithFunction ... ";
	for(int A_  =dim_A[0]/2 ; A_  < dim_A[0]+dim_A[0]/2  ; A_  ++)
	for(int A__ =dim_A[1]/2 ; A__ < dim_A[1]+dim_A[1]/2  ; A__ ++)
	for(int A___=dim_A[2]/2 ; A___< dim_A[2]+dim_A[2]/2  ; A___++)
		assert(A.at(A_%dim_A[0],A__%dim_A[1],A___%dim_A[2]) == (A_-dim_A[0]/2)+10*(A__-dim_A[1]/2)+100*(A___-dim_A[2]/2)+0.1);
	std::cout << "OK\n";



	B.fill1WithFunction(B.dim().size()
		, [](double i,int d)->double{ return i;}
		, [](Eigen::Matrix<double,3,1> r)->double{ return r.x()+10*r.y()+0.2;}
	);

	std::cout << "\nassert B.fill1WithFunction ... ";
	for(int B_  =0 ; B_  < dim_B[0] ; B_  ++)
	for(int B__ =0 ; B__ < dim_B[1] ; B__ ++)
		assert(B.at(B_,B__) == B_+10*B__+0.2);
	std::cout << "OK\n";

	std::cout << "dim_B sizes = " << dim_B << " \n";
	std::cout << "\nPrint B = \n";
	B.print("Bᵨᵩ",12);
	B.shiftByHalf();
	std::cout << "\nPrint B.shiftByHalf() = \n";
	B.print("B..",12);

	C.fill1WithFunction(C.dim().size()
		, [](double i,int d)->double{ return i;}
		, [](Eigen::Matrix<double,3,1> r)->double{ return r.x()+0.3;}
	);

	std::cout << "\nassert C.fill1WithFunction ... ";
	for(int C_  =0 ; C_  < dim_C[0] ; C_  ++)
		assert(C.at(C_) == C_ + 0.3);
	std::cout << "OK\n";
	std::cout << "dim_C sizes = " << dim_C << " \n";
	std::cout << "\nPrint C = \n";
	C.print("Cᵪ",12);
	C.shiftByHalf();
	std::cout << "\nPrint C.shiftByHalf() = \n";
	C.print("C.",12);

	D.fill1WithFunction(D.dim().size()
		, [](double i,int d)->double{ return i;}
		, [](Eigen::Matrix<double,3,1> r)->double{ return r.x()+10*r.y()+0.4;}
	);

	std::cout << "\nassert D.fill1WithFunction ... ";
	for(int D_  =0 ; D_  < dim_D[0] ; D_  ++)
	for(int D__ =0 ; D__ < dim_D[1] ; D__ ++)
		assert(D.at(D_,D__) == D_+10*D__+0.4);
	std::cout << "OK\n";
	std::cout << "dim_D sizes = " << dim_D << " \n";
	std::cout << "\nPrint D = \n";
	D.print("Dᵢᵣ",12);
	D.shiftByHalf();
	std::cout << "\nPrint D.shiftByHalf() = \n";
	D.print("D..",12);


	std::cout << "\n\n---------------------------\nTesting fill2WithFunction()\n---------------------------\n";
	std::cout.precision(4);

	std::cout << "\ntest Fill2_d1_12x12\n";
	std::vector<std::size_t> dim_Fill2_d1_12x12({12,12}); // first 1d partilce has 12 points, second 1d particle has 12 points
	NDimTable<double> Fill2_d1_12x12(dim_Fill2_d1_12x12);

	Fill2_d1_12x12.fill2WithFunction(
	          1 // two 1d particles, next to each other
		, 0 // 1st particle starts at 0
		, 1 // 2nd particle starts at 1
		, [](double i_1, double i_2,int d)->double{ return i_1 - i_2;}
		, [](Eigen::Matrix<double,3,1> r)->double{ return r.x();}
	);
	Fill2_d1_12x12.print(true);
	
	std::cout << "\nassert Fill2_d1_12x_x_x12 ... ";
	for(size_t A_   =0 ; A_   < dim_Fill2_d1_12x12[0] ; A_   ++)
	for(size_t A__  =0 ; A__  < dim_Fill2_d1_12x12[1] ; A__  ++)
		assert(Fill2_d1_12x12.at({A_,A__}) == ((double)(A_*1.0 - A__*1.0))  );
	std::cout << "OK\n";

	std::cout << "\ntest Fill2_d1_12x_x_x12\n";
	std::vector<std::size_t> dim_Fill2_d1_12x_x_x12({12,2,2,12}); // first 1d partilce has 12 points, second 1d particle has 12 points
	NDimTable<double> Fill2_d1_12x_x_x12(dim_Fill2_d1_12x_x_x12);

	Fill2_d1_12x_x_x12.fill2WithFunction(
	          1 // two 1d particles, next to each other
		, 0 // 1st particle starts at 0
		, 3 // 2nd particle starts at 3
		, [](double i_1, double i_2,int d)->double{ return i_1 - i_2;}
		, [](Eigen::Matrix<double,3,1> r)->double{ return r.x();}
	);
	Fill2_d1_12x_x_x12.print(true);
	
// FIXME BTW: is that correct behaviour? What do I need?
	std::cout << "\nassert Fill2_d1_12x_x_x12 ... ";
	for(size_t A_   =0 ; A_   < dim_Fill2_d1_12x_x_x12[0] ; A_   ++)
	for(size_t A__  =0 ; A__  < dim_Fill2_d1_12x_x_x12[1] ; A__  ++)
	for(size_t A___ =0 ; A___ < dim_Fill2_d1_12x_x_x12[2] ; A___ ++)
	for(size_t A____=0 ; A____< dim_Fill2_d1_12x_x_x12[3] ; A____++)
		assert(Fill2_d1_12x_x_x12.at({A_,A__,A___,A____}) == ((double)(A_*1.0 - A____*1.0))  );
	std::cout << "OK\n";


	std::cout << "\ntest Fill2_d1_12x6x6x3\n";
	std::vector<std::size_t> dim_Fill2_d1_12x6x6x3({3,6,6,12}); // first 2d partilce has 3x6 points, second 2d particle has 6x12 points
	NDimTable<double> Fill2_d1_12x6x6x3(dim_Fill2_d1_12x6x6x3);

	Fill2_d1_12x6x6x3.fill2WithFunction(
	          2 // two 2d particles, next to each other
		, 0 // 1st particle starts at 0
		, 2 // 2nd particle starts at 2
		, [](double i_1, double i_2,int d)->double{ return i_1*2 - i_2;}
		, [](Eigen::Matrix<double,3,1> r)->double{ return r.x()+r.y()/1000.0;}
	);
	Fill2_d1_12x6x6x3.print(true);

	std::cout << "\nassert Fill2_d1_12x6x6x3 ... ";
	for(size_t A_   =0 ; A_   < dim_Fill2_d1_12x6x6x3[0] ; A_   ++)
	for(size_t A__  =0 ; A__  < dim_Fill2_d1_12x6x6x3[1] ; A__  ++)
	for(size_t A___ =0 ; A___ < dim_Fill2_d1_12x6x6x3[2] ; A___ ++)
	for(size_t A____=0 ; A____< dim_Fill2_d1_12x6x6x3[3] ; A____++)
		assert(Fill2_d1_12x6x6x3.at({A_,A__,A___,A____}) == ((double)(A_*2.0-A___)+(A__*2.0-A____)/1000.0)  );
	std::cout << "OK\n";



//	std::vector<const NDimTable<double>*> parts({&A,&B,&C,&D});
//	NDimTable<double> T(parts);
//	
//	std::cout << "dim_T sizes = " << T.dim() << " \n";
//	std::cout << "\nassert T ... ";
//	//T.print("Tᵦᵧᵥᵨᵩᵪᵢᵣ",12);
//	for(size_t A_  =0 ; A_  < dim_A[0] ; A_  ++)
//	for(size_t A__ =0 ; A__ < dim_A[1] ; A__ ++)
//	for(size_t A___=0 ; A___< dim_A[2] ; A___++)
//	for(size_t B_  =0 ; B_  < dim_B[0] ; B_  ++)
//	for(size_t B__ =0 ; B__ < dim_B[1] ; B__ ++)
//	for(size_t C_  =0 ; C_  < dim_C[0] ; C_  ++)
//	for(size_t D_  =0 ; D_  < dim_D[0] ; D_  ++)
//	for(size_t D__ =0 ; D__ < dim_D[1] ; D__ ++)
//		assert(T.atSafe({A_,A__,A___,B_,B__,C_,D_,D__}) == 
//				/*A.at(A_,A__,A___) =*/ ( A_+10*A__+100*A___+0.1 )*
//				/*B.at(B_,B__)      =*/ ( B_+10*B__+0.2          )*
//				/*C.at(C_)          =*/ ( C_ + 0.3               )*
//				/*D.at(D_,D__)      =*/ ( D_+10*D__+0.4          )
//		      );
//	std::cout << "OK\n";
};

