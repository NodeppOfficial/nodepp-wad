# NODEPP-WAD
a simple WAD reader written in Nodepp

## Example
### Read File

```cpp
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
```

### Write File
```cpp
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
```
