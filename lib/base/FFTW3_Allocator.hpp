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

#include <memory>
#include <fftw3.h>
#include <limits>

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

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for float precision.
template <>
struct FFTW3_Allocator<float>::FFTW3_Memory
{
	static void* malloc(size_t n) { 
		std::cout << "----- fftwf_malloc("<<n<<")\n";
		return fftwf_malloc(n);
	}

	static void free(void *p) {
		std::cout << "----- fftwf_FREE\n";
		fftwf_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for float precision.
template <>
struct FFTW3_Allocator<std::complex<float> >::FFTW3_Memory
{
	static void* malloc(size_t n) { 
		std::cout << "----- fftwf_malloc("<<n<<")\n";
		return fftwf_malloc(n);
	}

	static void free(void *p) {
		std::cout << "----- fftwf_FREE\n";
		fftwf_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for double precision.
template <>
struct FFTW3_Allocator<double>::FFTW3_Memory
{
	static void* malloc(size_t n) {
		return fftw_malloc(n);
	}

	static void free(void *p) {
		fftw_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for double precision.
template <>
struct FFTW3_Allocator<std::complex<double> >::FFTW3_Memory
{
	static void* malloc(size_t n) {
		return fftw_malloc(n);
	}

	static void free(void *p) {
		fftw_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for long double precision.
template <>
struct FFTW3_Allocator<long double>::FFTW3_Memory
{
	static void* malloc(size_t n) {
		return fftwl_malloc(n);
	}

	static void free(void *p) {
		fftwl_free(p);
	}
};

/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for long double precision.
template <>
struct FFTW3_Allocator<std::complex<long double> >::FFTW3_Memory
{
	static void* malloc(size_t n) {
		return fftwl_malloc(n);
	}

	static void free(void *p) {
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
		return fftwq_malloc(n);
	}

	static void free(void *p) {
		fftwq_free(p);
	}
};
/// Specialization of struct FFTW3_Allocator<>::FFTW3_Memory for float128 precision.
template <>
struct FFTW3_Allocator<std::complex<float128> >::FFTW3_Memory
{
	static void* malloc(size_t n) {
		return fftwq_malloc(n);
	}

	static void free(void *p) {
		fftwq_free(p);
	}
};
#endif

#endif


