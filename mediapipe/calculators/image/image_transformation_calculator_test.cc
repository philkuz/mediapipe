#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/flags/flag.h"
#include "absl/strings/substitute.h"
#include "mediapipe/framework/calculator.pb.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/calculator_runner.h"
#include "mediapipe/framework/deps/file_path.h"
#include "mediapipe/framework/formats/image_format.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/gtest.h"
#include "mediapipe/framework/port/opencv_imgcodecs_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/googletest.h"
#include "third_party/OpenCV/core.hpp"  // IWYU pragma: keep
#include "third_party/OpenCV/core/mat.hpp"

namespace mediapipe {

namespace {

absl::flat_hash_set<int> computeUniqueValues(const cv::Mat& mat) {
  // Compute the unique values in cv::Mat
  absl::flat_hash_set<int> unique_values;
  for (int i = 0; i < mat.rows; i++) {
    for (int j = 0; j < mat.cols; j++) {
      unique_values.insert(mat.at<unsigned char>(i, j));
    }
  }
  return unique_values;
}

TEST(ImageTransformationCalculatorTest, NearestNeighborResizing) {
  cv::Mat input_mat;
  cv::cvtColor(cv::imread(file::JoinPath("./",
                                         "/mediapipe/calculators/"
                                         "image/testdata/binary_mask.png")),
               input_mat, cv::COLOR_BGR2GRAY);
  Packet input_image_packet = MakePacket<ImageFrame>(
      ImageFormat::FORMAT_GRAY8, input_mat.size().width, input_mat.size().height);
  input_mat.copyTo(formats::MatView(&(input_image_packet.Get<ImageFrame>())));

  std::vector<std::pair<int, int>> output_dims{
      {256, 333}, {512, 512}, {1024, 1024}};

  for (auto& output_dim : output_dims) {
    Packet input_output_dim_packet =
        MakePacket<std::pair<int, int>>(output_dim);
    std::vector<std::string> scale_modes{"FIT", "STRETCH"};
    for (const auto& scale_mode : scale_modes) {
      CalculatorGraphConfig::Node node_config =
          ParseTextProtoOrDie<CalculatorGraphConfig::Node>(
              absl::Substitute(R"(
          calculator: "ImageTransformationCalculator"
          input_stream: "IMAGE:input_image"
          input_stream: "OUTPUT_DIMENSIONS:image_size"
          output_stream: "IMAGE:output_image"
          options: {
            [mediapipe.ImageTransformationCalculatorOptions.ext]: {
              scale_mode: $0
              interpolation_mode: NEAREST
            }
          })",
                               scale_mode));

      CalculatorRunner runner(node_config);
      runner.MutableInputs()->Tag("IMAGE").packets.push_back(
          input_image_packet.At(Timestamp(0)));
      runner.MutableInputs()
          ->Tag("OUTPUT_DIMENSIONS")
          .packets.push_back(input_output_dim_packet.At(Timestamp(0)));

      MP_ASSERT_OK(runner.Run());
      const auto& outputs = runner.Outputs();
      ASSERT_EQ(outputs.NumEntries(), 1);
      const std::vector<Packet>& packets = outputs.Tag("IMAGE").packets;
      ASSERT_EQ(packets.size(), 1);
      const auto& result = packets[0].Get<ImageFrame>();
      ASSERT_EQ(output_dim.first, result.Width());
      ASSERT_EQ(output_dim.second, result.Height());

      auto unique_input_values = computeUniqueValues(input_mat);
      auto unique_output_values =
          computeUniqueValues(formats::MatView(&result));
      EXPECT_THAT(unique_input_values,
                  ::testing::ContainerEq(unique_output_values));
    }
  }
}

TEST(ImageTransformationCalculatorTest,
     NearestNeighborResizingWorksForFloatInput) {
  cv::Mat input_mat;
  cv::cvtColor(cv::imread(file::JoinPath("./",
                                         "/mediapipe/calculators/"
                                         "image/testdata/binary_mask.png")),
               input_mat, cv::COLOR_BGR2GRAY);
  Packet input_image_packet = MakePacket<ImageFrame>(
      ImageFormat::FORMAT_VEC32F1, input_mat.size().width, input_mat.size().height);
  cv::Mat packet_mat_view =
      formats::MatView(&(input_image_packet.Get<ImageFrame>()));
  input_mat.convertTo(packet_mat_view, CV_32FC1, 1 / 255.f);

  std::vector<std::pair<int, int>> output_dims{
      {256, 333}, {512, 512}, {1024, 1024}};

  for (auto& output_dim : output_dims) {
    Packet input_output_dim_packet =
        MakePacket<std::pair<int, int>>(output_dim);
    std::vector<std::string> scale_modes{"FIT", "STRETCH"};
    for (const auto& scale_mode : scale_modes) {
      CalculatorGraphConfig::Node node_config =
          ParseTextProtoOrDie<CalculatorGraphConfig::Node>(
              absl::Substitute(R"(
          calculator: "ImageTransformationCalculator"
          input_stream: "IMAGE:input_image"
          input_stream: "OUTPUT_DIMENSIONS:image_size"
          output_stream: "IMAGE:output_image"
          options: {
            [mediapipe.ImageTransformationCalculatorOptions.ext]: {
              scale_mode: $0
              interpolation_mode: NEAREST
            }
          })",
                               scale_mode));

      CalculatorRunner runner(node_config);
      runner.MutableInputs()->Tag("IMAGE").packets.push_back(
          input_image_packet.At(Timestamp(0)));
      runner.MutableInputs()
          ->Tag("OUTPUT_DIMENSIONS")
          .packets.push_back(input_output_dim_packet.At(Timestamp(0)));

      MP_ASSERT_OK(runner.Run());
      const auto& outputs = runner.Outputs();
      ASSERT_EQ(outputs.NumEntries(), 1);
      const std::vector<Packet>& packets = outputs.Tag("IMAGE").packets;
      ASSERT_EQ(packets.size(), 1);
      const auto& result = packets[0].Get<ImageFrame>();
      ASSERT_EQ(output_dim.first, result.Width());
      ASSERT_EQ(output_dim.second, result.Height());

      auto unique_input_values = computeUniqueValues(packet_mat_view);
      auto unique_output_values =
          computeUniqueValues(formats::MatView(&result));
      EXPECT_THAT(unique_input_values,
                  ::testing::ContainerEq(unique_output_values));
    }
  }
}

