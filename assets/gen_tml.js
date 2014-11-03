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
 * [scene_name, duration, num_ele]
 */
function getScenes(tpl) {
    var ret = [];
    var comps = tpl["compositions"];
    var cnt = 0;
    for (var name in comps) {
        if (name[0] == '#') {
            var cmp = comps[name];
            var layers = cmp.layers;
            var num_ele = 0;
            for (var ln in layers) {
                if (ln.uri[14] == '@')
            }
            // Thanks to tiny js -- ugly linked list implementeation
            ret[cnt] = [name, cmp.duration, num_ele];
            cnt++;
        }
    }
    return ret;
}

/*
 * Parse config JSON and return users resources list 
 * [name, comp]
 */
function genResourcesComp(js, width, height) {
    var medias = js.medias;
    var idx = 0;
    var ret = [];
    for (var m in medias) {
        var comp = {};
        var name = "@" + idx;
        comp.duration = 5.0;
        if (m.duration)
            comp.duration = m.duration;
        comp.resolution.width = width;
        comp.resolution.height = height;
        comp.layers = [];

        var l = {};
        l.uri = "file://" + m.filename;
        l.time = 0;
        l.start = 0;
        l.duration = comp.duration;
        l.last = comp.duration;
        l.tranform = {};
        l.tranform["0"] = [
            width / 2.0, height / 2.0, 0, 
            m.x + m.w / 2.0, m.y + m.h / 2.0, 0, 
            width / m.w, height / m.h, 1,
            0, 0, 0
        ];
        comp.layers[0] = l;

        ret[idx] = [name, comp];
        idx++;
    }
}

/*
 * return [scene_name, comp_name1, comp_name2...] in order
 */
function fit(comps, scenes) {

}

/*
 * Generate result tml file based on fitted data
 */
function fillTML(tplJS, fitted) {

}

var tplJS = eval(tpljs); // Template json
var userJS = eval(userjs);

var scenes = getScenes(tplJS);
var width = tplJS.compositions[scenes[0][0]].resolution.width;
var height = tplJS.compositions[scenes[0][0]].resolution.height;
var comps = genResourcesComp(userJS, width, height);

console.log(scenes);
console.log("-----------------");
console.log(comps);

//for (var comp in comps) {
//    tplJS.compositions[comps[0]] = comp[1];
//}

//fillTML(tplJS, fit(comps, getScenes(tpl)));

var result = {hello: "world"};
