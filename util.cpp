
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "util.h"


//---------------------------------------------------------------------------//
// 概要   : 
// 関数名 : 
// 引数   : 
// 返り値 : 
//---------------------------------------------------------------------------//
void PrintAPIErr( APIErr *perr)
{

#ifdef  AE_OS_WIN
    wchar_t    str[1024];
    swprintf( str, 1024, L"!! AEGP Err ( code : %d   file : %hs   line : %d )", perr->Err, perr->FileName.c_str(), perr->Line );
    OutputDebugString( str );
#else
	char str[1024];
	snprintf( str, 1024, "!! AEGP Err ( code : %d   file : %s   line : %d )", perr->Err, perr->FileName.c_str(), perr->Line );
    printf( "%s", str );
#endif

}


//---------------------------------------------------------------------------//
// DEBUG system


#ifdef  _DEBUG

void DebugPrint(char *format, ...)
{
	char str[1024];
	va_list args;

	va_start(args, format);

	vsnprintf( str, 1024, format, args );

	va_end(args);

	OutputDebugStringA(str);
    
}

#endif  /* _DEBUG */




//---------------------------------------------------------------------------//
// スピード計測

#if  _PROFILE
#ifdef  AE_OS_WIN


#define MAX_PROFILE_INDEX	(16)
ProfileData	Profiles[MAX_PROFILE_INDEX];

static LARGE_INTEGER StartCounter = { 0LL };

void BeginProfile()
{
    QueryPerformanceCounter( &StartCounter );
}

void EndProfile()
{
    LARGE_INTEGER now, freq;
    double time;
	double overhead;

    if( QueryPerformanceCounter( &now ) &&
        QueryPerformanceFrequency( &freq ) )
    {
        time = (double)(now.QuadPart - StartCounter.QuadPart) / (double)freq.QuadPart;

		wchar_t	str[1024];
		swprintf( str, 1024, L"total : %f ms\n", time * 1000.0 );
        OutputDebugString(str);

		for(int i=0;i<MAX_PROFILE_INDEX; i++)
		{
			ProfileData *profile = &Profiles[i];

			if( profile->isCounting )
			{
				time = (double)(profile->sum.QuadPart) / (double)freq.QuadPart;
			
				// オーバーヘッド計測
				profile->isCounting = false;

				int lapcount = profile->lapCount;

				for(int t=0;t<lapcount;t++)
				{
					BeginProfileLap(i);
					EndProfileLap(i);
				}

				overhead = (double)(profile->sum.QuadPart) / (double)freq.QuadPart;

				swprintf( str, 1024, L"%02d : %f ms  overhead= %f ms  %d\n", i, time * 1000.0, overhead * 1000.0, profile->lapCount );
				OutputDebugString(str);

			}

			profile->isCounting = false;
		}
    }
    else
    {
        OutputDebugString(L"time : err\n");
    }
}



void BeginProfileLap(int index)
{
	ProfileData *profile = &Profiles[index];

	if( profile->isCounting == false )
	{
		profile->isCounting = true;
		profile->sum.QuadPart = 0LL;
		profile->lapCount = 0;
	}
		
	QueryPerformanceCounter( &profile->lapStart );	// 開始時間
}

void EndProfileLap(int index)
{
	ProfileData *profile = &Profiles[index];

	if( profile->isCounting == true )
	{
		LARGE_INTEGER now;
	
		QueryPerformanceCounter( &now );

		profile->sum.QuadPart += now.QuadPart - profile->lapStart.QuadPart;
		profile->lapCount++;
	}
}


#else

#endif  /* AE_OS_WIN */
#endif  /* _DEBUG */








//---------------------------------------------------------------------------//
// 画像処理関連




//---------------------------------------------------------------------------------------//
//概要:     ガンマテーブルの作成
//引数:     table : テーブルへのポインタ
//---------------------------------------------------------------------------------------//
void CreateGanmmaTable(u_char table[256], float Ganmma)
{
    int t;

    for(t=0;t<256;t++)
    {
        table[t] = (u_char)(255.0f * pow(((float)t / 255.0f), 1.0f / Ganmma));
    }

}



//---------------------------------------------------------------------------------------//
//名前:     SetDebugPixel()
//引数:     output : 出力画像
//			x : x座標
//			y : y座標
//返り値:   なし
//概要:     デバック用に指定した座標に色を置く
//---------------------------------------------------------------------------------------//
template<typename PixelType>
static inline void getDebugPixel(PixelType *p)	{ p->red=255; p->green=0; p->blue=0; p->alpha=255; }

