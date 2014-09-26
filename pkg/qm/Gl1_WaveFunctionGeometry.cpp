// 2014 © Janek Kozicki <cosurgi@gmail.com>

/*********************************************************************************
*
* W A V E   F U N C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL
#include "Gl1_WaveFunctionGeometry.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(Gl1_WaveFunctionGeometry)
	);

#include "WaveFunctionState.hpp"
#include <yade/pkg/common/GLDrawFunctors.hpp>
#include <yade/lib/opengl/OpenGLWrapper.hpp>
#include <yade/lib/opengl/GLUtils.hpp>

CREATE_LOGGER(Gl1_WaveFunctionGeometry);
bool Gl1_WaveFunctionGeometry::absolute=true;
bool Gl1_WaveFunctionGeometry::partReal=true;
bool Gl1_WaveFunctionGeometry::partImaginary=true;
bool Gl1_WaveFunctionGeometry::probability=true;
bool Gl1_WaveFunctionGeometry::renderSmoothing=true;
bool Gl1_WaveFunctionGeometry::renderInterpolate=true;
int  Gl1_WaveFunctionGeometry::renderSpecular=10;
int  Gl1_WaveFunctionGeometry::renderAmbient=15;
int  Gl1_WaveFunctionGeometry::renderDiffuse=100;
int  Gl1_WaveFunctionGeometry::renderShininess=50;
Gl1_WaveFunctionGeometry::~Gl1_WaveFunctionGeometry(){};

void Gl1_WaveFunctionGeometry::go(
	const shared_ptr<Shape>& shape, 
	const shared_ptr<State>& state,
	bool wire,
	const GLViewInfo&
)
{
	if(not(absolute or partReal or partImaginary or probability)) return; // nothing to draw
	GaussianAnalyticalPropagatingWavePacket_1D* packet=dynamic_cast<GaussianAnalyticalPropagatingWavePacket_1D*>(state.get());
	Vector3r col = shape->color;
/* 1D LINES
	if(partReal) {
		glBegin(GL_LINE_STRIP);
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0))) ); // display partReal part in bluish color
		for(Real x=-4.0 ; x<4.0 ; x+=0.01 )
		{
			std::complex<Real> wfval = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
			glVertex3d(x,std::real(wfval),0);
		}
		glEnd();
	}

	if(imag) {
		glBegin(GL_LINE_STRIP);
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4))) ); // display imag part in reddish color
		for(Real x=-4.0 ; x<4.0 ; x+=0.01 )
		{
			std::complex<Real> wfval = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
			glVertex3d(x,std::imag(wfval),0);
		}
		glEnd();
	}

	if(abs) {
		glBegin(GL_LINE_STRIP);
		glColor3v(shape->color); // display abs in intended shape color
		for(Real x=-4.0 ; x<4.0 ; x+=0.01 )
		{
			std::complex<Real> wfval = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
			glVertex3d(x,std::abs(wfval),0);
		}
		glEnd();
	}
*/
// 2D SURFACE
	Real step=0.1;
	Real startX= -4.0;
	Real endX  =  4.0;
	Real startY= -4.0;
	Real endY  =  4.0;
	std::vector<std::vector<Real> > waveVals;
	waveVals.resize(int((endX-startX)/step)+1);
	FOREACH(std::vector<Real>& yy, waveVals) {yy.resize(int((endX-startX)/step)+1,0);};

	if(partReal) { // FIXME - merge those four into one loop
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				waveVals[i][j]=std::real(wfvalx0*wfvaly0);
			}
		}
		drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)))); // display partReal part in bluish color
	}
	if(partImaginary) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				waveVals[i][j]=std::imag(wfvalx0*wfvaly0);
			}
		}
		drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4)))); // display imag part in reddish color
	}
	if(absolute) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				waveVals[i][j]=std::abs(wfvalx0*wfvaly0);
			}
		}
		drawSurface(waveVals,Vector3r(col.cwiseProduct(Vector3r(0.4,1.0,0.4)))); // display abs value in geenish color
	}
	if(probability) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				waveVals[i][j]=std::real( (wfvalx0*wfvaly0)*std::conj(wfvalx0*wfvaly0) );
			}
		}
		drawSurface(waveVals,col);
	}

