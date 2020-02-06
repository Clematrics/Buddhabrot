#include "generator.h"

#include <algorithm>
#include <random>

#include "helper.h"
#include "image.h"
#include "mandelbrot_helper.h"
#include "monte_carlo_sampler.h"

generator::generator(std::shared_ptr<abstractImage> image_ptr_in, generator_properties& properties_in, generator_parameters& parameters_in, generator_runtime_parameters& runtime_parameters_in) {
	image_ptr = image_ptr_in;
	properties = properties_in;
	parameters = parameters_in;
	runtime_parameters = runtime_parameters_in;

	pool_points_done = 0;
	total_points_done = 0;

	m_status = status::Stopped;
	initiate();
}

generator::~generator() {
	stop();
}

void generator::join_all_threads_and_clear() {
	for (auto& thread : threads) {
		thread.join();
	}
	threads.clear();
	threads_points_done.clear();
	threads_batch_size.clear();
}

void generator::set_parameters(generator_parameters& parameters_in) {
	if (m_status != status::Stopped)
		return;
	parameters = parameters_in;
}

void generator::set_runtime_parameters(generator_runtime_parameters& runtime_parameters_in) {
	if (m_status != status::Stopped)
		return;
	runtime_parameters = runtime_parameters_in;
	initiate();
}

void generator::initiate() {
	if (m_status != status::Stopped)
		return;

	m_order = order::Pause;
	m_status = status::Paused;
	for (size_t i {0} ; i < runtime_parameters.threads_number ; i++) {
		threads_points_done.emplace_back(0);
		threads_batch_size.emplace_back(0);
		threads.emplace_back(std::thread([i, this]{ this->task(i); }));
	}
}

void generator::resume() {
	if (m_status != status::Paused)
		return;

	m_order = order::Run;
	m_status = status::Running;
}

void generator::pause() {
	if (m_status != status::Running)
		return;

	m_order = order::Pause;
	m_status = status::Paused;
}

void generator::finish_bash() {
	if (m_status != status::Running)
		return;

	m_order = order::FinishBatch;
}

void generator::stop() {
	m_order = order::Stop;
	m_status = status::Stopping;
	join_all_threads_and_clear();
	m_status = status::Stopped;
}

status generator::get_status() {
	return m_status;
}

std::vector<std::pair<Int, Int>> generator::progress() {
	std::vector<std::pair<Int, Int>> res;
	std::lock_guard<std::mutex> lock(access_progress_mutex);
	for (size_t i { 0 } ; i < runtime_parameters.threads_number ; i++) {
		res.emplace_back(threads_points_done[i], threads_batch_size[i]);
	}
	return res;
}

std::pair<Int, Int> generator::pool_progress() {
	Int ongoing { std::accumulate(threads_points_done.begin(), threads_points_done.end(), (Int)0) };
	return std::make_pair(pool_points_done + ongoing, runtime_parameters.pool_batch_size);
}

std::pair<Int, Int> generator::total_progress() {
	Int ongoing { std::accumulate(threads_points_done.begin(), threads_points_done.end(), (Int)0) };
	return std::make_pair(total_points_done + ongoing, runtime_parameters.points_target);
}

Int generator::request_batch(size_t thread_index) {
	std::lock_guard<std::mutex> lock(access_progress_mutex);
	Int ongoing { std::accumulate(threads_batch_size.begin(), threads_batch_size.end(), (Int)0) };
	Int batch_target { runtime_parameters.thread_batch_size };

	if (runtime_parameters.pool_batch_size != 0) { // points_target == 0 means that there is no limit of points for the pool
		Int remaining_in_pool { runtime_parameters.pool_batch_size - pool_points_done - ongoing }; // should be always greater or equal than 0
		if (batch_target > remaining_in_pool)
			batch_target = remaining_in_pool;
	}

	if (runtime_parameters.points_target != 0) { // points_target == 0 means that there is no limit of points in total
		Int remaining_in_total { runtime_parameters.points_target - total_points_done - ongoing }; // should be always greater or equal than 0
		if (batch_target > remaining_in_total)
			batch_target = remaining_in_total;
	}

	threads_batch_size[thread_index] = batch_target;
	return batch_target;
}

void generator::save_progress(size_t thread_index, Int& batch_done, Int& batch_target) {
	if (batch_done == 0) // nothing to save
		return;

	std::lock_guard<std::mutex> lock(access_progress_mutex);
	pool_points_done += batch_done;
	total_points_done += batch_done;
	if (pool_points_done == runtime_parameters.pool_batch_size) // reset the pool progression when the goal was reached
		pool_points_done = 0;

	batch_done = 0;
	batch_target = 0;

	threads_points_done[thread_index] = 0;
	threads_batch_size[thread_index] = 0;
}