template<> 
inline void getDebugPixel<PF_Pixel16>(PF_Pixel16 *p)	{ p->red=0x8000; p->green=0; p->blue=0; p->alpha=0x8000; }

template<typename PixelType> void SetDebugPixel(PixelType *out_ptr, PF_LayerDef *output, int x, int y)
{
	PixelType	debug_pixel;
	getDebugPixel( &debug_pixel );

	out_ptr[y*GET_WIDTH(output) + x] = debug_pixel;
}

template<typename PixelType> void SetDebugPixel(PixelType *out_ptr, PF_LayerDef *output, long target )
{
	PixelType	debug_pixel;
	getDebugPixel( &debug_pixel );

	out_ptr[target] = debug_pixel;
}

// 明示的インスタンス化
template void SetDebugPixel<PF_Pixel16>(PF_Pixel16 *out_ptr, PF_LayerDef *output, int x, int y);
template void SetDebugPixel<PF_Pixel8>(PF_Pixel8 *out_ptr, PF_LayerDef *output, int x, int y);

template void SetDebugPixel<PF_Pixel16>(PF_Pixel16 *out_ptr, PF_LayerDef *output, long target );
template void SetDebugPixel<PF_Pixel8>(PF_Pixel8 *out_ptr, PF_LayerDef *output, long target);




//---------------------------------------------------------------------------//
// 概要   : 
// 関数名 : 
// 引数   : 
// 返り値 : 
//---------------------------------------------------------------------------//
template<typename PixelType>
void BlendLine(     BlendingInfo<PixelType>    *pinfo,                 // 
                    double          length,                 // このパターンの長さ
                    long            blend_target,           // ブレンド元のターゲット(input)
                    long            out_target,             // ブレンド先のターゲット(output)
                    int             ref_offset,             // ブレンド参照先のターゲット(input)
                    int             next_pixel_step_in,     // 次のピクセルへ移動するときこの値を足す(input)
                    int             next_pixel_step_out,    // 次のピクセルへ移動するときこの値を足す(output)
                    bool            ratio_invert,
                    bool            no_line_weight)
{
    double  len;
    int     blend_count;            // ブレンドするピクセルの数 切り上げ
    double  pre_ratio       = 0.0;
    
    if( no_line_weight )
        len = (length * 0.5 ); // 全体の底辺
    else
        len = (length * (double)pinfo->LineWeight ); // 全体の底辺

    blend_count = CEIL(len);


    // 境界の逆方向からブレンドを行うので、その分移動 //
    blend_target    += (blend_count-1) * next_pixel_step_in;
    out_target      += (blend_count-1) * next_pixel_step_out;
    

    // 全体の～ : ブレンド全体の三角形、 なし :そのピクセルの三角形
    // 底辺×高さ÷２ = 底辺×((底辺/全体の底辺)×全体の高さ)÷２ 
    //                          ↑相似な三角形なので
    // = l(底辺) * l(底辺) * 0.5(全体の高さ) * 0.5(÷2) / len(全体の底辺)
    int t;
    for(t=0;t<blend_count;t++)
    {
        double  l           = len - (float)(((int)CEIL(len)-1) - t);    // このピクセルでの底辺 CEIL(len)-1 は (1.000...1 ～ 2.0) -> 1.0としたいため
        double  ratio       = (l * l * 0.5 * 0.5) / len;
        double  r;

        r = ratio_invert ? 1.0 - (ratio - pre_ratio) : (ratio - pre_ratio);

        // ブレンド
        Blendingf(pinfo->in_ptr, pinfo->out_ptr, blend_target, blend_target+ref_offset, out_target, (float)r);

        pre_ratio = ratio;

        // 次のピクセルへ
        blend_target    -= next_pixel_step_in;
        out_target      -= next_pixel_step_out;
    }
}




// 明示的インスタンス化宣言
template void BlendLine<PF_Pixel8>(BlendingInfo<PF_Pixel8>    *pinfo, 
								   double          length,              
								   long            blend_target,        
								   long            out_target,          
								   int             ref_offset,          
								   int             next_pixel_step_in,  
								   int             next_pixel_step_out, 
								   bool            ratio_invert,
								   bool            no_line_weight);

template void BlendLine<PF_Pixel16>(BlendingInfo<PF_Pixel16>    *pinfo, 
								    double          length,              
								    long            blend_target,        
								    long            out_target,          
								    int             ref_offset,          
								    int             next_pixel_step_in,  
								    int             next_pixel_step_out, 
								    bool            ratio_invert,
								    bool            no_line_weight);