// 2D LINES
/* FIXME: add parameters, auto or specified by hand startX,endX (Y,Z), dX,dY,dZ
	if(partReal) {
		glColor3v(shape->color); // display abs in intended shape color
		for(Real x=-4.0 ; x<4.0 ; x+=0.1 )
		{
			glBegin(GL_LINE_STRIP);
			for(Real y=-4.0 ; y<4.0 ; y+=0.01 )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				glVertex3d(x    ,std::real(wfvalx0*wfvaly0),y);
			}
			glEnd();
		}
		for(Real y=-4.0 ; y<4.0 ; y+=0.1 )
		{
			glBegin(GL_LINE_STRIP);
			for(Real x=-4.0 ; x<4.0 ; x+=0.01 )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				glVertex3d(x    ,std::real(wfvalx0*wfvaly0),y);
			}
			glEnd();
		}
	}
*/
};

void Gl1_WaveFunctionGeometry::calcNormalVectors(
	const std::vector<std::vector<Real> >& waveVals, // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	std::vector<std::vector<Vector3r> >& wavNormV    // normal vectors necessary for propoer OpenGL rendering of the faces
)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	Real step  =  0.1;
	Real startX= -4.0;
	Real endX  =  4.0;
	Real startY= -4.0;
	Real endY  =  4.0;
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

void Gl1_WaveFunctionGeometry::prepareGlSurfaceMaterial()
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

void Gl1_WaveFunctionGeometry::glDrawSurface(
	const std::vector<std::vector<Real> >& waveVals,      // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	const std::vector<std::vector<Vector3r> >& wavNormV,  // normal vectors necessary for proper OpenGL rendering of the faces
	Vector3r col                                          // color in which to draw the surface
)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	Real step  =  0.1;
	Real startX= -4.0;
	Real endX  =  4.0;
	Real startY= -4.0;
	Real endY  =  4.0;
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
	//********************* draw back
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
		
void Gl1_WaveFunctionGeometry::glDrawSurfaceInterpolated(
	const std::vector<std::vector<Real> >&     waveVals,      // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	const std::vector<std::vector<Vector3r> >& wavNormV,      // normal vectors necessary for proper OpenGL rendering of the faces
	const std::vector<std::vector<Real> >&     extraWaveVals, // same, but shifted by +0.5,+0.5
	const std::vector<std::vector<Vector3r> >& extraWavNormV, // same, but shifted by +0.5,+0.5
	Vector3r col                                              // color in which to draw the surface
)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	Real step  =  0.1;
	Real step2 = step*0.5;
	Real step3 = step*1.5;
	Real step4 = step*2.0;
	Real startX= -4.0;
	Real endX  =  4.0;
	Real startY= -4.0;
	Real endY  =  4.0;
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
	//********************* draw back
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

Real Gl1_WaveFunctionGeometry::calcInterpolation_2D(
	const std::vector<std::vector<Real> >& val, // a 2D matrix for which the values will be interpolated
	Real posX,                            // position X at which to interpolate the values
	Real posY                             // position Y at which to interpolate the values
)
{
	Real ret=0;
	const int CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36
	const int STA=1-CHOSEN_RANGE/2; //-2-5;
	const int END=1+CHOSEN_RANGE/2; // 4+5;
	int      start_x=(int)(std::floor(posX))+STA
		,end_x  =(int)(std::floor(posX))+END
		,start_y=(int)(std::floor(posY))+STA
		,end_y  =(int)(std::floor(posY))+END;
	for(int x=start_x ; x<end_x ; ++x)
		for(int y=start_y ; y<end_y ; ++y)
			// same FIXME here - add option to select between sinc256 and spline36
			ret+=val[x][y]*spline36Interpolation((Real)(x)-posX)*spline36Interpolation((Real)(y)-posY);
	return ret;
}

Vector3r Gl1_WaveFunctionGeometry::calcInterpolation_2Dvector(
	const std::vector<std::vector<Vector3r> >& val, // a 2D matrix of Vector3r for which the values will be interpolated
	Real posX,                            // position X at which to interpolate the values
	Real posY                             // position Y at which to interpolate the values
)
{
	Vector3r ret(0,0,0);
	const int CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36
	const int STA=1-CHOSEN_RANGE/2; //-2-5;
	const int END=1+CHOSEN_RANGE/2; // 4+5;
	int      start_x=(int)(std::floor(posX))+STA
		,end_x  =(int)(std::floor(posX))+END
		,start_y=(int)(std::floor(posY))+STA
		,end_y  =(int)(std::floor(posY))+END;
	for(int x=start_x ; x<end_x ; ++x)
		for(int y=start_y ; y<end_y ; ++y)
			// same FIXME here - add option to select between sinc256 and spline36
			ret+=val[x][y]*spline36Interpolation((Real)(x)-posX)*spline36Interpolation((Real)(y)-posY);
	ret.normalize();
	return ret;
}

