#pragma once

#include <vector>
#include <unordered_map>
#include <Types.hpp>

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

// A group word is a list of indices to the generators.
using GroupWord = std::vector<i32>;
std::unordered_map<Permutation, GroupWord> generatePermutationGroup(std::vector<Permutation> generators, i32 groupSize);