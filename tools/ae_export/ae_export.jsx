/*******
 * src/define.js
 *******/
/*
 * Definitions
 */

// NULL as undefined
var NULL;

// Blend Mode map
var blendingModes = {
    // CS6
    4412: 0,
    4420: 1,
    4416: 2,
    4422: 3,
    4413: 4,
    4415: 5,
    4421: 6,
    4426: 7,
    4433: 8,
    // CC
    4612: 0,
    4620: 1,
    4616: 2,
    4622: 3,
    4613: 4,
    4615: 5,
    4621: 6,
    4626: 7,
    4633: 8
};

/*******
 * src/export.js
 *******/
/*
 * Export logic
 */
var Export = function() {
    this.comp = {};
    for(var i = 1; i <= app.project.numItems; i++) {
        var item = app.project.item(i);
        if('[object CompItem]' == item.toString()) {
            if(this.comp[item.name] && item.name[0] != '@' && item.name != '#+1') {
                throw "Duplicate composition name: " + item.name;
            }
            this.comp[item.name] = item;
        }
    }
};
Export.prototype.exportTo = function(filePath) {
    log('Export to: ' + filePath);
    this.tmlFile = new File(filePath);
    this.tmlFile.encoding = 'UTF8';
    this.tmlFile.open('w');
    this.files = {};
    this.colors = {};
    try {
        this.exportList = [];
        if(this.comp.MAIN)
            this.exportList = ['MAIN'];
        for(var k in this.comp) {
            if(k[0] == '#')
                this.exportList.push(k);
        }
        if(this.exportList.length <= 0) {
            throw "Main or scene compositions doesn't exist";
        }
        this.exported = {};
        this.compsCount = 0;
        for(var k in this.exportList) {
            this.compsCount += this.getCompsCount(this.comp[this.exportList[k]]);
        }
        this.exported = {};
        this.exportedCount = 0;
        this.tmlFile.write('{"version":0.01,"main":"MAIN","compositions":{');
        var comma = false;
        while(this.exportList.length > 0) {
            var compName = this.exportList.pop();
            if(this.exported[compName])
                continue;
            if(comma)
                this.tmlFile.write(',');
            comma = true;
            var expComp = this.exportComp(this.comp[compName]);
            this.tmlFile.write('"' + compName +'":');
            this.tmlFile.write(obj2str(expComp));
            log('  Write comp');
        }
        //this.tmlFile.write('},"usedfiles":');
        //this.tmlFile.write(obj2str(this.files));
        //this.tmlFile.write(',"usedcolors":');
        //this.tmlFile.write(obj2str(this.colors));
        this.tmlFile.write('}}');
    } finally {
        this.tmlFile.close();
    }
    alert('Export Done!');
};
Export.prototype.getCompsCount = function(comp) {
    if(this.exported[comp.name])
        return 0;
    this.exported[comp.name] = true;
    var count = 1;
    for(var i = 1; i <= comp.layers.length; i++) {
        var layer = comp.layers[i];
        if(layer && layer.source && '[object CompItem]' == layer.source.toString()) {
            count += this.getCompsCount(this.comp[layer.source.name]);
        }
    }
    return count;
};
Export.prototype.exportComp = function(comp) {
    if(this.exported[comp.name])
        return NULL;
    this.exported[comp.name] = true;
    log('Export Comp: ' + comp.name);
    setProgressStatus('Composition: ' + comp.name);
    var ret = {};
    ret.resolution = {
        width: comp.width,
        height: comp.height
    };
    ret.duration = comp.duration;
    ret.layers = [];
    setSubProgress(0);
    for(var i = 1; i <= comp.layers.length; i++) {
        log('  Export Layer: ' + i);
        var exportedLayer = this.exportLayer(comp.layers[i]);
        if(NULL != exportedLayer)
            ret.layers.push(exportedLayer);
        setSubProgress(i * 100 / comp.layers.length);
        if(!isProgressWindowVisible())
            throw 'Export Canceled';
    }
    log('  Export layer finish');
    this.exportedCount++;
    setMainProgress(this.exportedCount * 100 / this.compsCount);
    return ret;
};
Export.prototype.exportLayer = function(layer) {
    var ret = {};
    var source = layer.source;
    var type;
    if(source)
        type = source.toString();
    else
        type = layer.toString();
    log('    Layer type: ' + type)
    if('[object CompItem]' == type) {
        this.exportList.push(source.name);
        ret.uri = 'composition://' + source.name;
    } else if('[object FootageItem]' == type) {
        var path;
        if(!source.file) {
            // color source
            var color = source.mainSource.color;
            path = source.width + ',' + source.height + ',' 
                + color[0] + ',' + color[1] + ',' + color[2];
            ret.uri = 'color://' + path;
            //this.colors[path] = {
            //    color: color,
            //    width: source.width,
            //    height: source.height
            //};
        } else {
            path = relPath(source.file.fullName);
            ret.uri = 'file://' + path;
        }
        //this.files[path] = {
        //    width: source.width,
        //    height: source.height
        //};
        log('    Export Footage: ' + path);
    } else if('[object TextLayer]' == type) {
        if(!this.textCounter)
            this.textCounter = 0;
        ret.uri = 'text://' + layer("Source Text").value.text.replace(/[\n\r]/, '') + '@' + (this.textCounter++);
        var txtProp = {};
        var txtExport = function (key, aeKey, correction) {
            var proced = false;
            var prevVal = -314.159;
            var prop = {};
            for(var t = layer.inPoint; ; t += 0.1) {
                if(t > layer.outPoint) {
                    if(proced)
                        break;
                    t = layer.outPoint;
                    proced = true;
                }
                var st = layer("Source Text").valueAtTime(t, false);
                var val = st[aeKey];
                if(correction)
                    val = correction(val);
                if(val != prevVal) {
                    prevVal = val;
                    prop[t] = val;
                }
            }
            txtProp[key] = prop;
        };
        txtExport('text', 'text', function(val) {
            return val.replace(/[\n\r]/, '');
        });
        txtExport('size', 'fontSize');
        txtExport('justification', 'justification', function (val) {
            var preset = {
                6813:0, // left
                6815:1, // center
                6814:2, // right
            };
            if(preset[val])
                return preset[val];
            return 0; // default left
        });
        txtExport('color', 'fillColor', function(val) {
            var r = val[0];
            var g = val[1];
            var b = val[2];
            r =  Math.ceil(r * 127);
            g =  Math.ceil(g * 127);
            b =  Math.ceil(b * 127);
            return r * 128 * 128 + g * 128 + b;
        });
        txtExport('tracking', 'tracking', function(val) {
            return val * 0.01;
        });
        txtProp['scale_x'] = {'0':1};
        txtProp['scale_y'] = {'0':1};
        txtProp['bold'] = {'0':false};
        txtProp['italic'] = {'0':false};
        txtProp['font'] = {'0':'default'};
        ret['text-properties'] = txtProp;
    } else {
        log('    Unexpected layer type');
        return NULL;
    }

    ret.trkMat = layer.trackMatteType % 10 - 2;
    ret.visible = layer.enabled ? 1 : 0;
    ret.blend = blendingModes[layer.blendingMode];
    ret.time = layer.inPoint;
    ret.duration = layer.outPoint - layer.inPoint;
    ret.start = layer.inPoint - layer.startTime;
    ret.last = ret.duration;
    ret.properties = {};
    var defaultFilter = function () {
        var ret = [];
        for(k in arguments)
            ret = ret.concat(arguments[k]);
        return ret;
    }
    for(var pmk in PropertyMapping) {
        if(pmk == 'volume' && !layer.audioEnabled) {
            ret.properties['volume'] = {};
            ret.properties['volume'][ret.time] = [0];
            ret.properties['volume'][layer.outPoint] = [0];
            continue;
        }
        var propName = pmk;
        if(PropertyMapping[pmk].name)
            propName = PropertyMapping[pmk].name;
        log('    Export property: (' + pmk + ')' + propName);
        var proced = false;
        var alias = PropertyMapping[pmk].alias;
        if(!alias)
            alias = {};
        for(var t = ret.time; ; t += 0.1) {
            if(t > layer.outPoint) {
                if(proced)
                    break;
                t = layer.outPoint;
                proced = true;
            }
            
            if(!getArgs) function getArgs(clayer, t, pmk) {
                var args = [];
                var take = PropertyMapping[pmk].map;
                if(typeof(take) == 'string') {
                    take = [take];
                }
                var notNull = false;
                for(var pnk in take) {
                    var val;
                    try {
                        var kpath = take[pnk];
                        if(typeof(kpath) == 'string')
                            kpath = kpath.split('/');
                        val = clayer;
                        for(var pk in kpath) {
                            var pname = kpath[pk];
                            if(isFinite(pname))
                                pname = pname * 1;
                            if(!val.property(pname) && alias[pname]) {
                                pname = alias[pname];
                            }
                            val = val.property(pname);
                        }
                        val = val.valueAtTime(t, false);
                        notNull = true;
                    } catch(e) {
                        val = NULL;
                    }
                    args.push(val);
                }
                if(!notNull) {
                    throw 'all null';
                }
                for(var k in args) {
                    if(undefined == args[k] || NULL == args[k])
                        args[k] = 0;
                }
                var result = PropertyMapping[pmk].set;
                if(!result)
                    result = defaultFilter;
                result = result.apply([], args);
                return result;
            }
            var result = [];
            try {
                result = getArgs(layer, t, pmk);
                if(pmk == 'transform') {
                    var pl = layer.parent;
                    while(pl) {
                        result = result.concat(getArgs(pl, t, pmk));
                        pl = pl.parent;
                    }
                }
            } catch(e) {
                if(e == 'all null')
                    continue;
                throw e;
            }

            if(!ret.properties[propName]) {
                ret.properties[propName] = {};
            }
            ret.properties[propName]['' + t] = result;
        }
        if(ret.properties[propName])
            ret.properties[propName] = this.simplifyProperties(ret.properties[propName], PropertyMapping[pmk].error);
    }
    return ret;
};
Export.prototype.simplifyProperties = function(props, error) {
    var n = 0;
    for(var k in props) {
        if(++n > 3)
            break;
    }
    if(n < 4)
        return props;
    var keep = {};
    var keys = [];
    var prevKey;
    var saveNext = true;
    for(var k in props) {
        if(saveNext) {
            saveNext = false;
            keep[k] = props[k];
        }
        if(keys.length > 1) {
            var midKey = keys[Math.floor(keys.length / 2)];
            var assume = this.interpolate(keys[0], props[keys[0]], k, props[k], midKey);
            if(!near(assume, props[midKey], error)) {
                keep[prevKey] = props[prevKey];
                keep[k] = props[k];
                keys = [];
                saveNext = true;
            }
        }
        prevKey = k;
        keys.push(k);
    }
    if(keys.length > 0) {
        var k = keys[keys.length - 1];
        keep[k] = props[k];
    }
    return keep;
}
Export.prototype.interpolate = function(startTime, startValue, endTime, endValue, getTime) {
    var ratio = (getTime - startTime) / (endTime - startTime);
    var ret = [];
    for(k in startValue) {
        ret[k] = startValue[k] + ratio * (startValue[k] - endValue[k]);
    }
    return ret;
}

