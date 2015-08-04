// 2015 © Janek Kozicki <cosurgi@gmail.com>

// FIXME (think what it really is, check definitions): An N dimensional linear (vector) space over scalar K.
//       ADDED LATER:   probably it's really just a tensor. But mostly in cartesian space. (very little there is to convert spaces, right now)
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
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <Eigen/Core>

#include <boost/thread/mutex.hpp>
#include <boost/serialization/nvp.hpp>
///  #include "lib/base/Math.hpp"   // allow basic testing first

#ifdef YADE_FFTW3
#include "lib/base/FFTW3_Allocator.hpp"
#endif

template <typename C> std::ostream & operator<<(std::ostream & os, const std::vector<C>& dim)
{
	for(size_t i=0 ; i<dim.size() ; i++ )
	{
		os << dim[i] ;
		if(i!=dim.size()-1) os << ",";
	}
	return os;
};

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

		template<typename Num> struct real_type                     {typedef Num type;};
		template<typename Num> struct real_type<std::complex<Num> > {typedef Num type;};
		typedef typename real_type<K>::type not_complex;

		template<typename L> friend class NDimTable;
	public:
		typedef std::vector<std::size_t>  DimN;
		typedef std::vector<value_type>   DimReal;
	friend class boost::serialization::access;
	private: template<class ArchiveT> void serialize(ArchiveT & ar, unsigned int version){
		ar & BOOST_SERIALIZATION_NVP(rank_d);
		ar & BOOST_SERIALIZATION_NVP(dim_n);
		ar & BOOST_SERIALIZATION_NVP(total);
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(parent);
	}
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
			#ifdef DEBUG_NDIMTABLE
			printDebugInfo();
			#endif
			dirty=true;
		}
		inline void calcDimRankTotal(const std::vector<std::size_t>& d) {
			dim_n  = d;
			rank_d = dim_n.size();
			total  = 1;
			BOOST_FOREACH( std::size_t d_n, dim_n ) { total *= d_n; };	
			#ifdef DEBUG_NDIMTABLE
			printDebugInfo();
			#endif
			dirty=true;
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
			if(ret >= dim_n[0]) throw std::out_of_range("calcPositionSafe ret >= dim_n[0] , "+boost::lexical_cast<std::string>(ret)+" >= "+boost::lexical_cast<std::string>(dim_n[0])); // verify that i⁰<n⁰
			for(std::size_t m=1; m<rank_d ; m++) {
				std::size_t i      = position[m];     // get position at mᵗʰ dimension, eg. i¹
				std::size_t n      = dim_n[m];        // get array        dimension n , eg. n¹
				if(i >= n) throw std::out_of_range("calcPositionSafe i >= n , "+boost::lexical_cast<std::string>(i)+" >= "+boost::lexical_cast<std::string>(n));
				//ret = ret*n+i;
				ret *= n;
				ret += i;
			};
			return ret;
		};
	public:
		//friend bool unitTesting<typename K>(const NDimTable<K>&);
		// empty constructor
		NDimTable() : parent(std::size_t(0)) , rank_d(0), dim_n({}), total(0), dirty(true)
		{
#ifdef DEBUG_NDIM_RAM
std::cerr << "EMPTY constr. NDimTable()           : " << ++ZZ::NDimTable_Instances << "\n";
#endif
		}; //http://en.cppreference.com/w/cpp/container/vector/vector
		NDimTable(const std::vector<std::size_t>& d)                  : parent(std::size_t(0))
		{
#ifdef DEBUG_NDIM_RAM
std::cerr << "SIZE constr. NDimTable(DimN)        : " << ++ZZ::NDimTable_Instances << "\n";
#endif
			resize(d     );
		};
		NDimTable(const std::vector<std::size_t>& d, value_type init) : parent(std::size_t(0))
		{
#ifdef DEBUG_NDIM_RAM
std::cerr << "INIT constr. NDimTable(DimN, init)  : " << ++ZZ::NDimTable_Instances << "\n";
#endif
			resize(d,init);
		};
		// copy constructor
		NDimTable(const NDimTable& other) 
			: parent(static_cast<const parent&>(other)), rank_d(other.rank_d), dim_n(other.dim_n), total(other.total), dirty(true)
		{
#ifdef DEBUG_NDIM_RAM
std::cerr << "COPY constr.(const NDimTable& other): " << ++ZZ::NDimTable_Instances << "\n";
#endif
			#ifdef DEBUG_NDIMTABLE
			std::cerr << "move failed! rank:" << rank_d << "\n";
			#endif
		};
		template<typename L> NDimTable(const NDimTable<L>& other) 
			: parent(other.begin(),other.end()), rank_d(other.rank_d), dim_n(other.dim_n), total(other.total), dirty(true)
		{
#ifdef DEBUG_NDIM_RAM
std::cerr << "CoPy(!) constr.(const NDimTable<L>&): " << ++ZZ::NDimTable_Instances << "\n";
#endif
			#ifdef DEBUG_NDIMTABLE
			std::cerr << "conversion! rank:" << rank_d << "\n";
			#endif
		};
		// move constructor
		NDimTable(NDimTable&& other)
			: parent(static_cast<parent&&>(other)), rank_d(std::move(other.rank_d)), dim_n(std::move(other.dim_n)), total(std::move(other.total)), dirty(true)
		{
#ifdef DEBUG_NDIM_RAM
std::cerr << "MOVE constructor NDimTable()        : " << ++ZZ::NDimTable_Instances << "\n";
#endif
			#ifdef DEBUG_NDIMTABLE
			std::cerr << "moved! rank:" << rank_d << "\n";
			#endif
			other={};
			//add_size();
		};
		// tensor product constructor
		// careful - it's a vector of pointers. It's intended to be initialised with references
		NDimTable(const std::vector<const NDimTable*>& others)
			: parent(std::size_t(0)), dirty(true)
		{
#ifdef DEBUG_NDIM_RAM
std::cerr << "constructor calcTensorProduct(othrs):" << ++ZZ::NDimTable_Instances << "\n";
#endif
			calcTensorProduct(others); // fill in the data
		};

