#pragma once

#include <complex>

#include "types.h"

using namespace std::complex_literals;

// Properties cannot be changed once the generator has been created
struct generator_properties {
	uint16_t image_width         { 720 };
	uint16_t image_height        { 720 };
	std::complex<Real> corner_a  {  0.75 - 1.5i };
	std::complex<Real> corner_b  { -2.25 + 1.5i };
};

// Parameters control the behavior of sequences and can be changed
struct generator_parameters {
	Int iterations_to_escape     { 1000000 };
	Real escape_norm             { 4.0 };
	bool y_symetry               { false };
};

// Runtime parameters describe how to dispatch the computing of sequences
struct generator_runtime_parameters {
	uint32_t threads_number      { 4 };
	Int pool_batch_size          { 10000000 };
	Int thread_batch_size        { pool_batch_size / threads_number };
	Int points_target            { 1000000000 };
};

enum class status {
	Running,
	Stopping,
	Paused,
	Stopped
};

enum class order {
	Run,
	Pause,
	FinishBatch,
	Stop
};

std::string_view status_to_string(status s);