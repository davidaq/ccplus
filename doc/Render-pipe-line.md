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
 - **flags** - some additional flags slightly changing the behavior of the engine

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

The workflow of a single render process is as follows:

    +--------------------+     +---------------------------------+
    | Interpret Timeline | --> | Calculate Resource Requirements |
    +--------------------+     +---------------------------------+
                                                 |
                                                 |
                                                 V
    +-----------------------------+      +---------------------------+
    | Calculate Render Dependency |  <-- | Locate & Collect Resource |
    +-----------------------------+      +---------------------------+
                        |
                        |
    ====================|==================================================
                        |
               +--------+--------------------<----------------------+
               |        |                                           |
               |   OR   |         Done with each render object      ^
               |        V                                           |                
               |    +-----------------------+                       |
               |    | Split Video to frames |----------->-------+   |     
               |    +-----------------------+                   |   |
               |                                                |   |
               |                                                +->-+ OR
               V                                                |   |
      +----------------------+      +-----------------------+   |   |
      | Interpolate Timeline | ---> | Render Image Sequence |---+   |
      +----------------------+      +-----------------------+       |
                                                                    V
                                                                    |
    ================================================================|=======
                                                                    |
                                                                    |
       +------------------------------+     +-------------+         |
       | Merge Image & Audio to Video | <-- | Remix Audio |<--------+
       +------------------------------+     +-------------+
       
#### Interpret Timeline

The timeline is present as JSON format when saved in file or passed by stream.
Interpreting the timeline is the process of reading and converting the timeline
into calculation efficient in-memory data structure.

#### Calculate Resource Requirements

This step, the engine will scan over the timeline and figure out a listing of
all the resource needed to render the video clip, taking into account the start
time and duration specified in the input arguments.

#### Locate & Collect Resource

Resources should be located and collected into a place where the engine can
directly access. For most cases, this just involves downloading web resources.
On iOS, this process performs copying images/videos/music from the album into
app context.

If any missing resource is detecetd, the engine quits and throws an error unless
the user indicates a flag telling the engine try to continue even on error.

#### Calculate Render Dependency

In this step, the engine has to calculate how compositions and footages are
depended with each other taking time span into account. The on going renders
should be ordered using this dependency. The final render should only render
visible parts of the timeline of all animated elements.

#### Split Video to frames

Depended video footages will be split into single frames for easy use.

#### Interpolate Timeline

The timeline read from input only defines key points. In order to render out the
graphics, the timeline should be interoplated to every frame, so that we have
enough information for each frame of how it should be rendered.

#### Render Image Sequence

This step is just rendering each frame with the information calculated in the
previous step. Transforms, filters and masks should be apllied in this step.

#### Remix Audio

In this step, the audios of the resources/sub-compositions are mixed into one
audio according to the timeline. The mix depends on how objects with audio track
align on the time axis.


#### Merge Image & Audio to Video

The last step in the pipe line, is to compile the image sequences and audio in
to a h264 encoded and mp4 formated single video file. This would just be done
using FFmpeg.
