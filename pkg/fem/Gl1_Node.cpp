/*************************************************************************
*  Copyright (C) 2013 by Burak ER    burak.er@btu.edu.tr              	 *
*									 *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#ifdef YADE_OPENGL
#ifdef YADE_FEM


#include <pkg/fem/Gl1_Node.hpp>
#include <lib/opengl/OpenGLWrapper.hpp>
#include <pkg/common/Sphere.hpp>

namespace yade { // Cannot have #include directive inside.

bool Gl1_Node::wire;
bool Gl1_Node::stripes;
int  Gl1_Node::glutSlices;
int  Gl1_Node::glutStacks;
Real  Gl1_Node::quality;
bool  Gl1_Node::localSpecView;
vector<Vector3r> Gl1_Node::vertices, Gl1_Node::faces;
int Gl1_Node::glStripedSphereList=-1;
int Gl1_Node::glGlutSphereList=-1;
Real  Gl1_Node::prevQuality=0;

void Gl1_Node::go(const shared_ptr<Shape>& cm, const shared_ptr<State>& ,bool wire2, const GLViewInfo&)
{
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);

	Real r=(static_cast<Sphere*>(cm.get()))->radius;
	glColor3v(cm->color);
	if (wire || wire2) glutWireSphere(r,int(std::round(quality*glutSlices)),int(std::round(quality*glutStacks)));
	else {
		//Check if quality has been modified or if previous lists are invalidated (e.g. by creating a new qt view), then regenerate lists
		bool somethingChanged = (std::abs(quality-prevQuality)>0.001 || glIsList(glStripedSphereList)!=GL_TRUE);
		if (somethingChanged) {initStripedGlList(); initGlutGlList(); prevQuality=quality;}
		glScale(r,r,r);
		if(stripes) glCallList(glStripedSphereList);
		else glCallList(glGlutSphereList);
	}
	return;
}
YADE_PLUGIN((Gl1_Node));

void Gl1_Node::subdivideTriangle(Vector3r& v1,Vector3r& v2,Vector3r& v3, int depth){
	Vector3r v;
	//Change color only at the appropriate level, i.e. 8 times in total, since we draw 8 mono-color sectors one after another
	if (depth==int(quality) || quality<=0){
		v = (v1+v2+v3)/3.0;
		GLfloat matEmit[4];
		if (v[1]*v[0]*v[2]>0){
			matEmit[0] = 0.3f;
			matEmit[1] = 0.3f;
			matEmit[2] = 0.3f;
			matEmit[3] = 1.f;
		}else{
			matEmit[0] = 0.15f;
			matEmit[1] = 0.15f;
			matEmit[2] = 0.15f;
			matEmit[3] = 0.2f;
		}
 		glMaterialfv(GL_FRONT, GL_EMISSION, matEmit);
	}

	if (depth==1){//Then display 4 triangles
		Vector3r v12 = v1+v2;
		Vector3r v23 = v2+v3;
		Vector3r v31 = v3+v1;
		v12.normalize();
		v23.normalize();
		v31.normalize();
		//Use TRIANGLE_STRIP for faster display of adjacent facets
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3v(v1); glVertex3v(v1);
			glNormal3v(v31); glVertex3v(v31);
			glNormal3v(v12); glVertex3v(v12);
			glNormal3v(v23); glVertex3v(v23);
			glNormal3v(v2); glVertex3v(v2);
		glEnd();
		//terminate with this triangle left behind
		glBegin(GL_TRIANGLES);
			glNormal3v(v3); glVertex3v(v3);
			glNormal3v(v23); glVertex3v(v23);
			glNormal3v(v31); glVertex3v(v31);
		glEnd();
		return;
	}
	Vector3r v12 = v1+v2;
	Vector3r v23 = v2+v3;
	Vector3r v31 = v3+v1;
	v12.normalize();
	v23.normalize();
	v31.normalize();
	subdivideTriangle(v1,v12,v31,depth-1);
	subdivideTriangle(v2,v23,v12,depth-1);
	subdivideTriangle(v3,v31,v23,depth-1);
	subdivideTriangle(v12,v23,v31,depth-1);
}

void Gl1_Node::initStripedGlList() {
	if (!vertices.size()){//Fill vectors with vertices and facets
		//Define 6 points for +/- coordinates
		vertices.push_back(Vector3r(-1,0,0));//0
		vertices.push_back(Vector3r(1,0,0));//1
		vertices.push_back(Vector3r(0,-1,0));//2
		vertices.push_back(Vector3r(0,1,0));//3
		vertices.push_back(Vector3r(0,0,-1));//4
		vertices.push_back(Vector3r(0,0,1));//5
		//Define 8 sectors of the sphere
		faces.push_back(Vector3r(3,4,1));
		faces.push_back(Vector3r(3,0,4));
		faces.push_back(Vector3r(3,5,0));
		faces.push_back(Vector3r(3,1,5));
		faces.push_back(Vector3r(2,1,4));
		faces.push_back(Vector3r(2,4,0));
		faces.push_back(Vector3r(2,0,5));
		faces.push_back(Vector3r(2,5,1));
	}
	//Generate the list. Only once for each qtView, or more if quality is modified.
	glDeleteLists(glStripedSphereList,1);
	glStripedSphereList = glGenLists(1);
	glNewList(glStripedSphereList,GL_COMPILE);
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	// render the sphere now
	for (int i=0;i<8;i++)
		subdivideTriangle(vertices[(unsigned int)faces[i][0]],vertices[(unsigned int)faces[i][1]],vertices[(unsigned int)faces[i][2]],1+ (int) quality);
	glEndList();

}

void Gl1_Node::initGlutGlList(){
	//Generate the "no-stripes" display list, each time quality is modified
	glDeleteLists(glGlutSphereList,1);
	glGlutSphereList = glGenLists(1);
	glNewList(glGlutSphereList,GL_COMPILE);
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glutSolidSphere(1.0,int(std::max(quality*glutSlices,(Real)2.)),int(std::max(quality*glutStacks,(Real)3.)));
	glEndList();
}

} // namespace yade

#endif //YADE_FEM
#endif /* YADE_OPENGL */

