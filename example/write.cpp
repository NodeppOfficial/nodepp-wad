#include <nodepp/nodepp.h>
#include <wad/wad.h>

using namespace nodepp;

void onMain() {

    auto file = wad_t( "./FILE.wad", "w" );

    file.append_file( "FILEA", "./main.cpp" );
    file.append_file( "FILEB", "./main.cpp" );
    file.append_file( "FILEC", "./main.cpp" );

    file.format_wad().then([]( int count ){
        console::log( count, "<> files" );
    }).fail([=]( except_t err ){
        console::error( err.what() );
    });

}
