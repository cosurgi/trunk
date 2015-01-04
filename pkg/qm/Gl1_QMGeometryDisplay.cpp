// 2014 © Janek Kozicki <cosurgi@gmail.com>

/*********************************************************************************
*
* W A V E   F U N C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL
#include "Gl1_QMGeometryDisplay.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(Gl1_QMGeometryDisplay)
	);

#include "QMBody.hpp"
#include "QMState.hpp"
#include "QMStateAnalytic.hpp"
#include "QMStateDiscrete.hpp"
#include <pkg/common/GLDrawFunctors.hpp>
#include <lib/opengl/OpenGLWrapper.hpp>
#include <lib/opengl/GLUtils.hpp>
#include <lib/smoothing/Spline6Interpolate.hpp>

CREATE_LOGGER(Gl1_QMGeometryDisplay);
bool Gl1_QMGeometryDisplay::absolute=false;
bool Gl1_QMGeometryDisplay::partReal=true;
bool Gl1_QMGeometryDisplay::partImaginary=true;
bool Gl1_QMGeometryDisplay::probability=true;
bool Gl1_QMGeometryDisplay::renderSmoothing=true;
bool Gl1_QMGeometryDisplay::renderInterpolate=false;
int  Gl1_QMGeometryDisplay::renderSpecular=10;
int  Gl1_QMGeometryDisplay::renderAmbient=15;
int  Gl1_QMGeometryDisplay::renderDiffuse=100;
int  Gl1_QMGeometryDisplay::renderShininess=50;
Real Gl1_QMGeometryDisplay::step=0.1;
Real Gl1_QMGeometryDisplay::stepWait=0.1;
Real Gl1_QMGeometryDisplay::threshold3D=0.00000001;
Gl1_QMGeometryDisplay::~Gl1_QMGeometryDisplay(){};

void Gl1_QMGeometryDisplay::go(
	const shared_ptr<Shape>& shape, 
	const shared_ptr<State>& state,
	bool wire,
	const GLViewInfo&
)
{
	timeLimit.readWallClock();
	if(not(absolute or partReal or partImaginary or probability)) return; // nothing to draw
	QMGeometryDisplay* geometry       = static_cast<QMGeometryDisplay*>(shape.get());
	QMState*           packet         = static_cast<QMState*>(state.get());
	QMStateDiscrete*   packetDiscrete = dynamic_cast<QMStateDiscrete*>(state.get());
	Vector3r col = geometry->color;
// find extents to render
	startX= -geometry->halfSize[0];
	endX  =  geometry->halfSize[0];
	startY= -geometry->halfSize[1];
	endY  =  geometry->halfSize[1];
	startZ= -geometry->halfSize[2];
	endZ  =  geometry->halfSize[2];

	if(packetDiscrete) step=packetDiscrete->getStepPos();
	//step  =  0.1; // FIXME - get that from QMStateDiscrete or allow use to set something for QMStateAnalytic
	//if(packet->dim>1) step = 0.8;

// SERIOUS FIXME - remove redundant code!
	if(packet->dim == 1) {
// 1D LINES
	if(partReal) {
		glBegin(GL_LINE_STRIP);
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0))) ); // FIXME - color should be returned by some function display partReal part in bluish color
		for(Real x=startX ; x<endX ; x+=step )
		{
			//std::complex<Real> wfval = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0[0],0,packet->k0[0],packet->m,packet->a,packet->hbar);
			glVertex3d(x,0,std::real(packet->getValPos(Vector3r(x,0,0))));
		}
		glEnd();
	}

	if(partImaginary) {
		glBegin(GL_LINE_STRIP);
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4))) ); // display partImaginary in reddish color
		for(Real x=startX ; x<endX ; x+=step )
		{
			glVertex3d(x,0,std::imag(packet->getValPos(Vector3r(x,0,0))));
		}
		glEnd();
	}

	if(absolute) {
		glBegin(GL_LINE_STRIP);
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(0.4,1.0,0.4))) ); // display abs value in geenish color
		for(Real x=startX ; x<endX ; x+=step )
		{
			glVertex3d(x,0,std::abs(packet->getValPos(Vector3r(x,0,0))));
		}
		glEnd();
	}

	if(probability) {
		glBegin(GL_LINE_STRIP);
		glColor3v(shape->color); // for probability use original color
		for(Real x=startX ; x<endX ; x+=step )
		{
			std::complex<Real> wfval = packet->getValPos(Vector3r(x,0,0));
			glVertex3d(x,0,std::real((wfval)*std::conj(wfval)));
		}
		glEnd();
	}
	} else if(packet->dim == 2 and wire == false) {
// 2D SURFACE
	std::vector<std::vector<Real> > waveVals;
	waveVals.resize(int((endX-startX)/step)+1);
	FOREACH(std::vector<Real>& xx, waveVals) {xx.resize(int((endX-startX)/step)+1,0);};

	if(partReal) { // FIXME - merge those four into one loop
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				waveVals[i][j]=std::real(packet->getValPos(Vector3r(x,y,0)));
			}
			if(timeLimit.tooLong(stepWait)) break;
		}
		// FIXME - drawSurface or drawWires, this will speed up drawing wires (which are currently slower)
		drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)))); // display partReal part in bluish color
	}
	if(partImaginary) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				waveVals[i][j]=std::imag(packet->getValPos(Vector3r(x,y,0)));
			}
			if(timeLimit.tooLong(stepWait)) break;
		}
		drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4)))); // display partImaginary in reddish color
	}
	if(absolute) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				waveVals[i][j]=std::abs(packet->getValPos(Vector3r(x,y,0)));
			}
			if(timeLimit.tooLong(stepWait)) break;
		}
		drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(0.4,1.0,0.4)))); // display absolute value in geenish color
	}
	if(probability) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfval = packet->getValPos(Vector3r(x,y,0));
				waveVals[i][j]=std::real((wfval)*std::conj(wfval));
			}
			if(timeLimit.tooLong(stepWait)) break;
		}
		drawSurface(waveVals,col); // for probability use original color
	}
	} else if(packet->dim == 2 and wire == true) {
// 2D LINES
	if(partReal) {
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0))) ); // display partReal part in bluish color
		for(Real x=startX ; x<=endX ; x+=step/*,i++ */ )
		{
			glBegin(GL_LINE_STRIP);
			for(Real y=startY ; y<=endY ; y+=step/*,j++*/ )
			{
				glVertex3d(x,y,std::real(packet->getValPos(Vector3r(x,y,0))));
			}
			glEnd();
			if(timeLimit.tooLong(stepWait)) break;
		}
		for(Real y=startY ; y<=endY ; y+=step )
		{
			glBegin(GL_LINE_STRIP);
			for(Real x=startX ; x<=endX ; x+=step )
			{
				glVertex3d(x,y,std::real(packet->getValPos(Vector3r(x,y,0))));
			}
			glEnd();
			if(timeLimit.tooLong(stepWait)) break;
		}
	}
	if(partImaginary) {
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4))) ); // display partImaginary in reddish color
		for(Real x=startX ; x<=endX ; x+=step )
		{
			glBegin(GL_LINE_STRIP);
			for(Real y=startY ; y<=endY ; y+=step )
			{
				glVertex3d(x,y,std::imag(packet->getValPos(Vector3r(x,y,0))));
			}
			glEnd();
			if(timeLimit.tooLong(stepWait)) break;
		}
		for(Real y=startY ; y<=endY ; y+=step )
		{
			glBegin(GL_LINE_STRIP);
			for(Real x=startX ; x<=endX ; x+=step )
			{
				glVertex3d(x,y,std::imag(packet->getValPos(Vector3r(x,y,0))));
			}
			glEnd();
			if(timeLimit.tooLong(stepWait)) break;
		}
	}
	if(absolute) {
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(0.4,1.0,0.4))) ); // display abs value in geenish color
		for(Real x=startX ; x<=endX ; x+=step )
		{
			glBegin(GL_LINE_STRIP);
			for(Real y=startY ; y<=endY ; y+=step )
			{
				glVertex3d(x,y,std::abs(packet->getValPos(Vector3r(x,y,0))));
			}
			glEnd();
			if(timeLimit.tooLong(stepWait)) break;
		}
		for(Real y=startY ; y<=endY ; y+=step )
		{
			glBegin(GL_LINE_STRIP);
			for(Real x=startX ; x<=endX ; x+=step )
			{
				glVertex3d(x,y,std::abs(packet->getValPos(Vector3r(x,y,0))));
			}
			glEnd();
			if(timeLimit.tooLong(stepWait)) break;
		}
	}
	if(probability) {
		glColor3v(shape->color); // for probability use original color
		for(Real x=startX ; x<=endX ; x+=step )
		{
			glBegin(GL_LINE_STRIP);
			for(Real y=startY ; y<=endY ; y+=step )
			{
				std::complex<Real> wfval = packet->getValPos(Vector3r(x,y,0));
				glVertex3d(x,y,std::real((wfval)*std::conj(wfval)));
			}
			glEnd();
			if(timeLimit.tooLong(stepWait)) break;
		}
		for(Real y=startY ; y<=endY ; y+=step )
		{
			glBegin(GL_LINE_STRIP);
			for(Real x=startX ; x<=endX ; x+=step )
			{
				std::complex<Real> wfval = packet->getValPos(Vector3r(x,y,0));
				glVertex3d(x,y,std::real((wfval)*std::conj(wfval)));
			}
			glEnd();
			if(timeLimit.tooLong(stepWait)) break;
		}
	}
	} else if(packet->dim == 3) { // FIXME - add wire==true option, and draw just wires of triangles
		// FIXME,FIXME - clean up the mess. Draw real, imag, probability, etc.....
		MarchingCube mc;
		int gridSize=int((endX-startX)/step);
		Vector3r minMC(startX+step*0.5     ,startY+step*0.5     ,startZ+step*0.5     );
		Vector3r maxMC(endX  +step*0.5     ,endY  +step*0.5     ,endZ  +step*0.5     );
//		std::cout << "gridSize=" << gridSize << "\n"; // FIXME
		mc.init(gridSize,gridSize,gridSize,minMC,maxMC);
		///// mc.resizeScalarField(scalarField,sizeX,sizeY,sizeZ);	FIXME - to te linijki poniżej
		//std::vector<std::vector<std::vector<Real> > > waveVals3D; // FIXME - marching cubes templates for different dypes, maybe complex too??
		std::vector<std::vector<std::vector<Real> > > waveVals3D; // FIXME - marching cubes templates for different dypes, maybe complex too??
		waveVals3D         .resize(int((endX-startX)/step)+1);  // x position coordinate
		FOREACH(std::vector<std::vector<Real> >& xx, waveVals3D   ) {
			xx.resize(int((endY-startY)/step)+1);           // y position coordinate
			FOREACH(std::vector<Real>& yy, xx) {
				yy.resize(int((endZ-startZ)/step)+1,0); // z position coordinate
			};
		};
	
	//FIXME,FIXME,FIXME
	if(partReal) { // FIXME - merge those four into one loop
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				int k=0;
				for(Real z=startZ ; z<=endZ ; z+=step,k++ )
				{
					waveVals3D[i][j][k]=std::real(packet->getValPos(Vector3r(x,y,z))); // FIXME,FIXME - jakoś inaczej przechowywać
					// w tablicy, bo takie pętle są bez sensu i zajmują kupę czasu.
				}
			}
			if(timeLimit.tooLong(stepWait)) break;
		}
