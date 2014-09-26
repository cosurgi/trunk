// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* W A V E   F U N C T I O N   M A T E R I A L
*
*********************************************************************************/

/*! @brief WavePacketParameters contains information about material out of which each particle is made.
 *
 * Actually a wave function has no material. Although later it may come useful to categorize wavefunctions
 * as being one of fundamental indistinguishable particles in the standard model: quarks (up, down, etc.),
 * electrons (muon, tau), neutrinos, etc.
 *
 */
class WavePacketParameters: public Material
{
	public:
		virtual ~WavePacketParameters();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			WavePacketParameters
			, // base class
			Material
			, // class description
			"Actually wave function has no material. For now this class is empty only for purposes of\
			keeping compatibility with yade. Although maybe later it may come useful to distinguish \
			fundamental particles in the standard model: quarks (up, down, etc.),\
			electrons (muon, tau), neutrinos, etc.\n\
			\n\
			Important thing to note is that this Material is\
			supposed to be shared between various different instances of particles, thus\
			it will make sense to create material for a fundamental particle\n\
			\n\
			But how does this apply to the simples Schrodinger wavepacket?\n\
			\n\
			Maybe eg. fermions and bosons will derive from this class too."
			, // attributes, public variables
//			((bool,isWaveFunction,true,,"This is only a placeholder in WavePacketParameters, not used for anything."))
			((string,qtHide,"density qtHide",Attr::readonly,"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class."))
			, // constructor
			createIndex();
	);
	REGISTER_CLASS_INDEX(WavePacketParameters,Material);
};
REGISTER_SERIALIZABLE(WavePacketParameters);


