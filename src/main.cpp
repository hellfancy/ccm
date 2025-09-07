#include "bex/bex.hpp"
#include "ccmth.h"


static const char *ccm_help_txt= R"EOH(
用法: ccm算法 
例: 
稍后补充
)EOH";


static bexfun_info_t flist[] = {
    {"ccm", cmd_ccm, ccm_help_txt},
    {"", nullptr, nullptr},
};

bexfun_info_t * bxPluginFunctions(){
    return flist;
}


