#include <QApplication>
#include "Widget.h"
#include "demo.h"

int main(int argc, char* argv[]) 
{
#if 0
    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
#else
    //encode_video();
    //av_cut();
    exercise_encode_video();
#endif
}
