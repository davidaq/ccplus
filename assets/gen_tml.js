/*
 * Generate a TML file based on users' images and template tml file.
 * Try to fit users resources to scenes suitably.
 * Doc: http://dev.ccme.me/wiki?p=%E8%AE%BE%E8%AE%A1%E6%96%87%E6%A1%A3%2Fme%E8%A7%86%E9%A2%91%E6%A8%A1%E6%9D%BF%E6%8A%80%E6%9C%AF%E6%96%87%E6%A1%A3
 *
 * Input:
 * @tpljs = tpl tml;
 * @userjs = users resources description json;
 *
 * Output: 
 * @result = ret tml json;
 */

/* 
 * Parse tpl tml and return scenes list
 * {
 *  name: ~,
 *  duration: ~,
 *  num_ele: ~
 * }
 */
function getScenes(tpl) {
    var ret = [];
    var comps = tpl["compositions"];
    var cnt = 0;
    for (var name in comps) {
        if (name[0] == '#' && name != "#+1" && name != "#COVER") {
            var cmp = comps[name];
            var layers = cmp.layers;
            var num_ele = 0;
            for (var ln in layers) {
                if (layers[ln].uri[14] == '@') {
                    num_ele++;
                }
            }
            if (num_ele == 0) continue;
            ret.push({
                name: name,
                duration: cmp.duration,
                num_ele: num_ele,
                used: 0
            });
            cnt++;
        }
    }
    return ret;
}

/*
 * Parse config JSON and return users resources list 
 * {
 *  name: ~, 
 *  comp: ~
 * }
 */
function genResourcesComp(js, width, height) {
    var medias = js.medias;
    var ret = [];
    var idx = 0;
    for (var m = 0; m < medias.length; m++) {
        var md = medias[m];
        var comp = {};
        var name = "@" + idx;
        comp.duration = 5.0;
        if (md.duration)
            comp.duration = md.duration;
        if (md.type == "image") {
            comp.duration = 3.0;
        } else if (md.type == "video") {
            comp.duration = 5.0;
        }
        comp.resolution = {
            width: width,
            height: height
        };
        comp.layers = [];

        if (typeof(md.x) == "string") md.x = md.x * 1.0;
        if (typeof(md.y) == "string") md.y = md.y * 1.0;
        if (typeof(md.w) == "string") md.w = md.w * 1.0;
        if (typeof(md.h) == "string") md.h = md.h * 1.0;
        var l = {
            uri: "file://" + md.filename,
            time: 0,
            duration: comp.duration,
            start: 0,
            last: comp.duration,
            properties: {
                transform : {
                    0 : [
                        width / 2.0, height / 2.0, 0, 
                        md.x + md.w / 2.0, md.y + md.h / 2.0, 0, 
                        width / md.w, height / md.h, 1,
                        0, 0, 0
                    ]
                }
            }
        };
        comp.layers[0] = l;

        ret.push({
            name: name, 
            type: md.type,
            comp: comp
        });
        idx++;
    }
    return ret;
}

/*
 * return [scene_name, [comp_name1, comp_name2, ...]] in order
 * NOTE: scene_name can be duplicated
 */
