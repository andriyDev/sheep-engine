
#include "resources/skeleton.h"

#include <fstream>

#include "resources/transit/transit.h"
#include "utility/hton_extra.h"
#include "utility/json.h"

namespace transit {

template <>
absl::StatusOr<std::shared_ptr<Skeleton>> Load(const TransitDetails& details) {
  std::ifstream file(details.file, std::ios_base::binary | std::ios_base::in);
  if (!file.is_open()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Failed to open file \"" << details.file << "\""));
  }

  ASSIGN_OR_RETURN((const TransitHeader& header), ReadHeader(file));
  RETURN_IF_ERROR((VerifyHeader(header, "SKEL", 1, 0)));
  ASSIGN_OR_RETURN((const nlohmann::json& json_data),
                   ReadJson(file, header.json_length));
  ASSIGN_OR_RETURN((const std::vector<unsigned char>& data),
                   ReadData(file, header.data_length));

  ASSIGN_OR_RETURN((const json::json* bones),
                   json::GetRequiredArray(json_data, "bones"));
  if (header.data_length != bones->size() * sizeof(Skeleton::Bone::Pose)) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Data length does not match requested bones"));
  }

  std::shared_ptr<Skeleton> skeleton(new Skeleton());
  skeleton->bones.reserve(bones->size());
  for (const json::json& bone : *bones) {
    ASSIGN_OR_RETURN((const std::string& name),
                     json::GetRequiredString(bone, "name"));
    ASSIGN_OR_RETURN((const json::json* children),
                     json::GetRequiredArray(bone, "children"));
    Skeleton::Bone& bone = skeleton->bones.emplace_back();
    bone.name = name;
    bone.children.reserve(children->size());
    for (const json::json& child : *children) {
      if (!child.is_number_unsigned()) {
        return absl::FailedPreconditionError(
            STATUS_MESSAGE("Child index is not an unsigned integer"));
      }
      const unsigned int index = child.get<unsigned int>();
      if (index >= bones->size()) {
        return absl::FailedPreconditionError(
            STATUS_MESSAGE("Child index is out of valid range of bones"));
      }
      bone.children.push_back(index);
    }
  }

  std::vector<Skeleton::Bone::Pose> bind_poses;
  bind_poses.resize(bones->size());
  file.read((char*)bind_poses.data(),
            sizeof(Skeleton::Bone::Pose) * bones->size());
  unsigned int index = 0;
  for (Skeleton::Bone::Pose& pose : bind_poses) {
    skeleton->bones[index++].bind_pose = btoh(pose);
  }
  return skeleton;
}

}  // namespace transit
