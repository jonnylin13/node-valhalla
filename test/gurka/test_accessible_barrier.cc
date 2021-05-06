#include "gurka.h"
#include "test.h"
#include "baldr/rapidjson_utils.h"


using namespace valhalla;


class AccessibleBarriers : public ::testing::Test {
protected:
  static gurka::map map;

  static void SetUpTestSuite() {
    constexpr double gridsize = 100;

    const std::string ascii_map = R"(
             B-1--C
            /      \
           /        \
          /          \
         /            \
        A------2-------D
         \            /
          F------3---E
    )";

    const gurka::ways ways = {
        {"AB", {{"highway", "primary"}}},
        {"B1C", {{"highway", "primary"}}},
        {"CD", {{"highway", "primary"}}},
        {"A2D", {{"highway", "primary"}}},
        {"AF", {{"highway", "primary"}}},
        {"F3", {{"highway", "primary"}}},
        {"3E", {{"highway", "primary"}}},
        {"ED", {{"highway", "primary"}}},
    };

    // TODO: no access specified. Should be equal to access:no for barriers.
    const gurka::nodes nodes = {
        // gate is opened
        {"1", {{"barrier", "gate"}, {"access", "yes"}}},
        // access is not specified, gate is permanently closed.
        {"2", {{"barrier", "gate"}}},
        // gate open for bicycle only.
        {"3", {{"barrier", "lift_gate"}, {"bicycle", "yes"}}}, // {"access", "no"},
    };

    const auto layout = gurka::detail::map_to_coordinates(ascii_map, gridsize);
    map = gurka::buildtiles(layout, ways, nodes, {}, "test/data/accessible_barriers");
  }
};

gurka::map AccessibleBarriers::map = {};


TEST_F(AccessibleBarriers, Auto) {
  const std::string cost = "auto";
  auto result = gurka::do_action(valhalla::Options::route, map, {"A", "D"}, cost);
  gurka::assert::raw::expect_path(result, {"AB", "B1C", "B1C", "CD"});
}

TEST_F(AccessibleBarriers, Bicycle) {
  const std::string cost = "bicycle";
  auto result = gurka::do_action(valhalla::Options::route, map, {"A", "D"}, cost);
  gurka::assert::raw::expect_path(result, {"AF", "F3", "3E", "ED"});

}
