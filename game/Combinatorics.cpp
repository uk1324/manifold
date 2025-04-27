#include "Combinatorics.hpp"

std::vector<i32> firstCombination(i32 subSetSize) {
	std::vector<i32> result;
	for (i32 i = 0; i < subSetSize; i++) {
		result.push_back(i);
	}
	return result;
}

// Finds all the increasing sequences of subSetSize numbers from { 0, ..., setSize - 1 }.
// There is a bijection between subsets of subSetSize elements and the increasing sequences.
bool nextCombination(std::vector<i32>& v, i32 subSetSize, i32 setSize) {
	// Find an in v that can be increased, while keeping it an increasing sequence.
	i32 indexOfIndexThatCanBeIncreased = subSetSize - 1;
	// If this were an iterator, this calculation could probably be avoided. The iterator would just store the current index it is at.
	for (;;) {
		if (indexOfIndexThatCanBeIncreased < 0) {
			return false;
		}
		const auto maxPossibleValue = setSize - subSetSize + indexOfIndexThatCanBeIncreased;
		if (v[indexOfIndexThatCanBeIncreased] < maxPossibleValue) {
			break;
		}
		indexOfIndexThatCanBeIncreased--;
	}
	v[indexOfIndexThatCanBeIncreased]++;

	// Set all of the values at indices bigger than indexOfIndexThatCanBeIncreased to the consecutive values after it. 
	// So for example if we are in the state 
	// 0 4 5
	// with subset size = 3 and set size = 6 then the indexOfIndexThatCanBeIncreased will be 0 then it gets incremented to
	// 1 4 5
	// And anfter that the loop sets the remaning values to
	// 1 2 3
	// This simulates starting the inner loops from the beginning.
	for (i32 i = indexOfIndexThatCanBeIncreased + 1; i < subSetSize; i++) {
		v[i] = v[indexOfIndexThatCanBeIncreased] + i - indexOfIndexThatCanBeIncreased;
	}
	return true;
}

i32 nChoosek(i32 n, i32 k) {
	i32 result = 1;
	for (i32 i = 0; i < k; i++) {
		result *= n;
		n--;
	}
	// If I just calculated the factorial then I could do only a single division, but then the factorial might overflow. Although if the factorial overflowed then because the previous product is bigger it would also overflow.
	while (k > 0) {
		result /= k;
		k--;
	}
	return result;
}

CombinationsIter::CombinationsIter(i32 setSize, i32 subsetSize)
	: setSize(setSize)
	, subsetSize(subsetSize)
	, values(firstCombination(subsetSize)) {}

bool CombinationsIter::next() {
	return nextCombination(values, subsetSize, setSize);
}

auto CombinationsIter::begin() -> decltype(values.begin()) {
	return values.begin();
}

auto CombinationsIter::end() -> decltype(values.end()) {
	return values.end();
}
