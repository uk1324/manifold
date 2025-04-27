#pragma once
#include <vector>
#include <Types.hpp>

std::vector<i32> firstCombination(i32 subSetSize);
// Enumerates all the increasing sequences of subSetSize numbers from { 0, ..., setSize - 1 }.
// There is a bijection between subsets of subSetSize elements and the increasing sequences.
bool nextCombination(std::vector<i32>& v, i32 subSetSize, i32 setSize);

struct CombinationsIter {
	CombinationsIter(i32 setSize, i32 subsetSize);

	bool next();

	std::vector<i32> values;

	auto begin() -> decltype(values.begin());
	auto end() -> decltype(values.end());

	i32 setSize;
	i32 subsetSize;
};

i32 nChoosek(i32 n, i32 k);

/*
Example
CombinationsIter combination(5, 3);
do {
	for (const auto& item : combination) {
		std::cout << item << " ";
	}
	std::cout << '\n';
} while (combination.next());
*/