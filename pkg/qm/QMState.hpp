// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/

/*! @brief QMState contains quantum state information
 *
 */
class QMState: public State
{
	public:
		virtual Complexr getValPos(Vector3r xyz){};          /// return complex quantum aplitude at given positional representation coordinates
//		virtual Complexr getValInv(Vector3r xyz){};          /// return complex quantum aplitude at given wavenumber representation coordinates
		virtual Complexr getValIJKPos(Vector3i ijk){};       /// return complex quantum aplitude at given positional grid coordinates
//		virtual Complexr getValIJKInv(Vector3i ijk){};       /// return complex quantum aplitude at given wavenumber grid coordinates
		virtual int      getDim(){ return dim;};             /// return number of dimensions
		virtual Vector3r getExtentsIJK(){};                  /// return grid size
		virtual Real     getStepPos(){};                     /// return grid step, two point distance in the mesh in positional representation
//		virtual Real     getStepInv(){};                     /// return grid step, two point distance in the mesh in wavenumber representation
//		virtual Vector3r getHalfSizeInv(){};                 /// return size in wavenumber representation

//		virtual Complexr calcScalarProduct(shared_ptr<QMState>& other); // calculate <φ|ψ>

		virtual ~QMState();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMState
			, // base class
			State
			, // class description
			"Quantum mechanical state."
			, // attributes, public variables
			((string,qtHide,"angMom angVel blockedDOFs densityScaling inertia isDamped mass qtHide refOri refPos vel inertia",Attr::readonly,
			"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class.\
			Yes, even mass must be removed, although it is back in few of derived classes."))
			((int     ,dim     ,,Attr::readonly,"Describes in how many dimensions this quantum particle resides. First Vector3r[0] is used, then [1], then [2]."))
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMState,State);
};
REGISTER_SERIALIZABLE(QMState);


