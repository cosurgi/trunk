// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMInteraction.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMInteractionPhysics)
	(QMInteractionGeometry)
	(Ip2_QMParameters_QMParameters_QMInteractionPhysics)
	(Ip2_Material_QMParameters_QMInteractionPhysics)
	(Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   P H Y S I C S     (constitutive parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(QMInteractionPhysics);
// !! at least one virtual function in the .cpp file
QMInteractionPhysics::~QMInteractionPhysics(){};


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(QMInteractionGeometry);
// !! at least one virtual function in the .cpp file
QMInteractionGeometry::~QMInteractionGeometry(){};


/*********************************************************************************
*
* I N T E R A C T I O N   P H Y S I C S   constitutive parameters of the contact
*
*********************************************************************************/
CREATE_LOGGER(Ip2_QMParameters_QMParameters_QMInteractionPhysics);
void Ip2_QMParameters_QMParameters_QMInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

CREATE_LOGGER(Ip2_Material_QMParameters_QMInteractionPhysics);
void Ip2_Material_QMParameters_QMInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*
*********************************************************************************/
CREATE_LOGGER(Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics);

bool Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics::go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I){
}

