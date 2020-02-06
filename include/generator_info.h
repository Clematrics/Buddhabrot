#pragma once

#include <complex>

#include "types.h"

using namespace std::complex_literals;


enum class sampler_type {
	Uniform,
	MonteCarlo
};

// Properties cannot be changed once the generator has been created
struct generator_properties {
	uint16_t image_width         { 720 };
	uint16_t image_height        { 720 };
	// ensure that corner_a < corner_b coordinate-wise
	std::complex<Real> corner_a  { -2.25 - 1.5i };
	std::complex<Real> corner_b  { +0.75 + 1.5i };

	sampler_type sampler_t       { sampler_type::MonteCarlo };
	// MonteCarlo properties
	Int layers                   { 2 };
	Int layer_resolution         { 8 };
};

// Parameters control the behavior of sequences and can be changed
struct generator_parameters {
	Int iterations_to_escape     { 1000000 };
	Int minimum_iterations       { 100000  };
	Real escape_norm             { 4.0 };
	bool y_symetry               { false };
};

// Runtime parameters describe how to dispatch the computing of sequences
struct generator_runtime_parameters {
	uint32_t threads_number      { 4 };
	Int pool_batch_size          { 100000 };
	Int thread_batch_size        { pool_batch_size / threads_number };
	Int points_target            { 0 };
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