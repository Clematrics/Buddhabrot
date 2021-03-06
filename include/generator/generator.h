#pragma once

#include <complex>
#include <cstdint>
#include <thread>
#include <memory>
#include <mutex>
#include <vector>

#include "image/abstract_image.h"
#include "generator/generator_info.h"
#include "types.h"

using namespace std::complex_literals;

class generator {
public:
	generator(std::shared_ptr<abstractImage> image, generator_properties& properties, generator_parameters& parameters, generator_runtime_parameters& runtime_parameters);
	~generator();

	void set_parameters(generator_parameters& parameters);
	void set_runtime_parameters(generator_runtime_parameters& runtime_parameters);
	void initiate();
	void resume();
	void pause();
	void finish_bash();
	void stop();

	status get_status();
	std::vector<std::pair<Int, Int>> progress();
	std::pair<Int, Int> pool_progress();
	std::pair<Int, Int> total_progress();
private:
	void task(size_t thread_index);
	void join_all_threads_and_clear();

	Int request_batch(size_t thread_index);
	void save_progress(size_t thread_index, Int& batch_done, Int& batch_target);

	std::mutex image_ptr_mutex;
	std::shared_ptr<abstractImage> image_ptr;
public:
	generator_properties properties;
	generator_parameters parameters;
	generator_runtime_parameters runtime_parameters;

private:
	// runtime data
	status m_status;
	order m_order;
	std::vector<std::thread> threads;

	std::mutex sample_mutex; // lock any access to the sampler
	std::mutex access_progress_mutex; // lock any access to the batch size of each thread and the total progress i.e when requesting a batch or saving progress
	std::vector<Int> threads_points_done;
	std::vector<Int> threads_batch_size;
	Int pool_points_done;
	Int total_points_done;
};