#include "Polyhedra.hpp"
#include <game/DoublyConnectedEdgeList.hpp>
#include <engine/Math/Angles.hpp>
#include <unordered_map>

std::vector<Quat> cubeDirectIsometries() {
	std::vector<Quat> result;

	result.push_back(Quat::identity);

	{
		// axes though centers of faces
		Vec3 axes[]{
 			Vec3(1.0f, 0.0f, 0.0f),
			Vec3(0.0f, 1.0f, 0.0f),
			Vec3(0.0f, 0.0f, 1.0f),
		};
		for (const auto& axis : axes) {
			result.push_back(Quat(PI<f32> / 2.0f, axis));
			result.push_back(Quat(-PI<f32> / 2.0f, axis));
			result.push_back(Quat(PI<f32>, axis));
		}
	}
	{
		// axes though midpoints of edges
		Vec3 axes[]{
			Vec3(0.0f, 1.0f, 1.0f),
			Vec3(0.0f, -1.0f, 1.0f),
			Vec3(1.0f, 0.0f, 1.0f),
			Vec3(-1.0f, 0.0f, 1.0f),
			Vec3(1.0f, 1.0f, 0.0),
			Vec3(1.0f, -1.0f, 0.0),
		};
		for (const auto& axis : axes) {
			result.push_back(Quat(PI<f32>, axis.normalized()));
		}
	}
	{
		// axes though vertices
		Vec3 axes[]{
			// These are just the vertices of the top face
			Vec3(1.0f, 1.0f, 1.0f),
			Vec3(-1.0f, 1.0f, 1.0f),
			Vec3(-1.0f, -1.0f, 1.0f),
			Vec3(1.0f, -1.0f, 1.0f),
		};
		for (const auto& axis : axes) {
			result.push_back(Quat(TAU<f32> / 3.0f, axis.normalized()));
			result.push_back(Quat(-TAU<f32> / 3.0f, axis.normalized()));
		}
	}
	return result;
}

struct Permutation {
	static Permutation identity(i32 size);
	static Permutation uninitialized(i32 size);
	Permutation(std::initializer_list<i32> values);
	static Permutation fromOneIndexed(std::initializer_list<i32> values);

	// f(i) = data[i]
	std::vector<i32> data;

	i32 size() const;

	i32& operator()(i32 i);
	const i32 operator()(i32 i) const;
	void operator*=(const Permutation& other);
	Permutation operator*(const Permutation& other) const;

	bool operator==(const Permutation& other) const;
private:
	explicit Permutation() {};
};

template <>
struct std::hash<Permutation> {
	usize operator()(const Permutation& p) const;
};

Quat mat3ToQuatUnchecked(const Mat3& ma) {
	//auto m = ma.transposed();
	auto m = ma;
	Quat result = Quat::identity;
	f32 t = 1.0f;
	if (m(2, 2) < 0) {
		if (m(0, 0) > m(1, 1)) {
			t = 1 + m(0, 0) - m(1, 1) - m(2, 2);
			result = Quat(t, m(0, 1) + m(1, 0), m(2, 0) + m(0, 2), m(1, 2) - m(2, 1));
		} else {
			t = 1 - m(0, 0) + m(1, 1) - m(2, 2);
			result = Quat(m(0, 1) + m(1, 0), t, m(1, 2) + m(2, 1), m(2, 0) - m(0, 2));
		}
	} else {
		if (m(0, 0) < -m(1, 1)) {
			t = 1 - m(0, 0) - m(1, 1) + m(2, 2);
			result = Quat(m(2, 0) + m(0, 2), m(1, 2) + m(2, 1), t, m(0, 1) - m(1, 0));
		} else {
			t = 1 + m(0, 0) + m(1, 1) + m(2, 2);
			result = Quat(m(1, 2) - m(2, 1), m(2, 0) - m(0, 2), m(0, 1) - m(1, 0), t);
		}
	}
	result *= 0.5 / sqrt(t);
	return result.normalized();
}

