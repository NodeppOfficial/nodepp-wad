#ifndef NODEPP_WAD
#define NODEPP_WAD

#include <nodepp/path.h>
#include <nodepp/map.h>
#include <nodepp/fs.h>

namespace nodepp { class wad_t {
protected:

    struct HEADER    { char magic[4]; uint count=0, offset=0; };
    struct DIRECTORY { uint size=0, offset=0; char name[8];   };

    struct NODE {
        map_t<string_t,string_t> file_list; bool mode =0;
        queue_t<DIRECTORY> dir;             bool state=0;
        string_t path; file_t fd; HEADER hdr;
    };  ptr_t<NODE> obj;

public: wad_t() : obj( new NODE() ) {}

   ~wad_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    wad_t( string_t path, bool mode ) : obj( new NODE() ) {

        obj->fd   = file_t( path, mode? "w" : "r" );
        obj->mode = mode; obj->state = 1;
        obj->path = path; 
    
    if( !mode ){

        do {
            obj->fd.pos(0); auto raw = obj->fd.read( sizeof(HEADER) );
            memcpy( &obj->hdr, raw.get(), sizeof(HEADER) );
            if( memcmp( obj->hdr.magic+1, "WAD", 3 )!=0 )
              { process::error("invalid WAD format"); }
                obj->fd.pos( obj->hdr.offset );
        } while(0);

        for( auto x=0; x<obj->hdr.count; x++ ){ try {
             auto dir = DIRECTORY();
             auto raw = obj->fd.read( sizeof(DIRECTORY) );
             memcpy( &dir, raw.get(), sizeof(DIRECTORY) );
             obj->dir.push( dir );
        } catch(...) { break; } }

    }}

    void set_file( string_t name, string_t path ) const {
        if( name.empty() || !obj->mode || !obj->state ){ goto ERROR; } while( name.size()<8 ){ name.push('\0'); }
        obj->file_list[name] = path; return; ERROR:; process::error( "something went wrong" );
    }

    file_t get_file( string_t name ) const {
        if( name.empty() || obj->mode || !obj->state ){ goto ERROR; } while( name.size()<8 ){ name.push('\0'); }
        do { file_t file ( obj->path, "r" ); auto n=obj->dir.first(); while( n!=nullptr ){
             if( memcmp( name.get(), (void*) n->data.name, 8 )==0 ){
                 file.set_range( n->data.offset, n->data.offset + n->data.size );
             return file; } n = n->next; }
        } while(0); ERROR:; process::error("such file or directory does not exists"); return file_t();
    }

    ptr_t<string_t> get_file_list() const noexcept {
        ptr_t<string_t> data ( obj->dir.size() ); uint x=0;
        auto n=obj->dir.first(); while( n!=nullptr && obj->state ){
             data[x] = string_t( n->data.name, 8 );
        n = n->next; x++; } return data;
    }

    void free() const { if( !obj->state || !obj->mode ){ return; }

        do { obj->fd.pos(0); 
             ptr_t<char> header( sizeof(HEADER)+1, 0x00 ); 
             memcpy( header.get(), &obj->hdr, sizeof(HEADER) );
             obj->fd.write( header );
        } while(0); uint offset = sizeof(HEADER);

        for( auto x: obj->file_list.keys() ){
             auto item=DIRECTORY(); memcpy( &item.name, x.get(), 8 );
                  item.size   = fs::file_size( obj->file_list[x] );
                  item.offset = offset; offset += item.size;
                  obj->dir.push( item );
             obj->fd.write( stream::await( fs::readable( obj->file_list[x] ) ));
        }

        auto n=obj->dir.first(); while( n!=nullptr ){
             ptr_t<char> directory ( sizeof(DIRECTORY)+1, 0x00 ); 
             memcpy( directory.get(), &n->data, sizeof(DIRECTORY) );
             obj->fd.write( directory );
        n=n->next; }

        do { obj->fd.pos(0); 
             obj->hdr.offset = offset; 
             obj->hdr.count  = obj->dir.size();
             memcpy( &obj->hdr.magic, "IWAD", 4 );
             ptr_t<char> header ( sizeof( HEADER )+1, 0x00 ); 
             memcpy( header.get(), &obj->hdr, sizeof(HEADER) );
             obj->fd.write( header );
        } while(0);

    }

};}

namespace nodepp { namespace wad {

    file_t read( string_t path, string_t name ) { wad_t wad( path, 0 ); return wad.get_file(name); }

    void write( string_t path, map_t<string_t,string_t> file_list ) {
        wad_t wad ( path, 1 ); for( auto x: file_list.keys() )
            { wad.set_file( x, file_list[x] ); }
    }

    wad_t read( string_t path ) { return wad_t( path, 0 ); }

}}

#endif
