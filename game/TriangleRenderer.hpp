#pragma once

#include <vector>
//#include <engine/Graphics/Vbo.hpp>
//#include <engine/Graphics/Vbo.hpp>
#include <gfx/Instancing.hpp>
#include <game/MeshUtils.hpp>

template<typename Vertex>
struct TriangleRenderer {
	template<typename Shader>
	static TriangleRenderer make(Vbo& instancesVbo);

	i32 currentIndex() const;
	std::vector<i32> indices;
	std::vector<Vertex> vertices;
	Vbo vbo;
	Ibo ibo;
	Vao vao;

	i32 addVertex(const Vertex& vertex);
	void addTri(i32 i0, i32 i1, i32 i2);
	void addQuad(i32 i00, i32 i01, i32 i11, i32 i10);
	void tri(const Vertex& v0, const Vertex& v1, const Vertex& v2);
};

template<typename Vertex>
template<typename Shader>
TriangleRenderer<Vertex> TriangleRenderer<Vertex>::make(Vbo& instancesVbo) {
	auto vbo = Vbo::generate();
	auto ibo = Ibo::generate();
	auto vao = createInstancingVao<Shader>(vbo, ibo, instancesVbo);
	return TriangleRenderer<Vertex>{
		.vbo = std::move(vbo),
			.ibo = std::move(ibo),
			.vao = std::move(vao),
	};
}

template<typename Vertex>
i32 TriangleRenderer<Vertex>::currentIndex() const {
	return i32(indices.size());
}

template<typename Vertex>
i32 TriangleRenderer<Vertex>::addVertex(const Vertex& vertex) {
	const auto index = vertices.size();
	vertices.push_back(vertex);
	return i32(index);
}

template<typename Vertex>
void TriangleRenderer<Vertex>::addTri(i32 i0, i32 i1, i32 i2) {
	indicesAddTri(indices, i0, i1, i2);
}

template<typename Vertex>
void TriangleRenderer<Vertex>::addQuad(i32 i00, i32 i01, i32 i11, i32 i10) {
	indicesAddQuad(indices, i00, i01, i11, i10);
}

template<typename Vertex>
void TriangleRenderer<Vertex>::tri(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
	addTriangle(addVertex(v0), addVertex(v1), addVertex(v2));
}
