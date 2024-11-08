// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ranges>
#include <type_traits>

#include "base/base64.h"
#include "base/command_line.h"
#include "base/debug/crash_logging.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "components/variations/processed_study.h"
#include "components/variations/proto/layer.pb.h"
#include "components/variations/proto/study.pb.h"
#include "components/variations/proto/variations_seed.pb.h"
#include "components/variations/variations_seed_store.h"
#include "third_party/zlib/google/compression_utils.h"

namespace {
#define GET_VALUE2(obj, name) result.Set(#name, ToValue(obj.name()))

template <typename T>
base::Value ToValue(const T& array) {
  return base::Value(array);
}

template <typename T>
  requires(std::is_constructible_v<base::Value, T>)
base::Value ToValue(const T& array) {
  return base::Value(array);
}

template <typename T>
  requires(!std::is_constructible_v<base::Value, T> &&
           (std::is_integral_v<T> || std::is_enum_v<T>))
base::Value ToValue(const T& i) {
  return base::Value((int)i);
}

template <typename _Tp>
  requires(!std::is_constructible_v<base::Value, _Tp> &&
           std::ranges::common_range<_Tp>)
base::Value ToValue(const _Tp& array) {
  base::Value::List list;
  std::ranges::for_each(array, [&list](auto i) {
    list.Append(ToValue<std::ranges::range_value_t<_Tp>>(i));
  });
  return base::Value(std::move(list));
}

template <>
base::Value ToValue(const variations::Study_Experiment_FeatureAssociation&
                        feature_association) {
  base::Value::Dict result;
  GET_VALUE2(feature_association, enable_feature);
  GET_VALUE2(feature_association, disable_feature);
  GET_VALUE2(feature_association, forcing_feature_on);
  GET_VALUE2(feature_association, forcing_feature_off);
  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::Study_Experiment_Param& param) {
  base::Value::Dict result;
  GET_VALUE2(param, name);
  GET_VALUE2(param, value);
  return base::Value(std::move(result));
}

template <>
base::Value ToValue(
    const ::variations::Study_Experiment_OverrideUIString& override_ui_string) {
  base::Value::Dict result;
  GET_VALUE2(override_ui_string, name_hash);
  GET_VALUE2(override_ui_string, value);
  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::Study_Experiment& experment) {
  base::Value::Dict result;
#define V(FUNC)                          \
  FUNC(name)                             \
  FUNC(probability_weight)               \
  FUNC(google_web_experiment_id)         \
  FUNC(google_web_trigger_experiment_id) \
  FUNC(google_web_visibility)            \
  FUNC(google_app_experiment_id)         \
  FUNC(feature_association)              \
  FUNC(forcing_flag)                     \
  FUNC(param)                            \
  FUNC(type)                             \
  FUNC(override_ui_string)

#define GET_VALUE(name) GET_VALUE2(experment, name);
  V(GET_VALUE)
#undef GET_VALUE
#undef V

  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::LayerMemberReference& reference) {
  base::Value::Dict result;
#define V(FUNC)         \
  FUNC(layer_id)        \
  FUNC(layer_member_id) \
  FUNC(layer_member_ids)

#define GET_VALUE(name) GET_VALUE2(reference, name);
  V(GET_VALUE)
#undef GET_VALUE
#undef V

  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::Study_Filter& filter) {
  base::Value::Dict result;
#define V(FUNC)                  \
  FUNC(start_date)               \
  FUNC(end_date)                 \
  FUNC(min_version)              \
  FUNC(max_version)              \
  FUNC(min_os_version)           \
  FUNC(max_os_version)           \
  FUNC(channel)                  \
  FUNC(platform)                 \
  FUNC(locale)                   \
  FUNC(exclude_locale)           \
  FUNC(form_factor)              \
  FUNC(exclude_form_factor)      \
  FUNC(hardware_class)           \
  FUNC(exclude_hardware_class)   \
  FUNC(country)                  \
  FUNC(exclude_country)          \
  FUNC(is_low_end_device)        \
  FUNC(is_enterprise)            \
  FUNC(policy_restriction)       \
  FUNC(cpu_architecture)         \
  FUNC(exclude_cpu_architecture) \
  FUNC(google_group)             \
  FUNC(exclude_google_group)

#define GET_VALUE(name) GET_VALUE2(filter, name);
  V(GET_VALUE)
#undef GET_VALUE
#undef V

  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::Study& study) {
  base::Value::Dict result;
#define V(FUNC)                 \
  FUNC(name)                    \
  FUNC(expiry_date)             \
  FUNC(consistency)             \
  FUNC(layer)                   \
  FUNC(default_experiment_name) \
  FUNC(experiment)              \
  FUNC(filter)                  \
  FUNC(randomization_seed)      \
  FUNC(activation_type)

#define GET_VALUE(name) GET_VALUE2(study, name);
  V(GET_VALUE)
#undef GET_VALUE
#undef V

  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::Layer_LayerMember_SlotRange& range) {
  base::Value::Dict result;
#define V(FUNC) \
  FUNC(start)   \
  FUNC(end)

#define GET_VALUE(name) GET_VALUE2(range, name);
  V(GET_VALUE)
#undef GET_VALUE
#undef V

  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::Layer_LayerMember& member) {
  base::Value::Dict result;
#define V(FUNC) \
  FUNC(id)      \
  FUNC(slots)

#define GET_VALUE(name) GET_VALUE2(member, name);
  V(GET_VALUE)
#undef GET_VALUE
#undef V

  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::Layer& layer) {
  base::Value::Dict result;
#define V(FUNC)   \
  FUNC(id)        \
  FUNC(num_slots) \
  FUNC(members)   \
  FUNC(salt)      \
  FUNC(entropy_mode)

#define GET_VALUE(name) GET_VALUE2(layer, name);
  V(GET_VALUE)
#undef GET_VALUE
#undef V

  return base::Value(std::move(result));
}

template <>
base::Value ToValue(const variations::VariationsSeed& seed) {
  base::Value::Dict result;
#define V(FUNC)       \
  FUNC(serial_number) \
  FUNC(study)         \
  FUNC(country_code)  \
  FUNC(version)       \
  FUNC(layers)

#define GET_VALUE(name) GET_VALUE2(seed, name);
  V(GET_VALUE)
#undef GET_VALUE
#undef V

  return base::Value(std::move(result));
}

}  // namespace

