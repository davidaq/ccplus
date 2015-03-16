#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QTime>
#include "ccplus.hpp"
#include "config.hpp"

void printHelp() {
    qDebug() << "Usage: ccplus <input tml> [-option_key option_value ...] [-flag ...]";
    qDebug() << "    Options:";
    qDebug() << "\t" << "-o       :   output video file path, defaults to <input tml>.mp4";
    qDebug() << "\t" << "-fps     :   frames per second of the output video, defaults to 18";
    qDebug() << "\t" << "-dir     :   directory where rendering gets resource footage files,";
    qDebug() << "\t" << "             defaults to the directory of the input file";
    qDebug() << "\t" << "-assets  :   assets bundle path, defaults to \"assets\" relative to this executable";
    qDebug() << "    Flags:";
    qDebug() << "\t" << "-pot     :   force rendering everything in power of 2 sizes";
    qDebug() << "\t" << "-M       :   allow use for more memory";
    exit(1);
}

QString dirname(QString file) {
    QStringList pathList = file.split(QRegExp("[\\\\\\/]"));
    pathList.removeLast();
    return pathList.join(QDir::separator()) + QDir::separator();
}

int main(int argc, char *argv[]) {
    QSet<QString> valuedOptions, flagedOptions;
    valuedOptions << "o" << "fps" << "dir" << "assets";
    flagedOptions << "pot" << "M";

    QHash<QString, QString> options;
    QSet<QString> flags;
    QString key;
    QString inputPath;
    bool isOption = false;
    for(int i = 1; i < argc; i++) {
        QString arg(argv[i]);
        if(isOption) {
            options[key] = arg;
            isOption = false;
        } else {
            if(arg[0] == '-') {
                key = arg.mid(1);
                if(valuedOptions.contains(key)) {
                    isOption = true;
                } else if(flagedOptions.contains(key)) {
                    flags << key;
                } else {
                    qDebug() << "Warning: Ignored unrecognized flag or option " << arg;
                }
            } else {
                if(inputPath.isEmpty())
                    inputPath = arg;
                else {
                    qDebug() << "Warning: Only one input file is allowed, will ignore " << arg;
                }
            }
        }
    }
    if(inputPath.isEmpty()) {
        qDebug() << "Error: Must specify input tml file";
        printHelp();
    }

#define OPT(KEY, DEFAULT) (options.contains(#KEY) ? options[#KEY] : DEFAULT)
#define ABS_PATH(X) QDir().absoluteFilePath(X).toUtf8().data()

    inputPath = ABS_PATH(inputPath);
    CCPlus::setAssetsPath(ABS_PATH(OPT(assets, dirname(argv[0]) + "assets")));
    CCPlus::RenderTarget t;
    t.inputPath = inputPath.toUtf8().data();
    t.outputPath = ABS_PATH(OPT(o, inputPath + ".mp4"));
    t.footageDir = ABS_PATH(OPT(dir, dirname(inputPath)));
    t.fps = OPT(fps, "18").toInt();
    if(t.fps < 4)
        t.fps = 4;
    CCPlus::renderFlag = 0;
    if(flags.contains("pot")) {
        CCPlus::renderFlag |= CCPlus::FORCE_POT;
    }
    if(flags.contains("M")) {
        CCPlus::collectorTimeInterval     = 1;
        CCPlus::collectorWindowSize       = 10;
        CCPlus::collectorThreadsNumber    = 8;
        CCPlus::renderFlag |= CCPlus::LARGE_MEM;
    }
    QApplication app(argc, argv);
    QTime timer;
    timer.start();
    CCPlus::go(t);
    t.waitFinish();
    app.exit(0);
    qDebug() << "Elapsed: " << timer.elapsed() * 0.001 << "s";
    return 0;
}
