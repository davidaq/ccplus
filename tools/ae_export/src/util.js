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
                return '[' + ret + ']';
            } else if('[object Object]' == obj.toString()) {
                var cma = true;
                for(k in obj) {
                    if(!cma)
                        ret += ',';
                    cma = false;
                    ret += '"' + k + '":' + _obj2str(obj[k]);
                }
                return '{' + ret + '}';
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
