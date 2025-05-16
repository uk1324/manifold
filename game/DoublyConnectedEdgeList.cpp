#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include "DoublyConnectedEdgeList.hpp"
#include <unordered_map>
#include <HashCombine.hpp>
#include <optional>

using OrientedEdgeId = std::pair<DoublyConnectedEdgeList::VertexIndex, DoublyConnectedEdgeList::VertexIndex>;


struct OrientedEdgeIdHash {
	usize operator()(OrientedEdgeId id) const {
		return hashCombine(id.first, id.second);
	}
};

struct OrientedEdgeIdEqual {
	usize operator()(OrientedEdgeId a, OrientedEdgeId b) const {
		return a.first == b.first && a.second == b.second;
	}
};

void DoublyConnectedEdgeList::initialize(View<const Vec3> vertices, View<const i32> facesIndices, View<const i32> verticesPerFace) {

	std::unordered_map<OrientedEdgeId, HalfedgeIndex, OrientedEdgeIdHash, OrientedEdgeIdEqual> orientedEdgeIdToHalfedge;

	for (const auto& vertex : vertices) {
		this->vertices.push_back(Vertex{ .position = vertex });
	}

	i32 offsetInFacesIndices = 0;
	for (const auto faceVertexCount : verticesPerFace) {
		ASSERT(faceVertexCount >= 3);

		const FaceIndex faceIndex = faces.size();
		faces.push_back(Face{});
		const i32 faceVerticesStartOffset = vertices.size();

		const i32 faceHalfedgesStartOffset = halfedges.size();
		faces[faceIndex].halfedge = faceHalfedgesStartOffset;
		const auto faceEdgeCount = faceVertexCount;

		for (i32 i = 0; i < faceEdgeCount; i++) {
			const auto startVertexIndex = facesIndices[offsetInFacesIndices + i];
			const auto endVertexIndex = facesIndices[offsetInFacesIndices + (i + 1) % faceVertexCount];

			const auto halfedgeIndex = halfedges.size();
			halfedges.push_back(Halfedge{});
			auto& halfedge = halfedges.back();

			halfedge.origin = startVertexIndex;
			// This will get overriden multiple times.
			this->vertices[startVertexIndex].halfedge = halfedgeIndex;
			halfedge.face = faceIndex;

			const auto result = orientedEdgeIdToHalfedge.try_emplace(
				OrientedEdgeId{ startVertexIndex, endVertexIndex },
				halfedgeIndex
			);
			const auto newItem = result.second;
			// If it already exists then either it is more than 2 faces sharing a vertex (non-manifold feature) or the orientation of some face is wrong.
			ASSERT(newItem);
		}

		auto previousHalfedgeIndex = halfedges.size() - 1;
		for (i32 i = 0; i < faceEdgeCount; i++) {
			const auto startVertexIndex = facesIndices[offsetInFacesIndices + i];
			const auto endVertexIndex = facesIndices[offsetInFacesIndices + (i + 1) % faceVertexCount];

			const auto halfedgeIndex = faceHalfedgesStartOffset + i;
			auto& halfedge = halfedges[halfedgeIndex];
			halfedge.previous = previousHalfedgeIndex;
			halfedge.next = faceHalfedgesStartOffset + (i + 1) % faceEdgeCount;
			const auto twin = orientedEdgeIdToHalfedge.find(OrientedEdgeId{ endVertexIndex, startVertexIndex });
			if (twin == orientedEdgeIdToHalfedge.end()) {
				halfedge.twin = NULL_HALFEDGE_INDEX;
			} else {
				halfedge.twin = twin->second;
				auto& twinsTwin = halfedges[halfedge.twin].twin;
				// This probably should be detected also by the previous assert.
				ASSERT(twinsTwin == NULL_HALFEDGE_INDEX);
				twinsTwin = halfedgeIndex;
			}
			previousHalfedgeIndex = halfedgeIndex;
		}

		offsetInFacesIndices += faceVertexCount;
	}
	
	// The code below add twin halfedges to all the edges on the boundary and sets their face to NULL. The previous code just set those to null.
	// This makes iterating simpler. For example if you wanted to iterate around a vertex on a boundary without doing this then you would at some point encounter a null twin. Then to iterate over all faces you would need go back the other way (technically if the mesh is nonmanifold there could be a for example be 2 triangles sharing only a single vertex, because they are non-connected triangles comming you wouldn't be able to iterate (this also creates ambiogous ordering, because you can rotate on of the triangles 180 degrees for example)).

	std::vector<HalfedgeIndex> boundaryHalfedges;
	for (HalfedgeIndex i = 0; i < halfedges.size(); i++) {
		if (halfedges[i].twin == NULL_HALFEDGE_INDEX) {
			boundaryHalfedges.push_back(i);
		}
	}
	std::vector<bool> visited;
	visited.resize(boundaryHalfedges.size(), false);
	// The boundary halfedges essentially for a directed graph. And we want to either get though all the edges using either a single or multiple cycles. In the case of non-manifold meshes the "cycles" maybe visit the same vertex multiple times. For example think of something like the radioactive symbol (or maybe a pyramid triangulated into 3 layers with the holes being the radioactive symbol).
	// If the mesh is manifold the boundaries should be disjoint cycles.

	for (;;) {
		std::optional<HalfedgeIndex> startEdgeIndex;
		for (HalfedgeIndex i = 0; i < visited.size(); i++) {
			if (!visited[i]) {
				startEdgeIndex = boundaryHalfedges[i];
				visited[i] = true;
				break;
			}
		}
		if (!startEdgeIndex.has_value()) {
			break;
		}

		HalfedgeIndex currentEdgeIndex = *startEdgeIndex;
		HalfedgeIndex previousHalfedgeIndex = NULL_HALFEDGE_INDEX; // Filled in later.
		// This way pointers are set is based on just drawing and image of a hole and trying to make the hole go in the opposite way to the faces.
		do {
			const auto oppositeIndex = halfedges.size();
			halfedges.push_back(Halfedge{});
			auto& opposite = halfedges.back();

			auto& current = halfedges[currentEdgeIndex];
			current.twin = oppositeIndex;

			opposite.face = NULL_FACE_INDEX;
			opposite.origin = halfedges[current.next].origin;
			if (previousHalfedgeIndex != NULL_HALFEDGE_INDEX) {
				opposite.previous = halfedges[previousHalfedgeIndex].twin;
				halfedges[halfedges[previousHalfedgeIndex].twin].next = oppositeIndex;
			}
			opposite.twin = currentEdgeIndex;

			bool foundNext = false;
			for (i32 i = 0; i < boundaryHalfedges.size(); i++) {
				const auto& halfedge = boundaryHalfedges[i];
				// Could check if there are multiple. If it is a manifold then there shouldn't be.
				if (halfedges[halfedges[halfedge].next].origin == current.origin) {
					previousHalfedgeIndex = currentEdgeIndex;
					currentEdgeIndex = halfedge;
					foundNext = true;
					if (visited[i]) {
						ASSERT(halfedge == startEdgeIndex);
						const auto startBoundaryIndex = halfedges[*startEdgeIndex].twin;
						halfedges[startBoundaryIndex].previous = oppositeIndex;
						halfedges[oppositeIndex].next = startBoundaryIndex;
						break;
					} else {
						visited[i] = true;
						break;
					}
				}
			}
			ASSERT(foundNext);
		} while (currentEdgeIndex != startEdgeIndex);
	}
	// Could there be hole cycles of length 2? 
}

