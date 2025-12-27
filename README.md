# nodepp-wad: WAD File Format Handler

The nodepp-wad component provides a comprehensive and asynchronous C++ interface for reading, writing, and manipulating files in the WAD (Where's All the Data?) archive format, historically used by classic video games like Doom.

This library is a pure C++ logic implementation and has no external system dependencies outside of the core nodepp components.

## Dependencies & Cmake Integration
```bash
include(FetchContent)

FetchContent_Declare(
	nodepp
	GIT_REPOSITORY   https://github.com/NodeppOfficial/nodepp
	GIT_TAG          origin/main
	GIT_PROGRESS     ON
)
FetchContent_MakeAvailable(nodepp)

FetchContent_Declare(
	nodepp-wad
	GIT_REPOSITORY   https://github.com/NodeppOfficial/nodepp-wad
	GIT_TAG          origin/main
	GIT_PROGRESS     ON
)
FetchContent_MakeAvailable(nodepp-wad)

#[...]

target_link_libraries( #[...]
	PUBLIC nodepp nodepp-wad #[...]
)
```

## Key Features
- **WAD Standard Compliant:** Handles both IWAD (Internal WAD) and PWAD (Patch WAD) formats.
- **Asynchronous I/O:** Leverages nodepp's promise_t and coroutine system (format_wad) for non-blocking file operations.
- **Modular Appending:** Allows appending content from raw data, file paths, or complex nodepp streams/generators.
- **Directory Management:** Parses the WAD directory structure for efficient file lookup by name.

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

## License
**Nodepp-wad** is distributed under the MIT License. See the LICENSE file for more details.