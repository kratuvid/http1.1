#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <format>
#include <iostream>
#include <sstream>

#include "defines.hpp"

import http;

using testing::MatchesRegex;

#if HTTP_TEST_CONDITIONAL_EXPR == 1
#include "tests/log.inl"
#endif

