#pragma once


template<typename T>
T clamp(T value, T low, T high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}