/* ?? */	void subtract_size()
		{
#ifdef DEBUG_NDIM_RAM
		//	std::cerr << "subtract_size:" << ZZ::NDimTable_SIZE_TOTAL << "  →  ";
		//	ZZ::NDimTable_SIZE_TOTAL -= this->capacity()*sizeof(value_type); // + sizeof(&this);
		//	std::cerr                << ZZ::NDimTable_SIZE_TOTAL << "\n";
#endif
		}
/* ?? */	void add_size()
		{
#ifdef DEBUG_NDIM_RAM
		//	std::cerr << "add_size     :" << ZZ::NDimTable_SIZE_TOTAL << "  →  ";
		//	ZZ::NDimTable_SIZE_TOTAL += this->capacity()*sizeof(value_type); // + sizeof(&this);
		//	std::cerr                << ZZ::NDimTable_SIZE_TOTAL << "\n";
#endif
		}
/* ?? */	~NDimTable()
		{
#ifdef DEBUG_NDIM_RAM
std::cerr << "destructor                          : " << --ZZ::NDimTable_Instances << "\n";
#endif
		//	std::cerr << "destructor                          :" << ZZ::NDimTable_SIZE_TOTAL << "  →  ";
		//	ZZ::NDimTable_SIZE_TOTAL -= this->capacity()*sizeof(value_type); // + sizeof(&this);
		//	std::cerr                << ZZ::NDimTable_SIZE_TOTAL << "\n";
		};

/* ?? */	void parent_resize (size_t n)
		{
			subtract_size();
			parent::resize(n);
			add_size();
		}
/* ?? */	void parent_resize (size_t n, const value_type& val)
		{
			subtract_size();
			parent::resize(n,val);
			add_size();
		}

		// allocation
/* OK */	void resize(const std::vector<std::size_t>& d ) {
			if(dim_n==d) return;
			calcDimRankTotal(d);
			parent_resize(total);
		};
/* OK */	void resize(const std::vector<std::size_t>& d, value_type init) {
			calcDimRankTotal(d);
			parent_resize(total,init);
		};
/* OK */	void resize(const NDimTable& other) {
			if(dim_n==other.dim_n) return;
			calcDimRankTotal(other.dim_n);
			parent_resize(total);
		};
/* OK */	void resize(const NDimTable& other, value_type init) {
			if(dim_n==other.dim_n) return;
			calcDimRankTotal(other.dim_n);
			parent_resize(total,init);
		};

		// capacity
		std::size_t                     size_total()         const { return total;     }; // return total number of elements
		std::size_t                     size0(std::size_t n) const { return dim_n[n  ];}; // return size in nᵗʰ dimension (counting from 0)
		std::size_t                     size1(std::size_t n) const { return dim_n[n-1];}; // return size in nᵗʰ dimension (counting from 1)
		const std::vector<std::size_t>& dim()  const         { return dim_n;     }; // return dim_n
		std::size_t                     rank() const         { return rank_d;    }; // return number of dimensions
		void				set_num_threads(int nt) const { Threads::number=nt;};

		// at works for up to 3 dimensions, otherwise at(std::vector<std::size_t> >) must be used
		// last index always changes fastest
