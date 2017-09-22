
#ifndef __UTIL_H
#define __UTIL_H


#include <string>
#include <math.h>

#include "AEConfig.h"

#include "AE_Effect.h"
#include "A.h"
//#include "AE_EffectUI.h"
//#include "SPSuites.h"
//#include "AE_AdvEffectSuites.h"
//#include "AE_EffectCBSuites.h"
#include "AE_Macros.h"
//#include "AE_GeneralPlug.h"

#include "version.h"

#include "define.h"

#ifdef  AE_OS_WIN
#include <windows.h>
#endif

//---------------------------------------------------------------------------//
// エラー処理
#define ACALL( fct)                                             \
{                                                               \
    if ( err == A_Err_NONE )                                    \
        if (  A_Err_NONE != ( err = (fct) ))                    \
            throw   APIErr( __FILE__, __LINE__, err );          \
}


struct APIErr
{
    A_Err           Err;
    std::string     FileName;
    int             Line;

    APIErr( char *filename, int line, A_Err err )
    {
        Err         = err;
        FileName    = filename;
        Line        = line;
    };
};



void PrintAPIErr( APIErr *perr);




//---------------------------------------------------------------------------//
// Debugファイル
#ifdef  _DEBUG

#include <stdarg.h>

void DebugPrint(char *format, ...);

#define DEBUG_STR( str )        OutputDebugStringA( (str) );

#else

#define DEBUG_STR( str )        ((void)0)


#endif  /* _DEBUG */


#ifdef AE_OS_WIN

#else

// Mac
#include <stdarg.h>
void DebugPrint(char *format, ...);

#endif


//---------------------------------------------------------------------------//
// スピード測定
#define	_PROFILE	(0)

#if _PROFILE 

struct ProfileData
{
	bool			isCounting;
	LARGE_INTEGER	lapStart;
	LARGE_INTEGER	sum;
	int				lapCount;

	ProfileData()
	{
		isCounting	= false;
		lapStart.QuadPart	= 0LL;
		sum.QuadPart		= 0LL;
		lapCount			= 0;
	}
};
void BeginProfile();
void EndProfile();
void BeginProfileLap(int index);
void EndProfileLap(int index);

#define BEGIN_PROFILE()   BeginProfile()
#define END_PROFILE()     EndProfile()
#define BEGIN_LAP(x)   BeginProfileLap((x))
#define END_LAP(x)     EndProfileLap((x))

#else

#define BEGIN_PROFILE()   ((void)0)
#define END_PROFILE()     ((void)0)
#define BEGIN_LAP(x)		((void)0)
#define END_LAP(x)			((void)0)

#endif  /* _DEBUG */




//---------------------------------------------------------------------------//
// 画像処理関連
//---------------------------------------------------------------------------//




//#define   GET_WIDTH(layer)        ((layer)->width + 4-(  (((layer)->width-1)%4)  +1)) 
//#define   GET_WIDTH(layer)        ((layer)->width%4==0 ? (layer)->width : layer->width + 4-(layer->width%4))
//#define   GET_WIDTH(layer)        ((layer)->width)
#define GET_WIDTH(image)        ((image)->rowbytes/sizeof(PixelType)) // rowbytes = 横のバイト数 1ピクセル4バイトなので /4
#define GET_HEIGHT(image)       ((image)->height)




typedef unsigned long		KP_PIXEL32;	// 8bbit per channel
typedef unsigned long long	KP_PIXEL64;	// 16bit per channel

template<typename PixelType> static inline unsigned int getMaxValue() { return ~0; }
template <> inline unsigned int getMaxValue<PF_Pixel16>(){ return 0x8000; }
template <> inline unsigned int getMaxValue<PF_Pixel8>()	{ return 0xff; }






//----------- 算術系 ------------//
#define CEIL(a)         (int)ceil(a)    // floatの切り上げ //

#ifndef MAX
    #define MAX(a, b)   ((a) > (b) ? (a) : (b)) // AE側ヘッダーで定義されてる
#endif

#ifndef MIN
    #define MIN(a, b)   ((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
    #define ABS(a)      ((a) < 0 ? -(a) : (a))
#endif



#define GET_SIGN(a)     ((a) / ABS((a)))            // 符号を得る マイナスだったら -1、プラスだったら+1




