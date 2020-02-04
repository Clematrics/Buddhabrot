#pragma once

#include <map>
#include <queue>

#include "types.h"

class monte_carlo_tree {
public:
	monte_carlo_tree(uint16_t layers, uint16_t layer_resolution);
	using coordinate = std::pair<uint16_t, uint16_t>;
	using path = std::queue<coordinate>;

	path sample_path();
	void feedback(path& path, Int success, Int total);

	struct node {
	public:
		node(monte_carlo_tree* root, node* parent, uint16_t layers, uint16_t layer_resolution);

		bool has_children() const;
		void sample(monte_carlo_tree::path& p) const;
		void feedback(monte_carlo_tree::path& p, Int success, Int total);
		double score() const;

		std::pair<Int, Int> rate;
		std::map<monte_carlo_tree::coordinate, node> children;
	private:
		node* parent;			// non-owning ptr
		monte_carlo_tree* root; // non-owning ptr
	};
private:
	Int total;
	node root;
};