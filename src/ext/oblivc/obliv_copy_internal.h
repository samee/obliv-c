#pragma once

void ocShareMuxes(ProtocolDesc* pd,char* z,     
                  const char* x0,const char* x1,size_t n,size_t eltsize,
                  const bool* c,const __obliv_c__bool* wc,char* t);

void ocFromShared_impl(ProtocolDesc* pd, void* dest, const void* src,
                       size_t n,size_t bits,size_t bytes);