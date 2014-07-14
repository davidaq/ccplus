Timeline
========

A timeline describes how a video composition is. When saved as a file,
timeline will act like a project file (like *.gyp for gyp, *.sln for
visual studio and *.aep for Adobe After Effects), it contains all the
data describing how the footage files (resource images or videos) and
effects are "compiled" together into a final single video.

The timeline is serialized as a JSON document when persisted or sent
as streams. A specific file extension name is not required for saving
timeline as file, but *.tml is recommended.

Basic file structure
--------------------

A typical timeline is expressed as below when serialzed in JSON format:

    {
        "version" : 0.01,
        "main" : "renderthis",
        "compositions" : {
            "renderthis" : {
                "resolution" : {
                    "width" : 1280,
                    "height" : 720
                },
                "duration" : 12,
                "layers" : [
                    {
                        "uri" : "composition://subcomp",
                        "properties" : {
                            "position" : {
                                "0" : [100, 20]
                            },
                            "scale" : {
                                "0" : [0.1, 0.1],
                                "10" : [1, 1]
                            },
                            ......
                        },
                        "time" : 0,
                        "duration" : 10,
                        "start" : 5,
                        "last" : 12
                    }, {
                        "uri" : "text://hello world",
                        "style" : {
                            "font" : "Arial",
                            "size" : 100,
                            "lineheight" : 100,
                            "spacing" : 200,
                            "strokesize" : 25,
                            "fill" : "#000000",
                            "stroke" : "#FFFFFF"
                        }
                        ......
                    }, {
                        "uri" : "file://path to a footage image",
                        ......
                    }, {
                        "uri" : "http://web url to a footage",
                        ......
                    }, {
                        "uri" : "color://#FFFFFF",
                        ......
                    }
                ]
            },
            "subcomp" : { 
                ......
            }
        }
    }

A timeline should indicate a version indicating the minimal version of ccplus
engine to interpret this timeline. The "main" property tells which composition
should be rendered to the final video.

The "compositions" property is the main part of the timeline structure. As the
literal meaning, it is a set of compositions. The keys in this property are
the names of the compositions, used to uniquely identify compositions when
referenced in other parts.

A composition describes how footages, text or sub-compositions are composed 
along the time axis. The information should be enough for the render engine to
render the image of a frame at any given time. All compositions have fixed
resolution and time duration, anything not within this range is null, and
should be rendered transparent if forced to render.

Each composition is mainly made up with multiple layers described in the array
value of the "layers" property. The layers with lower indexes should render on
top of the layers with higher indexes. Each layer is a graphical or audio item
that could be seen or heard in the final rendered video.

The corresponding element of an layer is indicated by the "uri" property. Any
uri should start with a protocal, which describes what type of media this
element is. The "time" property is the time point within the **component**
where this layer should appear. The "duration" property is the time how long 
this layer will last visible in the composition. The "start" property is the 
time point within the **element of this layer** that will be shown as first
frame when this layer appears. The "last" property is the time duration which
the layer element would last in context of the layer element. If the "duration"
property and "last" property are not identical, the time axis of the element 
layer would stretch or shrink, causing fast or slow motion. If the "last"
property is larger than the actual duration of the element, looping will occur.
All time metrics are measured in seconds.

The "properties" property in a layer describes how this layer should be at some
key point time. The elements in the properties may define a transform or some
sort of filtering. The values of the elements may be literal (scalar number, 
vector or matrix), or timelined. Timelined values should be interpolated during
the timeline interpretation.


In-memory data structure
------------------------

*TODO: Waiting to be specified*
