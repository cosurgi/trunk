
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
	std::cout << "T1 address= "<< &T1 << "\n";
	NDimTable<float> T5(std::move(T1));
	T5.print();
	T1.print(); //this must segfault, so don't do it.
	std::cout << "T1 address= "<< &T1 << "\n";
	std::cout << "T5 address= "<< &T5 << "\n";
	std::cout << "restore content of T1=T5\n";
	T1=T5;
	std::cout << "T1 address= "<< &T1 << "\n";
	std::cout << "T5 address= "<< &T5 << "\n";
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


};