function fit(comps, scenes) {
    var cnt = comps.length;
    var used = {};
    var ret = [];
    function sortScenes(scenes) {
        var len = scenes.length;
        for (var i = 0; i < len; i++) {
            for (var j = i + 1; j < len; j++) {
                if (scenes[i].used > scenes[j].used) {
                    var tmp = scenes[i];
                    scenes[i] = scenes[j];
                    scenes[j] = tmp;
                }
            }
        }
    }
    /*
    * Simple version
    */
    var len = scenes.length;
    var k = 0;
    var preferredDuration = 5.0;
    var brutal = 0;
    for (var k= 0; cnt > 0 && brutal <= len; k++) {
        sortScenes(scenes);
        //console.log("----sorted", scenes);
        brutal++;
        k = k % len;
        var scene = scenes[k];
        var num_ele = scene.num_ele;
        if (num_ele > cnt) {
            continue;
        }
        var tmp_ret = [scene.name, []];
        for (var y = 0; y < comps.length; y++) {
            var comp = comps[y];
            if (used[comp.name]) continue;
            if (preferredDuration) {
                if (Math.abs(preferredDuration - scene.duration) > 1.0) {
                    continue;
                }
            } else {
                if (Math.abs(comp.comp.duration - scene.duration) > 1.0) {
                    continue;
                }
            }
            tmp_ret[1].push(comp.name); 
            used[comp.name] = true;
            cnt--;
            num_ele--;
            if (num_ele == 0)
                break;
        }
        if (tmp_ret[1].length > 0) {
            if (preferredDuration == 5.0) {
                preferredDuration = 3.0;
            } else if (preferredDuration == 3.0) {
                preferredDuration = 5.0;
            }
            brutal = 0;
            k = 0;
            scene.used++;
            ret.push(tmp_ret);
        }
    }
    //console.log(ret);
    return ret;
}

/*
 * Generate result tml file based on fitted data
 */
function fillTML(tplJS, fitted, userJS, wrapJS) {
    var candidates = [];
    var len = fitted.length;
    for (var i = 0; i < len; i++) {
        var fit = fitted[i];
        // Ugly clone!
        var cname = fit[0];
        var comp = JSON.parse(JSON.stringify(tplJS.compositions[cname]));
        var layers = comp.layers;
        var idx = 0;
        var overlap = 0;
        var cnt_layers = layers.length;
        for (var l = 0; l < cnt_layers; l++) {
            var layer = layers[l];
            if (layer.uri[14] == '@') {
                layer.uri = "composition://" + fit[1][idx];
                idx++;
            }
            if (layer.uri == 'composition://#+1') {
                if (i < len - 1) {
                    layer.uri = 'composition://$' + (i + 1);
                } else {
                    layer.uri = "composition://End";
                }
                overlap = comp.duration - layer.time;
            }
        }
        var tmp = ["$"+i, comp, overlap];
        candidates.push(tmp);
        tplJS.compositions[tmp[0]] = comp;
    }

    var comps_in_wrap = wrapJS.compositions;
    for (var cname in comps_in_wrap) {
        if (cname != "MAIN") {
            tplJS.compositions[cname] = comps_in_wrap[cname];
        }
    }
    // Generate main composition
    var main_name = tplJS.main;
    var ret = {
        resolution: {
            width: width,
            height: height
        },
        layers: []
    };
    tplJS.compositions[main_name] = ret;
    var currentTime = 0;
    var overlap = 0;
    function appendScene(name, comp) {
        var layer = {
            uri: "composition://" + name,
            time: currentTime,
            duration: comp.duration - overlap,
            start: overlap,
            last: comp.duration - overlap,
            properties: {
                transform: {
                    0 : [
                        0, 0, 0, 0, 0, 0,
                        1, 1, 1, 0, 0, 0
                    ]
                }
            }   
        };
        if (layer.duration < 0) 
            return;
        currentTime += layer.duration;
        ret.layers.push(layer);
    }

    // Fill text
    var textComp = tplJS.compositions['TitleText'];
    textComp.layers[0]["text-properties"]["text"]["0"] = userJS.videoTitle;
    // Append start
    var startComp = tplJS.compositions['Caption'];
    var startLayer = {
        start: 0,
        time: 0,
        last: startComp.duration,
        duration: startComp.duration,
        uri: "composition://Caption",
        properties: {
            transform: {
                0: [
                    0, 0, 0,
                    0, 0, 0,
                    1, 1, 1,
                    0, 0, 0
                ]
            }
        }
    };
    ret.layers.push(startLayer);

    // Append scenes
    for (var i = 0; i < candidates.length; i++) {
        appendScene(candidates[i][0], candidates[i][1]);

        overlap = candidates[i][2];
    }

    // Append end scenes
    appendScene("End", tplJS.compositions["End"]);

    // Append background music
    ret.duration = currentTime;
    if (userJS.musicURL && userJS.musicURL != "") {
        var music = {
            start: 0,
            time: 0,
            duration: ret.duration,
            last: ret.duration,
            properties: {
                volume: (function() {
                    // fade in & fade out
                    ret = {};
                    ret['0'] = 0;
                    ret['0.5'] = 1;
                    ret[''+(ret.duration-1.5)] = 1;
                    ret[''+ret.duration] = 0;
                    return ret;
                })(),
            },
            uri: "file://" + userJS.musicURL,
        };
        ret.layers.push(music);
    }
}

