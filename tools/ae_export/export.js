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
