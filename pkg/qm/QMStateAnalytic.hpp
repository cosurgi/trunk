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
 *  To allow propagation in time of these analytical solutions, the only universal attribute is current local time t
 *  (FIXME: currently nonrelativistic approach only).
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
"Analytic quantum mechanical state. It is expressed in terms of a mathematical function, which can be \
pretty arbitrary. It can be used to initialize the quantum mechanical state discretized into a grid or matrix, \
or to make comparisons between simulation results and analytical solutions. Derived classes will be specific \
analytic solutions to various cases."
			, // attributes, public variables
			((Real,t,0,,"Current local time in this packet $t=0$, FIXME: nonrelativistic approach."))
			, // constructor
			createIndex();
			lastOptimisationIter=-1;
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMStateAnalytic,QMState);

		const boost::shared_ptr<QMStateDiscrete>& prepareDiscrete(QMGeometry*);
	private:
		boost::shared_ptr<QMStateDiscrete> stateDiscreteOptimised;
		long lastOptimisationIter;

};
REGISTER_SERIALIZABLE(QMStateAnalytic);

// FIXME: class QMStateAnalyticMixed: public QMStateAnalytic // ← to allow mixed states, eg: (φ+ψ)/√2

