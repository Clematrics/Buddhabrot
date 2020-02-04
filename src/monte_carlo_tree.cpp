#include "monte_carlo_tree.h"

#include <algorithm>
#include <cmath>

#include <iostream>

monte_carlo_tree::monte_carlo_tree(uint16_t layers, uint16_t layer_resolution) : root(this, nullptr, layers, layer_resolution) {
	total = 0;
}

monte_carlo_tree::path monte_carlo_tree::sample_path() {
	path p;
	root.sample(p);
	return p;
}

void monte_carlo_tree::feedback(path& p, Int success, Int total) {
	this->total += total;

	root.feedback(p, success, total);
}

bool monte_carlo_tree::node::has_children() const {
	return !children.empty();
}

void monte_carlo_tree::node::sample(monte_carlo_tree::path& p) const {
	if (!has_children())
		return;

	auto it = std::max_element(children.begin(), children.end(), [](const auto& a, const auto& b){
		return a.second.score() < b.second.score();
	});
	p.push(it->first);
	children.at(it->first).sample(p);

	// coordinate c = std::make_pair(0, 0);
	// double best_score = children.at(c).score();
	// for (const auto& e : children) {
	// 	double score = e.second.score();
	// 	if (score > best_score) {
	// 		best_score = score;
	// 		c = e.first;
	// 	}
	// }
	// p.push(c);
	// children.at(c).sample(p);
}

void monte_carlo_tree::node::feedback(path& p, Int success, Int total) {
	rate.first += success;
	rate.second += total;
	if (p.empty())
		return;
	coordinate coord = p.front();
	p.pop();
	children.at(coord).feedback(p, success, total);
}

double monte_carlo_tree::node::score() const {
	if (rate.second == 0)
		return std::numeric_limits<double>::infinity();

	double success = rate.first;
	double local_total = rate.second;
	double total = parent->rate.second;
	return  success / local_total + std::sqrt(2. * std::log(total) / local_total );
}

monte_carlo_tree::node::node(monte_carlo_tree* root, node* parent, uint16_t layers, uint16_t layer_resolution) {
	rate.first = 0;
	rate.second = 0;
	this->root = root;
	this->parent = parent;

	if (layers <= 0)
		return;

	for (size_t i = 0; i < layer_resolution; i++)
		for (size_t j = 0; j < layer_resolution; j++)
			children.insert(std::make_pair(std::make_pair(i, j), node(root, this, layers - 1, layer_resolution)));
}