/* OK */	value_type atSafeInterpolated(const std::vector<not_complex>& pos)
		{
			DimN pos_floor(rank_d,0);
			DimN pos_ceil (rank_d,0);
			std::vector<not_complex> pos_dist(rank_d,0);
			for(size_t i=0 ; i<pos.size() ; i++)
			{
				pos_floor[i] = std::floor(pos[i]);
				pos_ceil [i] = std::ceil (pos[i]);
				pos_dist [i] = pos[i] - pos_floor[i];
			}
			value_type ret=0;
			for(size_t binary = 0 ; binary < std::pow(2,rank_d) ; binary++ ) // each bit says if it's floor or ceiling for given coord
			{
				DimN pos_corner(rank_d,0);
				double weight = 1.0;
				for(size_t i=0 ; i<pos.size() ; i++)
				{
					bool ith_ceil  = (((binary >> i) & 1)==1);
					pos_corner[i]  = ith_ceil ? pos_ceil [i] : pos_floor[i];
					weight        *= ith_ceil ? pos_dist [i] : (1.0 - pos_dist[i]);
				}
				ret += (parent::operator[](calcPositionSafe(pos_corner)))*weight;
			}
			return ret;
		};
/* OK*/		reference atSafe(const std::vector<size_t>& pos)                           { return parent::operator[](calcPositionSafe(pos));     };
/* OK */	reference at(const std::vector<size_t>& pos, std::size_t start=0)          { return parent::operator[](calcPosition(pos,start));   };
/* OK */	reference at(std::size_t i)                             { assert(rank_d==1); return parent::operator[](            i            ); };
/* OK */	reference at(std::size_t i,std::size_t j)               { assert(rank_d==2); return parent::operator[](            j+i*dim_n[1] ); };
/* OK */	reference at(std::size_t i,std::size_t j,std::size_t k) { assert(rank_d==3); return parent::operator[](k+dim_n[2]*(j+i*dim_n[1])); };
/* OK */	const reference atSafe(const std::vector<size_t>& pos)                 const{return parent::operator[](calcPositionSafe(pos));     };
/* OK */	const reference at(const std::vector<size_t>& pos, std::size_t start=0)const{return parent::operator[](calcPosition(pos,start));   };
/* OK */	const reference at(std::size_t i)                            const{ assert(rank_d==1); return parent::operator[](            i            ); };
/* OK */	const reference at(std::size_t i,std::size_t j)              const{ assert(rank_d==2); return parent::operator[](            j+i*dim_n[1] ); };
/* OK */	const reference at(std::size_t i,std::size_t j,std::size_t k)const{ assert(rank_d==3); return parent::operator[](k+dim_n[2]*(j+i*dim_n[1])); };

/* OK */	bool operator != (const NDimTable& o) { return not ((*this)==o); }
/* OK */	bool operator == (const NDimTable& o) {
			if( (rank_d != o.rank()) or (dim_n != o.dim())) return false;
			auto it_2 = o.begin();
			for(auto it_1 = this->begin(); it_1 != this->end(); ++it_1, ++it_2) if( *it_1 != *it_2) return false;
			return true;
		};
