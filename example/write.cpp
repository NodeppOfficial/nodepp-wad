#include <nodepp/nodepp.h>
#include <wad/wad.h>

using namespace nodepp;

void onMain() {

    wad::write( "./FILE.wad", map_t<string_t,string_t>({
        { "FILEA", "./main.cpp" },
        { "FILEB", "./main.cpp" },
        { "FILEC", "./main.cpp" },
    }) );

}