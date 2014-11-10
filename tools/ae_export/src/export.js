/*
 * Export logic
 */
var Export = function() {
    this.comp = {};
    for(var i = 1; i <= app.project.numItems; i++) {
        var item = app.project.item(i);
        if('[object CompItem]' == item.toString()) {
            if(this.comp[item.name] && item.name[0] != '@') {
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
        this.tmlFile.write('},"usedfiles":');
        this.tmlFile.write(obj2str(this.files));
        this.tmlFile.write(',"usedcolors":');
        this.tmlFile.write(obj2str(this.colors));
        this.tmlFile.write('}');
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
    } else if('[object TextLayer]' == type) {
        if(!this.textCounter)
            this.textCounter = 0;
        ret.uri = 'text://' + layer("Source Text").value.text + '@' + (this.textCounter++);
        var txtProp = {};
        var txtExport = function (key, aeKey, correction) {
            var proced = false;
            var prevVal = NULL;
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
        txtExport('text', 'text');
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
