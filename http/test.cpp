#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <sstream>

#include "defines.hpp"
import http;

using testing::MatchesRegex;

class LogFixture : public testing::Test {
protected:
  void SetUp() override {
    cerr_rdbuf = std::cerr.rdbuf();
    std::cerr.rdbuf(oss.rdbuf());
  }

  void TearDown() override {
    std::cerr.rdbuf(cerr_rdbuf);
  }

protected:
  std::streambuf* cerr_rdbuf;
  std::ostringstream oss;
};


TEST_F(LogFixture, Empty) {
  // HTTP_LOG_INFO("Stinky butts");
  std::cerr << "Stinky butts";
  EXPECT_THAT(oss.str(), MatchesRegex(R"(Stinky butts)"));
}
