/*************************************************************************
*  Copyright (C) 2013 by Burak ER                                 	 *
*									 *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#ifdef YADE_FEM
#include <pkg/fem/Lin4NodeTetra_Lin4NodeTetra_InteractionElement.hpp>
#include <algorithm>
#include <core/Scene.hpp>
#include <core/BodyContainer.hpp>
#include <core/State.hpp>
#include <pkg/common/Sphere.hpp>
#include <pkg/fem/Node.hpp>
#include <lib/base/Math.hpp>

namespace yade { // Cannot have #include directive inside.

YADE_PLUGIN((Lin4NodeTetra_Lin4NodeTetra_InteractionElement));
CREATE_LOGGER(Lin4NodeTetra_Lin4NodeTetra_InteractionElement);

Lin4NodeTetra_Lin4NodeTetra_InteractionElement::~Lin4NodeTetra_Lin4NodeTetra_InteractionElement(){

}
void Lin4NodeTetra_Lin4NodeTetra_InteractionElement::initialize(void){

}
} // namespace yade

#endif //YADE_FEM



