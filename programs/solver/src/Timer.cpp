#include "Timer.hpp"

std::chrono::time_point<std::chrono::steady_clock> Timer::startTime;

void Timer::start() {
    startTime = std::chrono::steady_clock::now();
}

double Timer::elapsedMs() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
    return static_cast<double>(duration.count()) / 1000.0;
}

bool Timer::isTimeExceeded(double limitMs) {
    return elapsedMs() >= limitMs;
}