//std::vector<Quat> icosahedronDirectIsometries() {
//	// If we think of the isometries as acting on a dodecahedron then the isometries can be divided into
//	// identity
//	// rotation with an axis though a face center. Pairs of faces represent an axis. There are 12 faces => 6 axes. The order of rotation is 5 so there are 4 unique rotations. (12 / 2) * 4.
//	// rotation with an axis though a vertex. There are 20 vertices => 10 axes. The oreder of rotation is 3 so there are 2 unique rotations. (20 / 2) * 2.
//	// rotation with an axis though the midpoint of an edge. There are 30 edges => 15 axes. There order of rotation is 2 so there is one unique rotation. (30 / 2) * 1.
//	// These groups are disjoint, because each group only contains elements of one order and each group contains elements of different orders.
//	// All of these are unique, because they have different orders. The orders of powers of an element are divisors of the order of that element. So for order 5 the divisors are 5, 1, for 3 its 3, 1 and for 2 its 2, 1
//	// This method generalizes to the symmetry groups of other polyhedra.
//
//	// The vertices can be split into mirror images. Then one half of them will represent the axes.
//	// The pairs of vertices represent an edge. Again for each pair there is an opposite pair.
//
//	using Word = std::vector<i32>;
//	std::unordered_map<Permutation, Word> generatedElements;
//
//	const auto groupSize = 60;
//
//	// https://math.stackexchange.com/questions/1999312/group-presentation-of-a-5-with-two-generators
//	//std::vector<Permutation> generators{
//	//	////Permutation{ 5, 1, 2, 3, 4 }, // order 5
//	//	////Permutation{ 2, 1, 4, 3, 5 } // order 2
//	//	//Permutation{ 4, 0, 1, 2, 3 }, // order 5
//	//	////Permutation{ 1, 0, 3, 2, 4 } // order 2 (0 1)(2 3)
//	//	//Permutation{ 0, 2, 1, 4, 3 } // order 2 (1 2)(3 4)
//
//	//	//Permutation{ 5, 1, 2, 3, 4 }, // order 5
//	//	//Permutation{ 2, 1, 4, 3, 5 } // order 2
//	//	Permutation{ 4, 0, 1, 2, 3 }, // order 5
//	//		//Permutation{ 1, 0, 3, 2, 4 } // order 2 (0 1)(2 3)
//	//	//Permutation{ 0, 2, 1, 4, 3 } // order 2 (1 2)(3 4)
//	//	Permutation{ 4, 3, 2, 1, 0 } // order 2 (1 5)(2 4) = (0 4)(1 3)
//	//};
//	//// https://en.wikipedia.org/wiki/Icosahedral_symmetry#Coxeter_group_generators
//	//// I just selected the ones with the right order.
//	//std::vector<Quat> generatorsQuaternions {
//	//	Quat(TAU<f32> / 5.0f, Vec3(10.9931, 9.08697, 1.0f).normalized()),
//	//	//Quat(TAU<f32> / 5.0f, Vec3(0.0f, -1.0f, (sqrt(5) + 1.0f) / 2.0f)),
//	//	Quat(TAU<f32> / 2.0f, Vec3(0.0f, 0.0f, 1.0f)),
//	//};
//
//	std::vector<Permutation> generators{
//		Permutation::fromOneIndexed({ 5, 1, 2, 3, 4 }), // (1 2 3 4 5)
//		Permutation::fromOneIndexed({ 2, 1, 4, 3, 5 }), // (1 2)(3 4)
//	};
//	// https://en.wikipedia.org/wiki/Icosahedral_symmetry#Coxeter_group_generators
//	// I just selected the ones with the right order.
//	const auto p = (sqrt(5.0f) + 1.0f) / 2.0f;
//	std::vector<Quat> generatorsQuaternions {
//		mat3ToQuatUnchecked(Mat3{
//			-1.0f / (2.0f * p), p / 2.0f, -0.5f,
//			p / 2.0f, 0.5f, 1.0f / (2.0f * p), 
//			0.5f, -1.0f / (2.0f * p), -p / 2.0f
//		}),
//		mat3ToQuatUnchecked(Mat3{
//			-0.5f, 1.0f / (2.0f * p), p / 2.0f,
//			1 / (2.0f * p) -p / 2.0f, 0.5f,
//			p / 2.0f, 0.5f, 1 / (2.0f * p)
//		})
//	};
//
//	const auto alphabetSize = generators.size();
//
//	auto generateFromWord = [&generators](const Word& word) {
//		if (word.size() == 0) {
//			return Permutation::identity(generators[0].size());
//		}
//		Permutation p = generators[word[0]];
//		for (i32 i = 1; i < word.size(); i++) {
//			p *= generators[word[i]];
//		}
//		return p;
//	};
//
//	// There is an algortihm that can find the order of a permutation group
//	// https://en.wikipedia.org/wiki/Schreier%E2%80%93Sims_algorithm
//
//	// To generate all the elements this has to go though word up to length 10.
//	// This means that it checks
//	// 3^10 + 3^9 + 3^8 + 3^7 + 3^6 + 3^5 + 3^4 + 3^3 + 3^2 + 3^1 = 88572
//	// words
//
//	i32 wordLetterCountCurrent = 1;
//	std::vector<i32> word;
//	while (generatedElements.size() < groupSize) {
//		word.clear();
//		word.resize(wordLetterCountCurrent);
//
//		// This is a really naive algorithm to generate the group. It basically just tries each word by generating permutations with repetition untill it generates all the elements.
//		// It would be faster to start with the generators and then apply all the generators to them. Then to what you get again apply all the generators. This would require only 3^10 operations.
//
//		// Instead of doing this could just iterate from 0 to alphabetSize^wordSize and create the representation in the alphabetSize-ary number system to get the word.
//		i32 currentLetter = 0;
//		for (;;) {
//			if (word[currentLetter] >= alphabetSize) {
//				if (currentLetter == 0) {
//					break;
//				} else {
//					currentLetter--;
//					word[currentLetter]++;
//					continue;
//				}
//			}
//
//			if (currentLetter < wordLetterCountCurrent - 1) {
//				currentLetter++;
//				word[currentLetter] = 0;
//			} else {
//				generatedElements.try_emplace(generateFromWord(word), Word(word));
//				word[currentLetter]++;
//			}
//		}
//
//		wordLetterCountCurrent++;
//	}
//	// You can get non identity words of any size, by basically going back and forth. For example if you have an element g of order 5 then g^(5k + 1) is equal to g. 
//	// If it exists then the hamiltonian path though the caley graph will have order equal to size of the group. If you have a permutation on n elements then the hamiltonian path can't be longer than n! so that the words with length bigger than that will not generate anything new.
//	// https://en.wikipedia.org/wiki/Lov%C3%A1sz_conjecture.
//	// Not every Caley graph has a Hamiltionoan cycle. For example if they are not connected. I guess if you has generators such that one, generated one half (subgroup) the other the other half (subgroup) then it wouldn't be connected.
//	// I guess you could split it into connected components, but then the generating set would need to include each element and it's inverse, because as stated in the above article there are directed Caley graphs that don't have a hamiltionial cycle.
//	// https://en.wikipedia.org/wiki/Steinhaus%E2%80%93Johnson%E2%80%93Trotter_algorithm
//
//	std::vector<Quat> r;
//
//	for (const auto& [permutation, word] : generatedElements) {
//		if (word.size() == 0) {
//			r.push_back(Quat::identity);
//			continue;
//		}
//		auto generatedQuat = generatorsQuaternions[word[0]];
//		for (i32 i = 1; i < word.size(); i++) {
//			generatedQuat = generatedQuat * generatorsQuaternions[word[i]];
//			generatedQuat = generatedQuat.normalized();
//		}
//		r.push_back(generatedQuat);
//	}
//
//	return r;
//}

