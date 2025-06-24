# frozen_string_literal: true

require "frame_helper_ruby"

using_task_library "image_preprocessing"
import_types_from "image_preprocessing"
import_types_from "std"
import_types_from "base"

describe OroGen.image_preprocessing.MonoTask do
    run_live

    attr_reader :task

    before do
        @task = syskit_deploy(OroGen.image_preprocessing.MonoTask.deployed_as("task"))
        if defined?(Runkit)
            Runkit.load_typekit "base"
        else
            Orocos.load_typekit "base"
        end

        camera_calibration = Types.frame_helper.CameraCalibration.new(
            fx: 1, fy: 1, cx: 0.5, cy: 0.5, d0: 0, d1: 0, d2: 0, d3: 0, width: 1280,
            height: 720, ex: 0, ey: 0, fisheye: false
        )
        @task.properties.calibration = camera_calibration
        @task.properties.frame_mode = "MODE_RGB"
        @task.properties.resize_algorithm = "INTER_LINEAR"
    end

    it "Changes the framemode of the input image to RGB" do
        @task.properties.convert_to_grayscale = false
        syskit_configure_and_start(@task)

        frame = Types.base.samples.frame.Frame.new
        FrameHelper.load_frame File.join(__dir__, "test_image.jpg"), frame
        output = expect_execution do
            syskit_write @task.frame_port, frame
        end.to do
            have_one_new_sample(task.oframe_port)
        end

        assert_equal :MODE_BGR, frame.frame_mode
        assert_equal :MODE_RGB, output.frame_mode
    end

    it "Converts the image to grayscale but keeps the desired output mode" do
        @task.properties.convert_to_grayscale = true
        syskit_configure_and_start(@task)

        frame = Types.base.samples.frame.Frame.new
        FrameHelper.load_frame File.join(__dir__, "test_image.jpg"), frame
        output = expect_execution do
            syskit_write @task.frame_port, frame
        end.to do
            have_one_new_sample(task.oframe_port)
        end

        assert_equal :MODE_RGB, output.frame_mode
        # Grayscale images have the same values of RGB for each pixel
        output_pixels = output.image.to_a.each_slice(3).to_a
        output_pixels.each do |pixel|
            assert_equal pixel.uniq.size, 1
        end
    end
end