//		std::cout << "---------------------------------------\n";
		////// FIXME - drawSurface or drawWires, this will speed up drawing wires (which are currently slower)
		////// drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)))); // display partReal part in bluish color
	
		mc.computeTriangulation(waveVals3D,threshold3D);
		glDrawMarchingCube(mc,  Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)))  ); // FIXME - color should be returned by some function
	} // FIXME END - merge those four into one loop
	if(partImaginary) { // FIXME - merge those four into one loop
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				int k=0;
				for(Real z=startZ ; z<=endZ ; z+=step,k++ )
				{
					waveVals3D[i][j][k]=std::imag(packet->getValPos(Vector3r(x,y,z)));
				}
			}
			if(timeLimit.tooLong(stepWait)) break;
		}
//		std::cout << "---------------------------------------\n";
		////// FIXME - drawSurface or drawWires, this will speed up drawing wires (which are currently slower)
		////// drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)))); // display partReal part in bluish color
	
		mc.computeTriangulation(waveVals3D,threshold3D);
		glDrawMarchingCube(mc,  Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4)))  ); // FIXME - color should be returned by some function
	} // FIXME END - merge those four into one loop
	if(absolute) { // FIXME - merge those four into one loop
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				int k=0;
				for(Real z=startZ ; z<=endZ ; z+=step,k++ )
				{
					waveVals3D[i][j][k]=std::abs(packet->getValPos(Vector3r(x,y,z))); // FIXME real,imag,abs,prob...
				}
			}
			if(timeLimit.tooLong(stepWait)) break;
		}
