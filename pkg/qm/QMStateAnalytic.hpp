// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMState.hpp"
#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   A N A L Y T I C A L   S T A T E
*
*********************************************************************************/

/*! @brief QMStateAnalytic contains quantum state information expressed using analytic formulas.
 *
 *  To allow propagation in time of these analytical solutions, the only universal attribute is current local time t.
 *  It is not in QMState, bacause in general scene->time is used. But for analytic packet, we can generate it for any time, not just scene->time
 *
 *  Member variables:
 *
 *    Real t   → local time in this analytic wavefunction
 *
 *    boost::shared_ptr<QMStateDiscrete>& stateDiscreteOptimised
 *             → it is used for faster display on screen. I has its own discrete representation. But (FIXME) the Gl1_QMGeometry makes
 *               sure that it's up to date. It respects `size` and `step` set in QMGeometry
 *               // FIXME.2 - after fixing all those functions in QMState, I guess that it can become a private variable member and expose it only
 *                            through a unified interface in QMState
 *
 */

class QMStateDiscrete;
class QMGeometry;
class QMStateAnalytic: public QMState
{
	public:
		virtual ~QMStateAnalytic();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMStateAnalytic
			, // base class
			QMState
			, // class description
"Analytic quantum mechanical state. It is expressed in terms of a mathematical function, which can be pretty arbitrary. It can be used to initialize \
the quantum mechanical state to be discretized into an NDimTable, or to make comparisons between simulation results and analytical solutions. \
Derived classes will be specific analytic solutions to various cases."
			, // attributes, public variables
			((Real,t,0,,"Current local time in this analytic packet. The theory must be predictive, so there must be always dependence on time."))
			, // constructor
			createIndex();
			lastOptimisationIter=-1;
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMStateAnalytic,QMState);

/* FIXME: make it	private: */
		boost::shared_ptr<QMStateDiscrete> stateDiscreteOptimised;
		long lastOptimisationIter;

};
REGISTER_SERIALIZABLE(QMStateAnalytic);

// FIXME: class QMStateAnalyticMixed: public QMStateAnalytic // ← to allow mixed states, eg: (φ+ψ)/√2


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   A N A L Y T I C A L   S T A T E   F U N C T O R
*
*********************************************************************************/

class St1_QMStateAnalytic: public St1_QMState
{
	public:
		virtual void go(const shared_ptr<State>&, const shared_ptr<Material>&, const Body*);
		FUNCTOR1D(QMStateAnalytic);
		YADE_CLASS_BASE_DOC(St1_QMStateAnalytic/* class name */, St1_QMState /* base class */
			, "Functor creating :yref:`QMState` from :yref:`QMParameters`." // class description
		);
/*FIXME, make it:	private: */
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms)
		{ throw std::logic_error("St1_QMStateAnalytic::getValPos was called directly.");};
};
REGISTER_SERIALIZABLE(St1_QMStateAnalytic);

