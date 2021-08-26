#pragma once

#include <gtest/gtest.h>
#include <fstream>
#include <string.h>
#include <algorithm>

std::ofstream GetLogFile() {
  auto testName = std::string(::testing::UnitTest::GetInstance()->current_test_info()->name());
  auto testCName = std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_case_name());
  std::replace(testName.begin(), testName.end(), '/', '_');
  std::replace(testCName.begin(), testCName.end(), '/', '_');
  std::ofstream myFile("reports/"+ testCName + "_" + testName + ".csv");
  return myFile;
}