//		std::cout << "---------------------------------------\n";
		////// FIXME - drawSurface or drawWires, this will speed up drawing wires (which are currently slower)
		////// drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)))); // display partReal part in bluish color
	
		mc.computeTriangulation(waveVals3D,threshold3D);
		glDrawMarchingCube(mc,  Vector3r(col.cwiseProduct(Vector3r(0.4,1.0,0.4)))  ); // FIXME - color should be returned by some function
	} // FIXME END - merge those four into one loop
	if(probability) { // FIXME - merge those four into one loop
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				int k=0;
				for(Real z=startZ ; z<=endZ ; z+=step,k++ )
				{
					waveVals3D[i][j][k]=std::real( (packet->getValPos(Vector3r(x,y,z)))*std::conj(packet->getValPos(Vector3r(x,y,z))) );
				}
			}
			if(timeLimit.tooLong(stepWait)) break;
		}
//		std::cout << "---------------------------------------\n";
		////// FIXME - drawSurface or drawWires, this will speed up drawing wires (which are currently slower)
		////// drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)))); // display partReal part in bluish color
	
		mc.computeTriangulation(waveVals3D,threshold3D);
		glDrawMarchingCube(mc,           col                                       ); // FIXME - color should be returned by some function
	} // FIXME END - merge those four into one loop

	} else {
		std::cerr << "4D or more plotting not ready yet\n";
	};
};
		
