#include "pyrosqt.h"
#include "thumbnailer.h"

#include <QApplication>
#include <QPalette>
#include <QFile>
#include <QMimeDatabase>

int create_thumbnail(QVector<QByteArray> args);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::setlocale(LC_NUMERIC, "C");

    QCoreApplication::setOrganizationName("PyrosQT");
    QCoreApplication::setApplicationName("Pyros");
    for (int i = 1; i < argc;i++){
        if (!strcmp(argv[i],"--help") ||!strcmp(argv[i],"-h")){
            // :^)
            qInfo("Usage:\t%s [OPTION]\n"
                  "\nOPTIONS:\n"
                  " --thumbnail <INTPUT_FILE> <OUTPUT_PATH>\tGenerate a thumbnail of file to output location\n"
                  " --search <TAG>...\t\t\t\tOpen with intal search set to specified tags\n"
                  " --help \t\t\t\t\tShow this message\n",
                  argv[0]);
            return 0;
        } else if (!strcmp(argv[i],"--thumbnail")){
            QVector<QByteArray> arguments;
            for (int j = i+1; j < argc;j++)
                arguments.append(argv[j]);

            return create_thumbnail(arguments);
        }
    }

    PyrosQT w;
    w.show();
    return a.exec();
}

int create_thumbnail(QVector<QByteArray> args)
{
    if (args.count() >= 2){
        QMimeDatabase db;
        QString mime = db.mimeTypeForFile(args.at(0)).name();

        Thumbnailer::load_external_thumbnailers();
        Thumbnailer::thumbnail_item thumb_item(args.at(0),args.at(1),mime.toUtf8());

        qDebug() << "FILE:" << args.at(0) << " WITH MIME:" << mime;
        QFile::remove(args.at(1));
        Thumbnailer::generate_thumbnail(thumb_item);
    } else {
        qCritical("--thumbnail requires an input and output file");
        return 1;
    }

    return 0;
}