//---------------------------------------------------------------------------//
//// 関数 ///////
//---------------------------------------------------------------------------//


//-------------------------------------//
//     Pixel比較関数
//-------------------------------------//

#define FAST_COMPARE_PIXEL(p1, p2)      (*(PackedPixelType*)&in_ptr[p1] != *(PackedPixelType*)&in_ptr[p2])


#ifdef AE_OS_WIN
#define ComparePixel(p0, p1)            RangeComparePixelNotEqual( &(info->in_ptr[p0]), &(info->in_ptr[p1]), info->range)
#define ComparePixelEqual(p0, p1)       RangeComparePixelEqual( &(info->in_ptr[p0]), &(info->in_ptr[p1]), info->range)



//---------------------------------------------------------------------------//
// Pixel比較関数
//---------------------------------------------------------------------------//
template <typename PixelType>
static inline bool RangeComparePixelNotEqual( const PixelType *p0, const PixelType *p1, const unsigned int range )
{
    unsigned int delta;
    delta = ABS(p0->red   - p1->red   ) +
            ABS(p0->green - p1->green ) +
            ABS(p0->blue  - p1->blue  ) + 
            ABS(p0->alpha - p1->alpha );
    
    return (delta > range);
}


template <typename PixelType>
static inline bool RangeComparePixelEqual( const PixelType *p0, const PixelType *p1, const unsigned int range )
{
    unsigned int delta;
    delta = ABS(p0->red   - p1->red   ) +
            ABS(p0->green - p1->green ) +
            ABS(p0->blue  - p1->blue  ) + 
            ABS(p0->alpha - p1->alpha );
    
    return (delta <= range);
}
#else
// for Mac
// うまくinline展開してくれないので
#define ComparePixel(p0, p1) ((unsigned int)( ABS(info->in_ptr[p0].red   - info->in_ptr[p1].red) +    \
								ABS(info->in_ptr[p0].green - info->in_ptr[p1].green) +  \
								ABS(info->in_ptr[p0].blue  - info->in_ptr[p1].blue) +   \
								ABS(info->in_ptr[p0].alpha - info->in_ptr[p1].alpha)) > info->range )

#define ComparePixelEqual(p0, p1) ((unsigned int)(ABS(info->in_ptr[p0].red   - info->in_ptr[p1].red) +   \
									ABS(info->in_ptr[p0].green - info->in_ptr[p1].green) + \
									ABS(info->in_ptr[p0].blue  - info->in_ptr[p1].blue) +  \
									ABS(info->in_ptr[p0].alpha - info->in_ptr[p1].alpha)) <= info->range )
#endif

// グラデーションを検知
// 3つのピクセルの平均値とCenterのピクセルを比較して、range以下だったらtrue。
// ref1,とcenterが別の色なのが前提
template <typename PixelType>
static inline bool DetectGradation( const PixelType *center, 
									const PixelType *ref1,
									const PixelType *ref2,
									const unsigned int range )
{
	// 平均ピクセルを作る
	PixelType	ave;
	ave.red		= (center->red + ref1->red + ref2->red)/3;
	ave.blue	= (center->blue + ref1->blue + ref2->blue)/3;
	ave.green	= (center->green + ref1->green + ref2->green)/3;
	ave.alpha	= (center->alpha + ref1->alpha + ref2->alpha)/3;

	// centerと平均が同じで、そのほか２つとは別の色
    return RangeComparePixelEqual( center, &ave, 1 * 255 * 4 );
}


