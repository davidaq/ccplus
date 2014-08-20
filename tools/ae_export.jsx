/*
 * Property maping
 */
var PropertyMapping = {
    ramp:{
        map:[
            'Effects/Ramp/Start of Ramp',
            'Effects/Ramp/Start Color',
            'Effects/Ramp/End of Ramp',
            'Effects/Ramp/End Color',
            'Effects/Ramp/Ramp Shape',
            'Effects/Ramp/Blend With Original'
        ],
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
    },
    mask:{
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
                var cuts = 10;
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
    },
    transform:{
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
    },
    opacity:{
        map:'Opacity',
        set:function(opac) {
            return [opac / 100];
        },
    },
    volume:{
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
    },
    gaussian:{
        map:['Effects/Gaussian Blur/Blurriness','Effects/Gaussian Blur/Blur Dimensions'],
        set:function(blurriness,dimensions) {
            return [
                blurriness, dimensions
            ];
        },
        error:[0.1, 0.001]
    },
    hsl1:{
        name:'hsl',
        map:[
            ['Effects','Hue/Saturation','Master Hue'],
            ['Effects','Hue/Saturation','Master Saturation'],
            ['Effects','Hue/Saturation','Master Lightness'],
        ],
        set:function(h,s,l) {
            return [
                h / 2, 1 + s / 100, 1 + l / 100
            ];
        }
    },
    hsl2:{
        name:'hsl',
        map:['Effects/Color Balance (HLS)/Hue','Effects/Color Balance (HLS)/Saturation','Effects/Color Balance (HLS)/Lightness'],
        set:function(h,s,l) {
            return [
                h / 2, 1 + s / 100, 1 + l / 100
            ];
        }
    },
    grayscale:{
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
    }
};
/*
 * Export logic
 */
var Export = function() {
    this.comp = {};
    for(var i = 1; i <= app.project.numItems; i++) {
        var item = app.project.item(i);
        if('[object CompItem]' == item.toString()) {
            if(this.comp[item.name]) {
                throw "Duplicate composition name: " + item.name;
            }
            this.comp[item.name] = item;
        }
    }
};
Export.prototype.exportTo = function(filePath) {
    log('Export to: ' + filePath);
    this.tmlFile = new File(filePath);
    this.tmlFile.open('w');
    this.files = {};
    this.colors = {};
    try {
        if(!this.comp.MAIN) {
            throw "Main composition doesn't exist";
        }
        this.exportList = ['MAIN'];
        var tml = {version:0.01, main:'MAIN', compositions:{}};
        while(this.exportList.length > 0) {
            var compName = this.exportList.pop();
            tml.compositions[compName] = this.exportComp(this.comp[compName]);
        }
        tml.usedfiles = this.files;
        tml.usedcolors = this.colors;
        this.tmlFile.write(obj2str(tml));
    } finally {
        this.tmlFile.close();
    }
    alert('Export Done!');
};
Export.prototype.exportComp = function(comp) {
    log('Export Comp: ' + comp.name);
    var ret = {};
    ret.resolution = {
        width: comp.width,
        height: comp.height
    };
    ret.duration = comp.duration;
    ret.layers = [];
    for(var i = 1; i <= comp.layers.length; i++) {
        log('  Export Layer: ' + i);
        var exportedLayer = this.exportLayer(comp.layers[i]);
        if(NULL != exportedLayer)
            ret.layers.push(exportedLayer);
    }
    return ret;
};
Export.prototype.exportLayer = function(layer) {
    var ret = {};
    var source = layer.source;
    if(!source)
        return NULL;
    var type = source.toString();
    log('    Layer type: ' + type)
    if('[object CompItem]' == type) {
        this.exportList.push(source.name);
        ret.uri = 'composition://' + source.name;
    } else if('[object FootageItem]' == type) {
        var path;
        if(!source.file) {
            // color source
            var color = source.mainSource.color;
            path = source.width + 'x' + source.height + '#' 
                + color[0] + '_' + color[1] + '_' + color[2];
            path = '(colors)/' + path + '.png';
            this.colors[path] = {
                color: color,
                width: source.width,
                height: source.height
            };
        } else {
            path = relPath(source.file.fullName);
        }
        ret.uri = 'file://' + path;
        this.files[path] = {
            width: source.width,
            height: source.height
        };
        log('    Export Footage: ' + path);
    } else {
        log('    Unexpected layer type');
        return NULL;
    }
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
        var propName = pmk;
        if(PropertyMapping[pmk].name)
            propName = PropertyMapping[pmk].name;
        log('    Export property: ' + pmk);
        var proced = false;
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

function __main__() {
    try {
        log('starting....');
        new Export().exportTo(app.project.file.fullName + '.tml');
    } catch (e) {
        alert(e);
    }
}
/* 
 * Utilities
 */
function near(arr1, arr2, error) {
    for(k in arr1) {
        var v = arr1[k] - arr2[k];
        var e = 0.1;
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
                for(k in obj) {
                    ret += ',' + _obj2str(obj[k]);
                }
                return '[' + ret + ']';
            } else if('[object Object]' == obj.toString()) {
                for(k in obj) {
                    ret += ',"' + k + '":' + _obj2str(obj[k]);
                }
                return '{' + ret + '}';
            }
        }
    }
    var ret = _obj2str(obj);
    ret = ret.replace(/,/g, ",\n");
    ret = ret.replace(/([\[\{]),/g, "$1");
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
var logFile = new File(projDir + 'log');
logFile.open('w');
function log(str) {
    logFile.writeln(str);
    logFile.close();
    logFile.open('a');
}
var NULL;
__main__();
logFile.close();

