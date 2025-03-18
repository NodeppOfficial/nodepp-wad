# NODEPP-WAD
a simple WAD reader written in Nodepp

## Example
### READER
```cpp

#include <nodepp/nodepp.h>
#include <wad/wad.h>

using namespace nodepp;

void onMain() {

    auto raw = wad::read( "./FILE.wad" );

    for( auto x: raw.get_file_list() ){
         console::log( "<>", x );
    }

    auto cin = raw.get_file( "FILENAME" );

    cin.onData([=]( string_t data ){
        console::log( "<>", data.size() );
    });

    stream::pipe( cin );

}

```

### WRITTER
```cpp

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

```
