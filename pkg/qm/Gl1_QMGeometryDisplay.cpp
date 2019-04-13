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
Gl1_QMGeometryDisplay::~Gl1_QMGeometryDisplay(){};

Gl1_QMGeometryDisplay::Gl1_QMGeometryDisplay()
{
	partsToDraw.resize(0);
	partsToDraw.push_back( [this](){ return menuSelection(g->partReal     )!="hidden" and not g->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(g->partImaginary)!="hidden" and not g->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(g->partAbsolute )!="hidden" and not g->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(g->partReal     )!="hidden" and     g->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(g->partImaginary)!="hidden" and     g->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(g->partAbsolute )!="hidden" and     g->partsSquared; } );

	drawStyle.resize(0);
	drawStyle.push_back  ( [this](){ return menuSelection(g->partReal     );                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(g->partImaginary);                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(g->partAbsolute );                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(g->partReal     );                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(g->partImaginary);                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(g->partAbsolute );                                   } );

	valueToDraw.resize(0);
	valueToDraw.push_back( [](std::complex<Real> a){ return std::real(a);                                      } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::imag(a);                                      } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::abs(a);                                       } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::pow(std::real(a),2);                          } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::pow(std::imag(a),2);                          } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::real(a*std::conj(a));                         } );

	colorToDraw.resize(0);
	colorToDraw.push_back( [](Vector3r col)        { return Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0))); } ); // display partReal               in bluish   color
	colorToDraw.push_back( [](Vector3r col)        { return Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4))); } ); // display partImaginary          in reddish  color
	colorToDraw.push_back( [](Vector3r col)        { return col;                                               } ); // display abs value              original color
	colorToDraw.push_back( [](Vector3r col)        { return Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0))); } ); // display partReal      (squred) in bluish   color
	colorToDraw.push_back( [](Vector3r col)        { return Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4))); } ); // display partImaginary (square) in reddish  color
	colorToDraw.push_back( [](Vector3r col)        { return col;                                               } ); // for probability use            original color
};

