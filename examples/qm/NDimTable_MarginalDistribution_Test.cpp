//#define DEBUG_NDIMTABLE

//#define YADE_FFTW3
#include <iostream>
#include <complex>
#include <boost/lexical_cast.hpp>
//#define DEBUG_NDIMTABLE
#include "lib/base/NDimTable.hpp"

int main(void){
	std::cout << "./NDimTable_MarginalDistribution_Test > test2.tmp 2>&1\ndiff -s test2.tmp NDimTable_MarginalDistribution_Test_correct_output.txt\n";
	std::cout << "\n";
	std::cout << "// Tᵦᵧᵥᵨᵩᵪᵢᵣ=AᵦᵧᵥBᵨᵩCᵪDᵢᵣ \n";
	std::cout << "//  ⋮⋮⋮⋮⋮⋮⋮⋮  ⋮⋮⋮ ⋮⋮ ⋮ ⋮⋮ \n";
	std::cout << "//  23243252  232 43 2 52 \n";
	std::cout << "\n";
	std::cout.precision(10);
	std::cout.setf(std::ios::fixed,std::ios::floatfield);

	std::vector<std::size_t> dim_A({2,3,2});
	std::vector<std::size_t> dim_B({4,3});
	std::vector<std::size_t> dim_C({2});
	std::vector<std::size_t> dim_D({5,2});

	std::cout << "\nAllocating A, B, C, D\n";
	NDimTable<double> A(dim_A),B(dim_B),C(dim_C),D(dim_D);

	for(int A_  =0 ; A_  < dim_A[0] ; A_  ++)
	for(int A__ =0 ; A__ < dim_A[1] ; A__ ++)
	for(int A___=0 ; A___< dim_A[2] ; A___++)
		A.at(A_,A__,A___) = A_+10*A__+100*A___+0.1;
	std::cout << "dim_A sizes = " << dim_A << " \n";
	std::cout << "\nPrint A = \n";
	A.print("Aᵦᵧᵥ",12);

	for(int B_  =0 ; B_  < dim_B[0] ; B_  ++)
	for(int B__ =0 ; B__ < dim_B[1] ; B__ ++)
		B.at(B_,B__) = B_+10*B__+0.2;
	std::cout << "dim_B sizes = " << dim_B << " \n";
	std::cout << "\nPrint B = \n";
	B.print("Bᵨᵩ",12);

	for(int C_  =0 ; C_  < dim_C[0] ; C_  ++)
		C.at(C_) = C_ + 0.3;
	std::cout << "dim_C sizes = " << dim_C << " \n";
	std::cout << "\nPrint C = \n";
	C.print("Cᵪ",12);

	for(int D_  =0 ; D_  < dim_D[0] ; D_  ++)
	for(int D__ =0 ; D__ < dim_D[1] ; D__ ++)
		D.at(D_,D__) = D_+10*D__+0.4;
	std::cout << "dim_D sizes = " << dim_D << " \n";
	std::cout << "\nPrint D = \n";
	D.print("Dᵢᵣ",12);

	std::vector<const NDimTable<double>*> parts({&A,&B,&C,&D});
	NDimTable<double> T(parts);
// accociativity test
// (A,B,C),D
	std::vector<const NDimTable<double>*> parts1({&A,&B,&C});
	NDimTable<double> TABC(parts1);
	std::vector<const NDimTable<double>*> parts2({&TABC,&D});
	NDimTable<double> TABCD_1(parts2);
// (A,B),(C,D)
	std::vector<const NDimTable<double>*> parts3({&A,&B});
	NDimTable<double> TAB(parts3);
	std::vector<const NDimTable<double>*> parts4({&C,&D});
	NDimTable<double> TCD(parts4);
	std::vector<const NDimTable<double>*> parts5({&TAB,&TCD});
	NDimTable<double> TABCD_2(parts5);
// A,(B,C,D)
	std::vector<const NDimTable<double>*> parts6({&B,&C,&D});
	NDimTable<double> TBCD(parts6);
	std::vector<const NDimTable<double>*> parts7({&A,&TBCD});
	NDimTable<double> TABCD_3(parts7);
// A,(B,C),D
	std::vector<const NDimTable<double>*> parts8({&B,&C});
	NDimTable<double> TBC(parts8);
	std::vector<const NDimTable<double>*> parts9({&A,&TBC,&D});
	NDimTable<double> TABCD_4(parts9);

	assert( T.compareEpsilon(TABCD_1,std::numeric_limits<double>::epsilon()      ) );
	assert( T.compareEpsilon(TABCD_2,std::numeric_limits<double>::epsilon()*40000) );
	assert( T.compareEpsilon(TABCD_3,std::numeric_limits<double>::epsilon()*80000) );
	assert( T.compareEpsilon(TABCD_4,std::numeric_limits<double>::epsilon()*50000) );

//std::cout << "---------------------------------------------------------";
//std::cout << "dim_T sizes = " << T.dim() << " \n";
//T.print();
//std::cout << "---------------------------------------------------------";
//std::cout << "dim_TABCD_1 sizes = " << TABCD_1.dim() << " \n";
//TABCD_1.print();
//std::cout << "---------------------------------------------------------";
//std::cout << "dim_TABCD_2 sizes = " << TABCD_2.dim() << " \n";
//TABCD_2.print();
//std::cout << "---------------------------------------------------------";
//std::cout << "dim_TABCD_3 sizes = " << TABCD_3.dim() << " \n";
//TABCD_3.print();
//std::cout << "---------------------------------------------------------";
//std::cout << "dim_TABCD_4 sizes = " << TABCD_4.dim() << " \n";
//TABCD_4.print();
///////////////////////  this fails, because of numerical errors multiplication is not associative
//	assert( T == TABCD_1 );
//	assert( T == TABCD_2 );
//	assert( T == TABCD_3 );
//	assert( T == TABCD_4 );
////
	
	std::cout << "dim_T sizes = " << T.dim() << " \n";
	std::cout << "\nassert T ... ";
	//T.print("Tᵦᵧᵥᵨᵩᵪᵢᵣ",12);
	for(size_t A_  =0 ; A_  < dim_A[0] ; A_  ++)
	for(size_t A__ =0 ; A__ < dim_A[1] ; A__ ++)
	for(size_t A___=0 ; A___< dim_A[2] ; A___++)
	for(size_t B_  =0 ; B_  < dim_B[0] ; B_  ++)
	for(size_t B__ =0 ; B__ < dim_B[1] ; B__ ++)
	for(size_t C_  =0 ; C_  < dim_C[0] ; C_  ++)
	for(size_t D_  =0 ; D_  < dim_D[0] ; D_  ++)
	for(size_t D__ =0 ; D__ < dim_D[1] ; D__ ++)
		assert(T.atSafe({A_,A__,A___,B_,B__,C_,D_,D__}) == 
				/*A.at(A_,A__,A___) =*/ ( A_+10*A__+100*A___+0.1 )*
				/*B.at(B_,B__)      =*/ ( B_+10*B__+0.2          )*
				/*C.at(C_)          =*/ ( C_ + 0.3               )*
				/*D.at(D_,D__)      =*/ ( D_+10*D__+0.4          )
		      );
	std::cout << "OK\n";
	std::cout << "\nNormalize A,B,C,D\n";
	double sumA=A.sumAll(), sumB=B.sumAll(), sumC=C.sumAll(), sumD=D.sumAll();
	std::cout << "sumA = " << sumA << "\n";
	std::cout << "sumB = " << sumB << "\n";
	std::cout << "sumC = " << sumC << "\n";
	std::cout << "sumD = " << sumD << "\n";
	
	NDimTable<double> Asizes(A),Bsizes(B),Csizes(C),Dsizes(D);
	A /= sumA;
	B /= sumB;
	C /= sumC;
	D /= sumD;
	double sumA2=A.sumAll(), sumB2=B.sumAll(), sumC2=C.sumAll(), sumD2=D.sumAll();
	std::cout << "after dividing by sum:\n";
	std::cout << "sumA2 = " << sumA2 << "\n";
	std::cout << "sumB2 = " << sumB2 << "\n";
	std::cout << "sumC2 = " << sumC2 << "\n";
	std::cout << "sumD2 = " << sumD2 << "\n";

	//A.print("Aᵦᵧᵥ",12);
	//B.print("Bᵨᵩ" ,12);
	//C.print("Cᵪ"  ,12);
	//D.print("Dᵢᵣ" ,12);

	std::vector<const NDimTable<double>*> partsNorm({&A,&B,&C,&D});
	NDimTable<double> Tnorm(partsNorm);
	std::cout << "dim_Tnorm sizes = " << Tnorm.dim() << " \n";
	double epsilon = std::numeric_limits<double>::epsilon();
	std::cout <<    "std::numeric_limits<double>::epsilon() = " << boost::lexical_cast<std::string>(epsilon) << "\n";
	std::cout << "\nassert Tnorm ... ";
	//Tnorm.print("Tᵦᵧᵥᵨᵩᵪᵢᵣ",12);
	for(size_t A_  =0 ; A_  < dim_A[0] ; A_  ++)
	for(size_t A__ =0 ; A__ < dim_A[1] ; A__ ++)
	for(size_t A___=0 ; A___< dim_A[2] ; A___++)
	for(size_t B_  =0 ; B_  < dim_B[0] ; B_  ++)
	for(size_t B__ =0 ; B__ < dim_B[1] ; B__ ++)
	for(size_t C_  =0 ; C_  < dim_C[0] ; C_  ++)
	for(size_t D_  =0 ; D_  < dim_D[0] ; D_  ++)
	for(size_t D__ =0 ; D__ < dim_D[1] ; D__ ++) {
//		std::cout <<   Tnorm.atSafe({A_,A__,A___,B_,B__,C_,D_,D__}) << "   ==   " << (( A_+10*A__+100*A___+0.1 )*
//                                                                                              ( B_+10*B__+0.2          )*
//                                                                                              ( C_ + 0.3               )*
//                                                                                              ( D_+10*D__+0.4          )/(sumA*sumB*sumC*sumD)) << "\n";
//		
//		std::cout << ( Tnorm.atSafe({A_,A__,A___,B_,B__,C_,D_,D__}) -              (( A_+10*A__+100*A___+0.1 )*
//                                                                                            ( B_+10*B__+0.2          )*
//                                                                                            ( C_ + 0.3               )*
//                                                                                            ( D_+10*D__+0.4          )/(sumA*sumB*sumC*sumD))) << "\n";
//
		assert( std::abs(Tnorm.atSafe({A_,A__,A___,B_,B__,C_,D_,D__}) - 
				/*A.at(A_,A__,A___) =*/ ( A_+10*A__+100*A___+0.1 )*
				/*B.at(B_,B__)      =*/ ( B_+10*B__+0.2          )*
				/*C.at(C_)          =*/ ( C_ + 0.3               )*
				/*D.at(D_,D__)      =*/ ( D_+10*D__+0.4          ) 
				    /(sumA*sumB*sumC*sumD) 
				  ) < epsilon
		      );
	}
	std::cout << "OK\n";
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::cout << "\nTrying marginal distributions now\n";
	std::cout << "// Tᵦᵧᵥᵨᵩᵪᵢᵣ=AᵦᵧᵥBᵨᵩCᵪDᵢᵣ \n";
	std::cout << "//  ⋮⋮⋮⋮⋮⋮⋮⋮  ⋮⋮⋮ ⋮⋮ ⋮ ⋮⋮ \n";
	std::cout << "//  23243252  232 43 2 52 \n";
	std::cout << "// Aᵦᵧᵥ     =Tᵦᵧᵥ..... \n";
	std::cout << "//  ⋮⋮⋮⋮⋮⋮⋮⋮  ⋮⋮⋮⋮⋮⋮⋮⋮ \n";
	std::cout << "//  23243252  23243252 \n";
	//std::cout << "dim_A sizes  = " << dim_A << " \n";
	NDimTable<double> Amarg =Tnorm.calcMarginalDistribution({1,1,1,0,0,0,0,0});
	std::cout << "// B   ᵨᵩ   =T...ᵨᵩ... \n";
	std::cout << "//  ⋮⋮⋮⋮⋮⋮⋮⋮  ⋮⋮⋮⋮⋮⋮⋮⋮ \n";
	std::cout << "//  23243252  23243252 \n";
	//std::cout << "dim_B sizes  = " << dim_B << " \n";
	NDimTable<double> Bmarg =Tnorm.calcMarginalDistribution({0,0,0,1,1,0,0,0});
	std::cout << "// C     ᵪ  =T.....ᵪ.. \n";
	std::cout << "//  ⋮⋮⋮⋮⋮⋮⋮⋮  ⋮⋮⋮⋮⋮⋮⋮⋮ \n";
	std::cout << "//  23243252  23243252 \n";
	//std::cout << "dim_C sizes  = " << dim_C << " \n";
	NDimTable<double> Cmarg =Tnorm.calcMarginalDistribution({0,0,0,0,0,1,0,0});
	std::cout << "// D      ᵢᵣ=T......ᵢᵣ \n";
	std::cout << "//  ⋮⋮⋮⋮⋮⋮⋮⋮  ⋮⋮⋮⋮⋮⋮⋮⋮ \n";
	std::cout << "//  23243252  23243252 \n";
	//std::cout << "dim_D sizes  = " << dim_D << " \n";
	NDimTable<double> Dmarg =Tnorm.calcMarginalDistribution({0,0,0,0,0,0,1,1});
	std::cout << "\n";

	Amarg.print("Aᵦᵧᵥ",12); A.print("Aᵦᵧᵥ",12); Asizes.print("Aᵦᵧᵥ",12);
	Bmarg.print("Bᵨᵩ" ,12); B.print("Bᵨᵩ" ,12); Bsizes.print("Bᵨᵩ" ,12);
	Cmarg.print("Cᵪ"  ,12); C.print("Cᵪ"  ,12); Csizes.print("Cᵪ"  ,12);
	Dmarg.print("Dᵢᵣ" ,12); D.print("Dᵢᵣ" ,12); Dsizes.print("Dᵢᵣ" ,12);
	std::cout << "\nassert calcMarginalDistribution ... ";
	assert( Amarg.compareEpsilon(A,std::numeric_limits<double>::epsilon()) );
	assert( Bmarg.compareEpsilon(B,std::numeric_limits<double>::epsilon()) );
	assert( Cmarg.compareEpsilon(C,std::numeric_limits<double>::epsilon()*5) ); // FIXME - epsilon*5 is because there's plenty of summation going inside, and epsilons do accumulate. So it's not wrong to multiply by 5. FIXME is because I should be able to determine somehow if it's 5 or another number
	assert( Dmarg.compareEpsilon(D,std::numeric_limits<double>::epsilon()*2) );
	std::cout << "OK\n";
	
	std::cout << "checking operator == and != ... ";
	NDimTable<double> A2(A);
	assert(not ( A2 != A ));
	assert(    ( A2 == A ));
	std::cout << "OK\n";
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::cout << "checking index contraction: 3,6,7 vs. " << T.marginalDistributionIndexContraction({5,3,3,5,6,9,6,7}, {0,0,1,0,0,0,1,1} ) << " \n";

	double sumAi=Asizes.integrateAll({dim_A[0]/sumA,dim_A[1]*1.0,dim_A[2]*1.0});
	double sumBi=Bsizes.integrateAll({dim_B[0]/sumB,dim_B[1]*1.0});
	double sumCi=Csizes.integrateAll({dim_C[0]/sumC});
	double sumDi=Dsizes.integrateAll({dim_D[0]/sumD,dim_D[1]*1.0});
	std::cout << "sumAi= " << sumAi<< "\n";
	std::cout << "sumBi= " << sumBi<< "\n";
	std::cout << "sumCi= " << sumCi<< "\n";
	std::cout << "sumDi= " << sumDi<< "\n";
	std::vector<const NDimTable<double>*> partsNorm2({&Asizes,&Bsizes,&Csizes,&Dsizes});
	NDimTable<double> TnormSizes(partsNorm2);
	
	std::cout << "\nassert TnormSizes ... ";
	//Tnorm.print("Tᵦᵧᵥᵨᵩᵪᵢᵣ",12);
	for(size_t A_  =0 ; A_  < dim_A[0] ; A_  ++)
	for(size_t A__ =0 ; A__ < dim_A[1] ; A__ ++)
	for(size_t A___=0 ; A___< dim_A[2] ; A___++)
	for(size_t B_  =0 ; B_  < dim_B[0] ; B_  ++)
	for(size_t B__ =0 ; B__ < dim_B[1] ; B__ ++)
	for(size_t C_  =0 ; C_  < dim_C[0] ; C_  ++)
	for(size_t D_  =0 ; D_  < dim_D[0] ; D_  ++)
	for(size_t D__ =0 ; D__ < dim_D[1] ; D__ ++) {
		assert( std::abs(TnormSizes.atSafe({A_,A__,A___,B_,B__,C_,D_,D__}) - 
				/*A.at(A_,A__,A___) =*/ ( A_+10*A__+100*A___+0.1 )*
				/*B.at(B_,B__)      =*/ ( B_+10*B__+0.2          )*
				/*C.at(C_)          =*/ ( C_ + 0.3               )*
				/*D.at(D_,D__)      =*/ ( D_+10*D__+0.4          ) 
				//    /(sumA*sumB*sumC*sumD) 
				  ) < epsilon
		      );
	}
	std::cout << "OK\n";
	double sumTi=TnormSizes.integrateAll({dim_A[0]/sumA,dim_A[1]*1.0,dim_A[2]*1.0,dim_B[0]/sumB,dim_B[1]*1.0,dim_C[0]/sumC,dim_D[0]/sumD,dim_D[1]*1.0});
	std::cout << "sumTi= " << sumTi<< "\n";
	
	std::cout << "\nTrying marginal distributions (using spatial sizes) now ... ";
	NDimTable<double> AmargSizes =TnormSizes.calcMarginalDistribution({1,1,1,0,0,0,0,0},{dim_A[0]/sumA,dim_A[1]*1.0,dim_A[2]*1.0,dim_B[0]/sumB,dim_B[1]*1.0,dim_C[0]/sumC,dim_D[0]/sumD,dim_D[1]*1.0});
	assert( AmargSizes.compareEpsilon(Asizes,std::numeric_limits<double>::epsilon()*700) );
	NDimTable<double> BmargSizes =TnormSizes.calcMarginalDistribution({0,0,0,1,1,0,0,0},{dim_A[0]/sumA,dim_A[1]*1.0,dim_A[2]*1.0,dim_B[0]/sumB,dim_B[1]*1.0,dim_C[0]/sumC,dim_D[0]/sumD,dim_D[1]*1.0});
	assert( BmargSizes.compareEpsilon(Bsizes,std::numeric_limits<double>::epsilon()*70) );
	NDimTable<double> CmargSizes =TnormSizes.calcMarginalDistribution({0,0,0,0,0,1,0,0},{dim_A[0]/sumA,dim_A[1]*1.0,dim_A[2]*1.0,dim_B[0]/sumB,dim_B[1]*1.0,dim_C[0]/sumC,dim_D[0]/sumD,dim_D[1]*1.0});
	assert( CmargSizes.compareEpsilon(Csizes,std::numeric_limits<double>::epsilon()*10) );
	NDimTable<double> DmargSizes =TnormSizes.calcMarginalDistribution({0,0,0,0,0,0,1,1},{dim_A[0]/sumA,dim_A[1]*1.0,dim_A[2]*1.0,dim_B[0]/sumB,dim_B[1]*1.0,dim_C[0]/sumC,dim_D[0]/sumD,dim_D[1]*1.0});
	assert( DmargSizes.compareEpsilon(Dsizes,std::numeric_limits<double>::epsilon()*50) );
	std::cout << "OK\n";
	AmargSizes.print("Aᵦᵧᵥ",12);
	BmargSizes.print("Bᵨᵩ" ,12);
	CmargSizes.print("Cᵪ"  ,12);
	DmargSizes.print("Dᵢᵣ" ,12);
	double sumAiS=AmargSizes.integrateAll({dim_A[0]/sumA,dim_A[1]*1.0,dim_A[2]*1.0});
	double sumBiS=BmargSizes.integrateAll({dim_B[0]/sumB,dim_B[1]*1.0});
	double sumCiS=CmargSizes.integrateAll({dim_C[0]/sumC});
	double sumDiS=DmargSizes.integrateAll({dim_D[0]/sumD,dim_D[1]*1.0});
	std::cout << "sumAiS= " << sumAiS<< "\n";
	std::cout << "sumBiS= " << sumBiS<< "\n";
	std::cout << "sumCiS= " << sumCiS<< "\n";
	std::cout << "sumDiS= " << sumDiS<< "\n";
	double sumAiS2=AmargSizes.sumAll();
	double sumBiS2=BmargSizes.sumAll();
	double sumCiS2=CmargSizes.sumAll();
	double sumDiS2=DmargSizes.sumAll();
	std::cout << "sumAiS2= " << sumAiS2<< "\n";
	std::cout << "sumBiS2= " << sumBiS2<< "\n";
	std::cout << "sumCiS2= " << sumCiS2<< "\n";
	std::cout << "sumDiS2= " << sumDiS2<< "\n";
	
	std::cout << "dim_A sizes = " << dim_A << " \n";
	std::cout << "dim_B sizes = " << dim_B << " \n";
	std::cout << "dim_C sizes = " << dim_C << " \n";
	std::cout << "dim_D sizes = " << dim_D << " \n";
//	std::cout << "dim_A+dim_B sizes = " << dim_A+dim_B << " \n";
//	std::cout << "dim_A+dim_C sizes = " << dim_A+dim_C << " \n";
//	std::cout << "dim_A+dim_C+dim_D sizes = " << dim_A+dim_C+dim_D << " \n";
//	std::cout << "dim_A+dim_B+dim_C+dim_D sizes = " << dim_A+dim_B+dim_C+dim_D << " \n";
};

