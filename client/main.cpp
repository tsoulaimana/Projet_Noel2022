#include "clientcrawler.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientCrawler w;
    w.show();

    return a.exec();
}
