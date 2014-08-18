require 'orocos'
require 'orocos/log'
require 'vizkit'

log = Orocos::Log::Replay.open(ARGV)
camera_port = log.bottom_camera.frame 
Orocos.initialize

widget = Vizkit.load "slider.ui"
widget.show

Orocos.run 'image_preprocessing::HSVSegmentationAndBlur' => "task",  "hsv_mosaicing::Task" => "t2" do
    task = Orocos::TaskContext.get("task")
    t2 = Orocos::TaskContext.get("t2")
    camera_port.connect_to task.frame
    task.binary_result.connect_to t2.frame
    Orocos.conf.load_dir('.')
    Orocos.conf.apply(task, ['default'])
    Orocos.conf.apply(t2, ['default'])
 
    task.configure
    task.start
  
    t2.configure
    t2.start

    widget.connect_to_task "task" do |task|
        c = lambda{|v| log.refresh}
        hMax.connect SIGNAL("valueChanged(int)"), PROPERTY(:hMax), :callback => c
        hMin.connect SIGNAL("valueChanged(int)"), PROPERTY(:hMin), :callback => c
        sMax.connect SIGNAL("valueChanged(int)"), PROPERTY(:sMax), :callback => c
        sMin.connect SIGNAL("valueChanged(int)"), PROPERTY(:sMin), :callback => c
        vMax.connect SIGNAL("valueChanged(int)"), PROPERTY(:vMax), :callback => c
        vMin.connect SIGNAL("valueChanged(int)"), PROPERTY(:vMin), :callback => c
        blur.connect SIGNAL("valueChanged(int)"), PROPERTY(:blur), :callback => c
        task.on_reachable do
            Vizkit.display task
        end
    end



    a = Vizkit.display camera_port
    t2.result.connect_to do |sample|
        row = sample.rows
        col = sample.cols
        sample.data.each_with_index do |v,i|
            x = (a.getWidth()/col * (i.modulo(col))) 
            y = (a.getHeight()/row * (i/col).floor)
            x2 = a.getWidth()/col / 2
            y2 = a.getHeight()/row / 2

            x = x+x2
            y = y+y2

            if v != 0
                a.addCircle(Qt::PointF.new(x, y), 10, Qt::Color.new(Qt::red) , 1, false)
            else
                a.addCircle(Qt::PointF.new(x, y), 10, Qt::Color.new(Qt::green), 1, false)
            end
        end
    end

    Vizkit.display t2
#    Vizkit.display task
#    Vizkit.display task.hDebug
#    Vizkit.display task.sDebug 
#    Vizkit.display task.vDebug 
    Vizkit.control log
    Vizkit.exec
end
