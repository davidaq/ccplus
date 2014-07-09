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

Basic structure
---------------

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
                            "position" : [100, 20],
                            "scale" : [
                                    {
                                        "time" : 0,
                                        "value" : [0.1, 0.1]
                                    }, {
                                        "time" : 10,
                                        "value" : [1, 1]
                                    }
                                ],
                            ......
                        },
                        "time" : 0,
                        "duration" : 10,
                        "starfrom" : 5,
                        "endat" : 12
                    }, {
                        "uri" : "text://hello world",
                        ......
                    }, {
                        "uri" : "file://path to a footage image",
                        ......
                    }, {
                        "uri" : "http://web url to a footage",
                    }
                ]
            },
            "subcomp" : { 
                ......
            }
        }
    }


