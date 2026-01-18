#pragma once

#include <chrono>

class Timer {
public:
    // Starts or restarts the global time count
    static void start();

    // Returns the elapsed time in milliseconds since start()
    static double elapsedMs();

    // Checks if the elapsed time has exceeded the provided limit (in ms)
    static bool isTimeExceeded(double limitMs);

private:
    static std::chrono::time_point<std::chrono::steady_clock> startTime;
};
