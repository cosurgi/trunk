// 2015 © Janek Kozicki <cosurgi@gmail.com>

// An N dimensional linear (vector) space over scalar K.
// This class allows storage of arbitrary dimensional arrays, or tables. It's primary purpose is for wavefunctions in quantum mechanics.
// It is dedicated to use with libfftw3 library, but will work (slower) without it.
// Underlying storage is a 1D std::vector<K,A> with custom fftw3 allocator A, information about dimensions is stored in another std::vector<int>
//   - it has operators for component-wise: + - * /
//   - as well as for     scalar arguments: + - * /
// I plan to use it with K=Real, then implement a second one for K=complex, where fftw3 split complex numbers will be used, based on K=Real.

#pragma once

#include <vector>
#include <lib/base/Math.hpp>

class NDimensionalLinearSpace {
};