void Gl1_QMGeometryDisplay::glDrawMarchingCube(MarchingCube& mc,Vector3r col)
{
	prepareGlSurfaceMaterial();
	glColor3v(col);

	const vector<Vector3r>& triangles 	= mc.getTriangles();
	int nbTriangles				= mc.getNbTriangles();
	const vector<Vector3r>& normals 	= mc.getNormals();	
	if(renderSmoothing){ glShadeModel(GL_SMOOTH);};
	glEnable(GL_NORMALIZE);
	glBegin(GL_TRIANGLES);
	for(int i=0;i<3*nbTriangles;++i)
	{
		glNormal3v(normals[  i]);
		glVertex3v(triangles[i]);     // * step?, scale, size? → cerr...
		glNormal3v(normals[++i]);
		glVertex3v(triangles[i]);
		glNormal3v(normals[++i]);
		glVertex3v(triangles[i]);
	}
	glEnd();	
};

void Gl1_QMGeometryDisplay::calcNormalVectors(
	const std::vector<std::vector<Real> >& waveVals, // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	std::vector<std::vector<Vector3r> >& wavNormV    // normal vectors necessary for propoer OpenGL rendering of the faces
)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	int lenX=wavNormV.size();
	int lenY=wavNormV[0].size();
	//FIXME - end

	// Now I have waveVals[i][j] filled with values to plot. So calculate all normals. First normals for triangles
	//      *        1                                                     
	//    / |1\    / | \     wavNormV[i][j] stores normal for vertex[i][j] averaged from all 4 neigbour normals (except for edges)
	//   *--x--*  4--0--2
	//    \ | /    \ | /
	//      *        3    
	Vector3r p0(0,0,0),p1(0,0,0),p2(0,0,0),p3(0,0,0),p4(0,0,0);
	Vector3r           n1(0,0,0),n2(0,0,0),n3(0,0,0),n4(0,0,0);
	int i=0;
	for(Real x=startX ; x<=endX ; x+=step,i++ )
	{
		int j=0;
		for(Real y=startY ; y<=endY ; y+=step,j++ )
		{
			                p0=Vector3r(x     ,y     ,waveVals[i  ][j  ]);
			if((j+1)<lenY){ p1=Vector3r(x     ,y+step,waveVals[i  ][j+1]);} else{ p1=p0;};
			if((i+1)<lenX){ p2=Vector3r(x+step,y     ,waveVals[i+1][j  ]);} else{ p2=p0;};
			if((j-1)>=0  ){ p3=Vector3r(x     ,y-step,waveVals[i  ][j-1]);} else{ p3=p0;};
			if((i-1)>=0  ){ p4=Vector3r(x-step,y     ,waveVals[i-1][j  ]);} else{ p4=p0;};
			n1 = (p2-p0).cross(p1-p0);
			n2 = (p3-p0).cross(p2-p0);
			n3 = (p4-p0).cross(p3-p0);
			n4 = (p1-p0).cross(p4-p0);
			int count=0;
			if(n1.squaredNorm()>0) { n1.normalize(); count++;};
			if(n2.squaredNorm()>0) { n2.normalize(); count++;};
			if(n3.squaredNorm()>0) { n3.normalize(); count++;};
			if(n4.squaredNorm()>0) { n4.normalize(); count++;};
			wavNormV[i][j] = Vector3r(n1+n2+n3+n4)/(1.0*count);
		}
	}
}