int main(int argc, const char** argv) {
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_STDERR;
  logging::InitLogging(settings);

  if (argc < 2) {
    LOG(ERROR) << "variations seed.json";
    return 0;
  }

  base::CommandLine command_line(argc, argv);

  std::span args = std::span(argv, argc).subspan(1);
  base::FilePath file_path = base::FilePath::FromUTF8Unsafe(args.front());

  std::string contents;
  if (!base::ReadFileToString(file_path, &contents)) {
    return 1;
  }

  std::optional<base::Value> seed_dict = base::JSONReader::Read(contents);
  if (!seed_dict) {
    return 2;
  }

  std::string* variations_compressed_seed_base64 =
      seed_dict->GetDict().FindString("variations_compressed_seed");
  if (!variations_compressed_seed_base64) {
    return 3;
  }

  // If the decode process fails, assume the pref value is corrupt and clear it.
  std::string decoded_data;
  if (!base::Base64Decode(*variations_compressed_seed_base64, &decoded_data)) {
    return 4;
  }
  std::string seed_data;
  if (!compression::GzipUncompress(decoded_data, &seed_data)) {
    return 5;
  }

  variations::VariationsSeed seed;
  if (!seed.ParseFromString(seed_data)) {
    return 6;
  }

  if (args.size() > 1 &&
      base::WriteFile(base::FilePath::FromUTF8Unsafe(args[1]),
                      ToValue(seed).DebugString())) {
    return 0;
  }

  LOG(ERROR) << ToValue(seed);
  return 0;
}
