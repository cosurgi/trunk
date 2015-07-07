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
* Q M   G E O M E T R Y    O P E N   G L   D I S P L A Y
*
*********************************************************************************/

class Gl1_QMGeometry: public GlShapeFunctor
{
	public:
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
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

		//! last step of a discrete wavefunction
		static Vector3r lastDiscreteStep;
		static Real lastDiscreteScale;

		// FIXME: where to put this?
		// temporary table used for renderRotated45
		NDimTable<Complexr> transformed;
		NDimTable<Complexr>& maybeTransform(QMStateDiscrete* qms,size_t dimSpatial, shared_ptr<QMDisplayOptions>& opt, NDimTable<Complexr>& arg);
};
REGISTER_SERIALIZABLE(Gl1_QMGeometry);

/*********************************************************************************
*
* Q M   I N T E R A C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

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

/*********************************************************************************
*
* Q M   O P E N   G L   D R A W I N G       draws NDimTable marginal distributions
*
*********************************************************************************/

class Gl1_NDimTable: public Serializable
{
	public:
		void drawNDimTable(NDimTable<Complexr>&, shared_ptr<QMDisplayOptions>&,const Vector3r&,bool forceWire,const GLViewInfo&);

		void drawSurface(const std::vector<std::vector<Real> >& waveVals,Vector3r col);
		void calcNormalVectors(const std::vector<std::vector<Real> >&,std::vector<std::vector<Vector3r> >&);
		void prepareGlSurfaceMaterial();
		void glDrawMarchingCube(MarchingCube& mc,Vector3r col,std::string drawStyle,bool wire);
		void glDrawSurface(const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,Vector3r);
		void glDrawSurfaceInterpolated(const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,const std::vector<std::vector<Real> >&,const std::vector<std::vector<Vector3r> >&,Vector3r);
		void interpolateExtraWaveValues(const std::vector<std::vector<Real> >&,std::vector<std::vector<Real> >&);
		void interpolateExtraNormalVectors(const std::vector<std::vector<Vector3r> >&,std::vector<std::vector<Vector3r> >&);
		Gl1_NDimTable();
		virtual ~Gl1_NDimTable();
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_STATICATTRS(
			  // class name
			Gl1_NDimTable
			, // base class
			Serializable
			, // class description
			"Renders :yref:`NDimTable`, which must be a marginal distribution (up to 3 dimensions)."
			, // attributes, public variables
		);
	private:
		TimeLimit timeLimit;
		MarchingCube mc;

		//! pointers to what's drawn right now
		shared_ptr<QMDisplayOptions> opt;
		NDimTable<Complexr>*         data;

		//! These are drawing functors, to speed up selection of drawing options
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
REGISTER_SERIALIZABLE(Gl1_NDimTable);

#endif

