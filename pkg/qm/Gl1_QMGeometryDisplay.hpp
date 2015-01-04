// 2014 © Janek Kozicki <cosurgi@gmail.com>
#pragma once
#ifdef YADE_OPENGL

#include <functional>
#include <pkg/common/GLDrawFunctors.hpp>
#include "QMGeometryDisplay.hpp"
#include <lib/time/TimeLimit.hpp>
#include <lib/computational-geometry/MarchingCube.hpp>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

class Gl1_QMGeometryDisplay: public GlShapeFunctor
{
	public:
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
		void drawSurface(const std::vector<std::vector<Real> >& waveVals,Vector3r col);
		void calcNormalVectors(const std::vector<std::vector<Real> >& waveVals,std::vector<std::vector<Vector3r> >& wavNormV);
		void prepareGlSurfaceMaterial();
		void glDrawMarchingCube(MarchingCube& mc,Vector3r col);
		void glDrawSurface(const std::vector<std::vector<Real> >& waveVals,const std::vector<std::vector<Vector3r> >& wavNormV,Vector3r col);
		void glDrawSurfaceInterpolated(const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,Vector3r col);
		void interpolateExtraWaveValues(const std::vector<std::vector<Real> >& waveVals,std::vector<std::vector<Real> >& extraWaveVals);
		void interpolateExtraNormalVectors(const std::vector<std::vector<Vector3r> >& wavNormV,std::vector<std::vector<Vector3r> >& extraWavNormV);
		Gl1_QMGeometryDisplay();
		virtual ~Gl1_QMGeometryDisplay();
		RENDERS(QMGeometryDisplay);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_STATICATTRS(
			  // class name
			Gl1_QMGeometryDisplay
			, // base class
			GlShapeFunctor
			, // class description
			"Renders :yref:`QMGeometryDisplay` object"
			, // static public attributes
// FIXME,FIXME - add option to draw points of discretisation (for discretized wavefunctions, do I need to derive :
// class Gl1_QMGeometryDiscreteDisplay : public Gl1_QMGeometryDisplay
			// FIXME - maybe implement ordering, and <hr> separator, similar way as with qtHide
			((bool,absolute         ,false,,"Show absolute probability"))
			((bool,partImaginary    ,true,,"Show only imaginary component"))
			((bool,partReal         ,true,,"Show only real component"))
			((bool,probability      ,true,,"Show probability, which is squared absolute value"))
			((int ,renderAmbient    ,30,,"Amount of ambient light falling on surface"))
			((int ,renderDiffuse    ,100,,"Amount of diffuse light reflected by surface"))
			((bool,renderInterpolate,false,,"Interpolate extra points in center of each square using sinc256(x) or spline36(x) interpolation as in [Kozicki2007g]_"))
			((int ,renderShininess  ,50,,"Amount of shininess of the surface"))
			((bool,renderSmoothing  ,true,,"Smooth the displayed surface"))
			((int ,renderSpecular   ,10,,"Amount of specular light reflected by surface"))
			((Real,step             ,0.1,,"Rendering step, careful - too small will make rendering extremely slow"))
			((Real,stepWait         ,0.1,,"Maximum rendering time in seconds. Abort if takes too long."))
			((Real,threshold3D      ,0.00000001,,"Isosurface value for 3D drawing, using marching cubes algorithm."))
		);
	private:
		TimeLimit timeLimit;
		MarchingCube mc;
		std::vector< std::function< bool    (                  ) > > partsToDraw;
		std::vector< std::function< Real    (std::complex<Real>) > > valueToDraw;
		std::vector< std::function< Vector3r(Vector3r          ) > > colorToDraw;

		// FIXME - after redundancy is removed, this should be removed too
		Real startX,startY,startZ,endX,endY,endZ;

		// FIXME(2) - when storage problems are resolved, remove this
		std::vector<std::vector<Real> >                              waveValues2D;
		std::vector<std::vector<std::vector<Real> > >                waveValues3D;
		// FIXME(2) - jakoś inaczej przechowywać w tablicy, bo takie pętle są bez sensu i zajmują kupę czasu.

};
REGISTER_SERIALIZABLE(Gl1_QMGeometryDisplay);

#endif


/*********************************************************************************
*
* Q U A N T U M   I N T E R A C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL
// This will come later, when I will have some interactions going on....
//class Gl1_QMInteractionPhysics: public GlIPhysFunctor
//{
//	public: 
//		virtual void go(const shared_ptr<IPhys>&,const shared_ptr<Interaction>&,const shared_ptr<Body>&,const shared_ptr<Body>&,bool wireFrame);
//		virtual ~Gl1_QMInteractionPhysics();
//		RENDERS(QMInteractionPhysics);
//		DECLARE_LOGGER;
//		YADE_CLASS_BASE_DOC_STATICATTRS(
//			  // class name
//			Gl1_QMInteractionPhysics
//			, // base class
//			GlIPhysFunctor
//			, // class description
//			"Render :yref:`QMInteractionPhysics` interactions."
//			, // attributes, public variables
//			((bool,abs,true,,"Show absolute probability"))
//			((bool,real,false,,"Show only real component"))
//			((bool,imag,false,,"Show only imaginary component"))
//		);
//};
//REGISTER_SERIALIZABLE(Gl1_QMInteractionPhysics);
#endif

