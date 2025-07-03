#pragma once

#include <engine/Math/Vec4.hpp>
#include <vector>

/*
Generating convex polytopes.

A convex polygon is fully defined by it's vertices, because it can be defined as the convex hull of it's vertices.

We can create dual of a polytope by choosing the vertices to be the centroids of the original cells.

Some polytopes have nice combinatorial constructions. For the cells of a simplex can be found by chosing k-tuples  of vertices. Similarly the cross polytope and the hypercube have nice constructions. They can be found in Coexter "Regular Polytopes".

For regular figures the edges faces and cells can be found by choosing tuples of vertices that are close to eachother in angular distance.

Vertices of some polytopes may be found on wikipedia. This website contains many more https://www.qfbox.info/4d/uniform.

https://en.wikipedia.org/wiki/Semiregular_polytope
Semiregular polytopes are vertex transitive polytopes that have regular polytopes as cells. There are only 3 of them. 

A more general class is the class of uniform polytopes. They are the vertex transitive polytopes that have uniform polytopes as cells. In 2D the uniform polytopes are just the regular polygons so 3D uniform polytopes are the same as 3D semiregular polytopes.
*/

struct Polytope {
	// Storing just the sets of vertices making a 3-cell for example would only work if all of them were of the same type. For example if all of them were simplicies. This is why this needs to store the references to the one lower cells instead of vertices.
	using PointN = std::vector<f32>;
	using CellN = std::vector<i32>;
	using CellsN = std::vector<CellN>;
	std::vector<PointN> vertices;
	std::vector<CellsN> cells;

	CellsN& cellsOfDimension(i32 n);
	const CellsN& cellsOfDimension(i32 n) const;
};

Polytope crossPolytope(i32 dimension);
i32 crossPolytopeSimplexCount(i32 dimensionOfCrossPolytope, i32 dimensionOfSimplex);

Polytope hypercube(i32 dimension);
i32 hypercubeCellCount(i32 dimensionOfHypercube, i32 dimensionOfCells);

Polytope subdiviedHypercube4(i32 divisionCount);

std::vector<i32> verticesOfFaceWithSortedEdges(const Polytope& p, i32 faceIndex);
std::vector<i32> verticesOfFaceWithSortedEdges(const Polytope& p, const Polytope::CellN& face);
 
// Normally the edges of a face of a polytope can be in any order. This function sorts them so that they are in a cyclic order, that is if an edge contains a vertex then next edge contains one vertex and the previous also contains one vertex. The vertices of the edges are not changed. If a polytope is nonorientable then it isn't possible to consistently orient the edges.
Polytope::CellN faceEdgesSorted(const Polytope& p, i32 faceIndex);


// regular
Polytope generate600cell();
Polytope make600cell();
Polytope generate120cell();
Polytope make120cell();
Polytope make24cell();
Polytope make5cell();

std::string outputPolytope4DataCpp(const char* name, const Polytope p);

// semi-regular
Polytope makeRectified5cell();
Polytope makeRectified5cell();
Polytope generateRectified600cell();
Polytope makeRectified600cell();
Polytope generateSnub24cell();
Polytope makeSnub24cell();


Polytope makeSubdiviedHypercube2();
// uniform