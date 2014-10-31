/*
 * Generate a TML file based on users' images and template tml file.
 * Try to fit users resources to scenes suitably.
 * Doc: http://dev.ccme.me/wiki?p=%E8%AE%BE%E8%AE%A1%E6%96%87%E6%A1%A3%2Fme%E8%A7%86%E9%A2%91%E6%A8%A1%E6%9D%BF%E6%8A%80%E6%9C%AF%E6%96%87%E6%A1%A3
 *
 * Input:
 * @tpl = tpl tml;
 * @userjs = users resources description json;
 *
 * Output: 
 * @result = ret tml json;
 */

// Some constants

var SIZE = [640, 640];

/* 
 * Parse tpl tml and return scenes list
 * [senes, duration, num_ele]
 */
function getScenes(tpl) {

}

/*
 * Parse config JSON and return users resources list 
 * [name, comp]
 */
function genResourcesComp(js) {

}

/*
 * return [scene, comp_name1, comp_name2...] in order
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

var comps = genResourcesComp(userJS);

var len = comps.length();
for (var i = 0; i < len; i++) {
    tplJS[comps[i]] = comps[i];
}

fillTML(tplJS, fit(comps, getScenes(tpl)));
