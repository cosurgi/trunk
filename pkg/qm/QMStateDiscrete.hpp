// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>
#include <lib/time/TimeLimit.hpp>
#include <lib/base/NDimTable.hpp>
#include "QMState.hpp"

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
 *  Member functions:
 *    vector<Real>    getSpatialSize()              ← spatial size in all dimensions, it is synchronized with body->shape->extents by St1_QMStateDiscrete
 *    void            setSpatialSize()              ← spatial size in all dimensions
 *
 *  Member variables:
 *
 *    vector<size_t>  gridSize                      ← grid size in all dimensions
 *
 *                       // FIXME  ↓  I think I can move it up from QMStateDiscreteGlobal to QMStateDiscrete
 *    NDimTable<Complexr> psiGlobal->psiGlobalTable ← the wavefunction, in discrete positional representation
 *    size_t              whichPartOfpsiGlobal      ← For entangled wavefunctions it says where this wavefunction starts in the entangled tensor
 *
 */
class QMStateDiscreteGlobal;  // FIXME !!!!!!!! FIXME !!!!!!!!!! - on sam powinien być swoim psiGlobal, a nie tu go trzymać....
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
/* FIXME - dubluje się z NDimTable::dim_n */			((vector<size_t>,gridSize , vector<size_t>({}), Attr::readonly ,"Lattice grid size used to describe the wave function. For FFT purposes that should be a power of 2."))
////////////////////////////// serializacja.......................
		((boost::shared_ptr<QMStateDiscreteGlobal>,psiGlobal           ,,Attr::hidden," // FIXME - on sam powinien być swoim psiGlobal, a nie tu go trzymać.... "))
		((vector<Real>                            ,spatialSize         ,,Attr::readonly," // cannot be public, because it's public in Box←QMGeometry::extents, it is synchronized by St1_QMStateDiscrete "))
		((int                                     ,whichPartOfpsiGlobal,,Attr::readonly," // For entangled wavefunctions it says where this wavefunction starts in the entangled tensor "))
			, // constructor
			createIndex();
			whichPartOfpsiGlobal=-1;
			spatialSize={};
			//nDimTable_PSI=boost::shared_ptr<NDimTable>(new NDimTable);
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
			.def("isEntangled",&QMStateDiscrete::isEntangled,"Tell if it's entangled")
			.def("partOfpsiGlobal",&QMStateDiscrete::partOfpsiGlobal,"If it's entangled, tell where it starts.")
			.def("atPsiGlobal",&QMStateDiscrete::atPsiGlobalExisting,"Return value at given table (grid) position.")
			.def("integratePsiGlobal",&QMStateDiscrete::integratePsiGlobal,"Return total probability.")
// FIXME - chciałem żeby at() odnosiło się do tej klasy, a nie globalExisting, a to po to, żebym mógł brać gridSize
//			.def("getPsiGlobalExisting",&QMStateDiscrete::getPsiGlobalExisting,"Return QMStateDiscreteGlobal.")
		);
		REGISTER_CLASS_INDEX(QMStateDiscrete,QMState);

		// Find min/max wavelength and wavenumber for this FFT grid
		/// return grid step, two point distance in the mesh in positional representation
		virtual Real     stepInPositionalRepresentation(int dim){ return deltaX(dim); /*return size.at(d) / gridSize[d];*/};
		Real deltaX   (int d)                                   { return spatialSize[d]/gridSize[d];};
		Real lambdaMin(int d){return 2*deltaX(d);};
		Real kMax     (int d){return Mathr::TWO_PI/lambdaMin(d);};
		Real kMin     (int d){return -kMax(d);};

		// Those functions convert index 'i' to respective position or momentum on the FFT grid
		Real   iToX     (Real i, int d){return (i*(      end(d)      )    +(gridSize[d]-i)* (       start(d)    )  )/gridSize[d]; };
		Real   iToK     (Real i, int d){return (i*kMax(d )+(gridSize[d]-i)*kMin(d ))/gridSize[d]; };
		size_t xToI     (Real x, int d){return (gridSize[d]*(x-(     start(d)      )  ))/(    spatialSize[d]    ); };
		Real   xToI_Real(Real x, int d){return (gridSize[d]*(x-(     start(d)      )  ))/(    spatialSize[d]    ); };
		size_t kToI     (Real k, int d){return (gridSize[d]*(k-kMin(d )))/(kMax(d)-kMin(d)); };
