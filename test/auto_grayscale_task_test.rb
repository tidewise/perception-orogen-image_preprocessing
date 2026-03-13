# frozen_string_literal: true

require "frame_helper_ruby"

using_task_library "image_preprocessing"
import_types_from "image_preprocessing"
import_types_from "std"
import_types_from "base"

describe OroGen.image_preprocessing.AutoGrayscaleTask do
    run_live

    attr_reader :task, :night_rgb, :day_rgb

    before do
        @task = syskit_deploy(
          OroGen.image_preprocessing.AutoGrayscaleTask.deployed_as("auto_grayscale_task")
        )

        if defined?(Runkit)
            Runkit.load_typekit "base"
        else
            Orocos.load_typekit "base"
        end

        @night_rgb = Types.base.samples.frame.Frame.new
        @day_rgb = Types.base.samples.frame.Frame.new
        FrameHelper.load_frame File.join(__dir__, "data", "night-rgb.jpg"), @night_rgb
        FrameHelper.load_frame File.join(__dir__, "data", "day-rgb.jpg"), @day_rgb
    end

    it "converts image to grayscale in low light with :SUM method" do
        task.properties.grayscale_method = :SUM
        syskit_configure_and_start(task)
        out =
            expect_execution do
                syskit_write task.frame_port, night_rgb
            end.to do
              emit task.grayscale_on_event
              have_one_new_sample task.oframe_port
            end

        night_gray_sum = Types.base.samples.frame.Frame.new
        FrameHelper.load_frame File.join(__dir__, "data", "night-gray-sum.png"),
                               night_gray_sum
        assert_equal night_gray_sum, out
    end

    it "converts image to grayscale in low light" do
        task.properties.grayscale_method = :OPENCV
        syskit_configure_and_start(task)
        night_rgb.time = t = Time.now
        out =
            expect_execution do
                syskit_write task.frame_port, night_rgb
            end.to do
              emit task.grayscale_on_event
              have_one_new_sample task.oframe_port
            end

        night_gray_opencv = Types.base.samples.frame.Frame.new
        FrameHelper.load_frame File.join(__dir__, "data", "night-gray-opencv.png"),
                               night_gray_opencv
        night_gray_opencv.time = t
        assert_equal night_gray_opencv, out
        assert_in_delta t, out.time, 1e-6
        assert_equal night_rgb.frame_mode, out.frame_mode
    end

    it "does not convert to grayscale in day light" do
        syskit_configure_and_start(task)
        day_rgb.time = t = Time.now
        out =
            expect_execution do
                syskit_write task.frame_port, day_rgb
            end.to do
              emit task.grayscale_off_event
              have_one_new_sample task.oframe_port
            end

        assert_in_delta t, out.time, 1e-6
        assert_equal day_rgb.frame_mode, out.frame_mode
    end

    it "does not evaluate grayscale frames" do
        gray = Types.base.samples.frame.Frame.new
        FrameHelper.load_frame File.join(__dir__, "data", "night-rgb.jpg"), gray
        gray.frame_mode = :MODE_GRAYSCALE

        syskit_configure_and_start(task)
        gray.time = t = Time.now
        out =
            expect_execution do
                syskit_write task.frame_port, gray
            end.to do
              emit task.no_op_event
              have_one_new_sample task.oframe_port
            end

        assert_in_delta t, out.time, 1e-6
        assert_equal :MODE_GRAYSCALE, out.frame_mode
    end

    it "switches to grayscale and back" do
        syskit_configure_and_start(task)
        expect_execution { syskit_write task.frame_port, night_rgb }
            .to_emit(task.grayscale_on_event)
        expect_execution { syskit_write task.frame_port, day_rgb }
            .to_emit(task.grayscale_off_event)
    end

    it "can ouput :MODE_GRAYSCALE" do
        task.properties.replicate_input_mode = false
        syskit_configure_and_start(task)
        night_rgb.time = t = Time.now
        out =
            expect_execution do
                syskit_write task.frame_port, night_rgb
            end.to do
              emit task.grayscale_on_event
              have_one_new_sample task.oframe_port
            end

        assert_in_delta t, out.time, 1e-6
        assert_equal :MODE_GRAYSCALE, out.frame_mode
    end

    it "handles different input sizes" do
        task.properties.replicate_input_mode = true
        syskit_configure_and_start(task)
        small = resize(night_rgb, 1.0 / 2)
        small_out =
            expect_execution do
                syskit_write task.frame_port, small
            end.to do
                emit task.grayscale_on_event
                have_one_new_sample task.oframe_port
            end

        assert_equal 205, small_out.size.width
        assert_equal 115, small_out.size.height

        big_out =
            expect_execution do
                syskit_write task.frame_port, night_rgb
            end.to do
                not_emit task.grayscale_off_event
                have_one_new_sample task.oframe_port
            end

        assert_equal 410, big_out.size.width
        assert_equal 230, big_out.size.height
    end

    def resize(frame, factor)
        resized = Types.base.samples.frame.Frame.new
        resized.frame_mode = frame.frame_mode
        resized.data_depth = frame.data_depth
        resized.size.width = factor * frame.size.width
        resized.size.height = factor * frame.size.height
        resized.pixel_size = frame.pixel_size
        resized.image = Array.new(
            resized.size.width * resized.size.height * resized.pixel_size, 0
        )
        FrameHelper.convert frame, resized
        resized
    end
end