void Gl1_QMGeometryDisplay::prepareGlSurfaceMaterial()
{
	GLfloat mat[4];
	mat[0] = renderSpecular/100.0;
	mat[1] = renderSpecular/100.0;
	mat[2] = renderSpecular/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat);
	mat[0] = renderAmbient/100.0;
	mat[1] = renderAmbient/100.0;
	mat[2] = renderAmbient/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat);
	mat[0] = renderDiffuse/100.0;
	mat[1] = renderDiffuse/100.0;
	mat[2] = renderDiffuse/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat);
	GLfloat sh=renderShininess;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,&sh);
}

void Gl1_QMGeometryDisplay::glDrawSurface(
	const std::vector<std::vector<Real> >& waveVals,      // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	const std::vector<std::vector<Vector3r> >& wavNormV,  // normal vectors necessary for proper OpenGL rendering of the faces
	Vector3r col                                          // color in which to draw the surface
)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	int lenX=wavNormV.size();
	int lenY=wavNormV[0].size();
	//FIXME - end

	// now draw surface
	prepareGlSurfaceMaterial();

	glEnable(GL_CULL_FACE);
	if(renderSmoothing){ glShadeModel(GL_SMOOTH);};
	//draw front
	glCullFace(GL_BACK);
	glColor3v(col);
	for(int i=0 ; i<lenX-1 ; i++ )
	{
		Real x=startX+i*step;
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=0 ; j<lenY ; j++ )
		{
			Real y=startY+j*step;
			glNormal3v(         wavNormV[i  ][j]);
			glVertex3f(x     ,y,waveVals[i  ][j]);
			glNormal3v(         wavNormV[i+1][j]);
			glVertex3f(x+step,y,waveVals[i+1][j]);
		}
		glEnd();
	}
	//********************* draw back side of this surface
	//glCullFace(GL_BACK); // unnecessary
	glColor3v(Vector3r(col.cwiseProduct(Vector3r(0.5,0.5,0.5)))); // back has darker colors
	for(int i=0 ; i<lenX-1 ; i++ )
	{
		Real x=startX+i*step;
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=lenY-1 ; j>=0 ; j-- )
		{
			Real y=startY+j*step;
			glNormal3v(         wavNormV[i  ][j]);
			glVertex3f(x     ,y,waveVals[i  ][j]);
			glNormal3v(         wavNormV[i+1][j]);
			glVertex3f(x+step,y,waveVals[i+1][j]);
		}
		glEnd();
	}
	glShadeModel(GL_FLAT);
}
		
