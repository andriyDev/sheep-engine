
#include "resources/skeleton.h"

#include "resources/transit/transit_write.h"
#include "utility/hton_extra.h"

namespace transit {

template <>
absl::Status Save(std::ostream& stream,
                  const std::shared_ptr<Skeleton>& skeleton) {
  json::json json_data;
  std::vector<json::json> bones;
  bones.reserve(skeleton->bones.size());
  for (const Skeleton::Bone& bone : skeleton->bones) {
    json::json& bone_json = bones.emplace_back(json::json::object());
    bone_json["name"] = bone.name;
    bone_json["children"] = bone.children;
  }
  json_data["bones"] = bones;

  std::stringstream json_ss;
  json_ss << json_data;
  const std::string& json_string = json_ss.str();

  TransitHeader header = CreateHeader("MESH");
  header.json_length = json_string.length();
  header.data_length = sizeof(Skeleton::Bone::Pose) * skeleton->bones.size();
  RETURN_IF_ERROR(WriteHeader(stream, header));
  stream.write(json_string.c_str(), json_string.length());
  std::vector<Skeleton::Bone::Pose> bind_pose;
  bind_pose.reserve(bones.size());
  for (const Skeleton::Bone& bone : skeleton->bones) {
    bind_pose.push_back(bone.bind_pose);
    bind_pose.back().position = htob(bind_pose.back().position);
    bind_pose.back().rotation = htob(bind_pose.back().rotation);
    bind_pose.back().scale = htob(bind_pose.back().scale);
  }
  stream.write((char*)bones.data(),
               sizeof(Skeleton::Bone::Pose) * skeleton->bones.size());
  if (stream.bad()) {
    return absl::FailedPreconditionError("Failed to write mesh to stream");
  }
  return absl::OkStatus();
}

}  // namespace transit