void Gl1_QMGeometryDisplay::go(
	const shared_ptr<Shape>& shape, 
	const shared_ptr<State>& state,
	bool wire,
	const GLViewInfo&
)
{
	timeLimit.readWallClock();

// FIXME - do NOT declare new variable, lost 2 hours here, how to avoid that?
	g = static_cast<QMGeometryDisplay*>(shape.get());
	if(menuSelection(g->partAbsolute)=="hidden" and menuSelection(g->partReal)=="hidden" and menuSelection(g->partImaginary)=="hidden") return; // nothing to draw

// FIXME - careful! use different names than class members!!!
	QMState*           packet         = static_cast<QMState*>(state.get());
	QMStateDiscrete*   packetDiscrete = dynamic_cast<QMStateDiscrete*>(state.get());
	Vector3r col = g->color;
	
// find extents to render
// FIXME(2) maybe move that into some renderConfig class, with default values in O.body[#].shape and override in this class.
	startX= -g->halfSize[0]; // + pos[0] ← FIXME
	endX  =  g->halfSize[0]; // + pos[0] ← FIXME
	startY= -g->halfSize[1]; // + pos[1] ← FIXME
	endY  =  g->halfSize[1]; // + pos[1] ← FIXME
	startZ= -g->halfSize[2]; // + pos[2] ← FIXME
	endZ  =  g->halfSize[2]; // + pos[2] ← FIXME

// FIXME(2) - allow to set some step in renderConfig for QMStateAnalytic in O.body.shape
	if(packetDiscrete) {
		                    g->step.x()=packetDiscrete->stepInPositionalRepresentation(0);
		if(packet->dim > 1) g->step.y()=packetDiscrete->stepInPositionalRepresentation(1);
		if(packet->dim > 2) g->step.z()=packetDiscrete->stepInPositionalRepresentation(2);
	
//OK	std::cerr << startX                   << " " << endX                   << "\n" 
//OK		  << packetDiscrete->start(0) << " " << packetDiscrete->end(0) << "\n" 
//OK	          << startY                   << " " << endY                   << "\n" 
//OK		  << packetDiscrete->start(1) << " " << packetDiscrete->end(1) << "\n" 
//OK		  << startZ                   << " " << endZ                   << "\n" 
//OK		  << packetDiscrete->start(2) << " " << packetDiscrete->end(2) << "\n"; 

	}

// FIXME(2) - perform here all requested tensor contractions: 3D→2D→1D, and slicing. Or maybe in O.body.shape, according to renderConfig?

	Real scalingFactor = (g->partsScale >= 0 ? ((g->partsScale==0)?(1):(g->partsScale)) : -1.0/g->partsScale);
	for(int draw=0 ; draw<partsToDraw.size() ; draw++) {
		if( partsToDraw[draw]() ) {
			switch(packet->dim) {
				// FIXME(2) - add following
				// 1D phase  , 2D phase  , 3D phase
				// 1D argand , 2D argand , 3D Dirac-Argand
				// FIXME(2) - add drawing momentum space
				case 1 :
					// FIXME(2) - add points, with point density reflecting the value
					// 1D points ← points scattered randomly, using density
					// 1D nodes  ← just points at the nodes
					// 1D big nodes  ← small spheres at the nodes, radius 1/10 of smallest step_x,step_y,step_z
					// if(points == true) ... else ...
					// 1D lines
					if(drawStyle[draw]()!="points") {
						if(drawStyle[draw]()=="nodes") glBegin(GL_POINTS); else glBegin(GL_LINE_STRIP);
						glColor3v( colorToDraw[draw](col) );
						for(Real x=startX ; x<endX ; x+=g->step.x() ) {
							glVertex3d(x,0,valueToDraw[draw] ((packet->getValPos(Vector3r(x,0,0)))) *scalingFactor);
						}
						glEnd();
					} // else "points"
				break;

				case 2:
					// FIXME(2) - add points, with point density reflecting the value
					// 2D points
					// if(points == true) ... else ...
					// 2D lines
					if(wire == true or drawStyle[draw]()=="wire") {
						glColor3v( colorToDraw[draw](col) );
						// FIXME!! ↓ to jest s założenia źle, bo albo rysuję dyskretną albo analitycznyą. 
						// FIXME!! ↓ Nie mogę tak wyznaczać rozmiaru ↓
						for(Real x=startX ; x< (endX - g->step.x()*0.5) ; x+=g->step.x()/*,i++ */ ) {
							glBegin(GL_LINE_STRIP);
							for(Real y=startY ; y< (endY - g->step.y()*0.5) ; y+=g->step.y()/*,j++*/ ) {
								glVertex3d(x,y,valueToDraw[draw] ((packet->getValPos(Vector3r(x,y,0)))) *scalingFactor);
							}
							glEnd();
							if(timeLimit.tooLong(g->stepWait)) break;
						}
						// FIXME!! ↓ to jest s założenia źle, bo albo rysuję dyskretną albo analitycznyą. 
						// FIXME!! ↓ Nie mogę tak wyznaczać rozmiaru ↓
						for(Real y=startY ; y< (endY - g->step.y()*0.5) ; y+=g->step.y() ) {
							glBegin(GL_LINE_STRIP);
							for(Real x=startX ; x< (endX - g->step.x()*0.5) ; x+=g->step.x() ) {
								glVertex3d(x,y,valueToDraw[draw] ((packet->getValPos(Vector3r(x,y,0)))) *scalingFactor);
							}
							glEnd();
							if(timeLimit.tooLong(g->stepWait)) break;
						}
						// FIXME - drawing wires is slower than drawSurface. Better to prepare data for wires, then call drawWires()
					} else {
					// 2D surface
						// FIXME!! ↓ to jest s założenia źle, bo albo rysuję dyskretną albo analitycznyą. 
						// FIXME!! ↓ Nie mogę tak wyznaczać rozmiaru ↓
						waveValues2D.resize(int((endX-startX)/g->step.x()));
						FOREACH(std::vector<Real>& xx, waveValues2D) {xx.resize(int((endY-startY)/g->step.y()),0);};
						int i=0;
						// FIXME!! ↓ to jest s założenia źle, bo albo rysuję dyskretną albo analitycznyą. 
						// FIXME!! ↓ Nie mogę tak wyznaczać rozmiaru ↓
						for(Real x=startX ; x< (endX - g->step.x()*0.5) ; x+=g->step.x(),i++ ) {
							int j=0;
							for(Real y=startY ; y< (endY - g->step.y()*0.5) ; y+=g->step.y(),j++ ) {
								waveValues2D[i][j]=valueToDraw[draw] ((packet->getValPos(Vector3r(x,y,0))))*scalingFactor;
							}
							if(timeLimit.tooLong(g->stepWait)) break;
						}
						drawSurface(waveValues2D,colorToDraw[draw](col));
					}
				break;

				case 3:
					// FIXME(2) - add points, with point density reflecting the value
					// 3D points
					// if(points == true) ... else ...
					// 3D lines
					// 3D surface
					if(true /* points == false */) {
						// FIXME!! ↓ to jest s założenia źle, bo albo rysuję dyskretną albo analitycznyą. 
						// FIXME!! ↓ Nie mogę tak wyznaczać rozmiaru ↓
						int gridSizex=int((endX-startX)/g->step.x());
						int gridSizey=int((endY-startY)/g->step.y());
						int gridSizez=int((endZ-startZ)/g->step.z());
						// FIXME(2) - reconsider if doing [draw] loop outside this if() slows things down - more reinitialization of mc 
						// FIXME(3) -                     ↓ +pos[0]                    ↓ +pos[1]                    ↓ + pos[2]
						Vector3r minMC(startX+g->step.x()*0.5         ,startY+g->step.y()*0.5         ,startZ+g->step.z()*0.5          );
						Vector3r maxMC(endX  +g->step.x()*0.5         ,endY  +g->step.y()*0.5         ,endZ  +g->step.z()*0.5          );
						mc.init(gridSizex,gridSizey,gridSizez,minMC,maxMC);
						// about waveValues3D FIXME(2) - resolve storage problems
						mc.resizeScalarField(waveValues3D,gridSizex,gridSizey,gridSizez);

						// FIXME!! ↓ to jest s założenia źle, bo albo rysuję dyskretną albo analitycznyą. 
						// FIXME!! ↓ Nie mogę tak wyznaczać rozmiaru ↓
						int i=0;
						for(Real x=startX ; x < (endX - g->step.x()*0.5) ; x+=g->step.x(),i++ ) {
							int j=0;
							for(Real y=startY ; y < (endY - g->step.y()*0.5) ; y+=g->step.y(),j++ ) {
								int k=0;
								for(Real z=startZ ; z < (endZ - g->step.z()*0.5) ; z+=g->step.z(),k++ ) {
									// FIXME(2) - to jest kopiowanie!
									// owszem - bez FFTW3 takie coś musi zostać, ale
									// z nowymi kontenerami muszę móc to ominąć
									// (potrzebne mi też będą kontrakcje na życzenie - tylko gdy rysuję)
//// FIXME(2) - should instead give just `const ref&` to this table, but GLDraw has problem with endXYZ - it draws one element too much!! (outside table bounds)
									waveValues3D[i][j][k]=valueToDraw[draw] (packet->getValPos(Vector3r(x,y,z)));
								}
							}
							if(timeLimit.tooLong(g->stepWait)) break;
						}
						mc.computeTriangulation(waveValues3D,g->threshold3D);
						// FIXME(2) - drawSurface or drawWires
						glDrawMarchingCube(mc,colorToDraw[draw](col));
					}
				break;

				default:
					if(timeLimit.messageAllowed(5))
						std::cerr << "4D or more dimensions plotting is not ready yet\n";
				break;
			}
		}
	}

};
		