// iToX_local,  xToI_local
// iToX_global, xToI_global
// start_local, end_local
// start_global, end_global     ← oczywiście chodzi o globalny układ współrzędnych, nie o globalny pakiet psi


		Real start(int d) { return (-spatialSize[d]*0.5 /*    FIXME????? co z tym położeniem?????   +pos[d]  */);};
		Real end  (int d) { return ( spatialSize[d]*0.5 /*    FIXME????? co z tym położeniem?????   +pos[d]  */);};
                                                         //         ↑ okazuje się, że mi rysuje w złym miejscu potencjały, i nie dziwne, bo Gl1_QMGeometry
							 // bierze start() i end(). I tam rysuje. Podczas gdy Functor jest wywoływany już przesunięty
							 // i obrócony żeby rysować obiekt w lokalnym układzie współrzędnych
							 // Ale to nie koniec: ja chciałem przesuwać siatki względem siebie, żeby się nie 
							 // nakładały ładunki i żeby nie było dzielenia przez zero!!!
							 // FIXME, FIXME, FIXME,  FIXME,  FIXME,  FIXME,  FIXME,  FIXME,  FIXME
							 //
                                                         // this FIXME is copied from Gl1_QMGeometry, I need to think about this.
                                                         //                                                                  1   2   3   4   5   
                                                         // FIXME? or not? problem is that N-nodes have (N-1) lines between: |---|---|---|---|---
                                                         //                                                                    1   2   3   4   5  ↓↓↓↓↓
                                                         // maybe change the start() and end() values in QMStateDiscrete ???


		size_t              dim()                                { return gridSize.size();};
		const vector<Real>& getSpatialSize()  const              { return spatialSize;    };
		void                setSpatialSize(const vector<Real> s) { spatialSize=s;         };

		void   setEntangled(size_t i) { whichPartOfpsiGlobal=i; };
		bool   isEntangled()          { return whichPartOfpsiGlobal >= 0; };
		size_t partOfpsiGlobal()      { if(whichPartOfpsiGlobal<0) throw std::runtime_error("\n\nQMStateDiscrete::partOfpsiGlobal() ERROR\n\n"); return size_t(whichPartOfpsiGlobal); };
		size_t partOfpsiGlobalZero()  { if(whichPartOfpsiGlobal<0) return 0; return size_t(whichPartOfpsiGlobal); };

		Real integratePsiGlobal();
// this is the real wavefunction - entangled, and s̳h̳a̳r̳e̳d̳ among other particles.
		bool                                      getPsiGlobalExists  () { return (bool)(psiGlobal);};
		boost::shared_ptr<QMStateDiscreteGlobal>& setPsiGlobal        (boost::shared_ptr<QMStateDiscreteGlobal>& s) { psiGlobal = s; return psiGlobal;};
		boost::shared_ptr<QMStateDiscreteGlobal>& getPsiGlobalNew     ();
		boost::shared_ptr<QMStateDiscreteGlobal>& getPsiGlobalExisting() { if(not psiGlobal) throw std::runtime_error("\n\nERROR: QMStateDiscrete::psiGlobal wanted, but doesn't exist\n\n"); return psiGlobal; };
		Complexr                                  atPsiGlobalExisting(std::vector<size_t> pos);

//ser// ← oznacza, że przenisłem te zmienne to serializacji powyżej w REGISTER_....
	private:
//ser//		boost::shared_ptr<QMStateDiscreteGlobal> psiGlobal; // FIXME - on sam powinien być swoim psiGlobal, a nie tu go trzymać....
		//boost::shared_ptr<NDimTable<Complexr>> nDimTable_PSI; // FIXING.... - to zamiast psiGlobal ??


		// when wavefunction gets entangled it gets a link (shared pointer) to the global wavefunction that involves all particles in question.
//,,,,"The FFT lattice grid: wavefunction values in position representation"
		// dim*whichPartOfpsiGlobal, dim*whichPartOfpsiGlobal+1, etc...  are the idx in size and gridSize

	protected:
		// Wavepacket size in position representation space, for each DOF.
		// In QMStateDiscreteGlobal can be higher than 4D due to tensor products between wavefunctions.
		// in QMStateDiscrete max size is 3D
//ser//		vector<Real>  spatialSize;          // cannot be public, because it's public in Box←QMGeometry::extents, it is synchronized by St1_QMStateDiscrete
	private:
//ser//		int           whichPartOfpsiGlobal; // For entangled wavefunctions it says where this wavefunction starts in the entangled tensor
		TimeLimit     timeLimit;
		
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
	private:
		virtual void calculateTableValuesPosition(const shared_ptr<QMParameters>& par, QMState*);
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms)
		{ throw std::logic_error("\n\nSt1_QMStateDiscrete was called directly.\n\n");};
};
REGISTER_SERIALIZABLE(St1_QMStateDiscrete);

