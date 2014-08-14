/*
 * Property maping
 */
var PropertyMapping = {
    transform:{
        map:['Position','Anchor Point','Scale','Rotation'],
        set:function(pos, anchor, scale, rotate) {
            if(!rotate)
                rotate = 0;
            return [
                pos[1], pos[0],
                anchor[1], anchor[0],
                scale[1] * 0.01, scale[0] * 0.01,
                rotate
            ];
        },
        error:[0.5, 0.5, 0.5, 0.5, 0.01, 0.01, 0.01]
    },
    opacity:{
        map:'Opacity',
        set:function(opac) {
            return [opac / 100];
        },
        error:[0.08]
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
        error:[0.1]
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
    this.tmlFile = new File(filePath);
    this.tmlFile.open('w');
    this.files = {}
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
        this.tmlFile.write(obj2str(tml));
    } finally {
        this.tmlFile.close();
    }
    alert('Export Done!');
};
Export.prototype.exportComp = function(comp) {
    var ret = {};
    ret.resolution = {
        width: comp.width,
        height: comp.height
    };
    ret.duration = comp.duration;
    ret.layers = [];
    for(var i = 1; i <= comp.layers.length; i++) {
        ret.layers.push(this.exportLayer(comp.layers[i]));
    }
    return ret;
};
Export.prototype.exportLayer = function(layer) {
    var ret = {};
    var source = layer.source;
    var type = source.toString();
    if('[object CompItem]' == type) {
        this.exportList.push(source.name);
        ret.uri = 'composition://' + source.name;
    } else if('[object FootageItem]' == type) {
        var path = relPath(source.file.fullName);
        ret.uri = 'file://' + path;
        this.files[path] = {
            width: source.width,
            height: source.height
        };
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
        var proced = false;
        for(var t = ret.time; ; t += 0.1) {
            if(t > layer.outPoint) {
                if(proced)
                    break;
                t = layer.outPoint;
                proced = true;
            }
            var args = [];
            var take = PropertyMapping[pmk].map;
            if(typeof(take) == 'string') {
                take = [take];
            }
            var notNull = false;
            for(var pnk in take) {
                var val;
                try {
                    var kpath = take[pnk].split('/');
                    val = layer;
                    for(var pk in kpath) {
                        val = val.property(kpath[pk]);
                    }
                    val = val.valueAtTime(t, false);
                    notNull = true;
                } catch(e) {
                    val = NULL;
                }
                args.push(val);
            }
            if(!notNull) {
                continue;
            }
            for(k in args) {
                if(undefined == args[k] || NULL == args[k])
                    args[k] = 0;
            }
            var result = PropertyMapping[pmk].set;
            if(!result)
                result = defaultFilter;
            result = result.apply([], args);
            if(!ret.properties[pmk]) {
                ret.properties[pmk] = {};
            }
            ret.properties[pmk]['' + t] = result;
        }
        if(ret.properties[pmk])
            ret.properties[pmk] = this.simplifyProperties(ret.properties[pmk], PropertyMapping[pmk].error);
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
        if(v > error[k] || v < -error[k]) {
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
var NULL;
__main__();