function uriToGlobal(js, globalPath) {
    for (var cname in js.compositions) {
        var comp = js.compositions[cname];
        for (var lid in comp.layers) {
            var layer = comp.layers[lid];
            if (globalPath != "" && 
                layer.uri.indexOf("file://") == 0 &&
                layer.uri[7] != '/') {
                if (globalPath[globalPath.length - 1] != '/')
                    globalPath = globalPath + '/';
                layer.uri = layer.uri.replace("file://", "file://" + globalPath);
            }
        }
    }
}

var tplJS = JSON.parse(tpljs); // Template json
var userJS = JSON.parse(userjs);
var templateDir = userJS.templateURL;
if (templateDir.lastIndexOf("/") != -1) {
    templateDir = templateDir.substr(0, templateDir.lastIndexOf("/"));
} else {
    templateDir = "";
}
uriToGlobal(tplJS, templateDir);
var wrapJS = JSON.parse(wrapjs);
if (assetPath.length > 0 && assetPath[assetPath.length - 1] != "/") 
    assetPath = assetPath + "/";
uriToGlobal(wrapJS, assetPath + "wrap");

var scenes = getScenes(tplJS);
var width = tplJS.compositions[scenes[0].name].resolution.width;
var height = tplJS.compositions[scenes[0].name].resolution.height;
var comps = genResourcesComp(userJS, width, height);

//console.log("--------SCENES---------");
//console.log(scenes);
//console.log("---------Resources Comp--------");
//console.log(comps);

for (var k in comps) {
    var comp = comps[k];
    tplJS.compositions[comp.name] = comp.comp;
}
//console.log(fit(comps, scenes));
fillTML(tplJS, fit(comps, scenes), userJS, wrapJS);

var result = tplJS;

//console.log("---------Final result--------");
//console.log(result);

function toHalf(tml) {
    var resizer = {
        transform: function(len) {
            var ret = [];
            var j = 0;
            for(var j = 0; j < len; j += 12) {
                for(var i = 0; i < 6; i++) {
                    ret.push(i + j);
                }
            }
            return ret;
        },
        mask: function(len) {
            var ret = [];
            for(var i = 0; i < len; i++)
                ret.push(i);
            return ret;
        },
        ramp: function() {
            return [0,1];
        },
        '4color': function() {
            return [0, 1, 5, 6, 10, 11, 15, 16];
        }
    };
    var scaleDownResizer = function() {
        return [6, 7, 8];
    };
    var resize = function(props, resizer) {
        for(var time in props) {
            var prop = props[time];
            var ikeys = resizer(prop.length);
            for(var ik in ikeys) {
                prop[ikeys[ik]] *= 0.5;
            }
        }
    };
    for(var k in tml.compositions) {
        var comp = tml.compositions[k];
        comp.resolution.width /= 2;
        comp.resolution.height /= 2;
        for(var lk in comp.layers) {
            layer = comp.layers[lk];
            for(var k in resizer) {
                resize(layer.properties[k], resizer[k]);
            }
            if(layer.uri.substr(0, 14) != 'composition://') {
                var temp = layer.properties.transform;
                for(var pk in layer.properties.transform) {
                    var prop = layer.properties.transform[pk];
                    var temp = [0, 0, 0, 0, 0, 0, 0.5, 0.5, 1, 0, 0, 0];
                    for(var ak = 0; ak < prop.length; ak++) {
                        temp.push(prop[ak]);
                    }
                    layer.properties.transform[pk] = temp;
                }
            }
        }
    }
    return tml;
}