#include <Put.hpp>

std::vector<Mat3> icosahedronDirectIsometries() {
	// If we think of the isometries as acting on a dodecahedron then the isometries can be divided into
	// identity
	// rotation with an axis though a face center. Pairs of faces represent an axis. There are 12 faces => 6 axes. The order of rotation is 5 so there are 4 unique rotations. (12 / 2) * 4.
	// rotation with an axis though a vertex. There are 20 vertices => 10 axes. The oreder of rotation is 3 so there are 2 unique rotations. (20 / 2) * 2.
	// rotation with an axis though the midpoint of an edge. There are 30 edges => 15 axes. There order of rotation is 2 so there is one unique rotation. (30 / 2) * 1.
	// These groups are disjoint, because each group only contains elements of one order and each group contains elements of different orders.
	// All of these are unique, because they have different orders. The orders of powers of an element are divisors of the order of that element. So for order 5 the divisors are 5, 1, for 3 its 3, 1 and for 2 its 2, 1
	// This method generalizes to the symmetry groups of other polyhedra.

	// The vertices can be split into mirror images. Then one half of them will represent the axes.
	// The pairs of vertices represent an edge. Again for each pair there is an opposite pair.

	using Word = std::vector<i32>;
	std::unordered_map<Permutation, Word> generatedElements;

	const auto groupSize = 60;

	// https://math.stackexchange.com/questions/1999312/group-presentation-of-a-5-with-two-generators
	//std::vector<Permutation> generators{
	//	////Permutation{ 5, 1, 2, 3, 4 }, // order 5
	//	////Permutation{ 2, 1, 4, 3, 5 } // order 2
	//	//Permutation{ 4, 0, 1, 2, 3 }, // order 5
	//	////Permutation{ 1, 0, 3, 2, 4 } // order 2 (0 1)(2 3)
	//	//Permutation{ 0, 2, 1, 4, 3 } // order 2 (1 2)(3 4)

	//	//Permutation{ 5, 1, 2, 3, 4 }, // order 5
	//	//Permutation{ 2, 1, 4, 3, 5 } // order 2
	//	Permutation{ 4, 0, 1, 2, 3 }, // order 5
	//		//Permutation{ 1, 0, 3, 2, 4 } // order 2 (0 1)(2 3)
	//	//Permutation{ 0, 2, 1, 4, 3 } // order 2 (1 2)(3 4)
	//	Permutation{ 4, 3, 2, 1, 0 } // order 2 (1 5)(2 4) = (0 4)(1 3)
	//};
	//// https://en.wikipedia.org/wiki/Icosahedral_symmetry#Coxeter_group_generators
	//// I just selected the ones with the right order.
	//std::vector<Quat> generatorsQuaternions {
	//	Quat(TAU<f32> / 5.0f, Vec3(10.9931, 9.08697, 1.0f).normalized()),
	//	//Quat(TAU<f32> / 5.0f, Vec3(0.0f, -1.0f, (sqrt(5) + 1.0f) / 2.0f)),
	//	Quat(TAU<f32> / 2.0f, Vec3(0.0f, 0.0f, 1.0f)),
	//};

	std::vector<Permutation> generators{
		Permutation::fromOneIndexed({ 5, 1, 2, 3, 4 }), // (1 2 3 4 5)
		Permutation::fromOneIndexed({ 2, 1, 4, 3, 5 }), // (1 2)(3 4)
	};
	// https://en.wikipedia.org/wiki/Icosahedral_symmetry#Coxeter_group_generators
	// I just selected the ones with the right order.
	const auto p = (sqrt(5.0f) + 1.0f) / 2.0f;
	std::vector<Mat3> generatorsQuaternions {
		Mat3{
			-1.0f / (2.0f * p), p / 2.0f, -0.5f,
			p / 2.0f, 0.5f, 1.0f / (2.0f * p), 
			0.5f, -1.0f / (2.0f * p), -p / 2.0f
		},
		Mat3{
			-0.5f, 1.0f / (2.0f * p), p / 2.0f,
			1 / (2.0f * p), -p / 2.0f, 0.5f,
			p / 2.0f, 0.5f, 1 / (2.0f * p)
		}
	};

	const auto alphabetSize = generators.size();

	auto generateFromWord = [&generators](const Word& word) {
		if (word.size() == 0) {
			return Permutation::identity(generators[0].size());
		}
		Permutation p = generators[word[0]];
		for (i32 i = 1; i < word.size(); i++) {
			p *= generators[word[i]];
		}
		return p;
	};

	// There is an algortihm that can find the order of a permutation group
	// https://en.wikipedia.org/wiki/Schreier%E2%80%93Sims_algorithm

	// To generate all the elements this has to go though word up to length 10.
	// This means that it checks
	// 3^10 + 3^9 + 3^8 + 3^7 + 3^6 + 3^5 + 3^4 + 3^3 + 3^2 + 3^1 = 88572
	// words

	i32 wordLetterCountCurrent = 1;
	std::vector<i32> word;
	while (generatedElements.size() < groupSize) {
		word.clear();
		word.resize(wordLetterCountCurrent);

		// This is a really naive algorithm to generate the group. It basically just tries each word by generating permutations with repetition untill it generates all the elements.
		// It would be faster to start with the generators and then apply all the generators to them. Then to what you get again apply all the generators. This would require only 3^10 operations.

		// Instead of doing this could just iterate from 0 to alphabetSize^wordSize and create the representation in the alphabetSize-ary number system to get the word.
		i32 currentLetter = 0;
		for (;;) {
			if (word[currentLetter] >= alphabetSize) {
				if (currentLetter == 0) {
					break;
				} else {
					currentLetter--;
					word[currentLetter]++;
					continue;
				}
			}

			if (currentLetter < wordLetterCountCurrent - 1) {
				currentLetter++;
				word[currentLetter] = 0;
			} else {
				generatedElements.try_emplace(generateFromWord(word), Word(word));
				word[currentLetter]++;
			}
		}

		wordLetterCountCurrent++;
	}
	// You can get non identity words of any size, by basically going back and forth. For example if you have an element g of order 5 then g^(5k + 1) is equal to g. 
	// If it exists then the hamiltonian path though the caley graph will have order equal to size of the group. If you have a permutation on n elements then the hamiltonian path can't be longer than n! so that the words with length bigger than that will not generate anything new.
	// https://en.wikipedia.org/wiki/Lov%C3%A1sz_conjecture.
	// Not every Caley graph has a Hamiltionoan cycle. For example if they are not connected. I guess if you has generators such that one, generated one half (subgroup) the other the other half (subgroup) then it wouldn't be connected.
	// I guess you could split it into connected components, but then the generating set would need to include each element and it's inverse, because as stated in the above article there are directed Caley graphs that don't have a hamiltionial cycle.
	// https://en.wikipedia.org/wiki/Steinhaus%E2%80%93Johnson%E2%80%93Trotter_algorithm

	const auto test = 
		Permutation::fromOneIndexed({ 3, 4, 5, 1, 2 }) * 
		Permutation::fromOneIndexed({ 3, 5, 1, 2, 4 });

	std::vector<Mat3> r;

	for (const auto& [permutation, word] : generatedElements) {
		if (word.size() == 0) {
			r.push_back(Mat3(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)));
			continue;
		}
		auto generatedQuat = generatorsQuaternions[word[0]];
		for (i32 i = 1; i < word.size(); i++) {
			put("% % %", generatedQuat[0].length(), generatedQuat[1].length(), generatedQuat[2].length());
			const auto g = generatedQuat * generatedQuat.transposed();
			//generatedQuat = generatedQuat * generatorsQuaternions[word[i]];
			generatedQuat = generatorsQuaternions[word[i]] * generatedQuat;
			//generatedQuat = generatedQuat.normalized();
		}
		r.push_back(generatedQuat);
	}
	put("% % %", generatorsQuaternions[0][0].length(), generatorsQuaternions[0][1].length(), generatorsQuaternions[0][2].length());
	put("% % %", generatorsQuaternions[1][0].length(), generatorsQuaternions[1][1].length(), generatorsQuaternions[1][2].length());
	const auto a = generatorsQuaternions[0] * generatorsQuaternions[0].transposed();
	const auto b = generatorsQuaternions[1] * generatorsQuaternions[1].transposed();

 	return r;
}

