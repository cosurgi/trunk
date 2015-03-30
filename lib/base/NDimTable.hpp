// 2015 © Janek Kozicki <cosurgi@gmail.com>

// FIXME (think what it really is, check definitions): An N dimensional linear (vector) space over scalar K.
// This class allows storage of arbitrary dimensional arrays, or tables. It's primary purpose is for wavefunctions in quantum mechanics.
// It is dedicated to use with libfftw3 library, but will work (slower) without it.
// Underlying storage is a 1D std::vector<K,A> with custom fftw3 allocator A, information about dimensions is stored in another std::vector<int>
//   - it has operators for component-wise: + - * /
//   - as well as for     scalar arguments: + - * /
//   - contraction along some dimensions, by summing all elements.
// I plan to use it with K=Real, then implement a second one for K=complex, where fftw3::split-complex numbers will be used, based on K=Real.

#pragma once

#include <stdexcept>
#include <memory>
#include <vector>
#include <cassert>
#include <boost/foreach.hpp>
///  #include "lib/base/Math.hpp"   // allow basic testing first

#ifdef YADE_FFTW3
#include "lib/base/FFTW3_Allocator.hpp"
#endif

template <typename K> // FIXME: do something so that only float, double, long double, float128 are allowed.
class NDimTable : private std::vector<K
	#ifdef YADE_FFTW3
	, FFTW3_Allocator<K>  // when FFTW3 is available, use it.
	#endif
	>
{
	private:
		typedef std::vector<K
			#ifdef YADE_FFTW3
			, FFTW3_Allocator<K>
			#endif
			>                  parent;
		
		typedef K                  value_type;
		typedef value_type*        pointer;
		typedef const value_type*  const_pointer;
		typedef value_type&        reference;
		typedef const value_type&  const_reference;
		typedef std::size_t        size_type;
		typedef std::ptrdiff_t     difference_type;
	public:
		typedef std::vector<std::size_t>  DimN;
		typedef std::vector<value_type>   DimReal;
	private:
		std::size_t                rank_d;      // rank, ᵈ
		DimN                       dim_n;       // array dimensions are n⁰ × n¹ × n² × … × nᵈ⁻¹
								// NOTE: actually for a tensor all nⁱ are equal, because that's the number
								// of dimensions in the manifold
		std::size_t                total;       // total number of elements

	private:
		inline void printDebugInfo() {
			std::cout << "new tensor rank=" <<rank_d << "\n";
			std::cout << "new tensor dim_n: "; BOOST_FOREACH( std::size_t d_n, dim_n ) std::cout << d_n << ", "; std::cout << "\n";
			std::cout << "new tensor total=" <<total << "\n";
		};

/* OK */	inline void calcTensorProductDimRankTotal(const std::vector<const NDimTable*>& others) {
			rank_d = 0;            // new tensor rank is the sum of all ranks
			for(std::size_t i=0 ; i<others.size() ; i++) { rank_d += others[i]->rank_d; };
			dim_n.resize(0);
			dim_n.reserve(rank_d); // dim_n is all the dimensions, one after another.
			for(std::size_t i=0 ; i<others.size() ; i++) { dim_n.insert( dim_n.end(), others[i]->dim_n.begin(), others[i]->dim_n.end() ); };
			total  = 1;            // total numer of elements (really explodes)
			BOOST_FOREACH( std::size_t d_n, dim_n ) { total *= d_n; };
			
			printDebugInfo();
		}
		inline void calcDimRankTotal(const std::vector<std::size_t>& d) {
			dim_n  = d;
			rank_d = dim_n.size();
			total  = 1;
			BOOST_FOREACH( std::size_t d_n, dim_n ) { total *= d_n; };
			
			printDebugInfo();
		}
		// element access: http://www.fftw.org/fftw3_doc/Row_002dmajor-Format.html#Row_002dmajor-Format
		// element is located at the position iᵈ⁻¹ + nᵈ⁻¹ *(iᵈ⁻² + nᵈ⁻² *( ... n³*( i² + n²*(i¹ + n¹ * i⁰))))
		// position is {i⁰, i¹, i², …, iᵈ⁻¹}
		// Row-major Format is used, the last dimension, iᵈ⁻¹, varies most quickly
		inline std::size_t calcPosition(const std::vector<size_t>& position, std::size_t start=0) {
			std::size_t ret(position[ start ]);     // ret = i⁰  ← position of element if table was 1D
			for(std::size_t m=1; m<rank_d ; m++) {
				ret *= dim_n[m];                // ret =      n¹ * i⁰
				ret += position[m+start];       // ret = i¹ + n¹ * i⁰
			};
			return ret;
		};
		inline std::size_t calcPositionSafe(const std::vector<size_t>& position) {
			std::size_t ret(position[0]);                  // store i⁰
			if(ret >= dim_n[0]) throw std::out_of_range(); // verify that i⁰<n⁰
			for(std::size_t m=1; m<rank_d ; m++) {
				std::size_t i      = position[m];     // get position at mᵗʰ dimension, eg. i¹
				std::size_t n      = dim_n[m];        // get array        dimension n , eg. n¹
				if(i >= n) throw std::out_of_range();
				//ret = ret*n+i;
				ret *= n;
				ret += i;
			};
			return ret;
		};
	public:
		//friend bool unitTesting<typename K>(const NDimTable<K>&);
		// empty constructor
		NDimTable() : parent(std::size_t(0))
		{}; //http://en.cppreference.com/w/cpp/container/vector/vector
		NDimTable(const std::vector<std::size_t>& d)                  : parent(std::size_t(0)) { resize(d     ); };
		NDimTable(const std::vector<std::size_t>& d, value_type init) : parent(std::size_t(0)) { resize(d,init); };
		// copy constructor
		NDimTable(const NDimTable& other) 
			: parent(static_cast<const parent&>(other)), rank_d(other.rank_d), dim_n(other.dim_n), total(other.total) 
		{};
		// tensor product constructor
		NDimTable(const std::vector<const NDimTable*>& others)
			: parent(std::size_t(0))
		{
			calcTensorProduct(others); // fill in the data
		};

		// allocation
/* OK */	void resize(const std::vector<std::size_t>& d ) {
			calcDimRankTotal(d);
			parent::resize(total);
		};
/* OK */	void resize(const std::vector<std::size_t>& d, value_type init) {
			calcDimRankTotal(d);
			parent::resize(total,init);
		};

		// capacity
		std::size_t                     size_total()         { return total;     }; // return total number of elements
		std::size_t                     size0(std::size_t n) { return dim_n[n  ];}; // return size in nᵗʰ dimension (counting from 0)
		std::size_t                     size1(std::size_t n) { return dim_n[n-1];}; // return size in nᵗʰ dimension (counting from 1)
		const std::vector<std::size_t>& dim()                { return dim_n;     }; // return dim_n
		std::size_t                     rank()               { return rank_d;    }; // return number of dimensions

		// at works for up to 3 dimensions, otherwise at(std::vector<std::size_t> >) must be used
		// last index always changes fastest
		reference atSafe(const std::vector<size_t>& pos)                           { return parent::operator[](calcPositionSafe(pos));     };
/* OK */	reference at(const std::vector<size_t>& pos, std::size_t start=0)          { return parent::operator[](calcPosition(pos,start));   };
/* OK */	reference at(std::size_t i)                             { assert(rank_d==1); return parent::operator[](            i            ); };
/* OK */	reference at(std::size_t i,std::size_t j)               { assert(rank_d==2); return parent::operator[](            j+i*dim_n[1] ); };
/* OK */	reference at(std::size_t i,std::size_t j,std::size_t k) { assert(rank_d==3); return parent::operator[](k+dim_n[2]*(j+i*dim_n[1])); };

		// // elementwise operations
		// operator=
		// operator+=
		// operator-=
		// // operators *= and /= implement  http://en.wikipedia.org/wiki/Hadamard_product_%28matrices%29
		// operator*=
		// valarray& operator*= (const valarray& rhs);
		// valarray& operator/= (const K val);
		// operator/=
		// abs()
		// pow()

		// // contractions (returns new container of different dimension, or works on a provided container of expected dimension)
		// //
		//
		//
		//
		//
		//
		//   metoda na testowanie kontrakcji, to jest znormalizować
		//   całą tablicę (podzielić wszystko przez sumę) przed
		//   zrobieniem iloczynu tensorowego.
		//
		//   Potem gdy zrobię kontrakcję, żeby przejść w drugą stronę
		//   powinienem otrzymać dokładnie to samo co miałem wcześniej.
		//
		//
		//
		//
		//
		//
		// // albo lepiej jako argument std::vector<int> (int to numer wymiaru)
		// // lub też std::vector<bool>, gdzie true jeśli trzeba sumować.
		// void contractThese(std::size_t which_to_sum1, NDimTable& output) {
		// 	// sum over       which_to_sum1    dimension
		// };
		// void contractThese(std::size_t which_to_sum1, std::size_t which_to_sum2, NDimTable& output) {
		// 	// sum over       which_to_sum2    dimension
		// };
		// void contractThese(std::size_t which_to_sum1, std::size_t which_to_sum2, std::size_t which_to_sum3, NDimTable& output) {
		// 	// sum over       which_to_sum3    dimension
		// };
		// 
		// void contractExceptForThese(std::size_t which_to_sum1, NDimTable& output) {
		// 	// sum over       which_to_sum1    dimension
		// };
		// void contractExceptForThese(std::size_t which_to_sum1, std::size_t which_to_sum2, NDimTable& output) {
		// 	// sum over       which_to_sum2    dimension
		// };
		// void contractExceptForThese(std::size_t which_to_sum1, std::size_t which_to_sum2, std::size_t which_to_sum3, NDimTable& output) {
		// 	// sum over       which_to_sum3    dimension
		// };

	private:
/* OK */	void increment(std::vector<std::size_t>& pos_i)
		{
			bool done(false);
			std::size_t rank_i(rank_d-1);
			// increment last index (last index varies fastest)
			pos_i[rank_i]++;
			// now carry over, if it goes higher than dim_n[rank_i]
			while( pos_i[rank_i] >= dim_n[rank_i] )
			{
				if(rank_i == 0) {
					pos_i[rank_i  ]=0;
					break;
				} else {
					pos_i[rank_i--]=0;
					pos_i[rank_i  ]++;
				}
			};
		};
	public:
		// tensor product
		// also can be called from constructor
/* OK */	void calcTensorProduct(const std::vector<const NDimTable*>& others)
		{
			assert(others.size()>=2);
			calcTensorProductDimRankTotal(others);
			parent::resize(total);      // now it's ready to calculate data
			// now I need an N-dimensional loop, to multiply everything by everything by everything by ...
			std::vector<std::size_t> pos_i(rank_d,0);
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total; total_i++) {
				// Tᵦᵧᵨᵩᵪ=AᵦᵧBᵨᵩCᵪ
				// we are calculating total_iᵗʰ element now, start with 1 then *= each dimension, last index varies fastest
				parent::operator[](total_i) = 1.0;

				// off to good start. Now I need to multiply total_iᵗʰ by everything else: Tᵦᵧᵨᵩᵪ=AᵦᵧBᵨᵩCᵪ
				// that means that for given total_i we need to travel all the others: A B C

				std::size_t that_other_i_starts_here(0);
				for(std::size_t others_i=0 ; others_i < others.size() ; others_i++){
					// from each other I pick relevant element by using current pos_i
					parent::operator[](total_i) *= const_cast<NDimTable*>(others[others_i])
									  ->at(pos_i , that_other_i_starts_here); // FIXME - zamiast ciągle liczyć at, to 
									  // zrobić print at(..) i zobaczyć jak się ten index zmienia.
									  // bo może zamiast wciąż liczyć bym mógł tylko dodawać?
					that_other_i_starts_here    += others[others_i]->rank_d;
				};
				increment(pos_i);
			}
		};

		// FFTW3, here or there?
                //        There is a separate class for complex<K>, which provides FFT and IFFT
};

// dobra, klasa zaczyna działać.
//
// teraz, żebym mógł chociaż cokolwiek policzyć, to muszę zrobić klasę dla liczb zespolonych z FFT, IFFT

