#pragma once

#include <engine/Math/Vec3.hpp>
#include <View.hpp>
#include <vector>

// Course
// https://cs184.eecs.berkeley.edu/su20/docs/HalfEdgePrimer

// https://computergraphics.stackexchange.com/questions/10754/half-edge-data-structure-with-holes


// https://computergraphics.stackexchange.com/questions/9566/how-to-handle-half-edge-boundary-edge-iteration-from-vertex
// Could iterate though each vertex. Check if it belongs to an edge that is at the boundary and select the one with the twin null for example. If the mesh is manifold there should only be one such edge going in a given direction.
struct DoublyConnectedEdgeList {
	using HalfedgeIndex = i32;
	using VertexIndex = i32;
	using FaceIndex = i32;
	static constexpr HalfedgeIndex NULL_HALFEDGE_INDEX = -1;
	static constexpr FaceIndex NULL_FACE_INDEX = -1;

	void initialize(View<const Vec3> vertices, View<const i32> facesIndices, View<const i32> verticesPerFace);

	struct Halfedge {
		HalfedgeIndex twin; 
		HalfedgeIndex next;
		HalfedgeIndex previous;
		FaceIndex face;
		// The halfedge points from the origin.
		VertexIndex origin;
	};

	struct Face {
		HalfedgeIndex halfedge;
	};

	struct Vertex {
		HalfedgeIndex halfedge;

		Vec3 position;
	};

	std::vector<Vertex> vertices;
	std::vector<Halfedge> halfedges;
	std::vector<Face> faces;

};