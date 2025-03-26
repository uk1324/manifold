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

	/*
	auto startHalfedge = vertex.halfedge
	auto currentHalfedge = startHalfedge
	do {
		// Boundary halfdege
		if (currentHalfedge.face == NULL) {
			continue;
		}

		// Process face.

		currentHalfedge = rotatePositivelyAroundOrigin(currentHalfedge);
	} while (currentHalfedge != startHalfedge);

	*/
	struct FacesAroundVertexIterator {
		FacesAroundVertexIterator(DoublyConnectedEdgeList& list, HalfedgeIndex halfedge);

		bool operator==(const FacesAroundVertexIterator& other) const;
		bool operator!=(const FacesAroundVertexIterator& other) const;

		FacesAroundVertexIterator& operator++();
		FaceIndex operator*() const;

		FacesAroundVertexIterator begin();
		FacesAroundVertexIterator end();

		DoublyConnectedEdgeList& list;
		HalfedgeIndex current;
		bool startedIterating;
		// To implement an iterator working using a regular for loop the pmp library uses a flag to simulate the do while loop.
		// https://github.com/pmp-library/pmp-library/blob/main/src/pmp/surface_mesh.h#L893
	};
	FacesAroundVertexIterator facesAroundVertex(VertexIndex vertexIndex);
	FacesAroundVertexIterator facesAroundVertex(const Vertex& vertex);

	struct VerticesAroundFaceIterator {
		VerticesAroundFaceIterator(DoublyConnectedEdgeList& list, HalfedgeIndex halfedge);

		bool operator==(const VerticesAroundFaceIterator& other) const;
		bool operator!=(const VerticesAroundFaceIterator& other) const;

		VerticesAroundFaceIterator& operator++();
		VertexIndex operator*() const;

		VerticesAroundFaceIterator begin();
		VerticesAroundFaceIterator end();

		DoublyConnectedEdgeList& list;
		HalfedgeIndex current;
		bool startedIterating;
	};
	FaceIndex faceReferenceToIndex(const Face& face) const;
	VertexIndex vertexReferenceToIndex(const Vertex& face) const;

	VerticesAroundFaceIterator verticesAroundFace(FaceIndex faceIndex);
	VerticesAroundFaceIterator verticesAroundFace(const Face& face);

	Vec3 computeFaceCentroid(FaceIndex face);
	Vec3 computeFaceCentroid(const Face& face);

	// The positive orietnation is the one that the input faces have.
	HalfedgeIndex rotatePositivelyAroundOrigin(HalfedgeIndex halfedge);

	std::vector<Vertex> vertices;
	std::vector<Halfedge> halfedges;
	std::vector<Face> faces;

};