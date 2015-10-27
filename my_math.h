#pragma once

template <typename T>
int sign_of(T value) {
	return -(std::signbit(value) * 2 - 1);
}