FlatShadingResult flatShadeRegularPolyhedron(View<const Vec3> vertices, View<const i32> facesIndices, i32 verticesPerFace) {
	ASSERT(facesIndices.size() % verticesPerFace == 0);
	const auto faceCount = facesIndices.size() / verticesPerFace;
	ASSERT(verticesPerFace >= 3);

	FlatShadingResult result;

 	for (i32 faceI = 0; faceI < faceCount; faceI++) {
		const auto faceOffset = faceI * verticesPerFace;
		auto indexFace = [&](i32 indexInFace) {
			return facesIndices[faceOffset + indexInFace];
		};
		const auto normal = cross(
			vertices[indexFace(1)] - vertices[indexFace(0)],
			vertices[indexFace(2)] - vertices[indexFace(0)]
		).normalized();
		for (i32 vertexI = 0; vertexI < verticesPerFace; vertexI++) {
			const auto vertexIndex = indexFace(vertexI);
			result.positions.push_back(vertices[vertexIndex]);
			result.normals.push_back(normal);
		}
		f32 triangleFanCentralVertexIndexInFace = 0;
		for (i32 i = 0; i < verticesPerFace - 2; i++) {
			result.indices.push_back(faceOffset + triangleFanCentralVertexIndexInFace);
			result.indices.push_back(faceOffset + i + 1);
			result.indices.push_back(faceOffset + (i + 2) % verticesPerFace);
		}
	}
	return result;
}

