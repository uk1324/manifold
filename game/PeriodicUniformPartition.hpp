#pragma once

#include <Types.hpp>

// After n - 1 is 0 so that the value for n would be the same as the value for 0.
template<typename T>
struct PeriodicUniformPartition {
	struct Point {
		Point(i64 i, i64 n, T min, T max);

		Point& operator++();
		bool operator==(const Point& other) const;
		Point& operator*();
		operator T() const;

		i64 i;
		i64 n;
		T min, max;
		T x;
	};

	PeriodicUniformPartition(T min, T max, i64 n);

	Point begin();
	Point end();

	i64 n;
	T min, max;
};


template<typename T>
PeriodicUniformPartition<T>::PeriodicUniformPartition(T min, T max, i64 n)
	: n(n)
	, min(min)
	, max(max) {}

template<typename T>
PeriodicUniformPartition<T>::Point PeriodicUniformPartition<T>::begin() {
	return Point(0, n, min, max);
}

template<typename T>
PeriodicUniformPartition<T>::Point PeriodicUniformPartition<T>::end() {
	return Point(n, n, min, max);
}

template<typename T>
PeriodicUniformPartition<T>::Point::Point(i64 i, i64 n, T min, T max)
	: i(i)
	, n(n)
	, min(min)
	, max(max)
	, x(min + (max - min) * (f32(i) / f32(n - 1))) {}

template<typename T>
typename PeriodicUniformPartition<T>::Point& PeriodicUniformPartition<T>::Point::operator++() {
	i++;
	x = min + (max - min) * (f32(i) / f32(n));
	return *this;
}

template<typename T>
bool PeriodicUniformPartition<T>::Point::operator==(const Point& other) const {
	return i == other.i;
}

template<typename T>
typename PeriodicUniformPartition<T>::Point& PeriodicUniformPartition<T>::Point::operator*() {
	return *this;
}

template<typename T>
PeriodicUniformPartition<T>::Point::operator T() const {
	return x;
}
