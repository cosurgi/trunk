// 2015 © Janek Kozicki <cosurgi@gmail.com>

// An N dimensional linear (vector) space over scalar K.
// This class allows storage of arbitrary dimensional arrays, or tables. It's primary purpose is for wavefunctions in quantum mechanics.
// It is dedicated to use with libfftw3 library, but will work (slower) without it.
// Underlying storage is a 1D std::vector<K,A> with custom fftw3 allocator A, information about dimensions is stored in another std::vector<int>
//   - it has operators for component-wise: + - * /
//   - as well as for     scalar arguments: + - * /
// I plan to use it with K=Real, then implement a second one for K=complex, where fftw3 split complex numbers will be used, based on K=Real.

#pragma once

#include <stdexcept>
#include <memory>
#include <vector>
#include <boost/foreach.hpp>
///  #include "lib/base/Math.hpp"   // allow basic testing first

#ifdef YADE_FFTW3
#include <FFTW3_Allocator.hpp>
#endif

template <typename K> // FIXME: do something so that only float, double, long double, float128 are allowed.
//    NDimensionalTable       ← better name?
class NDimensionalDiscreteRealLinearSpace : private std::vector<K
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

		std::size_t                rank_d;      // number of dimensions, ᵈ
		std::vector<std::size_t>   dim_n;       // array dimensions are n⁰ × n¹ × n² × … × nᵈ⁻¹
		std::size_t                total;       // total number of elements

	private:
		inline void calc_dim(const std::vector<std::size_t>& d) {
			dim_d  = d;
			rank_n = dim_d.size();
			total  = 1;
			BOOST_FOREACH( std::size_t d_n, dim_d ) { total *= d_n; };
		}
		// element is located at the position iᵈ⁻¹ + nᵈ⁻¹ *(iᵈ⁻² + nᵈ⁻² *( ... n³*( i² + n²*(i¹ + n¹ * i⁰))))
		// position is {i⁰, i¹, i², …, iᵈ⁻¹}
		// Row-major Format is used, the last dimension, iᵈ⁻¹, varies most quickly
		inline std::size_t calc_position(const std::vector<size_t>& position) {
			std::size_t ret(position[0]);
			for(std::size_t m=1; m<rank ; m++) {
				ret *= dim_d[m];
				ret += position[m];
			};
			return ret;
		};
		inline std::size_t calc_position_safe(const std::vector<size_t>& position) {
			std::size_t ret(position[0]);                  // store i⁰
			if(ret >= dim_d[0]) throw std::out_of_range(); // verify that i⁰<n⁰
			for(std::size_t m=1; m<rank ; m++) {
				std::size_t i      = position[m];     // get position at mᵗʰ dimension, eg. i¹
				std::size_t n      = dim_d[m];        // get array        dimension n , eg. n¹
				if(i >= n) throw std::out_of_range();
				//ret = ret*n+i;
				ret *= n;
				ret += i;
			};
			return ret;
		};
	public:
		// do not expose iterators
		// iterator:: ....

		// constructor
		NDimensionalDiscreteRealLinearSpace(){}; //http://en.cppreference.com/w/cpp/container/vector/vector
		// constructor for referencing a sub-array (???)

		// allocation
		void resize(const std::vector<std::size_t>& d ) {
			calc_dim(d);
			base::resize(total)
		};
		void resize(const std::vector<std::size_t>& d, value_type init) {
			calc_dim(d);
			parent::resize(total,init)
		};
		// capacity
		std::size_t                     size_total()         { return total;     }; // return total number of elements
		std::size_t                     size0(std::size_t n) { return dim_d[n  ];}; // return size in nᵗʰ dimension (counting from 0)
		std::size_t                     size1(std::size_t n) { return dim_d[n-1];}; // return size in nᵗʰ dimension (counting from 1)
		const std::vector<std::size_t>& dim()                { return dim_d;     }; // return dim_d
		std::size_t                     rank()               { return rank_n;    }; // return number of dimensions

		// element access: http://www.fftw.org/fftw3_doc/Row_002dmajor-Format.html#Row_002dmajor-Format
		// operator[] works for up to 4 dimensions, otherwise at(std::vector<std::size_t> >) must be used
		reference at(const std::vector<size_t>& pos) { return parent::operator[calc_position_safe(pos)]; };

		// elementwise operations
		operator=
		operator+=
		operator-=
		// operators *= and /= implement  http://en.wikipedia.org/wiki/Hadamard_product_%28matrices%29
		operator*=
		valarray& operator*= (const valarray& rhs);
		valarray& operator/= (const K val);
		operator/=
		abs()
		pow()

		// contractions (returns new container of different dimension, or works on a provided container of expected dimension)

		// FFTW3, here or there?
                //        There is a separate class for complex<K>, which provides FFT and IFFT
};

