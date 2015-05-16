// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/Scene.hpp>
#include <pkg/common/Box.hpp>
#include <py/wrapper/Menu.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   G E O M E T R Y   D I S P L A Y
*
*********************************************************************************/

/*! @brief QMGeometry contains geometrical information used to display the particle on screen.
 *
 * Quantum particles have no geometry in classical sense. So in this class only has
 * stuff needed to configure the display of the wavefunction in QMState.
 *
 * Things like color, or if it's a wireframe to display, etc.
 *
 * No physical/calculations related stuff here! Only display.
 *
 */
class QMGeometry: public Box
{
	public:
		virtual ~QMGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			QMGeometry
			, // base class
			Box
			, // class description
			"Geomterical information about the wavefunction of a particle. Mostly display configuration, like color or renderShininess, but also size and display precision step."
			, // attributes, public variables
/* Box::extents 	((Vector3r,halfSize,,,
			"Analytical wavefunction is infinite, but computers are finite, so this is the spatial size \
			in positional representation. It is used to create :yref:`Aabb`, so it also helps fiding particle collisions. \
			If the wavefunctions were really infinite, then it would work nicely, but due to computer limitations this \
			halfSize also covers the range of interactions between particles. Later it should work in 4D space-time.")) */
			((Menu,partAbsolute     ,Menu({"default wire"   ,"hidden","nodes","big nodes","points","wire","surface"}),,"Show absolute value of the wavefunction"))
			((Menu,partImaginary    ,Menu({"default surface","hidden","nodes","big nodes","points","wire","surface"}),,"Show imaginary component"))
			((Menu,partReal         ,Menu({"default surface","hidden","nodes","big nodes","points","wire","surface"}),,"Show real component"))
			((int ,partsScale       ,1.0,,"Scaling of the wavefunction or potential. Positive number multiplies, negative divides by absolute value."))
			((bool,partsSquared     ,false,,"Show squares of selected parts to draw (eg. squared partAbsolute is probability)"))
			((int ,renderAmbient    ,30,Attr::hidden,"Amount of ambient light falling on surface"))
			((int ,renderDiffuse    ,100,Attr::hidden,"Amount of diffuse light reflected by surface"))
			((bool,renderInterpolate,false,,"Interpolate extra points in center of each square using sinc256(x) or spline36(x) interpolation as in [Kozicki2007g]_"))
			((int ,renderShininess  ,50,Attr::hidden,"Amount of shininess of the surface"))
			((bool,renderSmoothing  ,true,,"Smooth the displayed surface"))
			((int ,renderSpecular   ,10,Attr::hidden,"Amount of specular light reflected by surface"))
			((Vector3r,step         ,Vector3r(0.1,0.1,0.1),,"Rendering step, careful - too small will make rendering extremely slow"))
			((Menu,stepRender       ,Menu({"default hidden","hidden","frame","stripes","mesh"}),,"Show the steps on the grid."))
			((Real,renderMaxTime    ,0.2,,"Maximum rendering time in seconds. Abort if takes too long."))
			((Real,threshold3D      ,0.0000001,,"Isosurface value for 3D drawing, using marching cubes algorithm."))
			, // constructor
			createIndex();
		);
		REGISTER_CLASS_INDEX(QMGeometry,Box);
};
REGISTER_SERIALIZABLE(QMGeometry);

