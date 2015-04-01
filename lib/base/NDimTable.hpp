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
#include <algorithm>    // std::transform
#include <cmath>
#include <complex>

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

		template<typename L> friend class NDimTable;
	public:
		typedef std::vector<std::size_t>  DimN;
		typedef std::vector<value_type>   DimReal;
	private:
	// Attributes
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
		NDimTable() : parent(std::size_t(0)) , rank_d(0), dim_n({}), total(0) {}; //http://en.cppreference.com/w/cpp/container/vector/vector
		NDimTable(const std::vector<std::size_t>& d)                  : parent(std::size_t(0)) { resize(d     ); };
		NDimTable(const std::vector<std::size_t>& d, value_type init) : parent(std::size_t(0)) { resize(d,init); };
		// copy constructor
		NDimTable(const NDimTable& other) 
			: parent(static_cast<const parent&>(other)), rank_d(other.rank_d), dim_n(other.dim_n), total(other.total) 
		{
//			std::cerr << "move failed! rank:" << rank_d << "\n";
		};
		template<typename L> NDimTable(const NDimTable<L>& other) 
			: parent(other.begin(),other.end()), rank_d(other.rank_d), dim_n(other.dim_n), total(other.total) 
		{
			std::cerr << "conversion! rank:" << rank_d << "\n";
		};
		// move constructor
		NDimTable(NDimTable&& other)
			: parent(static_cast<parent&&>(other)), rank_d(std::move(other.rank_d)), dim_n(std::move(other.dim_n)), total(std::move(other.total))
		{
//			std::cerr << "moved! rank:" << rank_d << "\n";
			other={};
		};
		// tensor product constructor
		// careful - it's a vector of pointers. It's intended to be initialised with references
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
/* OK */	void resize(const NDimTable& other) {
			calcDimRankTotal(other.dim_n);
			parent::resize(total);
		};

		// capacity
		std::size_t                     size_total()         const { return total;     }; // return total number of elements
		std::size_t                     size0(std::size_t n) const { return dim_n[n  ];}; // return size in nᵗʰ dimension (counting from 0)
		std::size_t                     size1(std::size_t n) const { return dim_n[n-1];}; // return size in nᵗʰ dimension (counting from 1)
		const std::vector<std::size_t>& dim()  const         { return dim_n;     }; // return dim_n
		std::size_t                     rank() const         { return rank_d;    }; // return number of dimensions

		// at works for up to 3 dimensions, otherwise at(std::vector<std::size_t> >) must be used
		// last index always changes fastest
		reference atSafe(const std::vector<size_t>& pos)                           { return parent::operator[](calcPositionSafe(pos));     };
/* OK */	reference at(const std::vector<size_t>& pos, std::size_t start=0)          { return parent::operator[](calcPosition(pos,start));   };
/* OK */	reference at(std::size_t i)                             { assert(rank_d==1); return parent::operator[](            i            ); };
/* OK */	reference at(std::size_t i,std::size_t j)               { assert(rank_d==2); return parent::operator[](            j+i*dim_n[1] ); };
/* OK */	reference at(std::size_t i,std::size_t j,std::size_t k) { assert(rank_d==3); return parent::operator[](k+dim_n[2]*(j+i*dim_n[1])); };
		const reference atSafe(const std::vector<size_t>& pos)                 const{return parent::operator[](calcPositionSafe(pos));     };