/* OK */	bool compareEpsilon(const NDimTable& o, not_complex eps) {
			if( (rank_d != o.rank()) or (dim_n != o.dim())) return false;
			auto it_2 = o.begin();
			DimN pos_i(rank_d,0);
			for(auto it_1 = this->begin(); it_1 != this->end(); ++it_1, ++it_2) {
				if( std::abs(*it_1 - *it_2) > eps ) 
				{
					std::cerr << "[" << pos_i << "] error: " << boost::lexical_cast<std::string>(std::abs(*it_1 - *it_2)) << " > " << boost::lexical_cast<std::string>(eps) << "\n";
					return false;
				}
				increment(pos_i);
			}
			return true;
		};
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

		// FIXME: should be 'K'-type not 'double'-type. But min(), max() works only with real numbers. 
		// FIXME: this is because potential should be real valued (but isn't)
		// FIXME: if it's 'double' here then better it should be 'Real' so that changing precision works correctly
		not_complex minReal() const {not_complex ret(std::real(this->front())); for(K v : (*this)){ret  = std::min(std::real(v),ret);}; return ret;};
		not_complex maxReal() const {not_complex ret(std::real(this->front())); for(K v : (*this)){ret  = std::max(std::real(v),ret);}; return ret;};
		// !!!!!!!!!!!
		// !IMPORTANT! for effciency, these do not copy construct new data, they modify in-place!
		NDimTable& abs()           {std::transform(this->begin(),this->end(),this->begin(),[ ](K& v){return std::abs(v    );}); return *this;};
		NDimTable& pow(const K& k) {std::transform(this->begin(),this->end(),this->begin(),[k](K& v){return std::pow(v,k  );}); return *this;};
		NDimTable& sqrt()          {std::transform(this->begin(),this->end(),this->begin(),[ ](K& v){return std::sqrt(v   );}); return *this;};
		NDimTable& conj()          {std::transform(this->begin(),this->end(),this->begin(),[ ](K& v){return std::conj(v   );}); return *this;};

		template<typename L> NDimTable& mult2Add(const NDimTable<L>& T,const K& k)
		                           {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[k](K& v,const L& l){return v+l*k;});return *this;};
		template<typename L> NDimTable& mult2Sub(const NDimTable<L>& T,const K& k)
		                           {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[k](K& v,const L& l){return v-l*k;});return *this;};
		template<typename L> NDimTable& mult1Sub(const K& k,const NDimTable<L>& T)
		                           {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[k](K& v,const L& l){return v*k-l;});return *this;};
		template<typename L> NDimTable& multMult(const NDimTable<L>& T,const K& k)
		                           {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[k](K& v,const L& l){return v*l*k;});return *this;};
		template<typename L> NDimTable& mult1Mult2Add(const K& k1,const NDimTable<L>& T,const K& k2)
		                           {std::transform(this->begin(),this->end(),T.begin(),this->begin(),[k1,k2](K& v,const L& l){return v*k1+l*k2;});return *this;};

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
			parent_resize(total);      // now it's ready to calculate data
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

/* OK */	DimN marginalDistributionIndexContraction(const DimN& pos,const std::vector<short int>& remain)
		{
			DimN ret={};
			for(size_t i=0 ; i<remain.size() ; i++) {
				if(remain[i]==1) ret.push_back(pos[i]);
			};
			return std::move(ret);
		};

/* OK */	K sumAll() const {K ret(0); for(K v : (*this)){ret += v;}; return ret;};
/* OK */	NDimTable<K> calcMarginalDistribution(std::vector<short int> remain)
		{
			assert(remain.size()==rank_d);
			DimN dim_new={};
			for(size_t i=0 ; i<rank_d ; i++) {
				assert(remain[i]==0 or remain[i]==1);
				if(remain[i]==1) dim_new.push_back(dim_n[i]);
			};
			NDimTable<K> ret(dim_new,0);
			DimN pos_i(rank_d,0);
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total; total_i++) {
				ret.at(marginalDistributionIndexContraction(pos_i,remain)) += parent::operator[](total_i);
				increment(pos_i);
			}
			return std::move(ret);
		};

/* OK */	K integrateAll(std::vector<not_complex> spatial_sizes) const
		{
			assert(spatial_sizes.size()==rank_d);
			not_complex cell_volume(1);
			for(size_t i=0 ; i<rank_d ; i++) cell_volume *= (not_complex)(spatial_sizes[i])/(not_complex)(dim_n[i]);
			K ret(0); for(K v : (*this)){ret += v*cell_volume;}; return ret;
		};
/* OK */	not_complex integrateAllNormSquared(std::vector<not_complex> spatial_sizes) const
		{
			assert(spatial_sizes.size()==rank_d);
			not_complex cell_volume(1);
			for(size_t i=0 ; i<rank_d ; i++) cell_volume *= (not_complex)(spatial_sizes[i])/(not_complex)(dim_n[i]);
			not_complex ret(0);
			for(K v : (*this)){ret += std::real(std::conj(v)*v)*cell_volume;};
			return ret;
		};
