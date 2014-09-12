function main() {
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
}


var logFile = new File(projDir + 'log');
logFile.open('w');
logFile.write(new Date().toGMTString());
logFile.close();
function log(str) {
    logFile.open('a');
    logFile.writeln(str);
    logFile.close();
}
var NULL;