/* OK */	const reference at(const std::vector<size_t>& pos, std::size_t start=0)const{return parent::operator[](calcPosition(pos,start));   };
/* OK */	const reference at(std::size_t i)                            const{ assert(rank_d==1); return parent::operator[](            i            ); };
/* OK */	const reference at(std::size_t i,std::size_t j)              const{ assert(rank_d==2); return parent::operator[](            j+i*dim_n[1] ); };
/* OK */	const reference at(std::size_t i,std::size_t j,std::size_t k)const{ assert(rank_d==3); return parent::operator[](k+dim_n[2]*(j+i*dim_n[1])); };

		// elementwise operations
		NDimTable& operator  = (const NDimTable& )=default;
		NDimTable& operator  = (      NDimTable&&)=default;
		NDimTable& operator -  (          ) {std::transform(this->begin(),this->end(),this->begin(),[ ](K& v){return  -v;});  return *this;}; 
		NDimTable& operator += (const K& k) {std::transform(this->begin(),this->end(),this->begin(),[k](K& v){return v+k;});  return *this;};
		NDimTable& operator -= (const K& k) {std::transform(this->begin(),this->end(),this->begin(),[k](K& v){return v-k;});  return *this;}; 
		NDimTable& operator *= (const K& k) {std::transform(this->begin(),this->end(),this->begin(),[k](K& v){return v*k;});  return *this;}; 
		NDimTable& operator /= (const K& k) {std::transform(this->begin(),this->end(),this->begin(),[k](K& v){return v/k;});  return *this;}; 

		template<typename L> NDimTable& operator += (const NDimTable<L>& T) {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[](K& v,const L& l){return v+l;});return *this;};
		template<typename L> NDimTable& operator -= (const NDimTable<L>& T) {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[](K& v,const L& l){return v-l;});return *this;};
		// !!!!!!!!!!!
		// !IMPORTANT! operators *= and /= implement  http://en.wikipedia.org/wiki/Hadamard_product_%28matrices%29
		template<typename L> NDimTable& operator *= (const NDimTable<L>& T) {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[](K& v,const L& l){return v*l;});return *this;};
		template<typename L> NDimTable& operator /= (const NDimTable<L>& T) {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[](K& v,const L& l){return v/l;});return *this;}; 

		// !!!!!!!!!!!
		// !IMPORTANT! for effciency, these do not copy construct new data, they modify in-place!
		NDimTable& abs()           {std::transform(this->begin(),this->end(),this->begin(),[ ](K& v){return std::abs(v    );}); return *this;}; 
		NDimTable& pow(const K& k) {std::transform(this->begin(),this->end(),this->begin(),[k](K& v){return std::pow(v,k  );}); return *this;};
		NDimTable& sqrt()          {std::transform(this->begin(),this->end(),this->begin(),[ ](K& v){return std::sqrt(v   );}); return *this;};
		NDimTable& conj()          {std::transform(this->begin(),this->end(),this->begin(),[ ](K& v){return std::conj(v   );}); return *this;};

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
/* OK */	int increment(std::vector<std::size_t>& pos_i) const
		{
			int number_of_times_it_carried_over(0);
			bool done(false);
			std::size_t rank_i(rank_d-1);
			// increment last index (last index varies fastest)
			pos_i[rank_i]++;
			// now carry over, if it goes higher than dim_n[rank_i]
			while( pos_i[rank_i] >= dim_n[rank_i] )
			{
				number_of_times_it_carried_over++;
				if(rank_i == 0) {
					pos_i[rank_i  ]=0;
					break;
				} else {
					pos_i[rank_i--]=0;
					pos_i[rank_i  ]++;
				}
			};
			return number_of_times_it_carried_over;
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

		void print(std::ostream& os) const
		{
			if(rank_d==0) return;
			std::vector<std::size_t> pos_i(rank_d,0);
			for(std::size_t total_i=0;total_i < total; total_i++)
			{
				os << parent::operator[](total_i) << " ";
				int n(increment(pos_i));
				for(int i=0;i<n;i++)
					os << "\n";
			}
		}
		void print() const { print(std::cout);};

		// FFTW3, here or there?

		void becomesFFT(NDimTable inp) // FIXME - powinno brać (const NDimTable& inp)
		{
//std::cerr << " ............ i9  \n";
			(*this)=inp; // FIXME - jakoś inaczej
//std::cerr << " ............ i10  \n";
			#ifdef YADE_FFTW3
			fftw_complex *in, *out;
			fftw_plan p;
			int N(inp.total);
			in  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
			out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
			if(rank_d == 1) {
				p = fftw_plan_dft_1d(inp.size0(0), in, out, FFTW_FORWARD, FFTW_ESTIMATE);
			} else
			if(rank_d == 2) {
				p = fftw_plan_dft_2d(inp.size0(0),inp.size0(1), in, out, FFTW_FORWARD, FFTW_ESTIMATE);
//std::cerr << " ............ i11 2d fft  \n";
			};
//std::cerr << " ............ i11  \n";
			
			for(int i=0;i<N;i++) {
				in[i][0]=std::real(inp.operator[](i));
				in[i][1]=std::imag(inp.operator[](i));
			}
		
//std::cerr << " ............ i12  \n";
			fftw_execute(p);
//std::cerr << " ............ i13  \n";
			
			for(int i=0;i<N;i++) {
				this->operator[](i)=value_type(out[i][0],out[i][1]);
			}
//std::cerr << " ............ i14  \n";
			
			fftw_destroy_plan(p);
			fftw_free(in);
			fftw_free(out);
			#else
			#error fftw3 library is needed
			#endif
		};
		
		void becomesIFFT(NDimTable inp) // FIXME - powinno brać (const NDimTable& inp)
		{
//std::cerr << " ............ i9  \n";
			(*this)=inp; // FIXME - jakoś inaczej
//std::cerr << " ............ i10  \n";
			#ifdef YADE_FFTW3
			fftw_complex *in, *out; // FIXME - uwaga - tu muszą być specjalizacje float.double/long double i complex
			fftw_plan p;
			int N(inp.total);
			in  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
			out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
//std::cerr << " ............ i11  \n";
			if(rank_d == 1) {
				p = fftw_plan_dft_1d(inp.size0(0), in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
			} else
			if(rank_d == 2) {
				p = fftw_plan_dft_2d(inp.size0(0),inp.size0(1), in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
//std::cerr << " ............ i11 2d ifft  \n";
			};
			
			for(int i=0;i<N;i++) {
				in[i][0]=std::real(inp.operator[](i));
				in[i][1]=std::imag(inp.operator[](i));
			}
//std::cerr << " ............ 12  \n";
		
			fftw_execute(p);
			
			for(int i=0;i<N;i++) {
				this->operator[](i)=value_type(out[i][0],out[i][1])/((value_type)(N));
			}
			
			fftw_destroy_plan(p);
			fftw_free(in);
			fftw_free(out);	
			//std::cout << "doIFFT_1D fftw3\n";
			#else
			#error fftw3 library is needed
			#endif
		}

};

template<typename K>
std::ostream& operator<<(std::ostream& os, const NDimTable<K>& o){	o.print(os);	return os;};
// FIXME - ambiguous, can do it later, maybe
// tylko tak sobie wymyśliłem, że bym robił `using namespace std`, a potem wywoływam pow(k,-1), i wtedy albo by się robiła
// odwrotność float, albo odwrotność elementów tablicy
//template<typename K, typename L> NDimTable<K> operator+(const L    & k,const NDimTable<K>& a){	NDimTable<K> r( a);	return std::move(r+=k);};
//template<typename K, typename L> NDimTable<K> operator-(const L    & k,const NDimTable<K>& a){	NDimTable<K> r(-a);	return std::move(r+=k);};
//template<typename K, typename L> NDimTable<K> operator*(const L    & k,const NDimTable<K>& a){	NDimTable<K> r( a);	return std::move(r*=k);};
//template<typename K, typename L> NDimTable<K> operator/(const L    & k,const NDimTable<K>& a){	NDimTable<K> r(a.pow(-1));	return std::move(r*=k);};

// FIXME - think - maybe change it in such a way that first argument is always modified.
// FIXME - then I could write the full formula in SchrodingerKosloffPropagator directly.
template<typename K, typename L> NDimTable<K> operator+(const NDimTable<K>& a,const L    & k)
{	
	NDimTable<K> r( a);	// don't copy, create empty, and fill in with correct data
//	NDimTable<K> r;		// FIXME does it make sense to try to prematurely optimise like this??
//	r.dim_n  = a.dim_n;
//	r.rank_d = a.rank_d;
//	r.total  = a.total;
	return std::move(r+=k);
};
template<typename K, typename L> NDimTable<K> operator-(const NDimTable<K>& a,const L    & k){	NDimTable<K> r( a);	return std::move(r-=k); };
template<typename K, typename L> NDimTable<K> operator*(const NDimTable<K>& a,const L    & k){	NDimTable<K> r( a);	return std::move(r*=k); };
template<typename K, typename L> NDimTable<K> operator/(const NDimTable<K>& a,const L    & k){	NDimTable<K> r( a);	return std::move(r/=k); };

// dobra, klasa zaczyna działać.
//
// teraz, żebym mógł chociaż cokolwiek policzyć, to muszę zrobić klasę dla liczb zespolonych z FFT, IFFT


//@@@@@@@@@@@@@@@@@@@@@!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//@@@@@@@@@@@@@@@@@@@@@!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//@@@@@@@@@@@@@@@@@@@@@!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//@@@@@@@@@@@@@@@@@@@@@!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//@@@@@@@@@@@@@@@@@@@@@!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//@@@@@@@@@@@@@@@@@@@@@!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*
		void doFFT_1D (const std::vector<Complexr>& in,std::vector<Complexr>& out);
		void doIFFT_1D(const std::vector<Complexr>& in,std::vector<Complexr>& out);
		void fftTest(); // debug + testing


// for (slow & safe) 2D, check http://stackoverflow.com/questions/17194451/how-to-use-eigen-fft-with-matrixxf
//			FFT<float> fft;
// 			Eigen::Matrix<float, dim_x, dim_y> in = setMatrix();
// 			Eigen::Matrix<complex<float>, dim_x, dim_y> out;
// 			
// 			for (int k = 0; k < in.rows(); k++) {
// 			    Eigen::Matrix<complex<float>, dim_x, 1> tmpOut;
// 			    fft.fwd(tmpOut, in.row(k));
// 			    out.row(k) = tmpOut;
// 			}
// 			
// 			for (int k = 0; k < in.EIG_MAT.cols(); k++) {
// 			    Eigen::Matrix<complex<float>, 1, dim_y> tmpOut;
// 			    fft.fwd(tmpOut, out.col(k));
// 			    out.col(k) = tmpOut;
// 			}
// for faster      - fftw3
//     even faster - CUDA fftw backend
void SchrodingerKosloffPropagator::doFFT_1D (const std::vector<Complexr>& inp,std::vector<Complexr>& outp)
{
	static bool mesg(false);
#ifdef YADE_FFTW3
	if(!mesg){std::cout<<"\n--> Using fftw3 library!\n\n";mesg=true;};

	fftw_complex *in, *out;
	fftw_plan p;
	int N(inp.size());
	in  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	
	for(int i=0;i<N;i++) {
		in[i][0]=std::real(inp[i]);
		in[i][1]=std::imag(inp[i]);
	}

	fftw_execute(p);
	
	for(int i=0;i<N;i++) {
		outp[i]=std::complex<Real>(out[i][0],out[i][1]);
	}
	
	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);
//std::cout << "doFFT_1D fftw3\n";
#else
	if(!mesg){std::cout<<"\n--> Warning: without fftw3 library calculations are a lot slower!\n\n";mesg=true;};
//	std::vector<Complexr> in(inp);
// ROTATE kTable instead.
//	std::rotate(in.begin(),in.begin()+(in.size()/2-0),in.end()); // prepare input: rotate to left by (size/2-1)
	// FIXME - muszę sprawdzić dokładniej, ale to  ↑  chyba dlatego ze mathematica numeruje od 1 a C++ od 0.
	static Eigen::FFT<Real>  fft;
	fft.fwd(outp,inp);                      // in mathematica that's InverseFourier[]*sqrt(N)
//	Real factor=std::sqrt(out.size());
//	FOREACH(Complexr& c, out ) c/=factor; // so I need to divide by sqrt(N) // FIXME - probably unnecessary, just a constant
// ROTATE kTable instead.
//	std::rotate(out.rbegin(),out.rbegin()+(out.size()/2-0),out.rend()); // prepare output: rotate to right by (size/2-1)
//std::cout << "doFFT_1D no fftw3\n";
#endif
}

void SchrodingerKosloffPropagator::doIFFT_1D(const NDimTable<Complexr>& inp,NDimTable<Complexr>& outp)
{
#ifdef YADE_FFTW3
	fftw_complex *in, *out;
	fftw_plan p;
	int N(inp.size());
	in  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	p = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
	
	for(int i=0;i<N;i++) {
		in[i][0]=std::real(inp[i]);
		in[i][1]=std::imag(inp[i]);
	}

	fftw_execute(p);
	
	for(int i=0;i<N;i++) {
		outp[i]=std::complex<Real>(out[i][0],out[i][1])/((Real)(N));
	}
	
	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);	
//std::cout << "doIFFT_1D fftw3\n";
#else
//	std::vector<Complexr> in(inp);
// ROTATE kTable instead.
//	std::rotate(in.begin(),in.begin()+(in.size()/2-0),in.end()); // prepare input: rotate to left by (size/2-1)
	static Eigen::FFT<Real>  fft;
	fft.inv(outp,inp);                      // in mathematica that's Fourier[]/sqrt(N)
//	Real factor=std::sqrt(out.size());
//	FOREACH(Complexr& c, out ) c*=factor; // so I need to multiply by sqrt(N) // FIXME - probably unnecessary, just a constant
// ROTATE kTable instead.
//	std::rotate(out.rbegin(),out.rbegin()+(out.size()/2-0),out.rend()); // prepare output: rotate to right by (size/2-1)
//std::cout << "doIFFT_1D no fftw3\n";
#endif
}