var PropertyMapping = [];
function mapProperty(options) {
    PropertyMapping.push(options);
}

/*******
 * src/main.js
 *******/
﻿function main() {
    showProgressWindow();
    PropertyMapping.sort(function(a, b) {
        if(a.order)
            a = a.order;
        else
            a = 0;
        if(b.order)
            b = b.order;
        else
            b = 0;
        return b - a;
    });
    try {
        log('starting....');
        new Export().exportTo(app.project.file.fullName + '.tml');
    } catch (e) {
        alert(e);
    }
    closeProgressWindow();
}


var logFile = new File(projDir + 'log');
logFile.open('w');
logFile.write(new Date().toGMTString() + "\n");
logFile.close();
function log(str) {
    logFile.open('a');
    logFile.writeln(str);
    logFile.close();
}
var NULL;


/*******
 * src/progress.js
 *******/
function progressWindow() {
    var windowName = 'CCPlus Export Util';
    var ret = Window.find('palette', windowName);
    if(ret)
        return ret;
    var ret = new Window('palette', windowName, NULL, {closeButton:false});
    ret.ui = {};

    ret.ui.statusText = ret.add('edittext', NULL, '');
    ret.ui.statusText.preferredSize.width = 300;
    ret.ui.statusText.active = false;
    ret.ui.mainProgressBar = ret.add('progressbar', NULL, 0, 100);
    ret.ui.mainProgressBar.preferredSize.width = 300;
    ret.ui.subProgressBar = ret.add('progressbar', NULL, 0, 100);
    ret.ui.subProgressBar.preferredSize.width = 300;

    ret.add('button', NULL, 'cancel').onClick = function() {
        ret.close();
    };
    ret.preferredSize.width = 310;
    return ret;
}
function showProgressWindow() {
    progressWindow().show();
    setProgressStatus('Starting export....');
    setMainProgress(0);
    setSubProgress(0);
}
function setProgressStatus(str) {
    var w = progressWindow();
    w.ui.statusText.text = str;
    w.update();
    $.sleep(50);
}
function setMainProgress(prog) {
    var w = progressWindow();
    w.ui.mainProgressBar.value = prog;
    w.update();
    $.sleep(50);
}
function setSubProgress(prog) {
    var w = progressWindow();
    w.ui.subProgressBar.value = prog;
    w.update();
    $.sleep(50);
}
function closeProgressWindow() {
    progressWindow().close();
}
function isProgressWindowVisible() {
    return progressWindow().visible;
}