TEST(ImageTransformationCalculatorTest, NearestNeighborResizingGpu) {
  cv::Mat input_mat;
  cv::cvtColor(cv::imread(file::JoinPath("./",
                                         "/mediapipe/calculators/"
                                         "image/testdata/binary_mask.png")),
               input_mat, cv::COLOR_BGR2RGBA);

  std::vector<std::pair<int, int>> output_dims{
      {256, 333}, {512, 512}, {1024, 1024}};

  for (auto& output_dim : output_dims) {
    std::vector<std::string> scale_modes{"FIT"};  //, "STRETCH"};
    for (const auto& scale_mode : scale_modes) {
      CalculatorGraphConfig graph_config =
          ParseTextProtoOrDie<CalculatorGraphConfig>(
              absl::Substitute(R"(
          input_stream: "input_image"
          input_stream: "image_size"
          output_stream: "output_image"

          node {
            calculator: "ImageFrameToGpuBufferCalculator"
            input_stream: "input_image"
            output_stream: "input_image_gpu"
          }

          node {
            calculator: "ImageTransformationCalculator"
            input_stream: "IMAGE_GPU:input_image_gpu"
            input_stream: "OUTPUT_DIMENSIONS:image_size"
            output_stream: "IMAGE_GPU:output_image_gpu"
            options: {
              [mediapipe.ImageTransformationCalculatorOptions.ext]: {
                scale_mode: $0
                interpolation_mode: NEAREST
              }
            }
          }
          node {
            calculator: "GpuBufferToImageFrameCalculator"
            input_stream: "output_image_gpu"
            output_stream: "output_image"
          })",
                               scale_mode));
      ImageFrame input_image(ImageFormat::FORMAT_SRGBA, input_mat.size().width,
                             input_mat.size().height);
      input_mat.copyTo(formats::MatView(&input_image));

      std::vector<Packet> output_image_packets;
      tool::AddVectorSink("output_image", &graph_config, &output_image_packets);

      CalculatorGraph graph(graph_config);
      MP_ASSERT_OK(graph.StartRun({}));

      MP_ASSERT_OK(graph.AddPacketToInputStream(
          "input_image",
          MakePacket<ImageFrame>(std::move(input_image)).At(Timestamp(0))));
      MP_ASSERT_OK(graph.AddPacketToInputStream(
          "image_size",
          MakePacket<std::pair<int, int>>(output_dim).At(Timestamp(0))));

      MP_ASSERT_OK(graph.WaitUntilIdle());
      ASSERT_THAT(output_image_packets, testing::SizeIs(1));

      const auto& output_image = output_image_packets[0].Get<ImageFrame>();
      ASSERT_EQ(output_dim.first, output_image.Width());
      ASSERT_EQ(output_dim.second, output_image.Height());

      auto unique_input_values = computeUniqueValues(input_mat);
      auto unique_output_values =
          computeUniqueValues(formats::MatView(&output_image));
      EXPECT_THAT(unique_input_values,
                  ::testing::ContainerEq(unique_output_values));
    }
  }
}

