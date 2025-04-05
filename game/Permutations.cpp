#include "Permutations.hpp"
#include "Assertions.hpp"
#include "HashCombine.hpp"

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

std::unordered_map<Permutation, GroupWord> generatePermutationGroup(std::vector<Permutation> generators, i32 groupSize) {
	std::unordered_map<Permutation, GroupWord> generatedElements;

	const auto alphabetSize = generators.size();

	auto generateFromWord = [&generators](const GroupWord& word) {
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
				generatedElements.try_emplace(generateFromWord(word), GroupWord(word));
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

	// There is an algortihm that can find the order of a permutation group
	// https://en.wikipedia.org/wiki/Schreier%E2%80%93Sims_algorithm

	return generatedElements;
}
