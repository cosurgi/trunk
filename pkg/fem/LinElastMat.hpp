/*************************************************************************
*  Copyright (C) 2013 by Burak ER                                 	 *
*									 *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#pragma once
#include <core/Material.hpp>
#include <limits>

namespace yade { // Cannot have #include directive inside.

/*! Elastic material */
class DeformableElementMaterial: public Material{
	public:
	virtual ~DeformableElementMaterial();
	// clang-format off
	YADE_CLASS_BASE_DOC_ATTRS_CTOR(DeformableElementMaterial,Material,"Deformable Element Material.",
		((Real,density,1,,"Density of the material.")),
		/*ctor*/ createIndex();
	);
	// clang-format on
	REGISTER_CLASS_INDEX(DeformableElementMaterial,Material);
};
REGISTER_SERIALIZABLE(DeformableElementMaterial);

class LinIsoElastMat: public DeformableElementMaterial{
	public:
	virtual ~LinIsoElastMat();
	// clang-format off
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(LinIsoElastMat,DeformableElementMaterial,"Linear Isotropic Elastic material",
			((Real,youngmodulus,.78e5,,"Young's modulus. Initially aluminium."))
			((Real,poissonratio,.33,,"Poisson ratio. Initially aluminium.")),
			createIndex();
		);
	// clang-format on
		REGISTER_CLASS_INDEX(LinIsoElastMat,DeformableElementMaterial);
};
REGISTER_SERIALIZABLE(LinIsoElastMat);


/*Standard damped linear elastic material*/
class LinIsoRayleighDampElastMat: public LinIsoElastMat{
	public:
	virtual ~LinIsoRayleighDampElastMat();
	// clang-format off
	YADE_CLASS_BASE_DOC_ATTRS_CTOR(LinIsoRayleighDampElastMat,LinIsoElastMat,"Elastic material with Rayleigh Damping.",
		((Real,alpha,0,,"Mass propotional damping constant of Rayleigh Damping."))
		((Real,beta,0,,"Stiffness propotional damping constant of Rayleigh Damping.")),
		createIndex();
	);
	// clang-format on
	REGISTER_CLASS_INDEX(LinIsoRayleighDampElastMat,LinIsoElastMat);
};
REGISTER_SERIALIZABLE(LinIsoRayleighDampElastMat);

} // namespace yade

