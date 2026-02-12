#pragma once
#include "logger-base.hpp"
#include <cstdio>

class LoggerStdout : public LoggerBase<LoggerStdout>
{
public:
  LoggerStdout() = default;
  ~LoggerStdout() = default;

  void InitializeImpl() {}

  void SetLevelImpl(LogLevel level)
  {
    level_ = level;
  }

  LogLevel GetLevelImpl()
  {
    return level_;
  }

private:
  LogLevel level_{LogLevel::INFO};
};