DoublyConnectedEdgeList::FacesAroundVertexIterator DoublyConnectedEdgeList::facesAroundVertex(VertexIndex vertexIndex) {
	auto& vertex = vertices[vertexIndex];
	CHECK(halfedges[vertex.halfedge].origin == vertexIndex);
	return FacesAroundVertexIterator(*this, vertex.halfedge);
}

DoublyConnectedEdgeList::FacesAroundVertexIterator DoublyConnectedEdgeList::facesAroundVertex(const Vertex& vertex) {
	return facesAroundVertex(vertexReferenceToIndex(vertex));
}

DoublyConnectedEdgeList::FaceIndex DoublyConnectedEdgeList::faceReferenceToIndex(const Face& face) const {
	// This is kind of stupid.
	// This is basically useful so that you can iterate using the iterator syntax without creating special interators for indicies.
	// Basically so that you can do this
	// for (const auto& face : list.faces)
	// Instead of 
	// for (DoublyConnectedEdgeList::FaceIndex faceIndex = 0; faceIndex++ < list.faces.size(); faceIndex++);
	return i32(&face - &*faces.begin());
}

DoublyConnectedEdgeList::VertexIndex DoublyConnectedEdgeList::vertexReferenceToIndex(const Vertex& vertex) const {
	return &vertex - &*vertices.begin();
}

