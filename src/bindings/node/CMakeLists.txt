cmake_minimum_required(VERSION 3.1)

message(STATUS "Building node-valhalla")

set(BINDING_DIR "${PROJECT_SOURCE_DIR}/node_modules/node-cmake")

include(NodeJS)
nodejs_init()

message(STATUS "Configuring node-valhalla bindings for NodeJS ${NODEJS_VERSION}")

add_nodejs_module(node-valhalla node.cc)
set_target_properties(node-valhalla PROPERTIES CXX_STANDARD 14)
set_target_properties(node-valhalla PROPERTIES FOLDER "Node Bindings")
target_link_libraries(node-valhalla valhalla)

target_include_directories(node-valhalla PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")

set(ARTIFACTS "")

add_custom_command(
  OUTPUT ${BINDING_DIR}/node-valhalla.node
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:node-valhalla> ${BINDING_DIR}
  DEPENDS node-valhalla ${BINDING_DIR}
)
list(APPEND ARTIFACTS "${BINDING_DIR/node-valhalla.node}")

message(STATUS "node-valhalla artifacts will be copied to: ${BINDING_DIR}")
add_custom_target(copy_node_artifacts ALL DEPENDS ${ARTIFACTS})
set_target_properties(copy_node_artifacts PROPERTIES FOLDER "Node Bindings")