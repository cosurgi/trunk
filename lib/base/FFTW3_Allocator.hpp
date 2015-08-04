// 2015 © Janek Kozicki <cosurgi@gmail.com>

// define custom allocator for use with fftw3, suppoorted types are:
//
//    float, double, long double, float128
//
// bibliography and source:
//    http://www.codeproject.com/Articles/4795/C-Standard-Allocator-An-Introduction-and-Implement
//    git clone https://gitorious.org/cpp-bricks/fftw.git
//    http://www.fftw.org/fftw3_doc/

#pragma once

#ifdef YADE_FFTW3

#ifdef DEBUG_NDIM_RAM
	#include <boost/multiprecision/cpp_int.hpp>
	using namespace boost::multiprecision;
	#include <map>
	//struct cmpr {
	//  bool operator()(void* a, void* b) { return (intptr_t)a < (intptr_t)b; };
	//}
	template<typename zzz>
	struct Zcc{
	//static cpp_int NDimTable_SIZE_TOTAL;
	static cpp_int NDimTable_Instances;
	static cpp_int NDimTable_Alloc;
	static std::map<void*,cpp_int /*,cmpr*/> NDimTable_Allocated;
	};
	using ZZ = Zcc<int>;
#endif
template<typename Type>
struct Thr{
//static cpp_int NDimTable_SIZE_TOTAL;
static int number;
};
using Threads = Thr<int>;

#include <memory>
#include <fftw3.h>
#include <limits>
#include <iostream>
#include <complex>

template <typename T>
class FFTW3_Allocator
{
	public:
		typedef T                  value_type;
		typedef value_type*        pointer;
		typedef const value_type*  const_pointer;
		typedef value_type&        reference;
		typedef const value_type&  const_reference;
		typedef std::size_t        size_type;
		typedef std::ptrdiff_t     difference_type;	

		//    convert an FFTW3_Allocator<T> to FFTW3_Allocator<U>
		template<typename U>
		struct rebind {
			typedef FFTW3_Allocator<U> other;
		};

		/* FIXME: why "explicit" was here?
		 * FIXME - it was to prevent copy-initialisation http://en.cppreference.com/w/cpp/language/explicit */
		inline /*explicit*/ FFTW3_Allocator()  {};
		inline             ~FFTW3_Allocator()  {};
		inline /*explicit*/ FFTW3_Allocator(const FFTW3_Allocator& )   {};
		template<typename U>
		inline /*explicit*/ FFTW3_Allocator(const FFTW3_Allocator<U>&) {};

		//    address
		inline pointer       address(reference r      ) { return &r; } //.. FIXME: is it necessary?
		inline const_pointer address(const_reference r) { return &r; } //.. FIXME: is it necessary?

		//    memory allocation
		inline pointer allocate(size_type n, typename std::allocator<void>::const_pointer = 0 /* FIXME: or maybe use this?  const void* = 0 */ ) {
			return static_cast<pointer>(FFTW3_Memory::malloc(n * sizeof(value_type)));
		}
		inline void deallocate(pointer p, size_type) {
			FFTW3_Memory::free(p);
		}
		//    size
		inline size_type max_size() const {
			return std::numeric_limits<size_type>::max() / sizeof(value_type);
		};
		//    construction/destruction
		inline void construct(pointer p)                      { ::new((void *)p) value_type()   ; };
		inline void construct(pointer p, const_reference val) { ::new((void *)p) value_type(val); };
		inline void destroy  (pointer p)                      { p->~value_type(); }

		// comparison
		inline bool operator==(FFTW3_Allocator const&  ) { return true;           }  //.. FIXME: is it necessary? (and correct?)
		inline bool operator!=(FFTW3_Allocator const& a) { return !operator==(a); }  //.. FIXME: is it necessary? (and correct?)

		// Low-level wrapper for the C API of FFTW3
		struct FFTW3_Memory;
};

///// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for int.
//template <>
//struct FFTW3_Allocator<int>::FFTW3_Memory
//{
//	static void* malloc(size_t n) {
//		#warning "FFTW will not work for int type"
//		return malloc(n *sizeof(int) );
//	}
//
//	static void free(void *p) {
//		free(p);
//	}
//};


/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for float precision.
template <>
struct FFTW3_Allocator<float>::FFTW3_Memory
{
	static void* malloc(size_t n) { 
		std::cout << "----- fftwd_malloc("<<n<<") <-- float    ?????????????????????????????\n";
		#ifdef DEBUG_NDIMTABLE
		std::cout << "----- fftwf_malloc("<<n<<")\n";
		#endif
		return fftwf_malloc(n);
	}

