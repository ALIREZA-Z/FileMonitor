#pragma once
#include <string>

// Returns a temp folder + socket file name
std::string getTempSocketPath(const std::string& name = "filemonitor.sock");