//---------------------------------------------------------------------------------------//
//概要:     ピクセル同士をブレンドする
//引数:     input,output:   おなじみ入出力画像へのポインタ
//          blend_target:   ブレンドするターゲット。結果はここのピクセルに入る
//          ref_target:     ブレンドするターゲット２。ここと上のやつのブレンドになる
//          ratio:          ブレンドする割合。1.0fでblend_targetのまんまになる 0.0fでref
//---------------------------------------------------------------------------------------//
template <typename PixelType>
static inline void BlendingPixelf(  PixelType *target_pixel,
                                    PixelType *ref_pixel,
                                    PixelType *output_pixel,
                                    float   ratio )
{
    unsigned int  max_value = getMaxValue<PixelType>();
    unsigned int alpha = (unsigned int)(max_value * ratio), r_alpha;

    r_alpha = max_value - alpha;

    if(target_pixel->alpha == max_value && ref_pixel->alpha == max_value )
    {
		// どちらも不透明
        output_pixel->alpha     = max_value;

        output_pixel->red       = (((target_pixel->red * alpha)+
                                    (ref_pixel->red * r_alpha))/max_value);
        output_pixel->green     = (((target_pixel->green * alpha)+
                                    (ref_pixel->green * r_alpha))/max_value);
        output_pixel->blue      = (((target_pixel->blue * alpha)+
                                    (ref_pixel->blue * r_alpha))/max_value);
    }
    else if(target_pixel->alpha == 0 )
    {
		// ターゲットが抜き
        output_pixel->alpha = (((target_pixel->alpha * alpha)+
                                        (ref_pixel->alpha * r_alpha))/max_value);

        output_pixel->red       = ref_pixel->red;
        output_pixel->green     = ref_pixel->green;
        output_pixel->blue      = ref_pixel->blue;
        
    }
    else if(ref_pixel->alpha == 0 )
    {
		// refが抜き
        output_pixel->alpha = (((target_pixel->alpha * alpha)+
                                        (ref_pixel->alpha * r_alpha))/max_value);

        output_pixel->red       = target_pixel->red;
        output_pixel->green     = target_pixel->green;
        output_pixel->blue      = target_pixel->blue;
    }
	else
	{
		// 半透明
        output_pixel->alpha = (((target_pixel->alpha * alpha)+
                                        (ref_pixel->alpha * r_alpha))/max_value);
        output_pixel->red       = (((target_pixel->red * alpha)+
                                    (ref_pixel->red * r_alpha))/max_value);
        output_pixel->green     = (((target_pixel->green * alpha)+
                                    (ref_pixel->green * r_alpha))/max_value);
        output_pixel->blue      = (((target_pixel->blue * alpha)+
                                    (ref_pixel->blue * r_alpha))/max_value);
	}
}




// float版のブレンディング命令 //
template <typename PixelType>
static inline void Blendingf(   PixelType	*in_ptr,
                                PixelType	*out_ptr,
                                long        blend_target,
                                long        ref_target,
                                long        output_target,          
                                float       ratio )
{
    BlendingPixelf<PixelType>(	&(in_ptr[blend_target]),
								&(in_ptr[ref_target]),
								&(out_ptr[output_target]),
								ratio );
}


// ガンマテーブル作成 //
void CreateGanmmaTable(u_char table[256], float Ganmma);


///// デバック色の種類 ///////
template<typename PixelType> void SetDebugPixel(PixelType *out_ptr, PF_LayerDef *output, int x, int y);
template<typename PixelType> void SetDebugPixel(PixelType *out_ptr, PF_LayerDef *output, long target );


#ifdef _DEBUG
#define DEBUG_PIXEL(out_ptr, output, x, y )    SetDebugPixel( (out_ptr), (output), (x), (y) )
#define DEBUG_TARGET_PIXEL(out_ptr, output, t )    SetDebugPixel( (out_ptr), (output), (t) )
#else
#define DEBUG_PIXEL(out_ptr, output, x, y )		((void)0)
#define DEBUG_TARGET_PIXEL(out_ptr, output, t ) ((void)0)
#endif



template<typename PixelType>
void BlendLine(     BlendingInfo<PixelType>    *pinfo,                 // 
                    double          length,                 // このパターンの長さ
                    long            blend_target,           // ブレンド元のターゲット(input)
                    long            out_target,             // ブレンド先のターゲット(output)
                    int             ref_offset,             // ブレンド参照先のターゲット(input)
                    int             next_pixel_step_in,     // 次のピクセルへ移動するときこの値を足す(input)
                    int             next_pixel_step_out,    // 次のピクセルへ移動するときこの値を足す(output)
                    bool            ratio_invert,
                    bool            no_line_weight);




//-------------------------------------------------------------------------------------------
// util
//-------------------------------------------------------------------------------------------
#define SAFE_DELETE(x)	if((x)!=NULL)	{ delete (x); (x) = NULL; }



#endif