/*******
 * src/propertymap/4color.js
 *******/
mapProperty({
    name: '4color',
    order: 10,
    map:[
        'Effects/4-Color Gradient/Point 1',
        'Effects/4-Color Gradient/Color 1',
        'Effects/4-Color Gradient/Point 2',
        'Effects/4-Color Gradient/Color 2',
        'Effects/4-Color Gradient/Point 3',
        'Effects/4-Color Gradient/Color 3',
        'Effects/4-Color Gradient/Point 4',
        'Effects/4-Color Gradient/Color 4',
        'Effects/4-Color Gradient/Blend',
        'Effects/4-Color Gradient/Opacity',
        'Effects/4-Color Gradient/Blending Mode',
    ],
    set:function(pos1, color1, pos2, color2, pos3, color3, pos4, color4, blend, opac, mode) {
        try {
            mode = [0, -1, 0, 1, 2, 3][mode];
        } catch(E) {
            mode = 0;
        }
        return [
            pos1[0], pos1[1],
            color1[2] * 255, color1[1] * 255, color1[0] * 255,
            pos2[0], pos2[1],
            color2[2] * 255, color2[1] * 255, color2[0] * 255,
            pos3[0], pos3[1],
            color3[2] * 255, color3[1] * 255, color3[0] * 255,
            pos4[0], pos4[1],
            color4[2] * 255, color4[1] * 255, color4[0] * 255,
            blend,
            opac / 100,
            mode
        ];
    }
});

