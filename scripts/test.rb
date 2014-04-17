require 'orocos'
require 'orocos/log'
require 'vizkit'



log = Orocos::Log::Replay.open(ARGV)
camera_port = log.find_port("/base/samples/frame/Frame")
Orocos.initialize

Orocos.run 'image_preprocessing_test' do
    mono = Orocos::TaskContext.get("mono_image_preprocessing")
    camera_port.connect_to mono.frame
    mono.start


    Vizkit.display camera_port
    Vizkit.display mono.oframe
    Vizkit.control log
    Vizkit.exec
end
