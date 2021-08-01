
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/strings/str_format.h>
#include <glog/logging.h>
#include <stdio.h>

#include <filesystem>
#include <fstream>

#include "gltf_mesh.h"
#include "resources/transit/transit_write.h"
#include "utility/status.h"

absl::Status ConvertFiles(const std::vector<char*>& filenames) {
  for (const char* file_cstr : filenames) {
    const std::string basename =
        std::filesystem::path(file_cstr).stem().generic_string();
    ASSIGN_OR_RETURN((const GltfModel& gltf), GltfModel::Load(file_cstr));
    for (const auto [name, primitive_array] : gltf.primitives) {
      for (int i = 0; i < primitive_array.size(); i++) {
        const GltfModel::Primitive primitive = primitive_array[i];
        const std::string out_filename =
            absl::StrFormat("%s_%s_%d.tmesh", basename, name, i);
        std::ofstream mesh_file(out_filename,
                                std::ios_base::out | std::ios_base::binary);
        if (!mesh_file.is_open()) {
          return absl::FailedPreconditionError(
              STATUS_MESSAGE("Failed to open output file " << out_filename));
        }
        RETURN_IF_ERROR(transit::Save(mesh_file, primitive.mesh));
        LOG(INFO) << "Wrote " << name << ", prim #" << i << " to file "
                  << out_filename;
        printf("Converted %s (primitive #%d) to %s\n", name.c_str(), i,
               out_filename.c_str());
      }
    }
  }
  return absl::OkStatus();
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  std::vector<char*> remaining_args = absl::ParseCommandLine(argc, argv);
  remaining_args.erase(remaining_args.begin());
  const absl::Status convert_status = ConvertFiles(remaining_args);
  if (!convert_status.ok()) {
    LOG(FATAL) << "Failed to convert files: " << convert_status;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
