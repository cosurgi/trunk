// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMState.hpp"
#include "QMStateAnalytic.hpp"
#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/GlobalEngine.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E   in   D I S C R E T E   form
*
*********************************************************************************/

/*! @brief QMStateDiscrete contains state information about each particle.
 *
 * The information is expressed using discrete numerical representations. Most likely an N-dimensional gird.
 * A spatial position representation is used. On this grid the complex amplitude is stored, which defines
 * the probability distribution. Grid size usually is in powers of two, because that's most useful for FFT.
 *
 */
class QMStateDiscrete: public QMState
{
	public:
		virtual ~QMStateDiscrete();
		void postLoad(QMStateDiscrete&)
		{ 
			std::cerr<<"\nQMStateDiscrete postLoad\n";
//			std::cerr<<"firstRun="<<firstRun<<"\n";
//			std::cerr<<"size="<<size<<"\n";
		}
		// FIXME: the lattice grid here vector<........>
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMStateDiscrete
			, // base class
			QMState
			, // class description
"Quantum mechanical state in a discrete representation. It can be initialized from anylytical representations \
or directly by filling in the discrete values in the table. It is used for numerical computations."
			, // attributes, public variables
			((bool,firstRun,true,,"It is used to mark that postLoad() already generated the wavefunction from its creator analytic function."))
			((boost::shared_ptr<QMStateAnalytic>,creator,,Attr::triggerPostLoad,"Analytic wavepacket used to create the discretized version for calculations. The analytic shape can be anything: square packets, triangle, Gaussian - as long as it is normalized. After it is used the boost::shared_ptr is deleted."))

//			((int,size,4096,,"Lattice size used to describe the wave function. For FFT purposes that should be a power of 2."))
//			((int,numSpatialDimensions,1,,"Number of spatial dimensions in which wavefunction exists"))
//			((std::vector<std::complex<Real> >,table,,,,"The FFT lattice grid "))
			//This is just Serialization test, FIXME: add this to self-tests
			//((std::vector< Real >,arealTable,,,,"The FFT lattice grid "))
			//((std::vector< std::vector< Real > >,table,,,,"The FFT lattice grid "))
			//((std::complex<Real>,complexNum,,,,"test complex "))

			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMStateDiscrete,QMState);
};
REGISTER_SERIALIZABLE(QMStateDiscrete);

