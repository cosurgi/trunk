// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMState.hpp"
#include "QMStateAnalytic.hpp"
#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   P O T E N T I A L   B A R R I E R
*
*********************************************************************************/

/*! @brief QMStateBarrier is a potential barrier
 *
 * Inside the geometric shape of Body.shape there is a potential barrier with potentialValue.
 *
 */
class QMStateBarrier: public QMState
{
	public:
		virtual ~QMStateBarrier();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMStateBarrier
			, // base class
			QMState
			, // class description
"Inside the geometric shape of Body.shape there is a potential barrier with potentialValue."
			, // attributes, public variables
			((Real      ,potentialValue,0,,"The value of potential inside the barrier."))
// FIXME 		((Menu      ,potentialType ,{"default barrier","barrier","square position"} ,,"0: barrier, 1: squared position."))
			((int       ,potentialType ,0,,"0: barrier, 1: squared position."))
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMStateBarrier,QMState);
	private:
		
};
REGISTER_SERIALIZABLE(QMStateBarrier);