void SchrodingerKosloffPropagator::fftTest()
{
	std::vector<Complexr> in(0),out(8);
	in.push_back(10);
	in.push_back(20);
	in.push_back(1);
	in.push_back(-4);
	in.push_back(5);
	in.push_back(6);
	in.push_back(0);
	in.push_back(1);
	FOREACH(const Complexr& c, in ) { std::cerr << c << ", " ; }; std::cerr << "\n";
	FOREACH(const Complexr& c, out) { std::cerr << c << ", " ; }; std::cerr << "\n";
	doFFT_1D(in,out);
	FOREACH(const Complexr& c, in ) { std::cerr << c << ", " ; }; std::cerr << "\n";
	FOREACH(const Complexr& c, out) { std::cerr << c << ", " ; }; std::cerr << "\n";
	doIFFT_1D(out,in);
	FOREACH(const Complexr& c, in ) { std::cerr << c << ", " ; }; std::cerr << "\n";
	FOREACH(const Complexr& c, out) { std::cerr << c << ", " ; }; std::cerr << "\n";
// debug output:
// FIXME, FIXME - add this to yade --check or test.
// FIXME, FIXME - this is important, because various FFT libraries divide by sqrt(N) or some other numbers.
//////////////////// that output was when I used rotateLeft(size/2-1)
// (10,-2.14306e-16), (20,4.71028e-16), (1,2.14306e-16), (-4,0), (5,-2.14306e-16), (6,-4.71028e-16), (1.57009e-16,2.14306e-16), (1,0), 
// (-0.767767,3.44975), (-10.253,4.94975), (-2.76777,-6.44975), (13.7886,0), (-2.76777,6.44975), (-10.253,-4.94975), (-0.767767,-3.44975), (2.47487,0), 
}
*/