TEST(ImageTransformationCalculatorTest,
     NearestNeighborResizingWorksForFloatTexture) {
  cv::Mat input_mat;
  cv::cvtColor(cv::imread(file::JoinPath("./",
                                         "/mediapipe/calculators/"
                                         "image/testdata/binary_mask.png")),
               input_mat, cv::COLOR_BGR2GRAY);
  Packet input_image_packet = MakePacket<ImageFrame>(
      ImageFormat::FORMAT_VEC32F1, input_mat.size().width, input_mat.size().height);
  cv::Mat packet_mat_view =
      formats::MatView(&(input_image_packet.Get<ImageFrame>()));
  input_mat.convertTo(packet_mat_view, CV_32FC1, 1 / 255.f);

  std::vector<std::pair<int, int>> output_dims{
      {256, 333}, {512, 512}, {1024, 1024}};

  for (auto& output_dim : output_dims) {
    std::vector<std::string> scale_modes{"FIT"};  //, "STRETCH"};
    for (const auto& scale_mode : scale_modes) {
      CalculatorGraphConfig graph_config =
          ParseTextProtoOrDie<CalculatorGraphConfig>(
              absl::Substitute(R"(
          input_stream: "input_image"
          input_stream: "image_size"
          output_stream: "output_image"

          node {
            calculator: "ImageFrameToGpuBufferCalculator"
            input_stream: "input_image"
            output_stream: "input_image_gpu"
          }

          node {
            calculator: "ImageTransformationCalculator"
            input_stream: "IMAGE_GPU:input_image_gpu"
            input_stream: "OUTPUT_DIMENSIONS:image_size"
            output_stream: "IMAGE_GPU:output_image_gpu"
            options: {
              [mediapipe.ImageTransformationCalculatorOptions.ext]: {
                scale_mode: $0
                interpolation_mode: NEAREST
              }
            }
          }
          node {
            calculator: "GpuBufferToImageFrameCalculator"
            input_stream: "output_image_gpu"
            output_stream: "output_image"
          })",
                               scale_mode));

      std::vector<Packet> output_image_packets;
      tool::AddVectorSink("output_image", &graph_config, &output_image_packets);

      CalculatorGraph graph(graph_config);
      MP_ASSERT_OK(graph.StartRun({}));

      MP_ASSERT_OK(graph.AddPacketToInputStream(
          "input_image", input_image_packet.At(Timestamp(0))));
      MP_ASSERT_OK(graph.AddPacketToInputStream(
          "image_size",
          MakePacket<std::pair<int, int>>(output_dim).At(Timestamp(0))));

      MP_ASSERT_OK(graph.WaitUntilIdle());
      ASSERT_THAT(output_image_packets, testing::SizeIs(1));

      const auto& output_image = output_image_packets[0].Get<ImageFrame>();
      ASSERT_EQ(output_dim.first, output_image.Width());
      ASSERT_EQ(output_dim.second, output_image.Height());

      auto unique_input_values = computeUniqueValues(packet_mat_view);
      auto unique_output_values =
          computeUniqueValues(formats::MatView(&output_image));
      EXPECT_THAT(unique_input_values,
                  ::testing::ContainerEq(unique_output_values));
    }
  }
}

}  // namespace
}  // namespace mediapipe