/*******
 * src/propertymap/gaussian.js
 *******/
mapProperty({
    name: 'gaussian',
    order: -5,
    map:['Effects/Gaussian Blur/Blurriness','Effects/Gaussian Blur/Blur Dimensions'],
    set:function(blurriness,dimensions) {
        return [
            blurriness, dimensions
        ];
    },
    error:[0.1, 0.001]
});

/*******
 * src/propertymap/grayscale.js
 *******/
mapProperty({
    name: 'grayscale',
    order: -5,
    map:[
        'Effects/Black & White/Reds',
        'Effects/Black & White/Yellows',
        'Effects/Black & White/Greens',
        'Effects/Black & White/Cyans',
        'Effects/Black & White/Blues',
        'Effects/Black & White/Magentas',
        'Effects/Black & White/Tint:',
        'Effects/Black & White/Tint Color',
    ],
    set:function(r,y,g,c,b,m,haveTint,tintColor) {
        var ret = [r,y,g,c,b,m,0,0];
        if(haveTint > 0) {
            var hs = getHueSat(tintColor);
            ret[6] = hs[0];
            ret[7] = hs[1];
        }
        return ret;
    }
});

/*******
 * src/propertymap/hsl.js
 *******/
mapProperty({
    name: 'hsl',
    order: -5,
    map:[
        ['Effects','Hue/Saturation','Master Hue'],
        ['Effects','Hue/Saturation','Master Saturation'],
        ['Effects','Hue/Saturation','Master Lightness'],
    ],
    alias:{
        'Hue/Saturation':'Color Balance (HLS)',
        'Master Hue':'Hue',
        'Master Saturation':'Saturation',
        'Master Lightness':'Lightness'
    },
    set:function(h,s,l) {
        return [
            h / 2, 1 + s / 100, 1 + l / 100
        ];
    }
});

