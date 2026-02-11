#include "logger-stdio.hpp"
#include <stdio.h>

void StdioLogger::InitializeImpl()
{
  printf("StdioLogger initialized\n");
  printf("  - prefix: '%s'\n", prefix_.c_str());
  printf("  - suffix: '%s'\n", suffix_.c_str());
  printf("  - separator: '%s'\n", separator_.c_str());
  printf("  - width: %zu\n", width_);
}

void StdioLogger::SetLevelImpl(LogLevel level)
{
  level_ = level;
}

LogLevel StdioLogger::GetLevelImpl()
{
  return level_;
}