FlatShadingResult flatShadeConvexPolygonSoup(View<const Vec3> vertices, View<const i32> facesIndices, View<const i32> verticesPerFace) {
	const auto faceCount = verticesPerFace.size();
	FlatShadingResult result;

	i32 faceOffset = 0;
	for (i32 faceI = 0; faceI < faceCount; faceI++) {
		auto indexFace = [&](i32 indexInFace) {
			return facesIndices[faceOffset + indexInFace];
		};
		const auto normal = cross(
			vertices[indexFace(1)] - vertices[indexFace(0)],
			vertices[indexFace(2)] - vertices[indexFace(0)]
		).normalized();

		const auto faceVertexCount = verticesPerFace[faceI];
		ASSERT(faceVertexCount >= 3);

		for (i32 vertexI = 0; vertexI < faceVertexCount; vertexI++) {
			const auto vertexIndex = indexFace(vertexI);
			result.positions.push_back(vertices[vertexIndex]);
			result.normals.push_back(normal);
		}
		// This is just a fan triangulation of a polygon.
		f32 triangleFanCentralVertexIndexInFace = 0;
		for (i32 i = 0; i < faceVertexCount - 2; i++) {
			result.indices.push_back(faceOffset + triangleFanCentralVertexIndexInFace);
			result.indices.push_back(faceOffset + i + 1);
			result.indices.push_back(faceOffset + (i + 2) % faceVertexCount);
		}
		faceOffset += faceVertexCount;
	}
	return result;
}