/*******
 * src/propertymap/mask.js
 *******/
mapProperty({
    name: 'mask',
    order: 5,
    map:['Masks/1/Mask Feather','Masks/1/maskShape'],
    set:function(feather, maskShape) {
        var ret = [];
        ret.push(feather[0]);
        ret.push(feather[1]);
        for(var k = 0; k < maskShape.vertices.length; k++) {
            var pnt = maskShape.vertices[k];
            ret.push(pnt[1]);
            ret.push(pnt[0]);
            var nk = (k + 1) % maskShape.vertices.length;
            var p0 = maskShape.vertices[k];
            var p1 = maskShape.outTangents[k];
            var p2 = maskShape.inTangents[nk];
            var p3 = maskShape.vertices[nk];
            p1[0] += p0[0];
            p1[1] += p0[1];

            p2[0] += p3[0];
            p2[1] += p3[1];
            var cuts = 7;
            var step = 1.0 / cuts;
            for(var i = 1; i < cuts; i++) {
                var t = step * i;
                var rt = 1 - t;
                var px = rt * rt * rt * p0[0] + 3 * t * rt * rt * p1[0] + 3 * t * t * rt * p2[0] + t * t * t* p3[0];
                var py = rt * rt * rt * p0[1] + 3 * t * rt * rt * p1[1] + 3 * t * t * rt * p2[1] + t * t * t* p3[1];
                ret.push(py);
                ret.push(px);
            }
        }
        return ret;
    },
});

/*******
 * src/propertymap/opacity.js
 *******/
mapProperty({
    name: 'opacity',
    order: -5,
    map:'Opacity',
    set:function(opac) {
        return [opac / 100];
    },
});

/*******
 * src/propertymap/ramp.js
 *******/
mapProperty({
    name: 'ramp',
    order: 10,
    map:[
        'Effects/Ramp/Start of Ramp',
        'Effects/Ramp/Start Color',
        'Effects/Ramp/End of Ramp',
        'Effects/Ramp/End Color',
        'Effects/Ramp/Ramp Shape',
        'Effects/Ramp/Blend With Original'
    ],
    alias:{
        'Ramp':'Gradient Ramp'
    },
    set:function(startPos, startColor, endPos, endColor, shape, blend) {
        return [
            shape == 1 ? -1 : 1,
            startPos[0], startPos[1],
            startColor[2] * 255, startColor[1] * 255, startColor[0] * 255,
            endPos[0], endPos[1],
            endColor[2] * 255, endColor[1] * 255, endColor[0] * 255,
            blend / 100
        ];
    }
});

/*******
 * src/propertymap/transform.js
 *******/