void generator::task(size_t thread_index) {
	using namespace std::chrono_literals;

	monte_carlo_sampler sampler(properties.corner_a, properties.corner_b, properties.layers, properties.layer_resolution);

	// // setup random generator
	// std::random_device rd;
	// std::ranlux48 engine(rd());
	// auto [real_m, real_M] = minmax(properties.corner_a.real(), properties.corner_b.real());
	// auto [imag_m, imag_M] = minmax(properties.corner_a.imag(), properties.corner_b.imag());
	// std::uniform_real_distribution<Real> real_distrib(real_m, real_M);
	// std::uniform_real_distribution<Real> imag_distrib(imag_m, imag_M);

	bool must_request_batch { true };
	Int batch_target { 0 };
	Int batch_done   { 0 };
	std::vector<std::complex<Real>> seq;
	seq.reserve(parameters.iterations_to_escape);

paused_state:
	std::this_thread::sleep_for(100ms);
	if (m_order == order::Run)
		goto running_state;
	if (m_order == order::Stop)
		goto stopped_state;
	goto paused_state;


running_state:
	while (batch_done == batch_target && (batch_target = request_batch(thread_index)) == 0) {
		std::this_thread::sleep_for(1s); // wait

		if (m_order == order::Pause)
			goto paused_state;
		if (m_order == order::Stop)
			goto stopped_state;
	}
	// batch requested with success

	// iterate on batch points
	while (batch_done < batch_target) {
		if (m_order == order::Pause)
			goto paused_state;
		if (m_order == order::Stop)
			goto stopped_state;

		// process one point
		sample_result sample;
		{
			std::lock_guard<std::mutex> lock(sample_mutex);
			sample = sampler.sample();
		}
		std::complex<Real> z0 = sample.sample;
		if (insideCardioids(z0)) {
			sample.feedback_result(0, parameters.iterations_to_escape);
			continue;
		}

		seq.clear();
		Int i = 0;
		std::complex<Real> z = z0;
		for ( ; i < parameters.iterations_to_escape && std::norm(z) < parameters.escape_norm ; i++) {
			seq.push_back(z);
			z = z * z + z0;
		}

		// the sequence didn't escaped before the limit : it is not taken into account
		if (i == parameters.iterations_to_escape || i < parameters.minimum_iterations) {
			sample.feedback_result(0, parameters.iterations_to_escape);
			continue;
		}
		if (i == 0 && std::norm(z) >= parameters.escape_norm) {
			// if the sample z0 is out of the norm at the first iteration, penalize the monte carlo tree
			sample.feedback_result(0, parameters.iterations_to_escape);
			continue;
		}

		// apply the sequence to the image and feedback the result to the sampler
		{
			Int successful_points = 0;

			// To avoid calling it each time in the lambda
			Real real_m = properties.corner_a.real();
			Real real_M = properties.corner_b.real();
			Real imag_m = properties.corner_a.imag();
			Real imag_M = properties.corner_b.imag();

			std::lock_guard<std::mutex> lock(image_ptr_mutex);
			std::for_each(seq.begin(), seq.end(), [&](auto z){
				if (z.real() < real_m
				||	real_M < z.real()
				||	z.imag() < imag_m
				||	imag_M < z.imag())
					return;

				uint16_t x = (z.real() - real_m) / (real_M - real_m) * static_cast<Real>(properties.image_width);
				uint16_t y = (z.imag() - imag_m) / (imag_M - imag_m) * static_cast<Real>(properties.image_height);

				image_ptr->incr(x, y);
				successful_points++;

				if (parameters.y_symetry) {
					uint16_t sym_y = properties.image_height - y - 1; // y is in [0, height-1], so -1 to get the result into [0,height-1] and avoid out of range
					if (sym_y != y)		// avoid increasing twice the center line if the image has an odd height
						image_ptr->incr(x, sym_y);
				}
			});

			sample.feedback_result(successful_points, parameters.iterations_to_escape);
		}
		batch_done++;
		threads_points_done[thread_index] = batch_done;
	}

	// batch finished, save points processed, reset progress and request new batch
	save_progress(thread_index, batch_done, batch_target);
	if (m_order == order::FinishBatch)
		goto paused_state;
	goto running_state;


stopped_state:
	save_progress(thread_index, batch_done, batch_target);
}