// "Introduction to geometry" page 273
// If you have a tetrahedron then a reflection in any plane going though an edge and the midpoint of the opposite edge swaps 2 faces. These are transpositions on the set of all faces so that each of these can be identified with a permutation of faces. Further consideration shows that by identifiying the reflection with the transpostion you can create an isomorphism from the tetrahedral group to S_4. The even permutations correspond to proper isometries and the odd permuations to improper ones so that the alternating group A_4 is isomorphic to the group of proper symmetries of a teterahedron.
// Smilarly coloring faces of the other polyhedra (sometimes multiples faces with the same color) can be used to give an isomorphism between:
// the group of proper symmetries of a cube and a tetrahedron is S_4
// the group of all symmetries of an icosahedron and a dodecahedron is S_5, and of the proper symmetries is A_5.

// How did someone find these colorings? Are they unique? How could you come up with them on the spot? It is mentioned that the colorings have no vertex edge or face in common, but this isn't enough, because you can still swap things that break the general sturcture, but still presevere the no vertex edge or face in common condition.
// I guess you could look at the orders of elements.


FlatShadingResult flatShadeConvexPolygonSoup(const PolygonSoup& polygonSoup) {
	return flatShadeConvexPolygonSoup(constView(polygonSoup.positions), constView(polygonSoup.facesVertices), constView(polygonSoup.verticesPerFace));
}