void Gl1_WaveFunctionGeometry::interpolateExtraWaveValues(
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
			extraWaveVals[i][j]=calcInterpolation_2D(waveVals,i+0.5,j+0.5);
		}
	}
}

void Gl1_WaveFunctionGeometry::interpolateExtraNormalVectors(
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
			extraWavNormV[i][j]=calcInterpolation_2Dvector(wavNormV,i+0.5,j+0.5);
		}
	}
}

void Gl1_WaveFunctionGeometry::drawSurface(const std::vector<std::vector<Real> >& waveVals,Vector3r col)
{
	//FIXME - get ranges from AABB or if not present - let user set them, and use some default.
	Real step  =  0.1;
	Real startX= -4.0;
	Real endX  =  4.0;
	Real startY= -4.0;
	Real endY  =  4.0;
	int lenX=waveVals.size();
	int lenY=waveVals[0].size();
	// FIXME - end

	std::vector<std::vector<Vector3r> > wavNormV;
	wavNormV.resize(int((endX-startX)/step)+1);
	FOREACH(std::vector<Vector3r>& yy, wavNormV) {yy.resize(int((endX-startX)/step)+1,Vector3r(0,0,0));};

	calcNormalVectors(waveVals,wavNormV);

	if(not renderInterpolate)
	{
		glDrawSurface(waveVals,wavNormV,col);
	} else {
		std::vector<std::vector<Vector3r> > extraWavNormV;
		extraWavNormV.resize(int((endX-startX)/step)+1);
		FOREACH(std::vector<Vector3r>& yy, extraWavNormV) {yy.resize(int((endX-startX)/step)+1,Vector3r(0,0,0));};
	
		std::vector<std::vector<Real> > extraWaveVals;
		extraWaveVals.resize(int((endX-startX)/step)+1);
		FOREACH(std::vector<Real>& yy, extraWaveVals) {yy.resize(int((endX-startX)/step)+1,0);};

		interpolateExtraWaveValues(waveVals,extraWaveVals);
		interpolateExtraNormalVectors(wavNormV,extraWavNormV);
		
		glDrawSurfaceInterpolated(waveVals,wavNormV,extraWaveVals,extraWavNormV,col);
	}

}

Real Gl1_WaveFunctionGeometry::spline36Interpolation(Real dist)
{ // http://www.path.unimelb.edu.au/%7Edersch/interpolator/interpolator.html
// this interpolation was also described in
// J. Kozicki , J. Tejchman , "Experimental investigations of strain
// localization in concrete using Digital Image Correlation (DIC) technique".
// Archives of Hydro–Engineering and Environmental Mechanics , Vol. 54, No 1,
// pages 3–24, 2007
	const int SPLINE_36_RANGE = 6;
	dist=std::abs(dist);
	return dist<0 ? 0 : dist<1 ? (
		(   13.0/11.0  * dist - 453.0/ 209.0 ) * dist -   3.0/ 209.0  
	) * dist + 1.0 : dist<2 ? (
		( -  6.0/11.0  * (dist-1) + 270.0/ 209.0 ) * (dist-1) - 156.0/ 209.0  
	) *(dist-1) : dist<3 ? (
		(    1.0/11.0  * (dist-2) -  45.0/ 209.0 ) * (dist-2) +  26.0/ 209.0  
	) *(dist-2) : 0;
}

Real Gl1_WaveFunctionGeometry::sinc256Interpolation(Real dist)
{ // http://www.path.unimelb.edu.au/%7Edersch/interpolator/interpolator.html
	const int SINC_256_RANGE = 16;
	dist=std::abs(dist)*Mathr::PI;
	if(dist==0) return 1;
	return ( std::sin(dist) / (dist) ) * ( std::sin(dist / 8.0) / (dist/8.0) );
}

#endif

