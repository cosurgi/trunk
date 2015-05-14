// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>
#include <lib/time/TimeLimit.hpp>
#include <lib/base/NDimTable.hpp>
#include "QMState.hpp"
#include "QMStateAnalytic.hpp"

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E   in   D I S C R E T E   form
*
*********************************************************************************/

/*! @brief QMStateDiscrete contains state information about each particle.
 *
 * The information is expressed using discrete numerical representation using NDimTable class.
 * A spatial position representation is used. On this grid the complex amplitude is stored, which also determines
 * the probability distribution. Grid size usually is in powers of two, because that's most useful for FFT.
 *
 */
class QMStateDiscrete: public QMState
{
	public:
		virtual ~QMStateDiscrete();
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
			((boost::shared_ptr<QMStateAnalytic>,creator,,Attr::hidden,"Analytic wavepacket used to create the discretized version for calculations. The analytic shape can be anything: square packets, triangle, Gaussian - as long as it is normalized."))
			((vector<size_t>,gridSize,vector<size_t>({}),,"Lattice grid size used to describe the wave function. For FFT purposes that should be a power of 2."))
			((vector<Real>,size,vector<Real>({}),,"Wavepacket size in position representation space, for each DOF. Can be higher than 4D due to tensor products between wavefunctions."))
			, // constructor
			createIndex();
			, // python bindings
			.def("deltaX"   ,&QMStateDiscrete::deltaX   ,"Get $\\Delta x$ - the position representation distance between two grid nodes. Same as stepInPositionalRepresentation().")
			.def("stepInPositionalRepresentation"   ,&QMStateDiscrete::stepInPositionalRepresentation   ,"Get $\\Delta x$ - the position representation distance between two grid nodes. Same as deltaX().")
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
		/// return grid step, two point distance in the mesh in positional representation
		virtual Real     stepInPositionalRepresentation(int dim){ return deltaX(dim); /*return size.at(d) / gridSize[d];*/};
		Real deltaX   (int d)                                   { return size[d]/gridSize[d];};
		Real lambdaMin(int d){return 2*deltaX(d);};
		Real kMax     (int d){return Mathr::TWO_PI/lambdaMin(d);};
		Real kMin     (int d){return -kMax(d);};

		// Those functions convert index 'i' to respective position or momentum on the FFT grid
		Real iToX(Real i, int d){return (i*(      end(d)      )    +(gridSize[d]-i)* (       start(d)    )  )/gridSize[d]; };
		Real iToK(Real i, int d){return (i*kMax(d )+(gridSize[d]-i)*kMin(d ))/gridSize[d]; };
		size_t xToI(Real x, int d){return (gridSize[d]*(x-(     start(d)      )  ))/(    size[d]    ); };
		size_t kToI(Real k, int d){return (gridSize[d]*(k-kMin(d )))/(kMax(d)-kMin(d)); };

		Real start(int d) { return (-size[d]*0.5+pos[d]);};
		Real end  (int d) { return ( size[d]*0.5+pos[d]);};

/* FIXME - haaaaa! chyba będę mogł to wywalić, gdy mapiszę St1_QMStateAnalytic !! */
		void calculateTableValuesPosition(St1_QMStateAnalytic* localCreator, const QMParameters* par, const QMStateAnalytic*);
		NDimTable<Complexr> tableValuesPosition; //,,,,"The FFT lattice grid: wavefunction values in position representation"

	private:
		TimeLimit timeLimit;
		
};
REGISTER_SERIALIZABLE(QMStateDiscrete);

class St1_QMStateDiscrete: public St1_QMState
{
	public:
		virtual void go(const shared_ptr<State>&, const shared_ptr<Material>&, const Body*);
		FUNCTOR1D(QMStateDiscrete);
		YADE_CLASS_BASE_DOC(St1_QMStateDiscrete/* class name */, St1_QMState /* base class */
			, "Functor creating :yref:`QMStateDiscrete` from :yref:`QMParametersDiscrete`." // class description
		);
/*FIXME, make it:	private: */
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms)
		{ throw std::logic_error("St1_QMStateDiscrete was called directly.");};
};
REGISTER_SERIALIZABLE(St1_QMStateDiscrete);