mapProperty({
    name: 'transform',
    order: 0,
    map:['Position','Anchor Point','Scale','X Rotation','Y Rotation','Z Rotation'],
    set:function(pos, anchor, scale, rotateX, rotateY, rotateZ) {
        if(!rotateX)
            rotateX = 0;
        if(!rotateY)
            rotateY = 0;
        if(!rotateZ)
            rotateZ = 0;
        return [
            pos[0], pos[1], pos[2],
            anchor[0], anchor[1], anchor[2],
            scale[0] * 0.01, scale[1] * 0.01, scale[2] * 0.01,
            rotateX, rotateY, rotateZ
        ];
    },
    error:[0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01]
});

/*******
 * src/propertymap/volume.js
 *******/
mapProperty({
    name: 'volume',
    order: -5,
    map:'Audio Levels',
    set:function(lvl) {
        if(typeof(lvl) == 'object') {
            var v = 0;
            var n = 0;
            for(k in lvl) {
                if(typeof(lvl[k]) == 'number') {
                    v += lvl[k];
                    n++;
                }
            }
            if(n > 0)
                v /= n;
            lvl = v;
        }
        return [(lvl + 48) / 48];
    },
})

/*******
 * src/util.js
 *******/
/* 
 * Utilities
 */
function near(arr1, arr2, error) {
    for(k in arr1) {
        var v = arr1[k] - arr2[k];
        var e = 0.03;
        if(error && error[0] && error.length) {
            k %= error.length;
            e = error[k];
        }
        if(v > e || v < -e) {
            return false;
        }
    }
    return true;
}
var projDir = app.project.file.fullName;
var p = projDir.lastIndexOf('/');
projDir = projDir.substr(0, p + 1);
function relPath(pathName) {
    if(pathName.substr(0, projDir.length) == projDir)
        pathName = pathName.substr(projDir.length);
    return pathName;
}
function obj2str(obj) {
    function _obj2str(obj) {
        if(obj === undefined)
            return 'null';
        else if(obj === false)
            return 'false';
        else if(obj === true)
            return 'true';
        var type = typeof(obj);
        if(type == 'number') {
            return obj;
        } else if(type == 'string') {
            return '"' + obj + '"';
        } else if(type == 'object') {
            var ret = '';
            if(obj.length !== undefined) {
                var cma = true;
                for(k in obj) {
                    if(!cma)
                        ret += ',';
                    cma = false;
                    ret += _obj2str(obj[k]);
                }
                return '[' + ret + "]\n";
            } else if('[object Object]' == obj.toString()) {
                var cma = true;
                for(k in obj) {
                    if(!cma)
                        ret += ',';
                    cma = false;
                    ret += '"' + k + '":' + _obj2str(obj[k]);
                }
                return '{' + ret + "}\n";
            }
        }
    }
    var ret = _obj2str(obj);
    return ret;
    ret = ret.replace(/([\]\}])/g, "\n$1");
    ret = ret.replace(/\[[\n\s]*\]/g, '[]');
    var lines = ret.split("\n");
    ret = '';
    var sp = '';
    for(k in lines) {
        var line = lines[k];
        if(line.indexOf(']') > -1 || line.indexOf('}') > -1) {
            sp = sp.substr(4);
        }
        ret += sp + line + "\n";
        if(line.indexOf('[') > -1 || line.indexOf('{') > -1) {
            sp += "    ";
        }
    }
    return ret;
}
function getHueSat(r, g, b) {
    var min = Math.min(r, g, b);
    var max = Math.max(r, g, b);
    var delta = max - min;
    if(delta <= 0)
        return [0, 0];
    var hue = 0;
    var sat = 0;
    var sum = max + min;
    if(sum < 1) {
        sat = delta / sum;
    } else {
        sat = delta / (2 - sum);
    }
    if(r == max) {
        hue = (g - b) / delta;
    } else if(g == max) {
        hue = 2 + (b - r) / delta;
    } else {
        hue = 4 + (r - g) / delta; 
    }
    return [hue, sat];
}

main();
