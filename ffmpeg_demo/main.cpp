#include <QApplication>
#include "Widget.h"
#include "demo.h"

int main(int argc, char* argv[]) 
{
    //encode_video();
    //av_cut();
    //exercise_encode_video();
    //encode_video_mux();


    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}
