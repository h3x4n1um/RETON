#pragma once

#include <fifo_map.hpp>
#include <nlohmann/json.hpp>

namespace reton {
	// https://github.com/nlohmann/json/issues/485

	// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
	template<class K, class V, class dummy_compare, class A>
	using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
	using fifo_json = nlohmann::basic_json<workaround_fifo_map>;
}