void Gl1_QMGeometryDisplay::glDrawSurfaceInterpolated(
	const std::vector<std::vector<Real> >&     waveVals,      // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	const std::vector<std::vector<Vector3r> >& wavNormV,      // normal vectors necessary for proper OpenGL rendering of the faces
	const std::vector<std::vector<Real> >&     extraWaveVals, // same, but shifted by +0.5,+0.5
	const std::vector<std::vector<Vector3r> >& extraWavNormV, // same, but shifted by +0.5,+0.5
	Vector3r col                                              // color in which to draw the surface
)
{
	Real step2 = step*0.5;
	Real step3 = step*1.5;
	Real step4 = step*2.0;
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	int lenX=wavNormV.size();
	int lenY=wavNormV[0].size();
	const int CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36
	//FIXME - end

	// now draw surface
	prepareGlSurfaceMaterial();

	glEnable(GL_CULL_FACE);
	if(renderSmoothing){ glShadeModel(GL_SMOOTH);};
	//draw front
	glCullFace(GL_BACK);
	glColor3v(col);
//
//   2 ... 4  ? (8)  triangles marked with "?" are not drawn in this loop
//   | \ / | ⋱ ⋰ |
// i ↑  3  ↓  6  ↑   the (7) and (8) are done in the next loop
// x | ⋰ ⋱ | / \ |
// ↑ 1  ?  5 ...(7)
// +→y,j
	for(int i=CHOSEN_RANGE/2 ; i<(lenX-1-CHOSEN_RANGE/2) ; i++ ) // skip margin CHOSEN_RANGE/2 where interpolation was impossible
	{
		Real x=startX+i*step;
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=CHOSEN_RANGE/2 ; j<(lenY-CHOSEN_RANGE/2-2) ; j+=2 /* must draw two quadrants at a time */ )
		{
			Real y=startY+j*step;
			glNormal3v(                     wavNormV[i  ][j  ]); // 1
			glVertex3f(x      ,y      ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                     wavNormV[i+1][j  ]); // 2
			glVertex3f(x+step ,y      ,     waveVals[i+1][j  ]); // 2
			glNormal3v(                extraWavNormV[i  ][j  ]); // 3
			glVertex3f(x+step2,y+step2,extraWaveVals[i  ][j  ]); // 3
			glNormal3v(                     wavNormV[i+1][j+1]); // 4
			glVertex3f(x+step ,y+step ,     waveVals[i+1][j+1]); // 4
			glNormal3v(                     wavNormV[i  ][j+1]); // 5
			glVertex3f(x      ,y+step ,     waveVals[i  ][j+1]); // 5
			glNormal3v(                extraWavNormV[i  ][j+1]); // 6
			glVertex3f(x+step2,y+step3,extraWaveVals[i  ][j+1]); // 6
			if((j+2)>=(lenY-CHOSEN_RANGE/2-2)) { // near the end draw the (7) and (8)
			glNormal3v(                     wavNormV[i  ][j+2]); // 7
			glVertex3f(x      ,y+step4,     waveVals[i  ][j+2]); // 7
			glNormal3v(                     wavNormV[i+1][j+2]); // 8
			glVertex3f(x+step ,y+step4,     waveVals[i+1][j+2]); // 8
			}
		}
		glEnd();
// now draw triangles marked with "?"
		glBegin(GL_TRIANGLES);
		for(int j=CHOSEN_RANGE/2 ; j<(lenY-CHOSEN_RANGE/2-2) ; j+=2 /* must draw two quadrants at a time */ )
		{
			Real y=startY+j*step;
			glNormal3v(                     wavNormV[i  ][j  ]); // 1
			glVertex3f(x      ,y      ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                extraWavNormV[i  ][j  ]); // 3
			glVertex3f(x+step2,y+step2,extraWaveVals[i  ][j  ]); // 3
			glNormal3v(                     wavNormV[i  ][j+1]); // 5
			glVertex3f(x      ,y+step ,     waveVals[i  ][j+1]); // 5

			glNormal3v(                     wavNormV[i+1][j+1]); // 4
			glVertex3f(x+step ,y+step ,     waveVals[i+1][j+1]); // 4
			glNormal3v(                     wavNormV[i+1][j+2]); // 8
			glVertex3f(x+step ,y+step4,     waveVals[i+1][j+2]); // 8
			glNormal3v(                extraWavNormV[i  ][j+1]); // 6
			glVertex3f(x+step2,y+step3,extraWaveVals[i  ][j+1]); // 6
		}
		glEnd();
	}
	//********************* draw back side of this surface
	//glCullFace(GL_BACK); // unnecessary
	glColor3v(Vector3r(col.cwiseProduct(Vector3r(0.5,0.5,0.5)))); // back has darker colors
	for(int i=CHOSEN_RANGE/2 ; i<(lenX-1-CHOSEN_RANGE/2) ; i++ )
	{
		Real x=startX+i*step;
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=(lenY-CHOSEN_RANGE/2-1/*3*/) ; j>CHOSEN_RANGE/2 ; j-=2 )
		{
			Real y=startY+j*step;
			glNormal3v(                     wavNormV[i  ][j  ]); // 1
			glVertex3f(x      ,y      ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                     wavNormV[i+1][j  ]); // 2
			glVertex3f(x+step ,y      ,     waveVals[i+1][j  ]); // 2
			glNormal3v(                extraWavNormV[i  ][j-1]); // 3
			glVertex3f(x+step2,y-step2,extraWaveVals[i  ][j-1]); // 3
			glNormal3v(                     wavNormV[i+1][j-1]); // 4
			glVertex3f(x+step ,y-step ,     waveVals[i+1][j-1]); // 4
			glNormal3v(                     wavNormV[i  ][j-1]); // 5
			glVertex3f(x      ,y-step ,     waveVals[i  ][j-1]); // 5
			glNormal3v(                extraWavNormV[i  ][j-2]); // 6
			glVertex3f(x+step2,y-step3,extraWaveVals[i  ][j-2]); // 6
			if((j-2)<=(CHOSEN_RANGE/2)) { // near the end draw the (7) and (8)
			glNormal3v(                     wavNormV[i  ][j-2]); // 7
			glVertex3f(x      ,y-step4,     waveVals[i  ][j-2]); // 7
			glNormal3v(                     wavNormV[i+1][j-2]); // 8
			glVertex3f(x+step ,y-step4,     waveVals[i+1][j-2]); // 8
			}
		}
		glEnd();