	static void free(void *p) {
		std::cout << "----- fftwd_free  (       ) <-- float    ?????????????????????????????\n";
		#ifdef DEBUG_NDIMTABLE
		std::cout << "----- fftwf_FREE\n";
		#endif
		fftwf_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for float precision.
template <>
struct FFTW3_Allocator<std::complex<float> >::FFTW3_Memory
{
	static void* malloc(size_t n) { 
		std::cout << "----- fftwd_malloc("<<n<<") <-- complex<float>    ?????????????????????????????\n";
		#ifdef DEBUG_NDIMTABLE
		std::cout << "----- fftwf_malloc("<<n<<")\n";
		#endif
		return fftwf_malloc(n);
	}

	static void free(void *p) {
		std::cout << "----- fftwd_free  (       ) <-- complex<float>    ?????????????????????????????\n";
		#ifdef DEBUG_NDIMTABLE
		std::cout << "----- fftwf_FREE\n";
		#endif
		fftwf_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for double precision.
template <>
struct FFTW3_Allocator<double>::FFTW3_Memory
{
	static void* malloc(size_t n) {
#ifdef DEBUG_NDIM_RAM
		std::cout << "----- fftwd_malloc("<<n<<") <-- double    ?????????????????????????????\n";
								// "??" ponieważ powinno być używane tylko complex<double>
#endif
		return fftw_malloc(n);
	}

	static void free(void *p) {
#ifdef DEBUG_NDIM_RAM
		std::cout << "----- fftwd_free  (       ) <-- double    ?????????????????????????????\n";
#endif
		fftw_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for double precision.
template <>
struct FFTW3_Allocator<std::complex<double> >::FFTW3_Memory
{
	static void* malloc(size_t n) {
#ifdef DEBUG_NDIM_RAM
std::cout << "----- fftwd_malloc("<<n<<") <-- complex<double>  :" << ZZ::NDimTable_Alloc/(1024*1024)<< " MB --> ";
ZZ::NDimTable_Alloc += n; // + sizeof(&this);
		void* ret = fftw_malloc(n);
ZZ::NDimTable_Allocated[ret]=n;
std::cerr<<ZZ::NDimTable_Alloc <<" bytes == "<< ZZ::NDimTable_Alloc/(1024*1024)<<" MB  (this one: "<< ZZ::NDimTable_Allocated[ret]/(1024*1024) << " MB)\n";
		return ret;
#else
		return fftw_malloc(n);
#endif
	}

	static void free(void *p) {
#ifdef DEBUG_NDIM_RAM
std::cout << "----- fftwd_free  (       ) <-- complex<double>  :" << ZZ::NDimTable_Alloc/(1024*1024)<< " MB --> ";
cpp_int ile_zwalniam = ZZ::NDimTable_Allocated[p];
cpp_int ile_bylo     = ZZ::NDimTable_Alloc;
ZZ::NDimTable_Alloc -= ile_zwalniam; // + sizeof(&this);
std::cerr<<ZZ::NDimTable_Alloc <<" bytes == "<< ZZ::NDimTable_Alloc/(1024*1024)<<" MB (still used: "
  << (ile_bylo - ZZ::NDimTable_Allocated[p])/(1024*1024) << " MB)\n";
ZZ::NDimTable_Allocated.erase(p);
#endif
		fftw_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for long double precision.
template <>
struct FFTW3_Allocator<long double>::FFTW3_Memory
{
	static void* malloc(size_t n) {
		std::cout << "----- fftwd_malloc("<<n<<") <-- long double    ?????????????????????????????\n";
		return fftwl_malloc(n);
	}

	static void free(void *p) {
		std::cout << "----- fftwd_free  (       ) <-- long double    ?????????????????????????????\n";
		fftwl_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for long double precision.
template <>
struct FFTW3_Allocator<std::complex<long double> >::FFTW3_Memory
{
	static void* malloc(size_t n) {
		std::cout << "----- fftwd_malloc("<<n<<") <-- complex<long double>    ?????????????????????????????\n";
		return fftwl_malloc(n);
	}

	static void free(void *p) {
		std::cout << "----- fftwd_free  (       ) <-- complex<long double>    ?????????????????????????????\n";
		fftwl_free(p);
	}
};

#ifdef FLOAT128_PRECISION
#include <boost/multiprecision/float128.hpp> 
/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for float128 precision.
template <>
struct FFTW3_Allocator<float128>::FFTW3_Memory
{
	static void* malloc(size_t n) {
		std::cout << "----- fftwd_malloc("<<n<<") <-- float128    ?????????????????????????????\n";
		return fftwq_malloc(n);
	}

	static void free(void *p) {
		std::cout << "----- fftwd_free  (       ) <-- float128    ?????????????????????????????\n";
		fftwq_free(p);
	}
};
/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for float128 precision.
template <>
struct FFTW3_Allocator<std::complex<float128> >::FFTW3_Memory
{
	static void* malloc(size_t n) {
		std::cout << "----- fftwd_malloc("<<n<<") <-- complex<float128>    ?????????????????????????????\n";
		return fftwq_malloc(n);
	}

	static void free(void *p) {
		std::cout << "----- fftwd_free  (       ) <-- complex<float128>    ?????????????????????????????\n";
		fftwq_free(p);
	}
};
#endif

#endif


