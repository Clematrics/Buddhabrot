#include "generator.h"

#include <algorithm>
#include <random>

#include "image.h"
#include "mandelbrot_helper.h"

generator::generator(uint16_t width, uint16_t height) {
	m_status = status::Paused;
	for (size_t i = 0; i < threads_number; i++) {
		threads.emplace_back(std::thread([this]{ this->task(); }));
		threads_progress.push_back(0.);
	}

	image_ptr = std::make_unique<image<uint64_t>>(width, height);
	runtime_batch_points = 0;
	runtime_total_points = 0;
}

template<class T>
std::pair<const T, const T> minmax(const T a, const T b)
{
    return (b < a) ? std::pair<const T, const T>(b, a)
                   : std::pair<const T, const T>(a, b);
}

void generator::task() {
	using namespace std::chrono_literals;

	// setup random generator
	std::random_device rd;
	std::ranlux48 engine(rd());
	auto real_minmax = minmax(boundaryA.real(), boundaryB.real());
	auto imag_minmax = minmax(boundaryA.imag(), boundaryB.imag());
	real real_min = real_minmax.first, real_max = real_minmax.second;
	real imag_min = imag_minmax.first, imag_max = imag_minmax.second;
	std::uniform_real_distribution<real> real_distrib(real_min, real_max);
	std::uniform_real_distribution<real> imag_distrib(imag_min, imag_max);

	while (m_status != status::Stopped && m_status != status::Stopping) {
		while(m_status == status::Paused) {
			std::this_thread::sleep_for(100ms);
		}

		// doing another thread_batch
		// acquiring size of the thread batch
		Int points_target = 0;
		threads_progress[0] = 0.;
		{
			std::lock_guard<std::mutex> lock(runtime_batch_points_mutex);
			points_target = std::min(batch_points - runtime_batch_points, batch_points_per_thread);
			runtime_batch_points += points_target;
		}
		if (points_target == 0) {
			// sleep if nothing to do, and retry
			std::this_thread::sleep_for(100ms);
			continue;
		}

		std::vector<std::complex<real>> seq;
		seq.reserve(std::min(iterations_to_escape, seq.max_size()));
		for (Int points_number = 0; points_number < points_target; /* points are added manually */) {
			// generate a random point inside the boundary
			std::complex<real> z0(real_distrib(engine), imag_distrib(engine));
			if (insideCardioids(z0))
				continue;

			seq.clear();
			Int i = 0;
			std::complex<real> z = z0;
			for ( ; i < iterations_to_escape && std::norm(z) < escape_norm; i++) {
				seq.push_back(z);

				z = z * z + z0;
			}

			// the sequence didn't escaped before the limit : it is not taken into account
			if (i == iterations_to_escape)
				continue;


			{
				std::lock_guard<std::mutex> lock(image_ptr_mutex);
				std::for_each(seq.begin(), seq.end(), [&](auto z){
					if (z.real() < real_min
					||	real_max < z.real()
					||	z.imag() < imag_min
					||	imag_max < z.imag())
						return;
					uint16_t x = (z.real() - real_min) / (real_max - real_min) * static_cast<real>(image_ptr->width());
					uint16_t y = (z.imag() - imag_min) / (imag_max - imag_min) * static_cast<real>(image_ptr->height());
					image_ptr->incr(x, y);
				});
			}

			points_number++;
			threads_progress[0] = points_number / points_target;
		}
	}
}