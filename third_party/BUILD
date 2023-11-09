# Copyright 2019 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

load("@rules_foreign_cc//foreign_cc:cmake.bzl", "cmake")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//visibility:public"])

exports_files(["LICENSE"])

cc_library(
    name = "glog",
    visibility = ["//visibility:public"],
    deps = select({
        "//mediapipe:android_x86": [
            "@com_github_glog_glog_no_gflags//:glog",
        ],
        "//mediapipe:android_x86_64": [
            "@com_github_glog_glog_no_gflags//:glog",
        ],
        "//mediapipe:android_arm": [
            "@com_github_glog_glog_no_gflags//:glog",
        ],
        "//mediapipe:android_arm64": [
            "@com_github_glog_glog_no_gflags//:glog",
        ],
        "//mediapipe:ios": [
            "@com_github_glog_glog_no_gflags//:glog",
        ],
        "//mediapipe:macos": [
            "@com_github_glog_glog//:glog",
        ],
        "//mediapipe:windows": [
            "@com_github_glog_glog//:glog",
        ],
        "//conditions:default": [
            "@com_github_glog_glog//:glog",
        ],
    }),
)

config_setting(
    name = "opencv_source_build",
    define_values = {
        "OPENCV": "source",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "opencv_ios_arm64_source_build",
    define_values = {
        "OPENCV": "source",
    },
    values = {
        "apple_platform_type": "ios",
        "cpu": "ios_arm64",
    },
)

config_setting(
    name = "opencv_ios_sim_arm64_source_build",
    define_values = {
        "OPENCV": "source",
    },
    values = {
        "apple_platform_type": "ios",
        "cpu": "ios_sim_arm64",
    },
)

config_setting(
    name = "opencv_ios_x86_64_source_build",
    define_values = {
        "OPENCV": "source",
    },
    values = {
        "apple_platform_type": "ios",
        "cpu": "ios_x86_64",
    },
)

config_setting(
    name = "opencv_ios_sim_fat_source_build",
    define_values = {
        "OPENCV": "source",
    },
    values = {
        "apple_platform_type": "ios",
        "ios_multi_cpus": "sim_arm64,x86_64",
    },
)

alias(
    name = "opencv",
    actual = select({
        ":opencv_source_build": "@com_github_opencv_opencv//:opencv",
        ":opencv_ios_sim_arm64_source_build": "@ios_opencv_source//:opencv",
        ":opencv_ios_arm64_source_build": "@ios_opencv_source//:opencv",
        ":opencv_ios_x86_64_source_build": "@ios_opencv_source//:opencv",
        "//conditions:default": ":opencv_binary",
    }),
    visibility = ["//visibility:public"],
)

bzl_library(
    name = "opencv_ios_xcframework_files_bzl",
    srcs = ["opencv_ios_xcframework_files.bzl"],
    visibility = ["//visibility:private"],
)

bzl_library(
    name = "opencv_ios_source_bzl",
    srcs = ["opencv_ios_source.bzl"],
    visibility = ["//visibility:private"],
)

alias(
    name = "opencv_binary",
    actual = select({
        "//mediapipe:android_x86": "@android_opencv//:libopencv_x86",
        "//mediapipe:android_x86_64": "@android_opencv//:libopencv_x86_64",
        "//mediapipe:android_arm": "@android_opencv//:libopencv_armeabi-v7a",
        "//mediapipe:android_arm64": "@android_opencv//:libopencv_arm64-v8a",
        "//mediapipe:ios": "@ios_opencv//:opencv",
        "//mediapipe:macos": "@macos_opencv//:opencv",
        "//mediapipe:windows": "@windows_opencv//:opencv",
        "//conditions:default": "@linux_opencv//:opencv",
    }),
)

cc_library(
    name = "libffmpeg",
    visibility = ["//visibility:public"],
    deps = ["@com_github_ffmpeg_ffmpeg//:ffmpeg"],
)

android_library(
    name = "androidx_annotation",
    exports = [
        "@maven//:androidx_annotation_annotation",
    ],
)

android_library(
    name = "androidx_appcompat",
    exports = [
        "@maven//:androidx_appcompat_appcompat",
    ],
)

android_library(
    name = "androidx_constraint_layout",
    exports = [
        "@maven//:androidx_constraintlayout_constraintlayout",
    ],
)

android_library(
    name = "androidx_core",
    exports = [
        "@maven//:androidx_core_core",
    ],
)

android_library(
    name = "androidx_legacy_support_v4",
    exports = [
        "@maven//:androidx_legacy_legacy_support_v4",
    ],
)

android_library(
    name = "androidx_material",
    exports = [
        "@maven//:com_google_android_material_material",
    ],
)

android_library(
    name = "androidx_recyclerview",
    exports = [
        "@maven//:androidx_recyclerview_recyclerview",
    ],
)

android_library(
    name = "camerax_camera2",
    exports = [
        "@maven//:androidx_camera_camera_camera2",
    ],
)

android_library(
    name = "camerax_core",
    exports = [
        "@maven//:androidx_camera_camera_core",
    ],
)

android_library(
    name = "camerax_lifecycle",
    exports = [
        "@maven//:androidx_camera_camera_lifecycle",
    ],
)

java_plugin(
    name = "autovalue_plugin",
    processor_class = "com.google.auto.value.processor.AutoValueProcessor",
    deps = [
        "@maven//:com_google_auto_value_auto_value",
        "@maven//:com_google_auto_value_auto_value_annotations",
    ],
)

java_plugin(
    name = "autobuilder_plugin",
    processor_class = "com.google.auto.value.processor.AutoBuilderProcessor",
    deps = [
        "@maven//:com_google_auto_value_auto_value",
        "@maven//:com_google_auto_value_auto_value_annotations",
    ],
)

java_library(
    name = "autovalue",
    exported_plugins = [
        ":autovalue_plugin",
        ":autobuilder_plugin",
    ],
    neverlink = 1,
    exports = [
        "@maven//:com_google_auto_value_auto_value",
        "@maven//:com_google_auto_value_auto_value_annotations",
    ],
)

java_import(
    name = "any_java_proto",
    jars = [
        "@com_google_protobuf//java/core:libcore.jar",
    ],
)
