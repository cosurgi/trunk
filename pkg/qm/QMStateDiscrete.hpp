// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMState.hpp"
#include "QMStateAnalytic.hpp"
#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>
#include <time.h>

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
		typedef std::vector<std::vector<std::vector<Complexr> > > Complexr3D;
		typedef std::vector<std::vector<     Vector3r         > > Vector3r3D;
		virtual Complexr getValPos(Vector3r xyz);                                   /// return complex quantum aplitude at given positional representation coordinates
		virtual Real     getStepPos(){ return positionSize[0 /*FIXME*/]/gridSize;}; /// return grid step, two point distance in the mesh in positional representation
		virtual ~QMStateDiscrete();
		void postLoad(QMStateDiscrete&);
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
			((bool      ,firstRun,true,Attr::readonly,"It is used to mark that postLoad() already generated the wavefunction from its creator analytic function."))
			((boost::shared_ptr<QMStateAnalytic>,creator,,Attr::triggerPostLoad,"Analytic wavepacket used to create the discretized version for calculations. The analytic shape can be anything: square packets, triangle, Gaussian - as long as it is normalized. After it is used the boost::shared_ptr is deleted."))
			((int       ,gridSize,4096,,"Lattice grid size used to describe the wave function. For FFT purposes that should be a power of 2."))
			((Vector3r  ,positionSize,Vector3r::Zero(),,"Wavepacket size in position representation space."))

			//This is just Serialization test, FIXME: add this to self-tests, like `yade -test` or `yade -check`
			//((std::vector< Real >,arealTable,,,,"The FFT lattice grid "))
			//((std::vector< std::vector< Real > >,table,,,,"The FFT lattice grid "))
			//((std::complex<Real>,complexNum,,,,"test complex "))

			, // constructor
			createIndex();
			, // python bindings
			.def("deltaX"   ,&QMStateDiscrete::deltaX   ,"Get $\\Delta x$ - the position representation distance between two grid nodes.")
			.def("lambdaMin",&QMStateDiscrete::lambdaMin,"Get minimum wavelength that this FFT grid can handle.")
			.def("kMax"     ,&QMStateDiscrete::kMax     ,"Get maximum wavenumber that this FFT grid can handle.")
			.def("kMin"     ,&QMStateDiscrete::kMin     ,"Get minimum wavenumber that this FFT grid can handle.")
			.def("iToX"     ,&QMStateDiscrete::iToX     ,"Get $x$ coordinate for i-th node, invoke example: iToX(0)")
			.def("iToK"     ,&QMStateDiscrete::iToK     ,"Get $k$ coordinate for i-th node, invoke example: iToK(0)")
			.def("xToI"     ,&QMStateDiscrete::xToI     ,"Get node number for $x$ coordinate, invoke example: xToI(0)")
			.def("kToI"     ,&QMStateDiscrete::kToI     ,"Get node number for $k$ coordinate, invoke example: kToI(0)")
		);
		REGISTER_CLASS_INDEX(QMStateDiscrete,QMState);

		// Find min/max wavelength and wavenumber for this FFT grid
		Real deltaX(     /* Real startX ,Real endX ,Real gridSize */){return (endX-startX)/gridSize;};
		Real lambdaMin(  /* Real startX ,Real endX ,Real gridSize */){return 2*deltaX(/*startX, endX, gridSize*/);};
		Real kMax(       /* Real startX ,Real endX ,Real gridSize */){return Mathr::TWO_PI/lambdaMin(/*startX, endX, gridSize*/);};
		Real kMin(       /* Real startX ,Real endX ,Real gridSize */){return -kMax(/*startX, endX, gridSize*/);};
		// Those functions convert index 'i' to respective position or momentum on the FFT grid
		Real iToX(Real i /*,Real startX ,Real endX ,Real gridSize */){return (i*endX                        +(gridSize-i)*startX                      )/gridSize; };
		Real iToK(Real i /*,Real startX ,Real endX ,Real gridSize */){return (i*kMax(/*startX,endX,gridSize*/)+(gridSize-i)*kMin(/*startX,endX,gridSize*/))/gridSize; };
		int  xToI(Real x /*,Real startX ,Real endX ,Real gridSize */){return (gridSize*(x-startX                      ))/(endX                        -startX                      ); };
		int  kToI(Real k /*,Real startX ,Real endX ,Real gridSize */){return (gridSize*(k-kMin(/*startX,endX,gridSize*/)))/(kMax(/*startX,endX,gridSize*/)-kMin(/*startX,endX,gridSize*/)); };

		Complexr3D tableValuesPosition ; //,,,,"The FFT lattice grid: wavefunction values in position representation"
		//Complexr3D /*Vector3r3D*/ tablePosition       ; //,,,,"The FFT lattice grid: position coordinates corresponding to table cells"))
		Complexr3D tableValueWavenumber; //,,,,"The FFT lattice grid: wavefunction values in wavenumber representation "))
		//Complexr3D /*Vector3r3D*/ tableWavenumber     ; //,,,,"The FFT lattice grid: wavenumber coordinates corresponding to table cells"))
	private:
		Real startX,startY,startZ,endX,endY,endZ,stepPos;
	
		// FIXME - put this in some other nice place.
		Real lastError;
		Real getClock(){ timeval tp; gettimeofday(&tp,NULL); return tp.tv_sec+tp.tv_usec/1e6; }
		bool errorAllowed(){if ( (getClock() - lastError)>2) {lastError=getClock(); return true;} else return false; }; // report error no more frequent than once per 2 seconds.
		
};
REGISTER_SERIALIZABLE(QMStateDiscrete);

