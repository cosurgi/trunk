/* CWBoon 2016 */
#ifdef YADE_POTENTIAL_BLOCKS

#pragma once
#ifdef YADE_OPENGL
	#include<pkg/common/GLDrawFunctors.hpp>
#endif
#include<pkg/dem/PotentialBlock.hpp>
#include<pkg/dem/PotentialBlock2AABB.hpp>
#include<pkg/common/GLDrawFunctors.hpp>
//#include<pkg/dem/MarchingCube.hpp>
#include <lib/computational-geometry/MarchingCube.hpp>
#include <vector>
#include <pkg/common/PeriodicEngines.hpp>

#include<vtkImplicitFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>

#include<vtkXMLUnstructuredGridWriter.h>

// https://codeyarns.com/2014/03/11/how-to-selectively-ignore-a-gcc-warning/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"
// Code that generates this warning, Note: we cannot do this trick in yade. If we have a warning in yade, we have to fix it! See also https://gitlab.com/yade-dev/trunk/merge_requests/73
// This method will work once g++ bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431#c34 is fixed.
#include<vtkTriangle.h>
#pragma GCC diagnostic pop

#include<vtkSmartPointer.h>
#include<vtkFloatArray.h>
#include<vtkCellArray.h>
#include<vtkCellData.h>
#include <vtkSampleFunction.h>
#include <vtkStructuredPoints.h>
#include<vtkStructuredPointsWriter.h>
#include<vtkWriter.h>
#include<vtkExtractVOI.h>
#include<vtkXMLImageDataWriter.h>
#include<vtkXMLStructuredGridWriter.h>
#include<vtkTransformPolyDataFilter.h>
#include<vtkTransform.h>
#include <vtkRenderWindowInteractor.h> 
#include<vtkXMLUnstructuredGridWriter.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>

#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkVectorText.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkTextActor3D.h>
#include <vtkCylinderSource.h>



class ImpFuncPB : public vtkImplicitFunction {
public:
    vtkTypeMacro(ImpFuncPB,vtkImplicitFunction);
    //void PrintSelf(ostream& os, vtkIndent indent);

    // Description
    // Create a new function
    static ImpFuncPB * New(void);
    vector<double>a; vector<double>b; vector<double>c; vector<double>d;
    double k; double r; double R; Eigen::Matrix3d rotationMatrix;
    bool clump;
    double clumpMemberCentreX;
    double clumpMemberCentreY;
    double clumpMemberCentreZ;
    // Description
    // Evaluate function
    double FunctionValue(double x[3]);
    double EvaluateFunction(double x[3]){ 
	//return this->vtkImplicitFunction::EvaluateFunction(x);
	return FunctionValue(x);
    };
    
    double EvaluateFunction(double x, double y, double z) {
	return this->vtkImplicitFunction::EvaluateFunction(x, y, z);
    };
    
    


    // Description
    // Evaluate gradient for function
   void EvaluateGradient(double x[3], double n[3]){ };

    // If you need to set parameters, add methods here

protected:
   ImpFuncPB();
   ~ImpFuncPB();
   ImpFuncPB(const ImpFuncPB&) {}
   void operator=(const ImpFuncPB&) {}

    // Add parameters/members here if you need
};

#ifdef YADE_OPENGL
class Gl1_PotentialBlock : public GlShapeFunctor
{	
	private :
		MarchingCube mc;
		Vector3r min,max;
		vector<vector<vector<Real > > > scalarField,weights;
		void generateScalarField(const PotentialBlock& pp);
		void calcMinMax(const PotentialBlock& cm);
		float oldIsoValue,oldIsoSec,oldIsoThick;
		Vector3r isoStep;
		Eigen::Matrix3d rotationMatrix;
		

	public :
		struct Leaf{
			Vector3r centre;
			Leaf(Vector3r pos){centre = pos;}
			Leaf(){centre = Vector3r(0,0,0);}
		};
		struct scalarF{
			 vector<vector<vector<float > > > scalarField2;
			 vector<Vector3r> triangles;
			 vector<Vector3r> normals;
			 int nbTriangles;
			
    
		};
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
		double evaluateF(const PotentialBlock& pp, double x, double y, double z);
		static vector<scalarF> SF ;
		//void clearMemory();
		
	//YADE_CLASS_BASE_DOC_STATICATTRS(Gl1_PotentialBlock,GlShapeFunctor,"Renders :yref:`Sphere` object",
		//(( vector<scalarF>, SF ," "))
	//);
	YADE_CLASS_BASE_DOC_STATICATTRS(Gl1_PotentialBlock,GlShapeFunctor,"Renders :yref:`Sphere` object",
		((int,sizeX,30,,"Number of divisions in the x direction for triangulation"))
		((int,sizeY,30,,"Number of divisions in the x direction for triangulation"))
		((int,sizeZ,30,,"Number of divisions in the y direction for triangulation"))
		((bool,store,true,,"Number of divisions in the z direction for triangulation"))
		((bool,initialized,false,,"Number of divisions in the z direction for triangulation"))
		((Real,aabbEnlargeFactor,1.3,,"some factor for displaying algorithm, try different value if you have problems with displaying"))
		
	);
	RENDERS(PotentialBlock);


};
REGISTER_SERIALIZABLE(Gl1_PotentialBlock);

#endif // YADE_OPENGL



class PotentialBlockVTKRecorder: public PeriodicEngine{	
  public:
	vtkSmartPointer<ImpFuncPB> function;
	
  virtual void action(void);
  YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(PotentialBlockVTKRecorder,PeriodicEngine,"Engine recording potential blocks as surfaces into files with given periodicity.",
	((string,fileName,,,"File prefix to save to"))
	((int,sampleX,30,,"size of contact point"))
	((int,sampleY,30,,"size of contact point"))
	((int,sampleZ,30,,"size of contact point"))
	((double,maxDimension,30,,"size of contact point"))
	((bool,twoDimension,false,,"size of contact point"))
	((bool,REC_INTERACTION,false,,"contact point and forces"))
	((bool,REC_COLORS,false,,"colors"))
	((bool,REC_VELOCITY,false,,"velocity"))
	((bool,REC_ID,true,,"id"))
	,
	function = ImpFuncPB::New();
	,
	
  );
};
REGISTER_SERIALIZABLE(PotentialBlockVTKRecorder);



class PotentialBlockVTKRecorderTunnel: public PeriodicEngine{	
  public:
	vtkSmartPointer<ImpFuncPB> function;
	
  virtual void action(void);
  YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(PotentialBlockVTKRecorderTunnel,PeriodicEngine,"Engine recording potential blocks as surfaces into files with given periodicity.",
	((string,fileName,,,"File prefix to save to"))
	((int,sampleX,30,,"size of contact point"))
	((int,sampleY,30,,"size of contact point"))
	((int,sampleZ,30,,"size of contact point"))
	((double,maxDimension,30,,"size of contact point"))
	((bool,twoDimension,false,,"size of contact point"))
	((bool,REC_INTERACTION,false,,"contact point and forces"))
	((bool,REC_COLORS,false,,"colors"))
	((bool,REC_VELOCITY,false,,"velocity"))
	((bool,REC_ID,true,,"id"))
	,
	function = ImpFuncPB::New();
	,
	
  );
};
REGISTER_SERIALIZABLE(PotentialBlockVTKRecorderTunnel);


#endif // YADE_POTENTIAL_BLOCKS



