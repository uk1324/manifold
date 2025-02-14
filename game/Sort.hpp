#pragma once

// https://gamedev.net/forums/topic/401772-fast-sorting-algorithm-for-depth-sorting/3666963/
// Could try radix sort: http://codercorner.com/RadixSortRevisited.htm
// http://codercorner.com/RadixSortRevisited.htm
// Search terms: "Sorting for mostly sorted data", "Depth sorting slow".

template<typename T, typename LessThan>
void bubbleSort(std::vector<T>& v, LessThan lessThan) {

	//auto n = i32(v.size());
	//bool swapped = false;
	//do {
	//	for (i32 i = 1; i <= n - 1; i++) {
	//		if (lessThan(v[i], v[i - 1])) {
	//			std::swap(v[i], v[i - 1]);
	//			swapped = true;
	//		}
	//	}
	//	n -= 1;
	//} while (!swapped);
	const auto size = i32(v.size());
	for (i32 i = 0; i < size - 1; i++) {
		bool swapped = false;
		for (i32 j = 0; j < size - i - 1; j++) {
			if (lessThan(v[j + 1], v[j])) {
				std::swap(v[j], v[j + 1]);
				swapped = true;
			}
		}
		if (!swapped) break;
	}
}

template<typename T, typename LessThan>
void selectionSort(std::vector<T>& v, LessThan lessThan) {
	const auto size = i32(v.size());
	for (i32 i = 0; i < size; i++) {
		i32 jMin = i;
		for (i32 j = i + 1; j < size; j++) {
			if (lessThan(v[j], v[jMin])) {
				jMin = j;
			}
		}
		if (jMin != i) {
			std::swap(v[i], v[jMin]);
		}
	}
}

template<typename T, typename LessThan>
void insertionSort(std::vector<T>& v, LessThan lessThan) {
	const auto size = v.size();
	for (i32 i = 1; i < size; ++i) {
		const auto a = v[i];
		i32 j = i - 1;
		for (; j >= 0 && lessThan(a, v[j]); j--) {
			v[j + 1] = v[j];
		}
		v[j + 1] = a;
	}
}