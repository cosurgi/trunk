// 2014 © Janek Kozicki <cosurgi@gmail.com>
#pragma once
#ifdef YADE_OPENGL

#include <pkg/common/GLDrawFunctors.hpp>
#include "QMGeometryDisplay.hpp"
#include <time.h>

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
		void glDrawSurface(const std::vector<std::vector<Real> >& waveVals,const std::vector<std::vector<Vector3r> >& wavNormV,Vector3r col);
		void glDrawSurfaceInterpolated(const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,Vector3r col);
		void interpolateExtraWaveValues(const std::vector<std::vector<Real> >& waveVals,std::vector<std::vector<Real> >& extraWaveVals);
		void interpolateExtraNormalVectors(const std::vector<std::vector<Vector3r> >& wavNormV,std::vector<std::vector<Vector3r> >& extraWavNormV);
		Real spline36Interpolation(Real dist);
		Real sinc256Interpolation(Real dist);
		// FIXME - those two functions can be written as a single template
		Real     calcInterpolation_2D      (const std::vector<std::vector<Real    > >& val,Real posX, Real posY);
		Vector3r calcInterpolation_2Dvector(const std::vector<std::vector<Vector3r> >& val,Real posX, Real posY);
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
		);
	private: // FIXME - after redundancy is removed, this should be removed too
		Real getClock(){ timeval tp; gettimeofday(&tp,NULL); return tp.tv_sec+tp.tv_usec/1e6; }
		bool tooLong(){return (getClock() - wallClock)>stepWait;};
		Real startX,startY,startZ,endX,endY,endZ,wallClock;
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

