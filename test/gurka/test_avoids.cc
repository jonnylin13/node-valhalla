#include "gurka.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/multi/geometries/register/multi_polygon.hpp>
#include <gtest/gtest.h>

#include "baldr/graphconstants.h"
#include "baldr/graphreader.h"
#include "loki/polygon_search.h"
#include "midgard/pointll.h"
#include "mjolnir/graphtilebuilder.h"

using namespace valhalla;
namespace bg = boost::geometry;
namespace vm = valhalla::midgard;

BOOST_GEOMETRY_REGISTER_POINT_2D(PointLL, double, bg::cs::geographic<bg::degree>, first, second)
BOOST_GEOMETRY_REGISTER_MULTI_POLYGON(std::vector<bg::model::polygon<PointLL>>)

class AvoidTest : public ::testing::Test {
protected:
  static gurka::map avoid_map;

  static void SetUpTestSuite() {
    const std::string ascii_map = R"(A------B------C
                                     |      |      |
                                     |      |      |
                                     D------E------F)";
    const gurka::ways ways = {{"ABC", {{"highway", "motorway"}, {"name", "High Street"}}},
                              {"DEF", {{"highway", "motorway"}, {"name", "Low Street"}}},
                              {"AD", {{"highway", "motorway"}, {"name", "1st Street"}}},
                              {"BE", {{"highway", "motorway"}, {"name", "2nd Street"}}},
                              {"CF", {{"highway", "motorway"}, {"name", "3rd Street"}}}};
    const auto layout = gurka::detail::map_to_coordinates(ascii_map, 10);
    // Add low area limit for avoid_polygons so it throws an error
    avoid_map = gurka::buildtiles(layout, ways, {}, {}, "test/data/gurka_shortcut",
                                  {{"service_limits.max_avoid_polygons_sqkm", "1"}});
    double dx = avoid_map.nodes.at("B").first - avoid_map.nodes.at("A").first;
    double dy = avoid_map.nodes.at("A").second - avoid_map.nodes.at("D").second;
  }
};

gurka::map AvoidTest::avoid_map = {};

TEST_F(AvoidPolygons, TestArea) {
  // define multipolygon with ~
  std::vector<bg::model::polygon<PointLL>> polys(2);
  bg::read_wkt(
      "POLYGON ((13.38625361 52.4652558, 13.38625361 52.48000128, 13.4181769 52.48000128, 13.4181769 52.4652558, 13.38625361 52.4652558))",
      polys[0]);
  bg::read_wkt(
      "POLYGON ((13.36862753 52.43148012, 13.36862753 52.4555968, 13.39944328 52.4555968, 13.39944328 52.43148012, 13.36862753 52.43148012))",
      polys[1]);
  double actual_area = bg::area(polys);

  // Add polygons to PBF
  Options options;
  auto avoid_polygons = options.mutable_avoid_polygons();
  for (auto& poly : polys) {
    auto* polygon = avoid_polygons->Add();
    for (auto& coord : poly.outer()) {
      auto* ll = polygon->add_coords()->mutable_ll();
      ll->set_lng(coord.lng());
      ll->set_lat(coord.lat());
    }
  }
  auto calc_area = loki::GetArea(options.avoid_polygons());

  EXPECT_EQ(actual_area, calc_area);

  // Add a ~ 3.5 sqkm avoid_polygon so it throws
  const std::unordered_map<std::string, std::string>& req_options = {
      {"/avoid_polygons",
       "[[[13.38625361, 52.4652558], [13.38625361, 52.48000128], [13.4181769, 52.48000128], [13.4181769, 52.4652558]]]"}};

  EXPECT_THROW(gurka::route(avoid_map, "A", "D", "auto", req_options), valhalla_exception_t);
}
