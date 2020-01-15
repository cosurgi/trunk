/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include "Interaction.hpp"
#include "GlobalEngine.hpp"
#include "Scene.hpp"

namespace yade { // Cannot have #include directive inside.

class Body;

class TimeStepper: public GlobalEngine{
	public:
		virtual void computeTimeStep(Scene* ) { throw; };
		virtual bool isActivated() {return (active && (scene->iter % timeStepUpdateInterval == 0));};
		virtual void action() { computeTimeStep(scene);} ;
		void setActive(bool a, int nb=-1) {active = a; if (nb>0) {timeStepUpdateInterval = (unsigned int)nb;}}
		
	// clang-format off
		YADE_CLASS_BASE_DOC_ATTRS(
			TimeStepper,GlobalEngine,"Engine defining time-step (fundamental class)",
			((bool,active,true,,"is the engine active?"))
			((unsigned int,timeStepUpdateInterval,1,,"dt update interval")));
	// clang-format on
};

REGISTER_SERIALIZABLE(TimeStepper);

} // namespace yade

