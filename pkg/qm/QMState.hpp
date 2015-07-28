// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>
#include "QMParameters.hpp"

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E   F U N C T O R
*
*********************************************************************************/
#include <boost/enable_shared_from_this.hpp>
class QMState;
class Law2_QMIGeom_QMIPhys_GlobalWavefunction;
class St1_QMState: public StateFunctor, public boost::enable_shared_from_this<St1_QMState>
{
	public:
		virtual void go(const shared_ptr<State>&, const shared_ptr<Material>&, const Body*);
		FUNCTOR1D(QMState);
		YADE_CLASS_BASE_DOC(St1_QMState/* class name */, StateFunctor /* base class */
			, "Functor creating :yref:`QMState` from :yref:`QMParameters`." // class description
		);
	private:
		friend class QMState;
		virtual void calculateTableValuesPosition(const shared_ptr<QMParameters>& par, QMState*) {throw;};
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms)
		{ throw std::runtime_error("\n\nSt1_QMState::getValPos was called directly.\n\n");};
};
REGISTER_SERIALIZABLE(St1_QMState);

// FIXME: grep: ↓ patrz też uwagi w QMState.hpp
//
//                   QMState                                              QMState, owns QMStateDiscreteGlobal (rename na psiNDimTable ?)
//                      ↑                                                    ↑     class psiNDimTable: public NDimTable, i już nie mam duplikowania gridSize!!!
//               QMStateDiscrete, owns QMStateDiscreteGlobal                 ↑         tylko wtedy co z 3D `gridSize.size()==3` oryginalnych pakietów?
//                      ↑                         ----------→ FIXME:         ↑         chyba OK, tylko będę musiał przerobić Gl1_QMGeometry::go linijka if(dimSpatial == 0 or dimSpatial>3 ) żeby uwzględnić od początku wyznaczanie kroków przestrzennych prawidłowo względem tego co jest całkowane a co nie, żeby to prawidłowo rysować.
//               QMStateAnalytic                                      QMStateAnalytic
//                  ↑        ↑                                           ↑        ↑
// QMPacketGaussianWave  QMPacketHarmonicEigenFunc       QMPacketGaussianWave  QMPacketHarmonicEigenFunc
//

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/

/*! @brief QMState contains quantum state information
 *
 *  Member variables:
 *
 *    bool wasGenerated                             ← marks whether the discrete grid was generated from analytical formula
 */
class QMState: public State
{
	public:
// FIXME? mogę w końcu to skasować?		virtual Complexr getValPos(Vector3r xyz, const QMParameters* par){throw;};    /// return complex quantum aplitude at given positional representation coordinates
//		virtual Complexr getValInv(Vector3r xyz){};          /// return complex quantum aplitude at given wavenumber representation coordinates
//		virtual Complexr getValIJKPos(Vector3i ijk){throw;}; /// return complex quantum aplitude at given positional grid coordinates
//		virtual Complexr getValIJKInv(Vector3i ijk){};       /// return complex quantum aplitude at given wavenumber grid coordinates
//		virtual int      getDim(){ return dim;};             /// return number of dimensions
//		virtual Vector3r getExtentsIJK(){throw;};            /// return grid size

//////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// FIXME - only this one has a nice name, and is actually implemented, fix the others.
/* FIXME - skasować? Przerobić? */		virtual Real     stepInPositionalRepresentation(){throw;}; /// return grid step, two point distance in the mesh in positional representation
//////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!


//		virtual Real     getStepInv(){};                     /// return grid step, two point distance in the mesh in wavenumber representation
//		virtual Vector3r getHalfSizeInv(){};                 /// return size in wavenumber representation

//		virtual Complexr calcScalarProduct(shared_ptr<QMState>& other); // calculate <φ|ψ>

		virtual ~QMState();
		bool isAnalytic () const { return not numericalState; };
		bool isNumeric  () const { return numericalState; };
		void setAnalytic()       { numericalState = false; blockedDOFs=State::DOF_ALL ; };
		void setNumeric ()       { numericalState = true;  blockedDOFs=State::DOF_NONE; };
		// this is needed by Gl1_QMGeometry
		void update     ();
		void setMaterialAndGenerator(shared_ptr<QMParameters>& par,shared_ptr<St1_QMState> generator) {qmParameters=par;st1_QMStateGen=generator;};
		void setLaw2Generator(shared_ptr<IGeom>& ig,shared_ptr<IPhys>& ip, shared_ptr<Interaction> I, shared_ptr<Law2_QMIGeom_QMIPhys_GlobalWavefunction> generator);

// Fx3		void set_FIXME_EXTRA_Generator(std::function<void()> extr)
// Fx3		{
// Fx3			FIXME_extra_generator_used = true;
// Fx3			FIXME_extra_generator      = extr;
// Fx3		};
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMState
			, // base class
			State
			, // class description
			"Quantum mechanical state."
			, // attributes, public variables
			((bool,wasGenerated  ,false,Attr::readonly,"It is used to remember if it was already generated."))
			((bool,numericalState,true ,Attr::readonly,"Tells whether this QMState is an analytical solution and does not need to be 'solved' for and 'propagated'."))
			((string,qtHide,"angMom angVel densityScaling inertia isDamped mass qtHide refOri refPos vel inertia",Attr::readonly,
			"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class.\
			Yes, even mass must be removed, although it is back in few of derived classes."))
// FIXME: with this description it actually becomes credible: qtHide →   "Space separated list of runtime-settable hidden    variables in qt4 interface."
// FIXME: same goes with this one: ((string,qtReadonly,"",Attr::readonly,"Space separated list of runtime-settable read-only variables in qt4 interface."))
// FIXME: it actually makes sense to put qtHide & qtReadonly inside Serializable class !!!!!
			((shared_ptr<QMParameters>                            ,qmParameters     ,,Attr::hidden," ... "))
			((shared_ptr<St1_QMState>                             ,st1_QMStateGen   ,,Attr::hidden," ... "))
			((shared_ptr<IGeom>                                   ,qmIG             ,,Attr::hidden," ... "))
			((shared_ptr<IPhys>                                   ,qmIP             ,,Attr::hidden," ... "))
			((shared_ptr<Interaction>                             ,qmI              ,,Attr::hidden," ... "))
			((shared_ptr<Law2_QMIGeom_QMIPhys_GlobalWavefunction> ,law2_generator   ,,Attr::hidden," ... "))
			, // constructor
			createIndex();
// Fx3			FIXME_extra_generator_used = false;
			, // python bindings
			.def("isAnalytic" ,&QMState::isAnalytic ,"Tells if this QMState is analytically propagated - only by increasing its time parameter")
			.def("isNumeric"  ,&QMState::isNumeric  ,"Tells if this QMState is numerically propagated")
			.def("setAnalytic",&QMState::setAnalytic,"Sets that this QMState is analytically propagated")
			.def("setNumeric" ,&QMState::setNumeric ,"Sets that this QMState is numerically propagated")
		);
		REGISTER_CLASS_INDEX(QMState,State);
	private:
//		shared_ptr<QMParameters> qmParameters;
//		shared_ptr<St1_QMState>  st1_QMStateGen;
//		shared_ptr<IGeom>        qmIG;
//		shared_ptr<IPhys>        qmIP;
//		shared_ptr<Interaction>  qmI;
//		shared_ptr<Law2_QMIGeom_QMIPhys_GlobalWavefunction>   law2_generator;


// Fx3		std::function<void()>    FIXME_extra_generator;
// Fx3		bool                     FIXME_extra_generator_used;
};
REGISTER_SERIALIZABLE(QMState);

