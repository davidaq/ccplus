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

