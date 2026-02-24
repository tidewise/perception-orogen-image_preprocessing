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

        @task.properties.on_trigger = 100
        @task.properties.off_trigger = 130

        @night_rgb = Types.base.samples.frame.Frame.new
        @day_rgb = Types.base.samples.frame.Frame.new
        FrameHelper.load_frame File.join(__dir__, "data", "night-rgb.jpg"), @night_rgb
        FrameHelper.load_frame File.join(__dir__, "data", "day-rgb.jpg"), @day_rgb
    end

    it "converts image to grayscale in low light" do
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
        assert_grayscale_data out.image
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
        assert_equal :MODE_BGR, out.frame_mode
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

    # @param image [Array] 3 Channel image where each pixel channels are stored
    # contiguously
    def assert_grayscale_data(image)
        image.each_slice(3) { |p| assert_equal 1, p.uniq.size }
    end
end