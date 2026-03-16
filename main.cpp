// ██████╗ ██╗         █████╗ ██████╗ ██████╗ ██████╗  ██████╗ ██╗  ██╗██╗███╗   ███╗ █████╗ ████████╗ ██████╗ ██████╗ 
// ██╔══██╗██║        ██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔═══██╗╚██╗██╔╝██║████╗ ████║██╔══██╗╚══██╔══╝██╔═══██╗██╔══██╗
// ██████╔╝██║        ███████║██████╔╝██████╔╝██████╔╝██║   ██║ ╚███╔╝ ██║██╔████╔██║███████║   ██║   ██║   ██║██████╔╝
// ██╔═══╝ ██║        ██╔══██║██╔═══╝ ██╔═══╝ ██╔══██╗██║   ██║ ██╔██╗ ██║██║╚██╔╝██║██╔══██║   ██║   ██║   ██║██╔══██╗
// ██║     ██║        ██║  ██║██║     ██║     ██║  ██║╚██████╔╝██╔╝ ██╗██║██║ ╚═╝ ██║██║  ██║   ██║   ╚██████╔╝██║  ██║
// ╚═╝     ╚═╝        ╚═╝  ╚═╝╚═╝     ╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝
// --------------------------------------------------------------------------------
//   Pi Approximator - Multithreaded Pi Approximation using Monte Carlo method
//   © 2026 ItzFalco08 | https://github.com/ItzFalco08/PiApproximator
// ================================================================================                                            

#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <cmath>
#include "./cpuname.hpp"
#include <future>
#include <iomanip>

struct Vec2 {
    double x;
    double y;

    constexpr Vec2() : x(0), y(0) {}
    constexpr Vec2(double x, double y) : x(x), y(y) {}

    void operator+=(const Vec2& v) { x += v.x; y += v.y; };
};

const float SIDE = 1.0f;
const float RADIUS = 0.5f;

long long dotCount;
int cpuPower;
std::atomic<long> globalWorkDone{0};

void progressPrinter(std::atomic<long>& workDone, long long work) {
    const int barWidth = 20;
    while (workDone < work) {
        float ratio = static_cast<float>(workDone.load()) / static_cast<float>(work);
        int filled = static_cast<int>(ratio * (float)barWidth);

        std::cout << "\r[" << std::string(filled, '#') << std::string(barWidth - filled, ' ') 
                  << "] " << std::fixed << std::setprecision(1) << (ratio * 100.0) << "%" << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::cout << "\r[####################] 100.0%" << std::endl;
}

Vec2 threadWork(long long Work) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, SIDE);

    double nrCircle = 0;
    const double centerX = 0.5, centerY = 0.5;

    for (long long i = 0; i < Work; ++i) {
        double px = dist(gen) - centerX;
        double py = dist(gen) - centerY;

        // Optimized: x*x + y*y <= r*r is faster than sqrt()
        if (px*px + py*py <= (RADIUS * RADIUS)) {
            nrCircle += 1.0;
        }

        globalWorkDone++;

        if (cpuPower < 100)  std::this_thread::sleep_for(std::chrono::milliseconds(100 - cpuPower));
    }
    return Vec2(nrCircle, (double)Work);
}

int main() {
    std::cout << "Dot Count: ";
    std::cin >> dotCount;
    if (dotCount <= 0) { 
        std::cout << "[Error] value must be positive integer" << std::endl; 
        return -1; 
    }

    std::cout << "CPU Power (0 - 100): ";
    std::cin >> cpuPower;
    if (cpuPower < 0 || cpuPower > 100) { 
        std::cout << "\n[Error] Please Enter a value from: [0, 100]" << std::endl; 
        return -1; 
    }

    int nrThreads = std::thread::hardware_concurrency();
    long long workPerThread = dotCount / nrThreads;
    long long actualTotal = workPerThread * nrThreads;

    std::cout << "Approximating PI on " << nrThreads << " threads of " << getCPUModel() << std::endl;
    
    // UI thread starts
    std::thread Tprogresslog(progressPrinter, std::ref(globalWorkDone), actualTotal);

    std::vector<std::future<Vec2>> futures;
    for (int i = 0; i < nrThreads; i++) {
        futures.push_back(std::async(std::launch::async, threadWork, workPerThread));
    }

    // Wait for calculations to finish, then UI
    Vec2 finalOutput;
    for (auto& future : futures) {
        finalOutput += future.get();
    }

    if (Tprogresslog.joinable()) Tprogresslog.join();

    std::cout << "\nApproximated pi: " << std::setprecision(10) << 4.0 * (finalOutput.x / finalOutput.y) << std::endl;

    return 0;
}