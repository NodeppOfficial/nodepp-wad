/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WAD
#define NODEPP_WAD

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/encoder.h>
#include <nodepp/promise.h>
#include <nodepp/tuple.h>
#include <nodepp/path.h>
#include <nodepp/map.h>
#include <nodepp/fs.h>
#include <nodepp/os.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class wad_t {
protected:

    struct HEADER    { char magic[4]; uint count=0, offset=0; };
    struct DIRECTORY { uint size=0, offset=0; char name[8];   };

    using T = type::pair<string_t,int>;
    using V = tuple_t<string_t,int>;

    struct NODE {
        queue_t<DIRECTORY> dir; bool state=0, used=0;
        map_t<string_t,function_t<V>> file_list;
        string_t path; file_t fd; HEADER hdr;
    };  ptr_t<NODE> obj;

    void _init_() const noexcept {
        memset(&obj->hdr,0,sizeof(HEADER));
        memcpy(&obj->hdr.magic,"IWAD", 4 );
        obj->hdr.offset = sizeof( HEADER );
    }

public:

    virtual ~wad_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    wad_t( string_t path, string_t mode ) : obj( new NODE() ) {
        obj->path = path; obj->fd = file_t( path, mode );
        obj->state= 1; _init_(); parse_wad();
    }

    wad_t() : obj( new NODE() ) { _init_(); }

    /*─······································································─*/

    void parse_wad() const {
    if( obj->fd.is_available() && obj->fd.size()!=0 ){

        do{ obj->fd.pos(0); auto raw = obj->fd.read( sizeof(HEADER) );
            memcpy( &obj->hdr, raw.get(), sizeof(HEADER) );
        if( memcmp( obj->hdr.magic+1, "WAD", 3 )!=0 )
          { throw except_t("invalid WAD format"); return; }
            obj->fd.pos( obj->hdr.offset );
          } while(0);

        for( auto x=obj->hdr.count; x-->0; ){ try {
             auto dir = DIRECTORY();
             auto raw = obj->fd.read( sizeof(DIRECTORY) );
             memcpy( &dir, raw.get(), sizeof(DIRECTORY) ); obj->dir.push(dir);
        } catch(...) { break; } }

    }}

    /*─······································································─*/

    promise_t<int,except_t> format_wad() const noexcept {

        auto write = generator::file::write();
        auto self  = type::bind( this );
        auto len   = type::bind( 0UL  );
        auto time  = type::bind( 0UL  );

    return promise_t<int,except_t>([=]( res_t<int> res, rej_t<except_t> rej ){
    try {

        if( self->obj->file_list.empty() ){ throw ""; } self->obj->fd.del_borrow();
        if( self->obj->state == 0 )       { throw ""; }

        process::add( coroutine::add( COROUTINE(){
            if( self->obj->state == 0 )                         { self->release(); return -1; }
            if(*time>0&&(process::now()-*time)>TIME_SECONDS(1) ){ self->release(); return -1; }
        coBegin

            coWait( self->is_used() ); self->use(); *time=process::now();

            if( self->obj->hdr.offset == sizeof( HEADER ) ) {
                self->obj->fd.pos(0); string_t dir(sizeof(HEADER),'\0');
                memcpy(dir.get(),(char*)&self->obj->hdr,sizeof(HEADER));
            self->obj->fd.write( dir ); } *len=0;

            self->obj->fd.pos( self->obj->hdr.offset );
            coYield(1); *time=process::now();

            do{ auto n = self->obj->file_list.data()[0].second();
            if( tuple::get<1>(n)== 0 ){ return 1; }
            if( tuple::get<1>(n)==-1 ){ break;    }
                self->obj->fd.write(tuple::get<0>(n));
               *len+=tuple::get<0>(n).size();return 1;
            } while(0);

            do{ auto n = self->obj->file_list.data()[0].first;
                auto item=DIRECTORY(); memcpy( &item.name, n.get(), 8 );
                     item.offset           = self->obj->hdr.offset;
                     item.size             =  *len;
                     self->obj->hdr.offset+=  *len;
                     self->obj->hdr.count ++; *len=0;
                     self->obj->file_list.erase( n );
                     self->obj->dir    .push( item );
            } while(0);

            if( !self->obj->file_list.empty() ){ coGoto(1); }
            coYield(2); *time=process::now();

            do{ auto n=self->obj->dir.first(); while( n!=nullptr ){
                string_t dir( sizeof(DIRECTORY), '\0' );
                memcpy(dir.get(),&n->data,sizeof(DIRECTORY) );
            self->obj->fd.write( dir ); n=n->next; }} while(0);

            do{ self->obj->fd.pos(0); string_t dir(sizeof(HEADER),'\0');
                memcpy(dir.get(),(char*)&self->obj->hdr,sizeof(HEADER));
            self->obj->fd.write( dir ); } while(0);

            res( self->obj->hdr.count ); self->release();

        coFinish
        }));

    } catch(...) { rej("something went wrong"); self->release(); } }); }

    /*─······································································─*/

    promise_t<string_t,except_t> get_data( string_t name ) const noexcept {

        auto self = type::bind( this );
        auto mane = type::bind( name );

    return promise_t<string_t,except_t>([=]( res_t<string_t> res, rej_t<except_t> rej ){
    try {

        if( mane->empty() || !self->obj->state ){ throw ""; }
     while( mane->size () <8 ){ mane->push('\0'); }

        do{ file_t file ( self->obj->path, "r" ); auto n=self->obj->dir.first();
     while( n!=nullptr ){
        if( memcmp( mane->get(), (void*) n->data.name, 8 )==0 ){
            file.set_range( n->data.offset, n->data.offset + n->data.size );
            res( stream::await(file) ); return;
          } n = n->next; }} while(0); throw "";

    } catch(...) { rej("such file or directory does not exists"); } }); }

    promise_t<file_t,except_t> get_file( string_t name ) const noexcept {

        auto self = type::bind( this );
        auto mane = type::bind( name );

    return promise_t<file_t,except_t>([=]( res_t<file_t> res, rej_t<except_t> rej ){
    try {

        if( mane->empty() || !self->obj->state ){ throw ""; }
     while( mane->size () <8 ){ mane->push('\0'); }

        do{ file_t file ( self->obj->path, "r" ); auto n=self->obj->dir.first();
     while( n!=nullptr ){
        if( memcmp( mane->get(), (void*) n->data.name, 8 )==0 ){
            file.set_range( n->data.offset, n->data.offset + n->data.size );
       res( file ); return; } n = n->next; }} while(0); throw "";

    } catch(...) { rej("such file or directory does not exists"); } }); }

    promise_t<wad_t,except_t> get_wad( string_t name ) const noexcept {

        auto self = type::bind( this );
        auto mane = type::bind( name );

    return promise_t<wad_t,except_t>([=]( res_t<wad_t> res, rej_t<except_t> rej ){
    try {

        auto nname = regex::join( "tmp_${0}_${1}_${2}.wad",
            encoder::key::generate(32), *mane,
            path::basename(self->obj->path,".wad")
        );

        auto dirnm = path::join( os::tmp(), nname );
        auto file  = fs::writable( dirnm );

        get_file( *mane ).then([=]( file_t raw ){
            file.onDrain.once([=](){ res(wad_t(dirnm,"r")); });
            stream::pipe( raw, file );
        }).fail([=]( except_t err ){ rej(err); });

    } catch(...) { rej("something went wrong"); } }); }

    /*─······································································─*/

    template< class T >
    void append_stream( string_t name, const T& str ) const {
        if( name.empty() || !obj->state )
          { throw except_t( "something went wrong" ); return; }
        if( has_file( name ) )
          { throw except_t("file already exists");    return; }
     while( name.size()<8 ){ name.push('\0'); }

        ptr_t<generator::file::read> _read_ = new generator::file::read();

        obj->file_list[name] = function_t<V>([=](){
            if( !str.is_available() ){ return V( nullptr     ,-1 ); }
            if((*_read_)( &str )==1 ){ return V( nullptr     , 0 ); }
            if(  _read_->state  <=0 ){ return V( nullptr     ,-1 ); }
                                       return V( _read_->data, 1 );
        });

    }

    void append_data( string_t name, string_t data ) const {
        if( name.empty() || !obj->state )
          { throw except_t( "something went wrong" ); return; }
        if( has_file( name ) )
          { throw except_t("file already exists");    return; }
     while( name.size()<8 ){ name.push('\0'); }

        ptr_t<bool> x = new bool(0);

        obj->file_list[name] = function_t<V>([=](){
            if(!*x ){ return V( nullptr,-1 ); }
                *x=0; return V( data   , 1 );
        });

    }

    void append_file( string_t name, string_t path ) const {
    return append_stream( name, fs::readable(path) );
    }

    /*─······································································─*/

    void get_file_list( function_t<void,string_t> cb ) const noexcept {
        auto n=obj->dir.first(); while( n!=nullptr && obj->state ){
        cb( string_t( n->data.name,8 ) ); n=n->next; }
    }

    ptr_t<string_t> get_file_list() const noexcept {
        ptr_t<string_t> data ( obj->dir.size() ); uint x=0;
        auto n=obj->dir.first(); while( n!=nullptr && obj->state ){
             data[x] = string_t( n->data.name, 8 );
        n=n->next; x++; } return data;
    }

    bool has_file( string_t name ) const noexcept {
        while( name.size()<8 ){ name.push('\0'); }
        auto n=obj->dir.first(); while( n!=nullptr && obj->state ){
        if( memcmp(n->data.name,name.get(),8)== 0 ){ return true; }
        n=n->next; } return false;
    }

    /*─······································································─*/

    file_t&    get_fd() const noexcept { return obj->fd;                }
    bool is_available() const noexcept { return obj->fd.is_available(); }
    bool is_closed()    const noexcept { return obj->fd.is_closed();    }

    /*─······································································─*/

    bool is_used()      const noexcept { return obj->used; }
    void close()        const noexcept { obj->fd.close(); }
    void use()          const noexcept { obj->used = 1; }
    void release()      const noexcept { obj->used = 0; }

    /*─······································································─*/

    void free() const { try {
         if( !obj->state || obj->file_list.empty() )
           { throw 0; } format_wad().await();
    } catch(...) {
         if(!regex::test( obj->path, os::tmp() ))
           { return; } fs::remove_file( obj->path );
    } obj->state=false; close(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace wad {

    void write( string_t path, map_t<string_t,string_t> file_list ) {
        wad_t wad ( path, "w" ); for( auto x: file_list.keys() )
            { wad.append_file( x, file_list[x] ); }
        wad.format_wad().await();
    }

    wad_t read( string_t path ) { return wad_t( path, "r" ); }

    file_t read( string_t path, string_t name ) {
        auto raw = wad_t( path, "r" ).get_file(name).await();
        if( !raw.has_value() ){ throw raw.error(); }
        return raw.value();
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