void Gl1_QMGeometryDisplay::glDrawMarchingCube(MarchingCube& mc,Vector3r col)
{
	prepareGlSurfaceMaterial();
	glColor3v(col);

	const vector<Vector3r>& triangles 	= mc.getTriangles();
	int nbTriangles				= mc.getNbTriangles();
	const vector<Vector3r>& normals 	= mc.getNormals();	
	if(g->renderSmoothing){ glShadeModel(GL_SMOOTH);};
	glEnable(GL_NORMALIZE);
	glBegin(GL_TRIANGLES);
	for(int i=0;i<3*nbTriangles;++i)
	{
		glNormal3v(normals[  i]);
		glVertex3v(triangles[i]);
		glNormal3v(normals[++i]);
		glVertex3v(triangles[i]);
		glNormal3v(normals[++i]);
		glVertex3v(triangles[i]);
	}
	glEnd();

// FIXME(2) - maybe draw using this approach instead?  http://webhome.csc.uvic.ca/~pouryash/depot/HPC_Course/OpenGLDrawingMethods.pdf
//glColorPointer(3, GL_FLOAT,0,mesh.vColor);
//glEnableClientState(GL_COLOR_ARRAY);
//glNormalPointer(GL_FLOAT, 0,mesh.vNorm);
//glEnableClientState (GL_NORMAL_ARRAY);
//glVertexPointer (3 , GL_FLOAT, 0, mesh.vPos);
//glEnableClientState (GL_VERTEX_ARRAY);
//glDrawElements (GL_TRIANGLES, ( GLsizei) mesh.  ctTriangles * 3, GL_UNSIGNED_SHORT , mesh.triangles);
//glDisableClientState (GL_COLOR_ARRAY);
//glDisableClientState (GL_NORMAL_ARRAY);
//glDisableClientState (GL_VERTEX_ARRAY);

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

	// FIXME!! ↓ to jest s założenia źle, bo albo rysuję dyskretną albo analitycznyą. 
	// FIXME!! ↓ Nie mogę tak wyznaczać rozmiaru ↓
	int i=0;
	for(Real x=startX ; x < (endX - g->step.x()*0.5) ; x+=g->step.x(),i++ )
	{
		int j=0;
		for(Real y=startY ; y < (endY - g->step.y()*0.5) ; y+=g->step.y(),j++ )
		{
			                p0=Vector3r(x            ,y            ,waveVals[i  ][j  ]);
			if((j+1)<lenY){ p1=Vector3r(x            ,y+g->step.y(),waveVals[i  ][j+1]);} else{ p1=p0;};
			if((i+1)<lenX){ p2=Vector3r(x+g->step.x(),y            ,waveVals[i+1][j  ]);} else{ p2=p0;};
			if((j-1)>=0  ){ p3=Vector3r(x            ,y-g->step.y(),waveVals[i  ][j-1]);} else{ p3=p0;};
			if((i-1)>=0  ){ p4=Vector3r(x-g->step.x(),y            ,waveVals[i-1][j  ]);} else{ p4=p0;};
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
	mat[0] = g->renderSpecular/100.0;
	mat[1] = g->renderSpecular/100.0;
	mat[2] = g->renderSpecular/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat);
	mat[0] = g->renderAmbient/100.0;
	mat[1] = g->renderAmbient/100.0;
	mat[2] = g->renderAmbient/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat);
	mat[0] = g->renderDiffuse/100.0;
	mat[1] = g->renderDiffuse/100.0;
	mat[2] = g->renderDiffuse/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat);
	GLfloat sh=g->renderShininess;
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
	if(g->renderSmoothing){ glShadeModel(GL_SMOOTH);};
	//draw front
	glCullFace(GL_BACK);
	glColor3v(col);
	for(int i=0 ; i<lenX-1 ; i++ )
	{
		Real x=startX+i*g->step.x();
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=0 ; j<lenY ; j++ )
		{
			Real y=startY+j*g->step.y();
			glNormal3v(                wavNormV[i  ][j]);
			glVertex3f(x            ,y,waveVals[i  ][j]);
			glNormal3v(                wavNormV[i+1][j]);
			glVertex3f(x+g->step.x(),y,waveVals[i+1][j]);
		}
		glEnd();
	}
	//********************* draw back side of this surface
	//glCullFace(GL_BACK); // unnecessary
	glColor3v(Vector3r(col.cwiseProduct(Vector3r(0.5,0.5,0.5)))); // back has darker colors
	for(int i=0 ; i<lenX-1 ; i++ )
	{
		Real x=startX+i*g->step.x();
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=lenY-1 ; j>=0 ; j-- )
		{
			Real y=startY+j*g->step.y();
			glNormal3v(                wavNormV[i  ][j]);
			glVertex3f(x            ,y,waveVals[i  ][j]);
			glNormal3v(                wavNormV[i+1][j]);
			glVertex3f(x+g->step.x(),y,waveVals[i+1][j]);
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
	Real stepx  = g->step.x()    ;
	Real stepx2 = g->step.x()*0.5;
	Real stepy  = g->step.y()    ;
	Real stepy2 = g->step.y()*0.5;
	Real stepy3 = g->step.y()*1.5;
	Real stepy4 = g->step.y()*2.0;
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	int lenX=wavNormV.size();
	int lenY=wavNormV[0].size();
	const int CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36
	//FIXME - end

	// now draw surface
	prepareGlSurfaceMaterial();

	glEnable(GL_CULL_FACE);
	if(g->renderSmoothing){ glShadeModel(GL_SMOOTH);};
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
		Real x=startX+i*stepx;
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=CHOSEN_RANGE/2 ; j<(lenY-CHOSEN_RANGE/2-2) ; j+=2 /* must draw two quadrants at a time */ )
		{
			Real y=startY+j*stepy;
			glNormal3v(                       wavNormV[i  ][j  ]); // 1
			glVertex3f(x       ,y       ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                       wavNormV[i+1][j  ]); // 2
			glVertex3f(x+stepx ,y       ,     waveVals[i+1][j  ]); // 2
			glNormal3v(                  extraWavNormV[i  ][j  ]); // 3
			glVertex3f(x+stepx2,y+stepy2,extraWaveVals[i  ][j  ]); // 3
			glNormal3v(                       wavNormV[i+1][j+1]); // 4
			glVertex3f(x+stepx ,y+stepy ,     waveVals[i+1][j+1]); // 4
			glNormal3v(                       wavNormV[i  ][j+1]); // 5
			glVertex3f(x       ,y+stepy ,     waveVals[i  ][j+1]); // 5
			glNormal3v(                  extraWavNormV[i  ][j+1]); // 6
			glVertex3f(x+stepx2,y+stepy3,extraWaveVals[i  ][j+1]); // 6
			if((j+2)>=(lenY-CHOSEN_RANGE/2-2)) { // near the end draw the (7) and (8)
			glNormal3v(                       wavNormV[i  ][j+2]); // 7
			glVertex3f(x       ,y+stepy4,     waveVals[i  ][j+2]); // 7
			glNormal3v(                       wavNormV[i+1][j+2]); // 8
			glVertex3f(x+stepx ,y+stepy4,     waveVals[i+1][j+2]); // 8
			}
		}
		glEnd();