/* ?? */	value_type integrateWithOther_BraKet(const NDimTable& other, std::vector<not_complex> spatial_sizes) const
		{ // calculates <this|other>=∫ψ(x,y,…) φ*(x,y,…) dx dy d…
			if(     (spatial_sizes.size()==rank_d)
			    and (rank_d == other.rank_d)
			    and (dim_n  == other.dim_n )
			    and (total  == other.total ) )
			{
				not_complex cell_volume(1);
				for(size_t i=0 ; i<rank_d ; i++) cell_volume *= (not_complex)(spatial_sizes[i])/(not_complex)(dim_n[i]);
				value_type ret(0);
				//for(K v : (*this)){ret += std::real(std::conj(v)*v)*cell_volume;};
				//{std::transform(this->begin(),this->end(),other.begin(),this->begin(),[](K& v,const L& l){return v*l;});return *this;};
				auto it1 = this->begin();
				auto it2 = other.begin();
				for( ; it1 != this->end() ; it1++ , it2++ )
				{
					ret += (std::conj(*it1) * (*it2) )*cell_volume;
				}
				return ret;
			} else {
				std::cerr << "\n\n ERROR: bad sizes of two wavefunctions:\n";
				std::cerr << " spatial_sizes : " << spatial_sizes                             << "\n";
				std::cerr << " rank_d        :( " << rank_d        <<" ) vs. ( " << other.rank_d   <<" ) \n";
				std::cerr << " dim_n         :( " << dim_n         <<" ) vs. ( " << other.dim_n    <<" ) \n";
				std::cerr << " total         :( " << total         <<" ) vs. ( " << other.total    <<" ) \n";
				return 0;
			}
		};
/* OK */	NDimTable<K> calcMarginalDistribution(std::vector<short int> remain, std::vector<not_complex> spatial_sizes,bool normalize=true,bool densityOnly=false)
		{
			assert(remain.size()==rank_d);
			assert(spatial_sizes.size()==rank_d);
/* */			std::vector<not_complex> new_spatial_sizes={};
			DimN dim_new={};
			not_complex cell_volume(1);
			for(size_t i=0 ; i<rank_d ; i++) {
				assert(remain[i]==0 or remain[i]==1);
				if(remain[i]==1) dim_new.push_back(dim_n[i]);
				else cell_volume *= (not_complex)(spatial_sizes[i])/(not_complex)(dim_n[i]);
/* */				if(remain[i]==1) new_spatial_sizes.push_back(spatial_sizes[i]);
			};
			NDimTable<K> ret(dim_new,0);
			DimN pos_i(rank_d,0);
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total; total_i++) {
				if(not densityOnly) ret.at(marginalDistributionIndexContraction(pos_i,remain)) += parent::operator[](total_i)*cell_volume;
				else    ret.at(marginalDistributionIndexContraction(pos_i,remain)) += parent::operator[](total_i)*std::conj(parent::operator[](total_i))*cell_volume;

/* */	//			        ret.at(marginalDistributionIndexContraction(pos_i,remain)) += parent::operator[](total_i)*std::abs (parent::operator[](total_i))*cell_volume;
/* */		//		        ret.at(marginalDistributionIndexContraction(pos_i,remain)) += parent::operator[](total_i)*std::conj(parent::operator[](total_i))*cell_volume;
/* ?? ok or FIXME?  */	//		ret.at(marginalDistributionIndexContraction(pos_i,remain)) += std::pow(std::abs(parent::operator[](total_i)),2)*cell_volume;
				increment(pos_i);
			}
/* */			if(normalize) ret /= std::sqrt(ret.integrateAllNormSquared(new_spatial_sizes));
			return std::move(ret);
		};

/* OK */	void shiftByHalf()
		{
			for(auto size : dim_n) if((size%2)==1) std::cerr << "\nERROR: NDimTable has o̲d̲d̲ ̲s̲i̲z̲e̲ in some direction, can't shift by half.\n";
			DimN pos_i(rank_d,0),newPos_i(rank_d,0);
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total/2; total_i++) {
				for(unsigned int _d_=0 ; _d_<rank_d ; _d_++)
					newPos_i[_d_] = (pos_i[_d_]+dim_n[_d_]/2)%dim_n[_d_];
				std::swap( parent::operator[](total_i) , at(newPos_i) );
				increment(pos_i);
			}
		};

		typedef std::function<not_complex(not_complex i, int d)>  IToK_func;
