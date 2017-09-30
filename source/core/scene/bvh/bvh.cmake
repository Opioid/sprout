target_sources(core
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/scene_bvh_builder.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scene_bvh_builder.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/scene_bvh_node.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/scene_bvh_node.inl"
  "${CMAKE_CURRENT_LIST_DIR}/scene_bvh_split_candidate.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scene_bvh_split_candidate.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/scene_bvh_tree.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scene_bvh_tree.hpp"
) 