// now draw triangles marked with "?"
		glBegin(GL_TRIANGLES);
		for(int j=CHOSEN_RANGE/2 ; j<(lenY-CHOSEN_RANGE/2-2) ; j+=2 /* must draw two quadrants at a time */ )
		{
			Real y=startY+j*stepy;
			glNormal3v(                       wavNormV[i  ][j  ]); // 1
			glVertex3f(x       ,y       ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                  extraWavNormV[i  ][j  ]); // 3
			glVertex3f(x+stepx2,y+stepy2,extraWaveVals[i  ][j  ]); // 3
			glNormal3v(                       wavNormV[i  ][j+1]); // 5
			glVertex3f(x       ,y+stepy ,     waveVals[i  ][j+1]); // 5

			glNormal3v(                       wavNormV[i+1][j+1]); // 4
			glVertex3f(x+stepx ,y+stepy ,     waveVals[i+1][j+1]); // 4
			glNormal3v(                       wavNormV[i+1][j+2]); // 8
			glVertex3f(x+stepx ,y+stepy4,     waveVals[i+1][j+2]); // 8
			glNormal3v(                  extraWavNormV[i  ][j+1]); // 6
			glVertex3f(x+stepx2,y+stepy3,extraWaveVals[i  ][j+1]); // 6
		}
		glEnd();
	}
	//********************* draw back side of this surface
	//glCullFace(GL_BACK); // unnecessary
	glColor3v(Vector3r(col.cwiseProduct(Vector3r(0.5,0.5,0.5)))); // back has darker colors
	for(int i=CHOSEN_RANGE/2 ; i<(lenX-1-CHOSEN_RANGE/2) ; i++ )
	{
		Real x=startX+i*stepx;
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=(lenY-CHOSEN_RANGE/2-1/*3*/) ; j>CHOSEN_RANGE/2 ; j-=2 )
		{
			Real y=startY+j*stepy;
			glNormal3v(                       wavNormV[i  ][j  ]); // 1
			glVertex3f(x       ,y       ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                       wavNormV[i+1][j  ]); // 2
			glVertex3f(x+stepx ,y       ,     waveVals[i+1][j  ]); // 2
			glNormal3v(                  extraWavNormV[i  ][j-1]); // 3
			glVertex3f(x+stepx2,y-stepy2,extraWaveVals[i  ][j-1]); // 3
			glNormal3v(                       wavNormV[i+1][j-1]); // 4
			glVertex3f(x+stepx ,y-stepy ,     waveVals[i+1][j-1]); // 4
			glNormal3v(                       wavNormV[i  ][j-1]); // 5
			glVertex3f(x       ,y-stepy ,     waveVals[i  ][j-1]); // 5
			glNormal3v(                  extraWavNormV[i  ][j-2]); // 6
			glVertex3f(x+stepx2,y-stepy3,extraWaveVals[i  ][j-2]); // 6
			if((j-2)<=(CHOSEN_RANGE/2)) { // near the end draw the (7) and (8)
			glNormal3v(                       wavNormV[i  ][j-2]); // 7
			glVertex3f(x       ,y-stepy4,     waveVals[i  ][j-2]); // 7
			glNormal3v(                       wavNormV[i+1][j-2]); // 8
			glVertex3f(x+stepx ,y-stepy4,     waveVals[i+1][j-2]); // 8
			}
		}
		glEnd();