//      ///The old Galizzi's lists
// 	if (!vertices.size()) {
// 		Real X = 0.525731112119133606;
// 		Real Z = 0.850650808352039932;
// 		vertices.push_back(Vector3r(-X,0,Z));//0
// 		vertices.push_back(Vector3r(X,0,Z));//1
// 		vertices.push_back(Vector3r(-X,0,-Z));//2
// 		vertices.push_back(Vector3r(X,0,-Z));//3
// 		vertices.push_back(Vector3r(0,Z,X));//4
// 		vertices.push_back(Vector3r(0,Z,-X));//5
// 		vertices.push_back(Vector3r(0,-Z,X));//6
// 		vertices.push_back(Vector3r(0,-Z,-X));//7
// 		vertices.push_back(Vector3r(Z,X,0));//8
// 		vertices.push_back(Vector3r(-Z,X,0));//9
// 		vertices.push_back(Vector3r(Z,-X,0));//10
// 		vertices.push_back(Vector3r(-Z,-X,0));//11
// 		faces.push_back(Vector3r(0,4,1));
// 		faces.push_back(Vector3r(0,9,4));
// 		faces.push_back(Vector3r(9,5,4));
// 		faces.push_back(Vector3r(4,5,8));
// 		faces.push_back(Vector3r(4,8,1));
// 		faces.push_back(Vector3r(8,10,1));
// 		faces.push_back(Vector3r(8,3,10));
// 		faces.push_back(Vector3r(5,3,8));
// 		faces.push_back(Vector3r(5,2,3));
// 		faces.push_back(Vector3r(2,7,3));
// 		faces.push_back(Vector3r(7,10,3));
// 		faces.push_back(Vector3r(7,6,10));
// 		faces.push_back(Vector3r(7,11,6));
// 		faces.push_back(Vector3r(11,0,6));
// 		faces.push_back(Vector3r(0,1,6));
// 		faces.push_back(Vector3r(6,1,10));
// 		faces.push_back(Vector3r(9,0,11));
// 		faces.push_back(Vector3r(9,11,2));
// 		faces.push_back(Vector3r(9,2,5));
// 		faces.push_back(Vector3r(7,2,11));}

