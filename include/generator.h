#pragma once

#include <complex>
#include <cstdint>
#include <thread>
#include <memory>
#include <mutex>
#include <vector>

#include "abstract_image.h"

using namespace std::complex_literals;

enum class status {
	Running,
	Stopping,
	Paused,
	Stopped
};

enum class displayMode {
	Linear,
	Logarithmic
};

class generator {
	using real = double;
	using Int = uint64_t;
public:
	generator(uint16_t width, uint16_t height);
	// void initialize();
private:
	void task(size_t thread_index);

public:
	// runtime data
	status m_status;
	std::vector<std::thread> threads;
	std::vector<Int> threads_progress;
	std::unique_ptr<abstractImage<Int>> image_ptr;
	std::mutex image_ptr_mutex;
	Int runtime_total_points;
	Int runtime_batch_points;
	std::mutex runtime_batch_points_mutex;

	// parameters
	uint32_t threads_number { 3 };
	Int batch_points { 10000000 };
	Int batch_points_per_thread { batch_points / threads_number };
	Int total_points_target { 1000000000 };
	std::complex<real> boundaryA {  0.75 - 1.5i };
	std::complex<real> boundaryB { -2.25 + 1.5i };
	Int iterations_to_escape { 1000000 };
	real escape_norm { 4.0 };

	bool realtime_drawing;
};