PolygonSoup regularPolyhedronPolygonSoup(View<const Vec3> vertices, View<const i32> facesIndices, i32 verticesPerFace){
	PolygonSoup result;
	for (const auto& vertex : vertices) {
		result.positions.push_back(vertex);
	}
	for (const auto& index : facesIndices) {
		result.facesVertices.push_back(index);
	}
	for (i32 i = 0; i < facesIndices.size() / verticesPerFace; i++) {
		result.verticesPerFace.push_back(verticesPerFace);
	}
	return result;
}

PolygonSoup dualPolyhedron(const PolygonSoup& polyhedron) {
	DoublyConnectedEdgeList mesh;
	mesh.initialize(constView(polyhedron.positions), constView(polyhedron.facesVertices), constView(polyhedron.verticesPerFace));

	PolygonSoup result;
	for (const auto& face : mesh.faces) {
		const auto centroid = mesh.computeFaceCentroid(face);
		result.positions.push_back(centroid);
	}
	for (const auto& vertex : mesh.vertices) {
		i32 count = 0;
		for (const auto& faceIndex : mesh.facesAroundVertex(vertex)) {
			result.facesVertices.push_back(faceIndex);
			count++;
		}
		result.verticesPerFace.push_back(count);
	}
	return result;
}

Permutation Permutation::identity(i32 size) {
	auto result = Permutation::uninitialized(size);
	for (i32 i = 0; i < size; i++) {
		result(i) = i;
	}
	return result;
}

Permutation Permutation::uninitialized(i32 size) {
	Permutation p;
	p.data.resize(size);
	return p;
}

Permutation::Permutation(std::initializer_list<i32> values) {
	data.insert(data.begin(), values.begin(), values.end());
}

Permutation Permutation::fromOneIndexed(std::initializer_list<i32> values) {
	auto result = Permutation::uninitialized(values.size());
	i32 i = 0;
	for (auto& value : values) {
		result(i) = value - 1;
		i++;
	}
	return result;
}

i32 Permutation::size() const {
	return data.size();
}

i32& Permutation::operator()(i32 i) {
	return data[i];
}

const i32 Permutation::operator()(i32 i) const {
	return data[i];
}

void Permutation::operator*=(const Permutation& other) {
	*this = *this * other;
}

Permutation Permutation::operator*(const Permutation& other) const {
	ASSERT(size() == other.size());
	auto r = Permutation::uninitialized(size());
	for (i32 i = 0; i < size(); i++) {
		r(i) = operator()(other(i));
	}
	return r;
}

bool Permutation::operator==(const Permutation& other) const {
	if (size() != other.size()) {
		return false;
	}
	return data == other.data;
}

usize std::hash<Permutation>::operator()(const Permutation& p) const {
	if (p.size() == 0) {
		return 0;
	}
	usize hash = std::hash<i32>()(p(0));
	for (i32 i = 1; i < p.size(); i++) {
		hash = hashCombine(hash, std::hash<i32>()(p(i)));
	}
	return hash;
}