// now draw triangles marked with "?"
		glBegin(GL_TRIANGLES);
		for(int j=(lenY-CHOSEN_RANGE/2-1/*3*/) ; j>CHOSEN_RANGE/2 ; j-=2 )
		{
			Real y=startY+j*stepy;
			glNormal3v(                       wavNormV[i  ][j  ]); // 1
			glVertex3f(x       ,y       ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                  extraWavNormV[i  ][j-1]); // 3
			glVertex3f(x+stepx2,y-stepy2,extraWaveVals[i  ][j-1]); // 3
			glNormal3v(                       wavNormV[i  ][j-1]); // 5
			glVertex3f(x       ,y-stepy ,     waveVals[i  ][j-1]); // 5

			glNormal3v(                       wavNormV[i+1][j-1]); // 4
			glVertex3f(x+stepx ,y-stepy ,     waveVals[i+1][j-1]); // 4
			glNormal3v(                       wavNormV[i+1][j-2]); // 8
			glVertex3f(x+stepx ,y-stepy4,     waveVals[i+1][j-2]); // 8
			glNormal3v(                  extraWavNormV[i  ][j-2]); // 6
			glVertex3f(x+stepx2,y-stepy3,extraWaveVals[i  ][j-2]); // 6
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
	std::vector<std::vector<Vector3r> > wavNormV(/*size=*/waveVals.size(),/*init=*/std::vector<Vector3r>(waveVals[0].size()));
	calcNormalVectors(waveVals,wavNormV);
	if(not g->renderInterpolate)
	{
		glDrawSurface(waveVals,wavNormV,col);
	} else {
		std::vector<std::vector<Vector3r> > extraWavNormV(/*size=*/waveVals.size(),/*init=*/std::vector<Vector3r>(waveVals[0].size()));
		std::vector<std::vector<Real    > > extraWaveVals(/*size=*/waveVals.size(),/*init=*/std::vector<Real    >(waveVals[0].size()));
		interpolateExtraWaveValues   (waveVals,extraWaveVals);
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