// now draw triangles marked with "?"
		glBegin(GL_TRIANGLES);
		for(int j=(lenY-CHOSEN_RANGE/2-1/*3*/) ; j>CHOSEN_RANGE/2 ; j-=2 )
		{
			Real y=startY+j*step;
			glNormal3v(                     wavNormV[i  ][j  ]); // 1
			glVertex3f(x      ,y      ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                extraWavNormV[i  ][j-1]); // 3
			glVertex3f(x+step2,y-step2,extraWaveVals[i  ][j-1]); // 3
			glNormal3v(                     wavNormV[i  ][j-1]); // 5
			glVertex3f(x      ,y-step ,     waveVals[i  ][j-1]); // 5

			glNormal3v(                     wavNormV[i+1][j-1]); // 4
			glVertex3f(x+step ,y-step ,     waveVals[i+1][j-1]); // 4
			glNormal3v(                     wavNormV[i+1][j-2]); // 8
			glVertex3f(x+step ,y-step4,     waveVals[i+1][j-2]); // 8
			glNormal3v(                extraWavNormV[i  ][j-2]); // 6
			glVertex3f(x+step2,y-step3,extraWaveVals[i  ][j-2]); // 6
		}
		glEnd();
	}
	glShadeModel(GL_FLAT);
}

void Gl1_QMGeometryDisplay::interpolateExtraWaveValues(
	const std::vector<std::vector<Real> >& waveVals,// a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	std::vector<std::vector<Real> >& extraWaveVals  // a 2D matrix shifted by +0.5,+0.5 from the previous one.
)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	const int CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36
	int lenX=waveVals.size();
	int lenY=waveVals[0].size();
	// FIXME - end

	for(int i=CHOSEN_RANGE/2 ; i<(lenX-CHOSEN_RANGE/2) ; i++ ) // skip borders of width CHOSEN_RANGE/2 - cannot interpolate there
	{
		for(int j=CHOSEN_RANGE/2 ; j<(lenY-CHOSEN_RANGE/2) ; j++ )
		{
			extraWaveVals[i][j]=spline6InterpolatePoint2D<Real,Real>(waveVals,i+0.5,j+0.5);
		}
	}
}

