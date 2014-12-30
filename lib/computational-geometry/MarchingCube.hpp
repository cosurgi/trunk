// 2004 © Olivier Galizzi
// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <lib/base/Math.hpp>

class MarchingCube 
{
	public  :
		MarchingCube ();
		~MarchingCube ();

		void init(int sx, int sy, int sz, const Vector3r& min, const Vector3r& max);
		void computeTriangulation(const vector<vector<vector<Real> > >& scalarField, Real iso);
		void resizeScalarField(vector<vector<vector<Real> > >& scalarField, int sx, int sy, int sz);

		const vector<Vector3r>& getTriangles() { return triangles; }
		const vector<Vector3r>& getNormals() { return normals; }
		int getNbTriangles() { return nbTriangles; }

	private :
		vector<Vector3r> triangles;
		vector<Vector3r> normals;
		int nbTriangles;
		Vector3r beta;
		Vector3r alpha;
		int sizeX,sizeY,sizeZ;
		Real isoValue;
		vector<vector<vector<Vector3r> > > positions;
		static const int edgeArray[256];
		static const int triTable[256][16];
		Vector3r aNormal;	
	
	private :
		/// triangulate cell  (x,y,z)
		void polygonize (const vector<vector<vector<Real> > >& scalarField, int x, int y, int z);
		/// compute normals of the triangles previously found with polygonizecalcule les normales des triangles trouver dans la case (x,y,z)
		/// @param n : indice of the first triangle to process
		void computeNormal(const vector<vector<vector<Real> > >& scalarField, int x, int y, int z,int offset, int triangleNum);
		/// interpolate coordinates of point vect (that is on isosurface) from coordinates of points vect1 et vect2
		void interpolate (const Vector3r&  vect1, const Vector3r& vect2, Real val1, Real val2,Vector3r& vect);
		/// Same as interpolate but in 1D
		Real interpolateValue(Real val1, Real val2, Real val_cible1, Real val_cible2);
		/// Compute normal to vertice or triangle inside cell (x,y,z)
		const Vector3r& computeNormalX(const vector<vector<vector<Real> > >& scalarField, int x, int y, int z);
		const Vector3r& computeNormalY(const vector<vector<vector<Real> > >& scalarField, int x, int y, int z);
		const Vector3r& computeNormalZ(const vector<vector<vector<Real> > >& scalarField, int x, int y, int z); 
};