DoublyConnectedEdgeList::VerticesAroundFaceIterator DoublyConnectedEdgeList::verticesAroundFace(FaceIndex faceIndex) {
	auto& halfedge = faces[faceIndex].halfedge;
	CHECK(halfedges[halfedge].face == faceIndex);
	return VerticesAroundFaceIterator(*this, halfedge);
}

DoublyConnectedEdgeList::VerticesAroundFaceIterator DoublyConnectedEdgeList::verticesAroundFace(const Face& face) {
	return verticesAroundFace(&face - &*faces.begin());
}

Vec3 DoublyConnectedEdgeList::computeFaceCentroid(FaceIndex face) {
	i32 count = 0;
	Vec3 centroid(0.0f);
	for (const auto vertexIndex : verticesAroundFace(face)) {
		const auto position = vertices[vertexIndex].position;
		count++;
		centroid += position;
	}
	centroid /= count;
	return centroid;
}

Vec3 DoublyConnectedEdgeList::computeFaceCentroid(const Face& face) {
	return computeFaceCentroid(faceReferenceToIndex(face));
}

DoublyConnectedEdgeList::HalfedgeIndex DoublyConnectedEdgeList::rotatePositivelyAroundOrigin(HalfedgeIndex halfedge) {
	auto& edge = halfedges[halfedge];
	return halfedges[edge.previous].twin;
}

DoublyConnectedEdgeList::FacesAroundVertexIterator::FacesAroundVertexIterator(DoublyConnectedEdgeList& list, HalfedgeIndex halfedge) 
	: list(list)
	, current(halfedge)
	, startedIterating(false) {}

bool DoublyConnectedEdgeList::FacesAroundVertexIterator::operator==(const FacesAroundVertexIterator& other) const {
	CHECK(&other.list == &list);
	return (current == other.current) && (startedIterating == other.startedIterating);
}

bool DoublyConnectedEdgeList::FacesAroundVertexIterator::operator!=(const FacesAroundVertexIterator& other) const {
	return !(*this == other);
}

DoublyConnectedEdgeList::FacesAroundVertexIterator& DoublyConnectedEdgeList::FacesAroundVertexIterator::operator++() {
	do {
		current = list.rotatePositivelyAroundOrigin(current);
	} while (list.halfedges[current].face == NULL_FACE_INDEX);
	startedIterating = true;
	return *this;
}

DoublyConnectedEdgeList::FaceIndex DoublyConnectedEdgeList::FacesAroundVertexIterator::operator*() const {
	return list.halfedges[current].face;
}

DoublyConnectedEdgeList::FacesAroundVertexIterator DoublyConnectedEdgeList::FacesAroundVertexIterator::begin() {
	auto copy = *this;
	copy.startedIterating = false;
	return copy;
}

DoublyConnectedEdgeList::FacesAroundVertexIterator DoublyConnectedEdgeList::FacesAroundVertexIterator::end() {
	auto copy = *this;
	copy.startedIterating = true;
	return copy;
}



DoublyConnectedEdgeList::VerticesAroundFaceIterator::VerticesAroundFaceIterator(DoublyConnectedEdgeList& list, HalfedgeIndex halfedge)
	: list(list)
	, current(halfedge)
	, startedIterating(false) {}

bool DoublyConnectedEdgeList::VerticesAroundFaceIterator::operator==(const VerticesAroundFaceIterator& other) const {
	CHECK(&other.list == &list);
	return (current == other.current) && (startedIterating == other.startedIterating);
}

bool DoublyConnectedEdgeList::VerticesAroundFaceIterator::operator!=(const VerticesAroundFaceIterator& other) const {
	return !(*this == other);
}

DoublyConnectedEdgeList::VerticesAroundFaceIterator& DoublyConnectedEdgeList::VerticesAroundFaceIterator::operator++() {
	current = list.halfedges[current].next;
	startedIterating = true;
	return *this;
}

DoublyConnectedEdgeList::VertexIndex DoublyConnectedEdgeList::VerticesAroundFaceIterator::operator*() const {
	return list.halfedges[current].origin;
}

DoublyConnectedEdgeList::VerticesAroundFaceIterator DoublyConnectedEdgeList::VerticesAroundFaceIterator::begin() {
	auto copy = *this;
	copy.startedIterating = false;
	return copy;
}

DoublyConnectedEdgeList::VerticesAroundFaceIterator DoublyConnectedEdgeList::VerticesAroundFaceIterator::end() {
	auto copy = *this;
	copy.startedIterating = true;
	return copy;
}
