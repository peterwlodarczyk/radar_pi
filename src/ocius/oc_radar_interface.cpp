#include "pi_common.h"
#include <string>

void OciusDumpImage(int radar)
{
    FILE* f = fopen("/tmp/t.txt", "w");
    static int count = 0;
    ++count;
    fprintf(f, "+++++%d", count);
    // GLint viewport[4];
    // glGetIntegerv( GL_VIEWPORT, viewport );

    // int x = viewport[2];
    // int y = viewport[3];
    // unsigned char *buffer = (unsigned char *)malloc( x * y * 4 );
    // glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, buffer );

    // unsigned char *e = (unsigned char *)malloc( x * y * 3 );

    // fprintf(f, "e=%p.", e);
    // if(buffer && e){
    //     for( int p = 0; p < x*y; p++ ) {
    //         e[3*p+0] = buffer[4*p+0];
    //         e[3*p+1] = buffer[4*p+1];
    //         e[3*p+2] = buffer[4*p+2];
    //     }
    // }
    // free(buffer);

    // wxImage image0(x,y);
    // image0.SetData(e);
    // wxImage image1 = image0.Mirror(true);
    // wxImage image2 = image0.Mirror(false);

    // fprintf(f, "%d,%d,%d,%d", viewport[0], viewport[1],viewport[2], viewport[3]);
    // image2.SetOption("quality", 50);

    // std::string filename = std::string("/tmp/radar") + std::to_string(radar) + ".jpg";
    // image2.SaveFile(filename.c_str(), wxBITMAP_TYPE_JPEG );

    fclose(f);
}