void Gl1_QMGeometryDisplay::interpolateExtraNormalVectors(
	const std::vector<std::vector<Vector3r> >& wavNormV,// a 2D matrix of normal vectors necessary for proper OpenGL rendering of the faces
	std::vector<std::vector<Vector3r> >& extraWavNormV  // a 2D matrix shifted by +0.5,+0.5 from the previous one
)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	const int CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36
	int lenX=wavNormV.size();
	int lenY=wavNormV[0].size();
	// FIXME - end

	for(int i=CHOSEN_RANGE/2 ; i<(lenX-CHOSEN_RANGE/2) ; i++ ) // skip borders of width CHOSEN_RANGE/2 - cannot interpolate there
	{
		for(int j=CHOSEN_RANGE/2 ; j<(lenY-CHOSEN_RANGE/2) ; j++ )
		{
			extraWavNormV[i][j]=spline6InterpolatePoint2D<Vector3r,Real>(wavNormV,i+0.5,j+0.5);
		}
	}
}

void Gl1_QMGeometryDisplay::drawSurface(const std::vector<std::vector<Real> >& waveVals,Vector3r col)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	int lenX=waveVals.size();
	int lenY=waveVals[0].size();
	// FIXME - end

	std::vector<std::vector<Vector3r> > wavNormV;
	wavNormV.resize(int((endX-startX)/step)+1);
	FOREACH(std::vector<Vector3r>& xx, wavNormV) {xx.resize(int((endX-startX)/step)+1,Vector3r(0,0,0));};

	calcNormalVectors(waveVals,wavNormV);

	if(not renderInterpolate)
	{
		glDrawSurface(waveVals,wavNormV,col);
	} else {
		std::vector<std::vector<Vector3r> > extraWavNormV;
		extraWavNormV.resize(int((endX-startX)/step)+1);
		FOREACH(std::vector<Vector3r>& xx, extraWavNormV) {xx.resize(int((endX-startX)/step)+1,Vector3r(0,0,0));};
	
		std::vector<std::vector<Real> > extraWaveVals;
		extraWaveVals.resize(int((endX-startX)/step)+1);
		FOREACH(std::vector<Real>& xx, extraWaveVals) {xx.resize(int((endX-startX)/step)+1,0);};

		interpolateExtraWaveValues(waveVals,extraWaveVals);
		interpolateExtraNormalVectors(wavNormV,extraWavNormV);
		
		glDrawSurfaceInterpolated(waveVals,wavNormV,extraWaveVals,extraWavNormV,col);
	}

}

#endif


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL

// This will come later, when I will have some interactions going on.... FIXME - draw potentials !!
//	CREATE_LOGGER(Gl1_QMInteractionPhysics);
//	bool Gl1_QMInteractionPhysics::abs=true;
//	bool Gl1_QMInteractionPhysics::real=false;
//	bool Gl1_QMInteractionPhysics::imag=false;
//	Gl1_QMInteractionPhysics::~Gl1_QMInteractionPhysics(){};
//	void Gl1_QMInteractionPhysics::go(const shared_ptr<IPhys>& ip, const shared_ptr<Interaction>& i, const shared_ptr<Body>& b1, const shared_ptr<Body>& b2, bool wireFrame)
//	{
//	}

#endif


