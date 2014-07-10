Render pipe line
================

This document describes the main idea of the whole workflow of the CCPlus
Render Engine hen rendering a video.

Before continue reading, please make sure that you have read and understand the 
concepts described in the [timeline data structure](data-structures/Timeline.md).

The black box
-------------

                        +----------------------+
    input arguments --> | CCPlus Render Engine | --> rendered final video
                        +----------------------+
                       
No matter in which platform, and how the CCPlus Render Engine exists, the
engine is just a black box to users. It takes input arguments and spits out
a h264 encoded and mp4 formated video file, maybe printing some progress 
information during the process.

The input arguments are:

 - **timeline** - describing how the video shoud be
 - **composition** - the composition to render, defaults to main set in timeline
 - **start** - time in seconds from where in the timeline should the start
                of the final video
 - **duration** - maximum duration of the output video
 - **output path** - file path where the output video should be saved
 - **quality** - a quality metric telling the engine whether to be faster
                   or to produce better graphics quality

#### How preview is done during a render

Since the engine is a black box, we can not see how the video looks like
before the video is fully rendered. But previewing is still achieveable
by dividing the render to pieces on the time axis. So a 10 min video may
be divided into 120 pieces of 10s video clips, and if they are rendered
in order, you can watch the first 10 secs when the next 10 secs are being
rendered. In the end, when all pieces are rendered, a join can be performed
to make a single video output. The join can be easily done by converting
the pieces into ts formats and converting back after join.

This divde and join pattern can also be used for distributed computing to
speed up rendering for server cluster solutions. This will be yet another
project of our team, and will not be further discussed here.

Pipe line in the box
--------------------

Despite piece dividing as described above, the workflow of a single render 
process is as follows:

    +--------------------+     +---------------------------------+
    | Interpret Timeline | --> | Calculate Resource Requirements |
    +--------------------+     +---------------------------------+
                                                 |
                                                 |
                                                 V
     +---------------------+      +---------------------------+
     | Pre-render Elements |  <-- | Locate & Collect Resource |
     +---------------------+      +---------------------------+
               |
               |
               V
      +----------------------+      +-----------------------+
      | Interpolate Timeline | ---> | Render Image sequence |
      +----------------------+      +-----------------------+
                                                   |
                                                   |
                                                   V
       +------------------------------+     +-------------+
       | Merge Image & Audio to Video | <-- | Remix Audio |
       +------------------------------+     +-------------+
       
