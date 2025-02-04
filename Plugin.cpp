/*****************************************************************************
 * Plugin.cpp
 *****************************************************************************
 * Copyright (C) 2015
 *
 * Authors: Yusuke Nakamura <muken.the.vfrmaniac@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *****************************************************************************/

#include "config.h"
#include "/usr/local/include/vapoursynth/VapourSynth.h"
#include "/usr/local/include/vapoursynth/VSHelper.h"
#include "TNLMeans.h"

static inline void set_option_int
(
    int         *opt,
    int          default_value,
    const char  *arg,
    const VSMap *in,
    const VSAPI *vsapi
)
{
    int e;
    *opt = int64ToIntS( vsapi->propGetInt( in, arg, 0, &e ) );
    if( e )
        *opt = default_value;
}

static inline void set_option_double
(
    double      *opt,
    double       default_value,
    const char  *arg,
    const VSMap *in,
    const VSAPI *vsapi
)
{
    int e;
    *opt = vsapi->propGetFloat( in, arg, 0, &e );
    if( e )
        *opt = default_value;
}

static void VS_CC initTNLMeans
(
    VSMap       *in,
    VSMap       *out,
    void       **instance_data,
    VSNode      *node,
    VSCore      *core,
    const VSAPI *vsapi
)
{
    TNLMeans *d = static_cast<TNLMeans *>(*instance_data);
    vsapi->setVideoInfo( &d->vi, 1, node );
}

static const VSFrameRef * VS_CC getFrameTNLMeans
(
    int             n,
    int             activation_reason,
    void          **instance_data,
    void          **frame_data,
    VSFrameContext *frame_ctx,
    VSCore         *core,
    const VSAPI    *vsapi
)
{
    TNLMeans *d = static_cast<TNLMeans *>(*instance_data);

    try
    {
        if( activation_reason == arInitial )
            d->RequestFrame( n, frame_ctx, core, vsapi );
        else if( activation_reason == arAllFramesReady )
            return d->GetFrame( n, frame_ctx, core, vsapi );
    }
    catch( std::bad_alloc &e )
    {
        std::string errMessage = "TNLMeans:  ";
        errMessage += e.what();
        vsapi->setFilterError( errMessage.c_str(), frame_ctx );
    }

    return nullptr;
}

static void VS_CC closeTNLMeans
(
    void        *instance_data,
    VSCore      *core,
    const VSAPI *vsapi
)
{
    TNLMeans *d = static_cast<TNLMeans *>(instance_data);
    vsapi->freeNode( d->node );
    delete d;
}

static void VS_CC createTNLMeans
(
    const VSMap *in,
    VSMap       *out,
    void        *user_data,
    VSCore      *core,
    const VSAPI *vsapi
)
{
    int     ax;
    int     ay;
    int     az;
    int     sx;
    int     sy;
    int     bx;
    int     by;
    double  a;
    double  h;
    int     ssd;
    set_option_int   ( &ax,    4, "ax",  in, vsapi );
    set_option_int   ( &ay,    4, "ay",  in, vsapi );
    set_option_int   ( &az,    0, "az",  in, vsapi );
    set_option_int   ( &sx,    2, "sx",  in, vsapi );
    set_option_int   ( &sy,    2, "sy",  in, vsapi );
    set_option_int   ( &bx,    1, "bx",  in, vsapi );
    set_option_int   ( &by,    1, "by",  in, vsapi );
    set_option_double( &a,   1.0, "a",   in, vsapi );
    set_option_double( &h,   0.5, "h",   in, vsapi );
    set_option_int   ( &ssd,   1, "ssd", in, vsapi );

    try
    {
        TNLMeans *d = new TNLMeans( ax, ay, az, sx, sy, bx, by, a, h, ssd, in, out, core, vsapi );
        if( d == nullptr )
            throw std::bad_alloc();

        vsapi->createFilter
        (
            in, out,
            "TNLMeans",
            initTNLMeans,
            getFrameTNLMeans,
            closeTNLMeans,
            fmParallel, 0, d, core
        );
    }
    catch( std::bad_alloc & )
    {
        vsapi->setError( out, "TNLMeans:  create failure (TNLMeans)!" );
        return;
    }
    catch( TNLMeans::bad_param &e )
    {
        std::string errMessage = "TNLMeans:  ";
        errMessage += e.what();
        errMessage += "!";
        vsapi->setError( out, errMessage.c_str() );
        return;
    }
    catch( TNLMeans::bad_alloc &e )
    {
        std::string errMessage = "TNLMeans:  allocation failure (";
        errMessage += e.what();
        errMessage += ")!";
        vsapi->setError( out, errMessage.c_str() );
        return;
    }
    catch( ... )
    {
        return;
    }
}

VS_EXTERNAL_API( void ) VapourSynthPluginInit
(
    VSConfigPlugin     config_func,
    VSRegisterFunction register_func,
    VSPlugin *         plugin
)
{
    config_func
    (
        "systems.innocent.tnlm", "tnlm",
        "TNLMeans rev" VSTNLMEANS_REV "-" VSTNLMEANS_GIT_HASH,
        VAPOURSYNTH_API_VERSION, 1, plugin
    );
    register_func
    (
        "TNLMeans",
        "clip:clip;ax:int:opt;ay:int:opt;az:int:opt;sx:int:opt;sy:int:opt;bx:int:opt;by:int:opt;a:float:opt;h:float:opt;ssd:int:opt;",
        createTNLMeans, nullptr, plugin
    );
}
