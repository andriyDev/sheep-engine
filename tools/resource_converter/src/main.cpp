
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <glog/logging.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  std::vector<char*> remaining_args = absl::ParseCommandLine(argc, argv);
  return 0;
}
