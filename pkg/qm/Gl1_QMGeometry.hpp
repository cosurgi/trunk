// 2014 © Janek Kozicki <cosurgi@gmail.com>
#pragma once
#ifdef YADE_OPENGL

#include <functional>
#include <pkg/common/GLDrawFunctors.hpp>
#include "QMGeometry.hpp"
#include "QMStateDiscrete.hpp"
#include <lib/time/TimeLimit.hpp>
#include <lib/computational-geometry/MarchingCube.hpp>
#include <py/wrapper/Menu.hpp>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

class Gl1_QMGeometry: public GlShapeFunctor
{
	public:
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
		void drawSurface(const std::vector<std::vector<Real> >& waveVals,Vector3r col);
		void calcNormalVectors(const std::vector<std::vector<Real> >& waveVals,std::vector<std::vector<Vector3r> >& wavNormV);
		void prepareGlSurfaceMaterial();
		void glDrawMarchingCube(MarchingCube& mc,Vector3r col,std::string drawStyle,bool wire);
		void glDrawSurface(const std::vector<std::vector<Real> >& waveVals,const std::vector<std::vector<Vector3r> >& wavNormV,Vector3r col);
		void glDrawSurfaceInterpolated(const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,Vector3r col);
		void interpolateExtraWaveValues(const std::vector<std::vector<Real> >& waveVals,std::vector<std::vector<Real> >& extraWaveVals);
		void interpolateExtraNormalVectors(const std::vector<std::vector<Vector3r> >& wavNormV,std::vector<std::vector<Vector3r> >& extraWavNormV);
		Gl1_QMGeometry();
		virtual ~Gl1_QMGeometry();
		RENDERS(QMGeometry);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_STATICATTRS(
			  // class name
			Gl1_QMGeometry
			, // base class
			GlShapeFunctor
			, // class description
			"Renders :yref:`QMGeometry` object"
			, // static public attributes
			((bool,analyticUsesStepOfDiscrete,true,,"Analytic wavefunctions will use the step of last discrete wavefunction plotted."))
			((bool,analyticUsesScaleOfDiscrete,true,,"Analytic wavefunctions will use the partsScale of last discrete wavefunction plotted."))
		);
	private:
		TimeLimit timeLimit;
		MarchingCube mc;
		
		//! pointer to what's drawn right now
		QMGeometry* g;
		QMStateDiscrete*   pd;
		//! last step of a discrete wavefunction
		static Vector3r lastDiscreteStep;
		static Real lastDiscreteScale;

		///! These are drawing functors, to speed up selection of drawing options
		std::vector< std::function< bool    (                  ) > > partsToDraw;
		std::vector< std::function< string  (                  ) > > drawStyle;
		std::vector< std::function< Real    (std::complex<Real>) > > valueToDraw;
		std::vector< std::function< Vector3r(Vector3r          ) > > colorToDraw;

		Vector3r start;
		// FIXME(2) - when storage problems are resolved, remove this
		std::vector<std::vector<Real> >                              waveValues2D;
		std::vector<std::vector<std::vector<Real> > >                waveValues3D;
		// FIXME(2) - jakoś inaczej przechowywać w tablicy, bo takie pętle są bez sensu i zajmują kupę czasu.

};
REGISTER_SERIALIZABLE(Gl1_QMGeometry);

#endif


/*********************************************************************************
*
* Q U A N T U M   I N T E R A C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL
// This will come later, when I will have some interactions going on....
//class Gl1_QMIPhys: public GlIPhysFunctor
//{
//	public: 
//		virtual void go(const shared_ptr<IPhys>&,const shared_ptr<Interaction>&,const shared_ptr<Body>&,const shared_ptr<Body>&,bool wireFrame);
//		virtual ~Gl1_QMIPhys();
//		RENDERS(QMIPhys);
//		DECLARE_LOGGER;
//		YADE_CLASS_BASE_DOC_STATICATTRS(
//			  // class name
//			Gl1_QMIPhys
//			, // base class
//			GlIPhysFunctor
//			, // class description
//			"Render :yref:`QMIPhys` interactions."
//			, // attributes, public variables
//			((bool,abs,true,,"Show absolute probability"))
//			((bool,real,false,,"Show only real component"))
//			((bool,imag,false,,"Show only imaginary component"))
//		);
//};
//REGISTER_SERIALIZABLE(Gl1_QMIPhys);
#endif

