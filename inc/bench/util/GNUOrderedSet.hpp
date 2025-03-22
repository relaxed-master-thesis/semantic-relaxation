#pragma once

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <functional>

using namespace __gnu_pbds;

// Ordered set that supports order statistics
template <typename T, typename Compare>
using ordered_set = tree<T, null_type, Compare, rb_tree_tag,
						 tree_order_statistics_node_update>;