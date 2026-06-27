#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Load stylesheet from resources
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString style = stream.readAll();
        app.setStyleSheet(style);
        styleFile.close();
        qDebug() << "✅ Stylesheet loaded successfully";
    } else {
        qDebug() << "❌ Failed to load stylesheet";
    }
    
    MainWindow window;
    window.show();
    return app.exec();
}