/* OK */	void becomeMinusKSquaredTable(const IToK_func& iToK)
		{
			for(auto size : dim_n) if((size%2)==1) std::cerr << "\nERROR: NDimTable has o̲d̲d̲ ̲s̲i̲z̲e̲ in some direction, -k² will be w̲r̲o̲n̲g̲.\n       FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1\n";
			DimN pos_i(rank_d,0);
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total; total_i++) {
				parent::operator[](total_i) = 0;
				for(unsigned int _d_=0 ; _d_<rank_d ; _d_++)
					parent::operator[](total_i) -= std::pow(iToK((pos_i[_d_]+dim_n[_d_]/2)%dim_n[_d_],_d_) ,2);
				increment(pos_i);
			}
		};

		typedef std::function<not_complex(not_complex i, int d)>     IToX_func;
		typedef std::function<value_type (Eigen::Matrix<not_complex,3,1>& xyz)> FunctionVals;
/* OK */	void fill1WithFunction(unsigned short int dim_, const IToX_func& iToX,const FunctionVals f, unsigned short int this_member=0)
		{
			if(dim_ != rank_d) {
				std::cerr << " rank_d        :( " << rank_d        <<" ) \n";
				std::cerr << " dim_n         :( " << dim_n         <<" ) \n";
				std::cerr << " total         :( " << total         <<" ) \n";
				std::cerr << " dim_ (given)  :( " << dim_          <<" ) \n";
				//throw std::out_of_range("\n\nERROR: NDimTable::fill1WithFunction works only for non-entangled wavefunctions.\n\n");
				std::cerr << " this_member   :( " << this_member   <<" ) \n";
			}
			if(dim_ > rank_d) {
				throw std::out_of_range("\n\nERROR: NDimTable::fill1WithFunction dim_ < rank_d !!.\n\n");
			}
			if(dim_ < rank_d and ( ((rank_d % dim_)!=0) or (int(this_member+1)*dim_ > int(rank_d)) )) {
				throw std::out_of_range("\n\nERROR: NDimTable::fill1WithFunction bad rank or dimensions.\n\n");
			}
			DimN pos_i(rank_d,0);
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total; total_i++) {
				Eigen::Matrix<not_complex,3,1> xyz(0,0,0);
				//FIXME: czy iToX() daje dobry wynik, gdy środek State::pos jest przesunięty?? Chyba raczej nie?
				//       a może musze to przesunięcie załatwiac osobno? Przy każdym wywołaniu tej methody, indywidaulnie?
				for(unsigned int _d_=0 ; _d_< dim_ ; _d_++) xyz[_d_]=iToX(pos_i[_d_ + this_member*dim_],_d_);
				parent::operator[](total_i) = f(xyz);
//std::cerr << total_i << " " << pos_i[0] << " " << dim_ << " " << xyz[0] << " " << f(xyz) << "\n";
				increment(pos_i);
			}
		};
		typedef std::function<not_complex(not_complex i, not_complex j, int d)>     IToX_func2;
/* OK */	void fill2WithFunction(  unsigned short int dim_
		                       , unsigned short int start_1_d, unsigned short int start_2_d
		                       , const IToX_func2& iToX2,const FunctionVals f)
		{
			if(rank_d % dim_ != 0) throw std::out_of_range("\n\nERROR: NDimTable::fill2WithFunction detected wrong tensor dimensions: rank_d \% dim_ != 0.\n\n");
			DimN pos_i(rank_d,0);
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total; total_i++) {
				Eigen::Matrix<not_complex,3,1> xyz(0,0,0);
				for(unsigned int _d_=0 ; _d_< dim_ ; _d_++) xyz[_d_]=iToX2(pos_i[_d_+start_1_d],pos_i[_d_+start_2_d],_d_);
				parent::operator[](total_i) = f(xyz);
				increment(pos_i);
			}
		};

/* ?? */	typedef std::function<value_type (Eigen::Matrix<not_complex,3,1>& xyz1,Eigen::Matrix<not_complex,3,1>& xyz2)> FunctionVals2;
/* ?? */	void fill2part_WithFunction(  unsigned short int dim_
/* będzie OK gdy z analitycznym porównam */
		                       , unsigned short int start_1_d, unsigned short int start_2_d
		                       , const IToX_func& iToX1, const IToX_func& iToX2, const FunctionVals2 f)
		{
			if(rank_d % dim_ != 0) throw std::out_of_range("\n\nERROR: NDimTable::fill2part_WithFunction detected wrong tensor dimensions: rank_d \% dim_ != 0.\n\n");
			DimN pos_i(rank_d,0);
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total; total_i++) {
				Eigen::Matrix<not_complex,3,1> xyz1(0,0,0),xyz2(0,0,0);
				for(unsigned int _d_=0 ; _d_< dim_ ; _d_++) xyz1[_d_]=iToX1(pos_i[_d_+start_1_d],_d_);
				for(unsigned int _d_=0 ; _d_< dim_ ; _d_++) xyz2[_d_]=iToX2(pos_i[_d_+start_2_d],_d_);
				parent::operator[](total_i) = f(xyz1,xyz2);
				increment(pos_i);
			}
		};

