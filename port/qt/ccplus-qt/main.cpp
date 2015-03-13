#include <QApplication>
#include <QDir>
#include <QDebug>
#include "ccplus.hpp"

void printHelp() {
    qDebug() << "Usage: ccplus <input tml> [-option_key option_value ...]";
    qDebug() << "\t" << "Options:";
    qDebug() << "\t" << "-o:       output video file path, defaults to <input tml>.mp4";
    qDebug() << "\t" << "-fps:     frames per second of the output video, default 15";
    qDebug() << "\t" << "-assets:  the ccplus assets bundle path, default to \"assets\" relative to this executable";
    exit(1);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QHash<QString, QString> options;
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
                isOption = true;
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
//    QDir::setCurrent();
    CCPlus::setAssetsPath("/Users/apple/Documents/workspace/ccplus/assets");
    CCPlus::RenderTarget t;
    t.inputPath = options["i"].toUtf8().data();
    t.outputPath = options["o"].toUtf8().data();
    t.fps = options.contains("fps") ? options["fps"].toInt() : 15;
    if(t.fps < 1)
        t.fps = 1;
    CCPlus::go(t);
    t.waitFinish();
}
