#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <format>
#include <iostream>
#include <sstream>

#include "defines.hpp"
import logger;

using testing::MatchesRegex;

class Logger : public testing::Test {
protected:
  void SetUp() override {
    m_cerr_rdbuf = std::cerr.rdbuf();
    std::cerr.rdbuf(m_oss.rdbuf());

    logger::force_disable_escape_codes();
  }

  void TearDown() override {
    std::cerr.rdbuf(m_cerr_rdbuf);
  }

  auto construct_regex(std::string_view type, std::string_view msg) {
    return std::format(R"({1:}:\s.+:\s.+:{0:}\s\s{2:}(:\s.+)?{0:}?)",
                       '\n', type, msg);
  }

protected:
  std::streambuf* m_cerr_rdbuf;
  std::ostringstream m_oss;
};


TEST_F(Logger, Empty) {
  constexpr std::string_view msg {""};

  LOG_INFO("{}", msg);
  EXPECT_THAT(m_oss.view(), MatchesRegex(construct_regex("INFO", msg)));

  m_oss.str("");
  LOG_INFO_ERRNO("{}", msg);
  EXPECT_THAT(m_oss.view(), MatchesRegex(construct_regex("INFO", msg)));
}

TEST_F(Logger, Info) {
  // Avoid regex special characters
  constexpr std::string_view msg {R"(Compressing objects: 100% 8/8, done)"};

  LOG_INFO("{}", msg);
  EXPECT_THAT(m_oss.view(), MatchesRegex(construct_regex("INFO", msg)));

  m_oss.str("");
  LOG_INFO_ERRNO("{}", msg);
  EXPECT_THAT(m_oss.view(), MatchesRegex(construct_regex("INFO", msg)));
}

TEST_F(Logger, Warning) {
  // Avoid regex special characters
  constexpr std::string_view msg {R"(Running low on buffer space)"};

  LOG_WARN("{}", msg);
  EXPECT_THAT(m_oss.view(), MatchesRegex(construct_regex("WARN", msg)));

  m_oss.str("");
  LOG_WARN_ERRNO("{}", msg);
  EXPECT_THAT(m_oss.view(), MatchesRegex(construct_regex("WARN", msg)));
}

TEST_F(Logger, Error) {
  // Avoid regex special characters
  constexpr std::string_view msg {R"(File descriptor is already invalid)"};

  LOG_ERROR("{}", msg);
  EXPECT_THAT(m_oss.view(), MatchesRegex(construct_regex("ERROR", msg)));

  m_oss.str("");
  LOG_ERROR_ERRNO("{}", msg);
  EXPECT_THAT(m_oss.view(), MatchesRegex(construct_regex("ERROR", msg)));
}

TEST_F(Logger, Fatal) {
  // Avoid regex special characters
  constexpr std::string_view msg {R"(Couldn't create the socket)"};

  std::string actual;
  try {
    LOG_FATAL("{}", msg);
  } catch (std::exception &e) {
    actual = e.what();
  } catch (...) {
    FAIL();
  }
  EXPECT_THAT(actual, MatchesRegex(construct_regex("FATAL", msg)));

  m_oss.str("");
  actual.clear();
  try {
    LOG_FATAL_ERRNO("{}", msg);
  } catch (std::exception &e) {
    actual = e.what();
  } catch (...) {
    FAIL();
  }
  EXPECT_THAT(actual, MatchesRegex(construct_regex("FATAL", msg)));
}
