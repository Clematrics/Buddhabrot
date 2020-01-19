#include "generator.h"

#include <algorithm>
#include <random>

#include "helper.h"
#include "image.h"
#include "mandelbrot_helper.h"

generator::generator(std::shared_ptr<abstractImage> image_ptr_in, generator_properties& properties_in, generator_parameters& parameters_in, generator_runtime_parameters& runtime_parameters_in) {
	image_ptr = image_ptr_in;
	properties = properties_in;
	parameters = parameters_in;
	runtime_parameters = runtime_parameters_in;

	total_progress = 0;

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
	threads_progress.clear();
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
		threads_progress.emplace_back(0);
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
		res.emplace_back(threads_progress[i], threads_batch_size[i]);
	}
	return res;
}

Int generator::request_batch(size_t thread_index) {
	std::lock_guard<std::mutex> lock(access_progress_mutex);
	Int done_or_ongoing { total_progress + std::accumulate(threads_batch_size.begin(), threads_batch_size.end(), 0) };
	Int remaining { runtime_parameters.points_target - done_or_ongoing }; // should be always greater or equal than 0
	Int batch_target { remaining };

	if (runtime_parameters.thread_batch_size <= remaining)
		batch_target = runtime_parameters.thread_batch_size;

	threads_batch_size[thread_index] = batch_target;
	return batch_target;
}

void generator::save_progress(size_t thread_index, Int& batch_done, Int& batch_target) {
	if (batch_done == 0) // nothing to save
		return;

	std::lock_guard<std::mutex> lock(access_progress_mutex);
	total_progress += batch_done;

	batch_done = 0;
	batch_target = 0;

	threads_progress[thread_index] = 0;
	threads_batch_size[thread_index] = 0;
}

void generator::task(size_t thread_index) {
	using namespace std::chrono_literals;

	// setup random generator
	std::random_device rd;
	std::ranlux48 engine(rd());
	auto [real_m, real_M] = minmax(properties.corner_a.real(), properties.corner_b.real());
	auto [imag_m, imag_M] = minmax(properties.corner_a.imag(), properties.corner_b.imag());
	std::uniform_real_distribution<Real> real_distrib(real_m, real_M);
	std::uniform_real_distribution<Real> imag_distrib(imag_m, imag_M);

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
		std::complex<Real> z0(real_distrib(engine), imag_distrib(engine));
		if (insideCardioids(z0))
			continue;

		seq.clear();
		Int i = 0;
		std::complex<Real> z = z0;
		for ( ; i < parameters.iterations_to_escape && std::norm(z) < parameters.escape_norm ; i++) {
			seq.push_back(z);
			z = z * z + z0;
		}

		// the sequence didn't escaped before the limit : it is not taken into account
		if (i == parameters.iterations_to_escape)
			continue;

		// apply the sequence to the image
		{
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
			});
		}
		batch_done++;
		threads_progress[thread_index] = batch_done;
	}
	// batch finished, save points processed, reset progress and request new batch
	save_progress(thread_index, batch_done, batch_target);
	if (m_order == order::FinishBatch)
		goto paused_state;
	goto running_state;


stopped_state:
	save_progress(thread_index, batch_done, batch_target);
}