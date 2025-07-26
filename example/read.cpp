#include <nodepp/nodepp.h>
#include <wad/wad.h>

using namespace nodepp;

void onMain() {

    auto file = wad_t( "./FILE.wad", "r" );

    for( auto x: file.get_file_list() ){ try {

        auto cin = file.get_file(x).await();
        cin.value().onData([=]( string_t data ){
            console::log( x, "<>", data.size(), data );
        }); stream::await( cin.value() );

    } catch( except_t err ) {
        console::error( err.what() );
    }}

}
