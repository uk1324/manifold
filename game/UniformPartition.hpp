#pragma once

#include <Types.hpp>

template<typename T>
struct UniformPartition {
	struct Point {
		Point(i64 i, i64 n, T min, T max);

		Point& operator++();
		bool operator==(const Point& other) const;
		Point& operator*();

		i64 i;
		i64 n;
		T min, max;
		T x;
	};

	UniformPartition(i64 n, T min, T max);

	Point begin();
	Point end();

	i64 n;
	T min, max;
};


template<typename T>
UniformPartition<T>::UniformPartition(i64 n, T min, T max)
	: n(n)
	, min(min)
	, max(max) {}

template<typename T>
UniformPartition<T>::Point UniformPartition<T>::begin() {
	return Point(0, n, min, max);
}

template<typename T>
UniformPartition<T>::Point UniformPartition<T>::end() {
	return Point(n, n, min, max);
}

template<typename T>
UniformPartition<T>::Point::Point(i64 i, i64 n, T min, T max)
	: i(i)
	, n(n)
	, min(min)
	, max(max)
	, x(min + (max - min) * (f32(i) / f32(n - 1))) {}

template<typename T>
typename UniformPartition<T>::Point& UniformPartition<T>::Point::operator++() {
	i++;
	x = min + (max - min) * (f32(i) / f32(n - 1));
	return *this;
}

template<typename T>
bool UniformPartition<T>::Point::operator==(const Point& other) const {
	return i == other.i;
}

template<typename T>
typename UniformPartition<T>::Point& UniformPartition<T>::Point::operator*() {
	return *this;
}