/* OK */	typedef std::function<not_complex (not_complex i, int d)>     XToI_func;
		// the transformationMap takes all coordinates in the `this` NDimTable and spits out a single coordinate for `other` NDimTable
/* OK */	typedef std::function<not_complex (std::vector<Eigen::Matrix<not_complex,3,1> >& xyzTab, unsigned int coordIdx)> transformationMap;

		// the makeTransformedFromOther takes `other` NDimTable and uses the transformationRules to generate `this` NDimTable
		// other.rank() == this->rank();
		// The dim_n might be actually different (because NaN is not supported - if it was then it would remember which values are undefined)
/* OK ?? */	void calcTransformedFromOther(
			/* FIXME: const */  NDimTable& other
			// dimSpatial are spatial dimensions, rank() may differ, because NDimTable may be a result of tensor product
			, unsigned short int dimSpatial
			, std::vector<transformationMap> transformationRules // a vector of functions
			, const IToX_func& iToX_this
			, const XToI_func& xToI_other)
		{
			if(rank_d != other.rank()) throw std::out_of_range("\n\nERROR: NDimTable::makeTransformedFromOther different rank() \n\n");
			if(rank_d %  dimSpatial !=  0  ) throw std::out_of_range("\n\nERROR: NDimTable::makeTransformedFromOther detected wrong tensor dimensions: rank_d \% dim_ != 0 \n\n");
			unsigned int number_of_particles = rank_d/dimSpatial;
			if(number_of_particles != 2) {
				std::cerr << "unsupported for now number_of_particles = " << number_of_particles << "\n";
				return;
			}
			// coordinates for each particle are in this table
			std::vector<Eigen::Matrix<not_complex,3,1> > xyzTabOther(number_of_particles,Eigen::Matrix<not_complex,3,1>(0,0,0));
			std::vector<Eigen::Matrix<not_complex,3,1> > xyzTabThis (number_of_particles,Eigen::Matrix<not_complex,3,1>(0,0,0));
			DimN pos_i    (rank_d,0);
			std::vector<not_complex> pos_other_for_interpolation(rank_d,0);
			if( number_of_particles*dimSpatial != rank() )
			{
				std::cerr << "\n\n sanity error! \n\n";
				return;
			};
			// helper function returning the coordinate idx of a given particle in the NDimTable
			auto part_pos = [=](unsigned int particle_no, unsigned int spatial_coord)->unsigned int{
				return particle_no*dimSpatial + spatial_coord;
			};
			// last index varies fastest
			for(std::size_t total_i=0;total_i < total; total_i++) {
				for(unsigned int _p_=0 ; _p_< number_of_particles ; _p_++) // go through each particle, to find its coordinates
					for(unsigned int _d_=0 ; _d_< dimSpatial ; _d_++) // go through spatial dimensions, like x, y, z
						xyzTabThis [_p_][_d_]=iToX_this(pos_i[part_pos(_p_,_d_)],part_pos(_p_,_d_));
				for(unsigned int _p_=0 ; _p_< number_of_particles ; _p_++)
					for(unsigned int _d_=0 ; _d_< dimSpatial ; _d_++)
						xyzTabOther[_p_][_d_]=transformationRules[_p_](xyzTabThis ,_d_);
				for(unsigned int _d_=0 ; _d_< dimSpatial ; _d_++)
					for(unsigned int _p_=0 ; _p_< number_of_particles ; _p_++)
						pos_other_for_interpolation[part_pos(_p_,_d_)] = xToI_other(xyzTabOther[_p_][_d_], part_pos(_p_,_d_));
				try { // FIXME: try-catch is very slow
					parent::operator[](total_i) = other.atSafeInterpolated(pos_other_for_interpolation);
				} catch(const std::out_of_range& e) {
					parent::operator[](total_i) = 0;
				};
				increment(pos_i);
			}
		};

		void print(std::ostream& os,std::string l,int width,bool skip_brackets=false) const
		{
			if(rank_d==0) return;
			std::vector<std::size_t> pos_i(rank_d,0);
			for(std::size_t total_i=0;total_i < total; total_i++)
			{
				if(skip_brackets) os << l <<                         std::setw(width) << parent::operator[](total_i) << "\t";
				else              os << l << "[" << pos_i << "]=" << std::setw(width) << parent::operator[](total_i) << "\t";
				int n(increment(pos_i));
				for(int i=0;i<n;i++)
					os << "\n";
			}
		}
		void print() const { print(std::cout,"",0);};
		void print(bool skip_brackets) const { print(std::cout,"",0,skip_brackets);};
		void print(std::ostream& os) const { print(os,"",0);};
		void print(std::string label,int width) const { print(std::cout,label,width);};

		// FFTW3, here or there?
	private:
		bool                       dirty;
		//std::vector<int>           dim_int; // FIXME: http://www.fftw.org/doc/New_002darray-Execute-Functions.html
		//fftw_plan                  p_FFT,p_IFFT; //   http://www.fftw.org/doc/Using-Plans.html
	public:
		void doFFT(const NDimTable& inp,bool forward)
		{
		{
			static boost::mutex mxFFT_FIXME;
			boost::mutex::scoped_lock scoped_lock(mxFFT_FIXME);// FIXME ←----- !! ponieważ ciągle robię nowe fftw_plan_dft(...) to muszę robić mutex

static bool called(false); //http://www.fftw.org/doc/Usage-of-Multi_002dthreaded-FFTW.html
if(not called) {std::cerr << "init "<< Threads::number <<" threads: " << fftw_init_threads() << "\n"; called=true;};
fftw_plan_with_nthreads(Threads::number/*16*/);//omp_get_max_threads());

			//(*this)=inp; // FIXME - jakoś inaczej
			this->resize(inp.dim()); // FIXME - jakoś inaczej
			#ifdef YADE_FFTW3
			fftw_complex *in, *out;
			in  = reinterpret_cast<fftw_complex*>(&(const_cast<NDimTable&>(inp). operator[](0)));//(fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
			out = reinterpret_cast<fftw_complex*>(&(                       this->operator[](0)));//(fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
			std::vector<int> dim_int(inp.dim().begin(),inp.dim().end());
// FIXME - fftw_plan_dft is not re-entrant. Must have mutex here. But (FIXME!!!!) better not create & destroy fftw_plan all the time!!
// w SchrodingerKosloffPropagator zrobię jakieś FFT.makePlan() a potem pilnuję żeby pętla była ciągle na tych samych NDimTable....
			fftw_plan p_FFT=fftw_plan_dft((int)rank_d,&dim_int[0], in, out, forward ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);
			//dirty=false;
			fftw_execute(p_FFT);
// FIXME - do not destroy & create plan all the time. Just make one and keep it!! Unless dimensions changed - by calling resize().
			if(not forward) this->operator/=((int)(inp.total));
			fftw_destroy_plan(p_FFT);
			#else
			#error fftw3 library is needed
			#endif
		}};
		void FFT()                             { this->becomesFFT(*this);  };
		void niceFFT() {
			this->shiftByHalf();
			this->becomesFFT(*this);
			this->operator/=( std::sqrt(1.0*total) );
			this->shiftByHalf();
		};
		void becomesFFT(const NDimTable& inp)  { doFFT(inp,true); };
		void IFFT()                            { this->becomesIFFT(*this); };
		void niceIFFT() {
			this->shiftByHalf();
			this->becomesIFFT(*this);
			this->operator*=( std::sqrt(1.0*total) );
			this->shiftByHalf();
		};
		void becomesIFFT(const NDimTable& inp) { doFFT(inp,false); };
};

template<typename K> NDimTable<K>  FFT(const NDimTable<K>& inp){ NDimTable<K> ret={}; ret.becomesFFT (inp); return std::move(ret); };
template<typename K> NDimTable<K> IFFT(const NDimTable<K>& inp){ NDimTable<K> ret={}; ret.becomesIFFT(inp); return std::move(ret); };

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
template<typename K, typename L> NDimTable<K> operator+(const NDimTable<K>& a,const L    & k){	NDimTable<K> r( a);	return std::move(r+=k); };
template<typename K, typename L> NDimTable<K> operator-(const NDimTable<K>& a,const L    & k){	NDimTable<K> r( a);	return std::move(r-=k); };
template<typename K, typename L> NDimTable<K> operator*(const NDimTable<K>& a,const L    & k){	NDimTable<K> r( a);	return std::move(r*=k); };
template<typename K, typename L> NDimTable<K> operator/(const NDimTable<K>& a,const L    & k){	NDimTable<K> r( a);	return std::move(r/=k); };

