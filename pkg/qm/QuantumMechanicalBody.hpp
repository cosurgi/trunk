// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/GlobalEngine.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   B O D Y
*
*********************************************************************************/

/*! @brief QuantumMechanicalBody contains single 'body' (a particle) expressed in terms of quantum mechanics.
 *
 */
class QuantumMechanicalBody: public Body
{
	public:
		virtual ~QuantumMechanicalBody();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QuantumMechanicalBody
			, // base class
			Body
			, // class description
			"Quantum mechanical body is a single 'body' (a particle) expressed in terms of quantum mechanics."
			, // attributes, public variables
//# FIXME: (Janek) Implementing Quantum Mechanics makes some DEM assumptions
//# invalid.  I think that we should rethink what base class State contains, so
//# that in QM we would not need to use this hack to hide some variables.
//# However it is great to note that only this little 'cosmetic' hack is needed
//# to make Quantum Mechanics possible in yade
//# See also: class QuantumMechanicalState, class QuantumMechanicalBody, gui/qt4/SerializableEditor.py
//# Also I think this makes a possibility to make another Body for SPH model (see hacks inside Body.hpp YADE_SPH)
//#
//# FIXME: so in fact if we decided to fix the inheritance tree we should remove
//#        following attributes from the base class
			((string,qtHide,"chain clumpId flags qtHide",Attr::readonly,
			"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class."))
			, // constructor
			, // python bindings
		);
};
REGISTER_SERIALIZABLE(QuantumMechanicalBody);

