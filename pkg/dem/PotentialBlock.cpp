/*CWBoon 2015 */
#ifdef YADE_POTENTIAL_BLOCKS
//Polyhedral blocks formulated as in Boon et al (2012) using an inner potential particle (Houlsby, 2009) to calculate the contact normal.
#include "PotentialBlock.hpp"

namespace yade { // Cannot have #include directive inside.

YADE_PLUGIN((PotentialBlock));

PotentialBlock::~PotentialBlock()
{
}


void PotentialBlock::addPlaneStruct()    { planeStruct.push_back(Planes());    }
//void PotentialBlock::addVertexStruct()   { vertexStruct.push_back(Vertices()); }
//void PotentialBlock::addEdgeStruct()     { edgeStruct.push_back(Edges());      }

//TODO: We can use the ordered (cwise or ccwise) vertices belonging to each plane, stored in the planeStruct for: 1. Visualisation without using CGAL and 2. Calculation of volume and inertia (more specifically, to create the pyramids and subsequently tetrahedra, used to calculate the volume and inertia below). By saving the ordered vertices for each plane, it is trivial to create a triangulation of them without needing the convex hull of CGAL.
//TODO: We need a check to merge duplicate faces during initialisation, so that when we write the plane coefficients a,b,c,d in the shape class, we don't allow duplicate combinations. This means that the planeNo will change, and we need to ensure it is updated everywhere, after we remove the duplicate faces


void PotentialBlock::postLoad(PotentialBlock& )
{

if (vertices.empty() and (not a.empty())) { // i.e. if the particle is not initialised

	int planeNo = a.size();

	/* Normalize the coefficients of the planes defining the particle faces */
	Vector3r planeNormVec;
	for (int i=0; i<planeNo; i++){
		planeNormVec = Vector3r(a[i],b[i],c[i]);
		if ( std::abs(planeNormVec.norm() - 1.0 ) > 1e-3) { /* Normalize only if the normal vectors are not normalized already */
//		if (planeNormVec.norm() > 1+1e-3) { /* Normalize only if the normal vectors are not normalized already */
			a[i] /= planeNormVec.norm();
			b[i] /= planeNormVec.norm();
			c[i] /= planeNormVec.norm();
			d[i] /= planeNormVec.norm();
		}
	}

	/* Check that the defined planes have the same number of coefficients */
	if ( b.size()!=a.size() or c.size()!=a.size() or d.size()!=a.size() ) {std::cout<<"The planes do not have the same number of coefficients. Check the input geometry!"<<endl;}

	/* Make sure the d[i] values given by the user are positive (i.e. the normal vectors of the faces point outwards) */
	for (int i=0; i<planeNo; i++){ if (d[i]<0) { a[i] *= -1; b[i] *= -1; c[i] *= -1; d[i] *= -1; } }

	/* Add a structure containing the vertices associated with each plane*/
	for (int i=0; i<planeNo; i++){ addPlaneStruct(); }

	// Calculate vertices
	calculateVertices();

	/* Calculate R as half the distance of the farthest vertices, if user hasn't set a positive value for R. */
	/* A reminder that R in the Potential Blocks is meant to represent a reference length, used to calculate the initial bisection search employed to identify points on the particle surfaces. Here, R does not affect the curvature of the faces, like in the Potetial Particles code. The faces of a Potential Block are always flat. */
	/* Although half the distance of the farthest vertices is in no case the circumradius, we just need a value around this order of magnitude for the bisection search code to run smoothly */
	if (R==0.0 and ( not vertices.empty() )) {
		Real maxDistance=0.0, Distance;
		for (unsigned int i=0; i<vertices.size()-1;i++){
			for (unsigned int j=i+1; i<vertices.size();i++){
				Distance = (vertices[i] - vertices[j]).norm();
				if (Distance > maxDistance) { maxDistance = Distance; }
			}
		}
		if (maxDistance>0.0) {
			R = maxDistance/2.;
		}
		calculateVertices(); //Recalculate vertices after calculating R: Not strictly necessary to recalculate, but kept for consistency
		if (R==0) { std::cout<<"R must be positive. Incorrect automatic calculation from the vertices."<<endl;}
	}
	assert(R>0.0);

	/* Set a default value for the attrs minAabb, maxAabb, used for visualisation in vtk, if they are left undefined */
	if (minAabb.norm()==0 and maxAabb.norm()==0) {
		minAabb = Vector3r(R,R,R);
		maxAabb = Vector3r(R,R,R);
	}

	// Calculate geometric properties: volume, centroid, inertia, principal orientation (inertia is calculated after the particle is centered to its centroid)
	Vector3r centr = Vector3r::Zero();
	Real Ixx, Iyy, Izz, Ixy, Ixz, Iyz;

	calculateInertia(centr, Ixx, Iyy, Izz, Ixy, Ixz, Iyz); //Calculate volume & centroid

	// An attempt to eliminate rounding errors from the calculation of vertices/volume/inertia
	if ( std::abs( centr.x() )/R < 1e-6 ) { centr.x()=0; }
	if ( std::abs( centr.y() )/R < 1e-6 ) { centr.y()=0; }
	if ( std::abs( centr.z() )/R < 1e-6 ) { centr.z()=0; }

	// If the particle is not centered to its centroid, center (translate) the face equations around the centroid and recalculate vertices, volume & inertia
	if (centr.norm()/R > 1e-6) {
		for (int i=0; i<planeNo; i++){
			d[i] = -(a[i]*centr.x() + b[i]*centr.y() + c[i]*centr.z() - d[i]);
//			if (d[i]<0) { a[i] *= -1; b[i] *= -1; c[i] *= -1; d[i] *= -1; }
		}
		position=centr;
		calculateVertices(); // Recalculate vertices for the centered particle faces
		calculateInertia(centr, Ixx, Iyy, Izz, Ixy, Ixz, Iyz); // Calculate inertia for the centered particle
	}

/* ------------------------------------------------------------------------------------------------------------------------------ */
	if ( std::abs(Ixy) + std::abs(Ixz) + std::abs(Iyz)<1e-15 ) {
		inertia = Vector3r(Ixx,Iyy,Izz);
	} else { //rotate the planes to the principal axes if they are not already rotated
		if( fabs(Ixx) < pow(10,-15) ){Ixx = 0.0;} //TODO: Check whether I should keep/modify these or if there is a case where they introduce bugs
		if( fabs(Iyy) < pow(10,-15) ){Iyy = 0.0;} // TODO: Maybe I could just check only the non-diagonal terms Ixy, Iyz, Ixz?
		if( fabs(Izz) < pow(10,-15) ){Izz = 0.0;} // Or I could normalise the checked inertia values to make them dimensionless, by dividing with pow(R,5) or V*pow(R,2) or something similar
		if( fabs(Ixy) < pow(10,-15) ){Ixy = 0.0;}
		if( fabs(Iyz) < pow(10,-15) ){Iyz = 0.0;}
		if( fabs(Ixz) < pow(10,-15) ){Ixz = 0.0;}

		char jobz = 'V'; char uplo = 'L'; int N=3; std::vector<Real> A (9); int lda=3; std::vector<Real> eigenValues (3); std::vector<Real> work (102); int lwork = 102; int info = 0; //FIXME: jobz, uplo, N, lda, lwork could be defined as "const" variables
		A[0] = Ixx; A[1] =-Ixy; A[2] =-Ixz;
		A[3] =-Ixy; A[4] = Iyy; A[5] =-Iyz;
		A[6] =-Ixz; A[7] =-Iyz; A[8] = Izz;
		dsyev_(&jobz, &uplo, &N, &A[0], &lda, &eigenValues[0], &work[0], &lwork, &info);

		Vector3r eigenVec1 (A[0],A[1],A[2]); eigenVec1.normalize();
		Vector3r eigenVec2 (A[3],A[4],A[5]); eigenVec2.normalize();
		Vector3r eigenVec3 (A[6],A[7],A[8]); eigenVec3.normalize();

		Eigen::Matrix3d lapackEigenVec;
		lapackEigenVec(0,0) = eigenVec1[0]; lapackEigenVec(0,1) = eigenVec2[0]; lapackEigenVec(0,2)=eigenVec3[0];
		lapackEigenVec(1,0) = eigenVec1[1]; lapackEigenVec(1,1) = eigenVec2[1]; lapackEigenVec(1,2)=eigenVec3[1];
		lapackEigenVec(2,0) = eigenVec1[2]; lapackEigenVec(2,1) = eigenVec2[2]; lapackEigenVec(2,2)=eigenVec3[2];

		/* make sure diagonals are positive */
		for (int i=0; i<3; i++){
			if ( lapackEigenVec(i,i) < 0 ) {
				lapackEigenVec(0,i) *= -1;
				lapackEigenVec(1,i) *= -1;
				lapackEigenVec(2,i) *= -1;
			}
		}

		Eigen::Matrix3d lapackEigenVal = Eigen::Matrix3d::Zero();
		lapackEigenVal(0,0) = eigenValues[0]; lapackEigenVal(1,1) = eigenValues[1]; lapackEigenVal(2,2) = eigenValues[2];

		Quaternionr q (lapackEigenVec);
		Real q0 = 0.5*sqrt(lapackEigenVec(0,0) + lapackEigenVec(1,1) + lapackEigenVec(2,2) + 1.0);
		Real q1 = ( lapackEigenVec(1,2) - lapackEigenVec(2,1) )/( 4*q0 );
		Real q2 = ( lapackEigenVec(2,0) - lapackEigenVec(0,2) )/( 4*q0 );
		Real q3 = ( lapackEigenVec(0,1) - lapackEigenVec(1,0) )/( 4*q0 );
		q.w()=q0; q.x()=q1; q.y()=q2; q.z()=q3; q.normalize();

		/* Principal inertia tensor and orientation*/
		inertia = Vector3r(lapackEigenVal(0,0),lapackEigenVal(1,1),lapackEigenVal(2,2));
		orientation = q.conjugate();

		/* Orient faces & vertices to the principal directions */
		Vector3r plane4;
		for (unsigned int i=0; i<a.size(); i++){
			plane4 = q * Vector3r(a[i],b[i],c[i]);
			a[i] = plane4.x();
			b[i] = plane4.y();
			c[i] = plane4.z();
		}

		for(unsigned int i=0; i<vertices.size(); i++) {
			vertices[i] =  q*vertices[i];
		}
	}

	for (unsigned int j=0; j<a.size(); j++){
		connectivity.push_back(planeStruct[j].vertexID);
	}

}
}


Real PotentialBlock::getDet(const Eigen::MatrixXd A){
	/* if positive, counter clockwise, 2nd point makes a larger angle */
	/* if negative, clockwise, 3rd point makes a larger angle */
	int rowNo = A.rows();  Real firstTerm = 0.0; Real secondTerm = 0.0;
	for(int i=0; i<rowNo-1; i++){
		firstTerm  += A(i,0)*A(i+1,1);
		secondTerm += A(i,1)*A(i+1,0);
	}
	return firstTerm-secondTerm;
}


Real PotentialBlock::getSignedArea(const Vector3r pt1, const Vector3r pt2, const Vector3r pt3){
	/* if positive, counter clockwise, 2nd point makes a larger angle */
	/* if negative, clockwise, 3rd point makes a larger angle */
	Eigen::MatrixXd triangle(4,2);
	triangle(0,0) = pt1.x();  triangle(0,1) = pt1.y(); // triangle(0,2) = pt1.z();
	triangle(1,0) = pt2.x();  triangle(1,1) = pt2.y(); // triangle(1,2) = pt2.z();
	triangle(2,0) = pt3.x();  triangle(2,1) = pt3.y(); // triangle(2,2) = pt3.z();
	triangle(3,0) = pt1.x();  triangle(3,1) = pt1.y(); // triangle(3,2) = pt1.z();
	Real determinant = getDet(triangle);
	return determinant; //triangle.determinant();
}


void PotentialBlock::calculateVertices() {
	std::vector<Real> D (3); std::vector<Real> Ax (9); Eigen::Matrix3d Aplanes;
	Real Distance;
	int vertCount=0; Real minDistance;
	int planeNo = a.size();

	Vector3r plane1, plane2, plane3;
	Real d1, d2, d3, detAplanes;

	std::vector<int> ipiv (3); int bColNo; int info, three; //FIXME: bColNo, three could be defined as "const int", instead of "int"
	bool inside; Vector3r vertex;
	Real plane;
	int vertexID;

	int closestVertex=0;
	vertices.clear();

	for (int i=0; i<planeNo; i++){ planeStruct[i].vertexID.clear(); } //Empty the planeStruct every time calculateVertices is ran

	for (int i=0; i<planeNo-2; i++){
		for (int j=i+1; j<planeNo-1; j++){
			for(int kk=j+1; kk<planeNo; kk++){

				plane1 = Vector3r(a[i],b[i],c[i]);
				plane2 = Vector3r(a[j],b[j],c[j]);
				plane3 = Vector3r(a[kk],b[kk],c[kk]);

				d1 = d[i]+r;
				d2 = d[j]+r;
				d3 = d[kk]+r;

				D[0]=d1;
				D[1]=d2;
				D[2]=d3;
				Ax[0]=plane1.x(); Ax[3]=plane1.y(); Ax[6]=plane1.z();   Aplanes(0,0) = Ax[0]; Aplanes(0,1) = Ax[3]; Aplanes(0,2) = Ax[6];
				Ax[1]=plane2.x(); Ax[4]=plane2.y(); Ax[7]=plane2.z();   Aplanes(1,0) = Ax[1]; Aplanes(1,1) = Ax[4]; Aplanes(1,2) = Ax[7];
				Ax[2]=plane3.x(); Ax[5]=plane3.y(); Ax[8]=plane3.z();   Aplanes(2,0) = Ax[2]; Aplanes(2,1) = Ax[5]; Aplanes(2,2) = Ax[8];

				detAplanes = Aplanes.determinant();

				if(fabs(detAplanes)>pow(10,-15) ){ //if (parallel == false) {

					bColNo=1; info=0; /* LU */ three =3;
					dgesv_( &three, &bColNo, Ax.data(), &three, ipiv.data(), D.data(), &three, &info);

					if (info!=0){
						//std::cout<<"linear algebra error"<<endl;
					}else{
						inside = true; vertex = Vector3r(D[0],D[1],D[2]);

						for (int m=0; m<planeNo; m++){
							plane = a[m]*vertex.x() + b[m]*vertex.y() + c[m]*vertex.z() - d[m]- r; if (plane>pow(10,-3)){inside = false;}
						}
						if (inside == true){

							/* Check for duplicate vertices: New vertices cannot be too close to existing ones */
							if (vertCount==0) { // Allow the first vertex to be written always
								vertices.push_back(vertex);
								vertCount++;

								vertexID = vertices.size()-1;
								planeStruct[i].vertexID.push_back(vertexID);
								planeStruct[j].vertexID.push_back(vertexID);
								planeStruct[kk].vertexID.push_back(vertexID);
							} else {
								minDistance=1.0e15;
								for (int n=0; n<vertCount; n++){
									Distance = (vertex-vertices[n]).norm();
									if (Distance < minDistance) { minDistance = Distance; closestVertex=n; }
								}

								if ( minDistance > (R > 0.0 ? R*1.0e-3 : 1.0e-6) ) {
									vertices.push_back(vertex);
									vertCount++;

									vertexID = vertices.size()-1;
									planeStruct[i].vertexID.push_back(vertexID); // If not duplicate, save new ID
									planeStruct[j].vertexID.push_back(vertexID);
									planeStruct[kk].vertexID.push_back(vertexID);
								} else {
									planeStruct[i].vertexID.push_back(closestVertex); // If duplicate save ID of duplicate
									planeStruct[j].vertexID.push_back(closestVertex);
									planeStruct[kk].vertexID.push_back(closestVertex);
								}
							}

							/* Structs associating: Vertices-Edges-Planes: Could become useful in the future */
							#if 0
							/* vertexStruct */
							vertexID = vertices.size()-1;
//							addVertexStruct();
//							int vertexID = vertexStruct.size()-1;
//							vertexStruct[vertexID].planeID.push_back(i);   /* Note that the planeIDs are arranged from small to large! */
//							vertexStruct[vertexID].planeID.push_back(j);   /* planeIDs are arranged in the same sequence as [a,b,c] and d */
//							vertexStruct[vertexID].planeID.push_back(kk);   /* vertices store information on planeIDs */


							/* edgeStruct */
							int vertexNo = vertices.size(); int edgeCount=0;
							for (int i=0; i<vertexNo; i++ ){
								for (int j=0; j<vertexNo; j++){
									if(i==j){continue;}
									int v1a = vertexStruct[i].planeID[0];
									int v2a = vertexStruct[i].planeID[1];
									int v3a = vertexStruct[i].planeID[2];
									int v1b = vertexStruct[j].planeID[0];
									int v2b = vertexStruct[j].planeID[1];
									int v3b = vertexStruct[j].planeID[2];

									if(  (v1a != v1b && v2a == v2b && v3a == v3b) || (v1a == v1b && v2a != v2b && v3a == v3b) || (v1a == v1b && v2a == v2b && v3a != v3b)  ){
										Real length = ( vertices[i] - vertices[j] ).norm();
										if(length<pow(10,-3) ){ continue; }
										addEdgeStruct();
										edgeStruct[edgeCount].vertexID.push_back(i); /* edges store information on vertexIDs */
										edgeStruct[edgeCount].vertexID.push_back(j);
										vertexStruct[i].edgeID.push_back(edgeCount); /* vertices store information on edgeIDs */
										vertexStruct[j].edgeID.push_back(edgeCount);
										edgeCount++;
									}
								}
							}

							/* planeStruct */
							vertexID = vertices.size()-1;
							planeStruct[i].vertexID.push_back(vertexID);   /* planes store information on vertexIDs */
							planeStruct[j].vertexID.push_back(vertexID);
							planeStruct[kk].vertexID.push_back(vertexID);
							#endif
						}
					}
				}
			}
		}
	}

	//Remove duplicate vertices from each planeStruct[j] and record the IDs of planes with less than 3 vertices (these planes are redundant)
	//This is done to guard the code against non-manifold geometrical inputs
	std::vector<int> pWLTTV; //planes With Less Than Three Vertices
	for (int j=0; j<planeNo; j++){
		std::stable_sort( planeStruct[j].vertexID.begin(), planeStruct[j].vertexID.end() );
		planeStruct[j].vertexID.erase( std::unique( planeStruct[j].vertexID.begin(), planeStruct[j].vertexID.end() ), planeStruct[j].vertexID.end() );
		if (planeStruct[j].vertexID.size()<3 ) { pWLTTV.push_back(j); }
	}

	//Sort the IDs of the planes to be removed in descending order, in order to start removing from last to first, using the reverse iterators: rbegin(), rend()
	std::stable_sort( pWLTTV.rbegin(), pWLTTV.rend() );
	int pWLTTVSize=pWLTTV.size();

	for (int j=0; j<pWLTTVSize; j++){
		a.erase(a.begin()+pWLTTV[j]);
		b.erase(b.begin()+pWLTTV[j]);
		c.erase(c.begin()+pWLTTV[j]);
		d.erase(d.begin()+pWLTTV[j]);

		if ( not phi_b.empty()     ) { phi_b.erase(phi_b.begin()+pWLTTV[j]);         }
		if ( not phi_r.empty()     ) { phi_r.erase(phi_r.begin()+pWLTTV[j]);         }
		if ( not cohesion.empty()  ) { cohesion.erase(cohesion.begin()+pWLTTV[j]);   }
		if ( not tension.empty()   ) { tension.erase(tension.begin()+pWLTTV[j]);     }
		if ( not jointType.empty() ) { jointType.erase(jointType.begin()+pWLTTV[j]); }

		planeStruct.erase(planeStruct.begin()+pWLTTV[j]);
	}

	//Here we repeat this check, after the redundant faces are removed, just in case
	if ( b.size()!=a.size() or c.size()!=a.size() or d.size()!=a.size() ) {std::cout<<"The planes do not have the same number of coefficients. Check the input geometry!"<<endl;}

}


void PotentialBlock::calculateInertia(Vector3r& centroid, Real& Ixx, Real& Iyy, Real& Izz,Real& Ixy, Real& Ixz, Real& Iyz){

	Vector3r pointInside(0,0,0), vertex, planeNormal, oriNormal(0,0,1), crossProd, rotatedCoord;
	Vector3r pt1, pt2, pt3, baseOnPolygon, oriBaseOnPolygon, centroidPyramid, centroidTetra, tempVert1, tempVert2, tempVert3, tempVert4;
	Real totalVolume=0.0, /*plane,*/ det, area, height, vol, areaPyramid, volumePyramid, heightTetra, tempArea, areaTri, tetraVol;
	Real detJ, x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;
	Quaternionr Qp;
	unsigned int h, kk, m, counter;
	int lastEntry;
	Eigen::MatrixXd B(4,2), vertexOnPlane;
	std::vector<Vector3r> verticesOnPlane, oriVerticesOnPlane, orderedVerticesOnPlane, oriOrderedVerticesOnPlane; //vector<int> oriOrderedVerticesOnplaneID;

	centroid = Vector3r::Zero();
	Ixx = 0.0; Iyy = 0.0; Izz= 0.0; Ixy = 0.0; Ixz= 0.0; Iyz=0.0;

	for (unsigned int j=0; j<a.size(); j++){

		if(not verticesOnPlane.empty()){ verticesOnPlane.clear(); oriVerticesOnPlane.clear(); }
		for (unsigned int i=0; i<planeStruct[j].vertexID.size();i++){ //iterate through the vertices of each face
			vertex = vertices[planeStruct[j].vertexID[i]]; /*local coordinates*/
			planeNormal = Vector3r(a[j],b[j],c[j]);
			crossProd = oriNormal.cross(planeNormal);
			Qp.w() = 1.0 + oriNormal.dot(planeNormal);
			Qp.x() = crossProd.x(); Qp.y() = crossProd.y();  Qp.z() = crossProd.z();
			Qp.normalize();
			if(crossProd.norm() < pow(10,-7)){ Qp = Quaternionr::Identity(); }
			rotatedCoord = Qp.conjugate()*vertex;

			verticesOnPlane.push_back(rotatedCoord);
			oriVerticesOnPlane.push_back(vertex);
		}

		if(verticesOnPlane.empty()){continue;}
//		if(verticesOnPlane.size()<3) {continue;} //Only calculate volume/inertia for faces with three or more vertices
		if(verticesOnPlane.size()<3) {std::cout<<"Face: "<<j<<" has less than 3 vertices! Check particle geometry"<<endl;}

		/* REORDER VERTICES counterclockwise positive*/
		h = 0; kk = 1; m = 2;
		pt1 = verticesOnPlane[h];
		pt2 = verticesOnPlane[kk];
		pt3 = verticesOnPlane[m];
		orderedVerticesOnPlane.push_back(pt1); oriOrderedVerticesOnPlane.push_back(oriVerticesOnPlane[0]);
		counter = 1;

		while(counter<verticesOnPlane.size()){
			while (m<verticesOnPlane.size()){
				pt1 = verticesOnPlane[h];
				pt2 = verticesOnPlane[kk];
				pt3 = verticesOnPlane[m];
				if (getSignedArea(pt1,pt2,pt3) < 0.0){
					/* clockwise means 3rd point is better than 2nd */
					kk=m; /*3rd point becomes 2nd point */
					pt2 = verticesOnPlane[kk];

				}/* else counterclockwise is good.  We need to find and see whether there is a point(3rd point) better than the 2nd point */
				/* advance m */
				m=m+1;
				while(m==h || m==kk){ m=m+1; }
			}
			//std::cout<<"h: "<<h<<", kk :"<<kk<<", m: "<<m<<endl;
			orderedVerticesOnPlane.push_back(pt2);
			oriOrderedVerticesOnPlane.push_back(oriVerticesOnPlane[kk]);

			h=kk;
			/* advance kk */
			kk=0;
			while(kk==h ){ kk=kk+1; }
			/* advance m */
			m=0;
			while(m==h || m==kk){ m=m+1; }
			counter++;
		}

		/* Save the IDs of the vertices of each face in an ordered sequence. Can be used to triangulate each face */
		std::vector<int> tempPlanes=planeStruct[j].vertexID;

		planeStruct[j].vertexID.clear();
		for (unsigned int i=0; i<oriOrderedVerticesOnPlane.size(); i++){
			for (unsigned int n=0; n<vertices.size(); n++){
				if( vertices[n][0]==oriOrderedVerticesOnPlane[i][0] and vertices[n][1]==oriOrderedVerticesOnPlane[i][1] and vertices[n][2]==oriOrderedVerticesOnPlane[i][2]  ) {
					planeStruct[j].vertexID.push_back(n);
				}
			}
		}

		vertexOnPlane = Eigen::MatrixXd(orderedVerticesOnPlane.size()+1,2);
		baseOnPolygon = Vector3r(0,0,0); oriBaseOnPolygon = Vector3r(0,0,0);

		for(unsigned int i=0; i< orderedVerticesOnPlane.size(); i++){
			vertexOnPlane(i,0)=orderedVerticesOnPlane[i].x(); vertexOnPlane(i,1)=orderedVerticesOnPlane[i].y(); //vertexOnPlane(i,2)=orderedVerticesOnPlane[i].z();
			//std::cout<<"vertexOnPlane0: "<<vertexOnPlane(i,0)<<", vertexOnPlane1: "<<vertexOnPlane(i,1)<<endl;
			baseOnPolygon += orderedVerticesOnPlane[i];
			oriBaseOnPolygon += oriOrderedVerticesOnPlane[i];
		}

		baseOnPolygon = baseOnPolygon/static_cast<Real>(orderedVerticesOnPlane.size());
		oriBaseOnPolygon = oriBaseOnPolygon/static_cast<Real>(oriOrderedVerticesOnPlane.size());
		lastEntry = orderedVerticesOnPlane.size();
		vertexOnPlane(lastEntry,0)=orderedVerticesOnPlane[0].x(); vertexOnPlane(lastEntry,1)=orderedVerticesOnPlane[0].y(); //vertexOnPlane(lastEntry,2)=orderedVerticesOnPlane[0].z();
		//std::cout<<"vertexOnPlane0: "<<vertexOnPlane(lastEntry,0)<<", vertexOnPlane1: "<<vertexOnPlane(lastEntry,1)<<endl;

		det    =  getDet(vertexOnPlane);
		area   =  0.5*det; //(vertexOnPlane.determinant());
		height = -1.0*( a[j]*pointInside.x() + b[j]*pointInside.y() + c[j]*pointInside.z() - d[j] - r );
		vol    =  1.0/3.0*area*height;
		totalVolume  += vol;
		//std::cout<<"orderedVerticesOnPlane.size(): "<<orderedVerticesOnPlane.size()<<", volume: "<<volume<<", area: "<<area<<", height: "<<height<<endl;

		areaPyramid = 0.0;
		centroidPyramid = Vector3r(0,0,0);
		volumePyramid = 0.0;
		heightTetra = 0.0;

		for(int i=0; i<vertexOnPlane.rows()-1; i++){
			B(0,0)=vertexOnPlane(i,0);   B(0,1)=vertexOnPlane(i,1);
			B(1,0)=vertexOnPlane(i+1,0); B(1,1)=vertexOnPlane(i+1,1);
			B(2,0)=baseOnPolygon.x();    B(2,1)=baseOnPolygon.y();
			B(3,0)=vertexOnPlane(i,0);   B(3,1)=vertexOnPlane(i,1);
			tempArea = getDet(B);
			areaTri = 0.5*tempArea;
			areaPyramid += areaTri;
			heightTetra = fabs(orderedVerticesOnPlane[i].z());
			tetraVol = 1.0/3.0*areaTri*heightTetra;
			volumePyramid += tetraVol;

			// tempVert1,2,3,4: vertices of the tetrahedron
			tempVert1 = oriOrderedVerticesOnPlane[i];
			tempVert3 = oriBaseOnPolygon;
			tempVert4 = pointInside;

			if( i==vertexOnPlane.rows()-2 ){
				tempVert2 = oriOrderedVerticesOnPlane[0];
			}else{
				tempVert2 = oriOrderedVerticesOnPlane[i+1];
			}

			centroidTetra = 0.25 * (tempVert1 + tempVert2 + tempVert3 + tempVert4);
			centroidPyramid += (tetraVol*centroidTetra);

			// Calculation of the inertia tensor for each tetrahedron.
			// See article F. Tonon, "Explicit Exact Formulas for the 3-D Tetrahedron Inertia Tensor in Terms of its Vertex Coordinates", http://www.scipub.org/fulltext/jms2/jms2118-11.pdf
			// Tonon's formulae work for any coordinate system, so here we calculate the inertia of each tetrahedron directly to the local centroid of the particle, avoiding Steiner.
			x1 = tempVert1.x(); y1 = tempVert1.y(); z1 = tempVert1.z();
			x2 = tempVert2.x(); y2 = tempVert2.y(); z2 = tempVert2.z();
			x3 = tempVert3.x(); y3 = tempVert3.y(); z3 = tempVert3.z();
			x4 = tempVert4.x(); y4 = tempVert4.y(); z4 = tempVert4.z();


			detJ=(x2-x1)*(y3-y1)*(z4-z1)+(x3-x1)*(y4-y1)*(z2-z1)+(x4-x1)*(y2-y1)*(z3-z1)
				-(x2-x1)*(y4-y1)*(z3-z1)-(x3-x1)*(y2-y1)*(z4-z1)-(x4-x1)*(y3-y1)*(z2-z1);
			detJ=std::abs(detJ);

			Ixx += detJ*(y1*y1+y1*y2+y2*y2+y1*y3+y2*y3+
				y3*y3+y1*y4+y2*y4+y3*y4+y4*y4+z1*z1+z1*z2+
				z2*z2+z1*z3+z2*z3+z3*z3+z1*z4+z2*z4+z3*z4+z4*z4)/60.;

			Iyy += detJ*(x1*x1+x1*x2+x2*x2+x1*x3+x2*x3+x3*x3+
				x1*x4+x2*x4+x3*x4+x4*x4+z1*z1+z1*z2+z2*z2+z1*z3+
				z2*z3+z3*z3+z1*z4+z2*z4+z3*z4+z4*z4)/60.;

			Izz += detJ*(x1*x1+x1*x2+x2*x2+x1*x3+x2*x3+x3*x3+x1*x4+
				x2*x4+x3*x4+x4*x4+y1*y1+y1*y2+y2*y2+y1*y3+
				y2*y3+y3*y3+y1*y4+y2*y4+y3*y4+y4*y4)/60.;

			// a' in the article etc.
			Iyz += detJ*(2*y1*z1+y2*z1+y3*z1+y4*z1+y1*z2+
				2*y2*z2+y3*z2+y4*z2+y1*z3+y2*z3+2*y3*z3+
				y4*z3+y1*z4+y2*z4+y3*z4+2*y4*z4)/120.;

			Ixz += detJ*(2*x1*z1+x2*z1+x3*z1+x4*z1+x1*z2+
				2*x2*z2+x3*z2+x4*z2+x1*z3+x2*z3+2*x3*z3+
				x4*z3+x1*z4+x2*z4+x3*z4+2*x4*z4)/120.;

			Ixy += detJ*(2*x1*y1+x2*y1+x3*y1+x4*y1+x1*y2+
				2*x2*y2+x3*y2+x4*y2+x1*y3+x2*y3+2*x3*y3+
				x4*y3+x1*y4+x2*y4+x3*y4+2*x4*y4)/120.;
		}
		centroid += centroidPyramid;
		orderedVerticesOnPlane.clear(); oriOrderedVerticesOnPlane.clear();
	}
	centroid = centroid/totalVolume;
	volume = totalVolume;
	verticesOnPlane.clear(); oriVerticesOnPlane.clear();
}

} // namespace yade

#endif // YADE_POTENTIAL_BLOCKS
