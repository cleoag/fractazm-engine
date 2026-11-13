// Generated only from successful checked body lowerings.
#pragma once
namespace mb3d_native {
inline bool native_external_returns_coordinates(uint id) {
switch(id) {
case 9525u:
case 10001u:
return true;
default: return false;
}}
}
namespace mb3d_native {
inline bool native_external_reads_caller_rout(uint id,uint count,uint traits,int option,uint flags) {
switch(id) {
case 7071u: return count==1u && traits==0u && option==0 && flags==1u;
default: return false;
}}
}
