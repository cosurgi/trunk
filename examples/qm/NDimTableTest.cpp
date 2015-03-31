
#define YADE_FFTW3
#include <iostream>
#include "lib/base/NDimTable.hpp"

typedef NDimTable<float> A;
A f(A a) {
	return a;
}

struct B : A {
	std::string s2; 
	int n;
	B() =default ;
	B(const B& other) = default ;
	B(const A& other) : A(other) {std::cout <<"COPY   B(const A& other)";};

B(B&& other): A(static_cast<A&&>(other)) { std::cout <<"MOVE  B(B&& other): A(static_cast<A&&>(other))";};
B(A&& other): A(static_cast<A&&>(other)) { std::cout <<"MOVE  B(A&& other): A(static_cast<A&&>(other))";};
B& operator=(const B&)=default;
B& operator=(B&&)=default;
		// implicit move contructor B::(B&&)        //	
		// calls A's move constructor               //	
		// calls s2's move constructor              //	
		// and makes a bitwise copy of n
};

struct C : B {
	~C() {}; // destructor prevents implicit move ctor C::(C&&)
};

struct D : B {
	D() {}
	~D() {}; // destructor would prevent implicit move ctor D::(D&&)
	D(D&&) = default; // force a move ctor anyway
};

int main(void){
	std::cout << "#./NDimTableTest > test.tmp 2>&1\n#diff -s test.tmp NDimTableTest_correct_output.txt\n";
	std::vector<std::size_t> dim1;
	dim1.push_back(3);
	dim1.push_back(4);
	//dim1.push_back(2);
	NDimTable<float> T1;
	T1.resize(dim1);
	for(int i=0 ; i<3; i++)
		for(int j=0 ; j<4 ; j++)
			//for(int k=0 ; k<4 ; k++)
				//T1.at(i,j,k)=100*i+10*j+k;
				  T1.at(i,j  )=100*i+10*j +1 ;

	for(int i=0 ; i<3; i++) {
		for(int j=0 ; j<4 ; j++) {
			//for(int k=0 ; k<2 ; k++) {
				//std::cout << T1.at(i,j,k) << " ";
				std::cout << T1.at(i,j) << " ";
			//};
			//std::cout << "\n";
		};
		std::cout << "\n";
	};
	
	std::cout << "\n\nchecking print() call\n\n";
	T1.print();	
	
	std::vector<std::size_t> dim2;
	dim2.push_back(5);
	//dim2.push_back(4);
	//dim1.push_back(2);
	std::cout << "\nCREATE T2\n";
	NDimTable<float> T2(dim2);
	for(int i=0 ; i<5; i++)
				T2.at(i    )=1000*i+1000       ;
	for(int i=0 ; i<5; i++)
				std::cout << T2.at(i  ) << " ";
	
	std::cout << "\n\nchecking print() call\n\n";
	T2.print();	
	
	std::cout << "\n";
	
	std::vector<const NDimTable<float>*> parts;
	parts.push_back(&T1);
	parts.push_back(&T2);
	NDimTable<float> T3(parts);
	std::cout << "constructed, print now ---- checking TENSOR PRODUCT\n";
	for(int i=0 ; i<3; i++) {
		for(int j=0 ; j<4 ; j++) {
			for(int k=0 ; k<5 ; k++) {
				std::cout << T3.at(i,j,k) << " (" << (100*i+10*j +1)*(1000*k+1000) << ")   " ;
				assert  (    T3.at(i,j,k) ==         (100*i+10*j +1)*(1000*k+1000) );
				//std::cout << T1.at(i,j) << " ";
			};
			std::cout << "\n";
		};
		std::cout << "\n";
	};

	std::cout << "checking print() call\n\n";
	T3.print();
	
	std::cout << "T4\n";
	NDimTable<float> T4;T4=T1;
	T1.print();
	T4.print();
	
	std::cout << "T5\n";
//	std::cout << "T1 address= "<< &T1 << "\n";
	NDimTable<float> T5(std::move(T1));
	T5.print();
	T1.print(); //this must segfault, so don't do it.
//	std::cout << "T1 address= "<< &T1 << "\n";
//	std::cout << "T5 address= "<< &T5 << "\n";
	std::cout << "restore content of T1=T5\n";
	T1=T5;
//	std::cout << "T1 address= "<< &T1 << "\n";
//	std::cout << "T5 address= "<< &T5 << "\n";
	T1.print(); //this must NOT segfault now.

	std::cout << "\n\nTESTS from http://en.cppreference.com/w/cpp/language/move_constructor\n\n";
	std::cout << "  Trying to move A (a1) // move-construct from rvalue temporary         ";
	A a1 = f(A(dim1));                        // move-construct from rvalue temporary
	std::cout << "  Trying to move A (a2)          ";
	A a2 = std::move(a1); // move-construct from xvalue

	std::cout << "  Trying to create A (first create)  ";
	A a4(dim1);
	std::cout << "  Trying to create B (first create)  ";
	B b1(dim1);
//	std::cout << "!! FIXME - 'move failed! rank:2' above!\n\n"; // FIXED by adding  B(A&& other): A(static_cast<A&&>(other)) 
	std::cout << "  Before move, b1 = \"" << b1 << "\"\n";
	B b2 = std::move(b1); // calls implicit move ctor
	std::cout << "  After move, b2 = \"" << b2 << "\"\n";
	std::cout << "  After move, b1 = \"" << b1 << "\"\n";

	std::cout << "  Trying to move C               ";
	C c1;
	C c2 = std::move(c1); // calls the copy constructor

	std::cout << "  Trying to move D               ";
	D d1;
	D d2 = std::move(d1);

	std::cout << "\n\n  TESTING operators! \n\n";
	T1.print();
	std::cout << "T1-=4;\n";T1-=4;  T1.print();
	std::cout << "T1+=4;\n";T1+=4;  T1.print();
	std::cout << "T1*=4;\n";T1*=4;  T1.print();
	std::cout << "T1/=4;\n";T1/=4;  T1.print();
	
	std::cout << "T1*=T1;\n";T1*=T1;  T1.print();
//	std::cout << "T1.sqrt();\n";T1.sqrt();  T1.print();
	std::cout << "T1/=T5;\n";T1/=T5;  T1.print();
	std::cout << "T1+=T1;\n";T1+=T1;  T1.print();
	std::cout << "T1-=T1;\n";T1-=T1;  T1.print();
	
	std::cout << "T6=T5-100; =========\n";
	A T6=T5-100;
//	A T6(T5);T6-=100;
	T6.print();
	T6.abs().print();
	T6.print();
	std::cout << "T6*=T6;\n";
	T6*=T6;
	T6.print();
	std::cout << "T6.sqrt();\n";
	T6.sqrt();
	T6.print();
	std::cout << "T6.pow(2.0001);\n";
	T6.pow(2.0001);
	T6.print();

	std::cout << "T5="<<T5<<"\n";
	std::cout << "(T5*2).print();\n";
	(T5*2).print();
	std::cout << "....above is FREE of the (T5*2).....\n\na4=T5*2;\n";
	a4=T5*2;
	a4.print();
	std::cout << "a4=a4*2;\n";
	a4=a4*2;
	a4.print();
	
	std::cout << "A a8=a4*2;\n";
	A a8=a4*2;
	a8.print();
	
	std::cout << "A a9(a4*2);\n";
	A a9(a4*2);
	a9.print();

	std::cout << "A a10=a9-1000; =========\n";
	A a10=a9-1000;
	a10.print();	
	std::cout << "A a11=a9/1000; =========\n";
	A a11=a9/1000;
	a11.print();	
	std::cout << "A a12=a9+1000; =========\n";
	A a12=a9+1000;
	a12.print();	
	std::cout << "A a13=a9*1000; =========\n";
	A a13=a9*1000;
	a13.print();	

	std::cout << "A a14=a10-a9 ; =========\n";
	A a14=a10-a9 ;
	a14.print();	
	std::cout << "A a15=a9 /a11; =========\n";
	A a15=a9/a11 ;
	a15.print();	
	std::cout << "A a16=a12-a9 ; =========\n";
	A a16=a12-a9 ;
	a16.print();	
	std::cout << "A a17=a13/a9 ; =========\n";
	A a17=a13/a9 ;
	a17.print();	
	std::cout << "-a11 ; =========\n";
	(-a11).print();
	
//	std::cout << "A a10=1000-a9; =========\n";	a10=1000-a9;               	a10.print();	           
//	std::cout << "A a11=1000/a9; =========\n";	a11=1000/a9;               	a11.print();	           
//	std::cout << "A a12=1000+a9; =========\n";	a12=1000+a9;               	a12.print();	           
//	std::cout << "A a13=1000*a9; =========\n";	a13=1000*a9;	a13.print();	

};

