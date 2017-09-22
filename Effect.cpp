
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"

#include "Param_Utils.h"
#include "version.h"
#include "util.h"


#include "define.h"

#include "upMode.h"
#include "downMode.h"
#include "8link.h"
#include "Lack.h"

#include "Effect.h"

//---------------------------------------------------------------------------//
// 定義
enum 
{
    PARAM_INPUT = 0,
    PARAM_WHITE_OPTION,
    PARAM_RANGE,
    PARAM_LINE_WEIGHT,
    PARAM_NUM,
};

//---------------------------------------------------------------------------//
// プロトタイプ
static PF_Err About (   PF_InData       *in_data,
                        PF_OutData      *out_data,
                        PF_ParamDef     *params[],
                        PF_LayerDef     *output );

static PF_Err GlobalSetup ( PF_InData       *in_data,
                            PF_OutData      *out_data,
                            PF_ParamDef     *params[],
                            PF_LayerDef     *output );

static PF_Err GlobalSetdown( PF_InData      *in_data,
                            PF_OutData      *out_data,
                            PF_ParamDef     *params[],
                            PF_LayerDef     *output );

static PF_Err ParamsSetup ( PF_InData       *in_data,
                            PF_OutData      *out_data,
                            PF_ParamDef     *params[],
                            PF_LayerDef     *output);

static PF_Err Render (  PF_InData       *in_data,
                        PF_OutData      *out_data,
                        PF_ParamDef     *params[],
                        PF_LayerDef     *output );

static PF_Err PopDialog (PF_InData		*in_data,
						 PF_OutData		*out_data,
						 PF_ParamDef		*params[],
						 PF_LayerDef		*output );


//---------------------------------------------------------------------------//
// util funcs
//---------------------------------------------------------------------------//
static inline void getWhitePixel(PF_Pixel16 *white)	
{ 
	PF_Pixel16 color = { 0x8000, 0x8000, 0x8000, 0x8000 };
	*white = color;
}

static inline void getWhitePixel(PF_Pixel8 *white )
{ 
	PF_Pixel8	color = { 0xFF, 0xFF, 0xFF, 0xFF };
	*white = color;
}

static inline void getNullPixel(PF_Pixel16 *null_pixel )
{ 
	PF_Pixel16	color = { 0x0, 0x0, 0x0, 0x0 };
	*null_pixel = color;
}

static inline void getNullPixel(PF_Pixel8 *null_pixel )
{ 
	PF_Pixel8	color = { 0x0, 0x0, 0x0, 0x0 };
	*null_pixel = color;
}






#if 0
template<typename PackedPixelType > 
static inline void ColorKey( PackedPixelType *in_ptr, int row_bytes, int height )
{
    int         limit, t=0;
    PackedPixelType	key;
	getWhitePixel( &key );	// 0xff or 0xffff

    limit = (row_bytes / sizeof(PackedPixelType)) * height;

    for( t=0; t<limit; t++)
    {
        if( key == in_ptr[t] )
        {
			in_ptr[t] = 0;
        }
    }
}
#endif


template<typename PixelType > 
static inline void preProcess( PixelType *in_ptr, int row_bytes, int height, PF_Rect *rect, bool is_white_trans )
{
    PixelType key;
	PixelType null_pixel;
	getWhitePixel( &key );	// 0xff or 0x8000
	getNullPixel( &null_pixel );

	int width = (row_bytes / sizeof(PixelType));
	
	int		top=0, left=width, right=0, bottom=0;
	bool	top_found=false, left_found=false;


	if( is_white_trans )
	{
		// 白を抜く
		// Alphaチャンネルは無視して、色が白だったら抜く
		int t=0;
		for(int j=0; j<height; j++)
		{
			if( !top_found )
			{
				top = j;
			}

			for(int i=0; i<width; i++)
			{
				if( key.red == in_ptr[t].red &&
					key.green == in_ptr[t].green &&
					key.blue == in_ptr[t].blue )
				{
					// 抜き色
					in_ptr[t] = null_pixel;
				}
				else if( in_ptr[t].alpha == 0 )
				{
					// すでに抜かれている
				}
				else
				{
					top_found = true;
					left_found = true;

					if( left > i )
					{
						left = i;
					}

					if( right < i )
					{
						right = i;
					}

					if( bottom < j )
					{
						bottom = j;
					}
				}
				t++;
			}
		}
	}
	else
	{
		// 白を抜かずに、領域情報だけ取得
		int t=0;
		for(int j=0; j<height; j++)
		{
			if( !top_found )
			{
				top = j;
			}

			for(int i=0; i<width; i++)
			{
				if(!(key.red == in_ptr[t].red && key.green == in_ptr[t].green && key.blue == in_ptr[t].blue ) &&
					 in_ptr[t].alpha != 0 )
				{
					top_found = true;
					left_found = true;

					if( left > i )
					{
						left = i;
					}

					if( right < i )
					{
						right = i;
					}

					if( bottom < j )
					{
						bottom = j;
					}
				}
				t++;
			}
		}
	}

	if( top_found )
		rect->top = top;
	else
		rect->top = 0;

	if( left_found )
		rect->left = left;
	else
		rect->left = 0;

	rect->right = right+1;
	rect->bottom = bottom+1;


}


//---------------------------------------------------------------------------//
// 概要   : Effectメイン
// 関数名 : EffectPluginMain
// 引数   : 
// 返り値 : 
//---------------------------------------------------------------------------//
DllExport
PF_Err EntryPointFunc(    PF_Cmd          cmd,
                            PF_InData       *in_data,
                            PF_OutData      *out_data,
                            PF_ParamDef     *params[],
                            PF_LayerDef     *output )
{
    PF_Err      err = PF_Err_NONE;
    
    try
    {
        switch (cmd)
        {
            case PF_Cmd_ABOUT:              // Aboutボタンを押した
                err = About(in_data, 
                            out_data, 
                            params, 
                            output);
                break;


            case PF_Cmd_GLOBAL_SETUP:       // Global setup 読み込まれた時1度だけ呼ばれるはず
                err = GlobalSetup(  in_data, 
                                    out_data,
                                    params, 
                                    output);
                break;

            case PF_Cmd_GLOBAL_SETDOWN:     // Global setdown 終了時1度だけ呼ばれるはず
                err = GlobalSetdown(in_data, 
                                    out_data,
                                    params, 
                                    output);
                break;

            case PF_Cmd_PARAMS_SETUP:       // パラメータの設定
                err = ParamsSetup(  in_data, 
                                    out_data, 
                                    params, 
                                    output);
                break;


            case PF_Cmd_RENDER:             // レンダリング
                err = Render(   in_data, 
                                out_data, 
                                params, 
                                output);
                break;

			case PF_Cmd_DO_DIALOG:
				err = PopDialog(in_data,
								out_data,
								params,
								output);
				break;

        }
    }
    catch( APIErr   api_err )
    {   // APIがエラーを返した
        
        PrintAPIErr( &api_err ); // プリント

        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
    catch(...)
    {
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }


    return err;
}



//---------------------------------------------------------------------------//
// 概要   : Aboutボタンを押したときに呼ばれる関数
// 関数名 : About
// 引数   : 
// 返り値 : 
//---------------------------------------------------------------------------//
static PF_Err About (   PF_InData       *in_data,
                        PF_OutData      *out_data,
                        PF_ParamDef     *params[],
                        PF_LayerDef     *output )
{
#if SK_STAGE_DEVELOP
    const char *stage_str= "Debug";
#elif SK_STAGE_BETA
    const char *stage_str= "beta";
#elif SK_STAGE_RELEASE
    const char *stage_str= "";
#endif

    char str[256];
    memset( str, 0, 256 );

    sprintf(    out_data->return_msg, 
                 "%s, v%d.%d.%d %s\n%s\n",
                NAME, 
                MAJOR_VERSION, 
                MINOR_VERSION,
                BUILD_VERSION,
                stage_str,
                str );

    return PF_Err_NONE;
}



//---------------------------------------------------------------------------//
// 概要   : プラグインが読み込まれた時に呼ばれる関数
// 関数名 : GlobalSetup
// 引数   : 
// 返り値 : 
//---------------------------------------------------------------------------//
static PF_Err GlobalSetup ( PF_InData       *in_data,
                            PF_OutData      *out_data,
                            PF_ParamDef     *params[],
                            PF_LayerDef     *output )
{
    // versionをpiplとあわせないといけないの&&PiPlは直値のみ
    // 使いづらいから使わないので0固定
    out_data->my_version    = PF_VERSION(2,0,0,0,0);    

	// input buffer を加工します
    out_data->out_flags  |= PF_OutFlag_I_WRITE_INPUT_BUFFER | PF_OutFlag_DEEP_COLOR_AWARE;
    out_data->out_flags2 |= PF_OutFlag2_I_AM_THREADSAFE;

    return PF_Err_NONE;
}



static PF_Err GlobalSetdown(PF_InData       *in_data,
                            PF_OutData      *out_data,
                            PF_ParamDef     *params[],
                            PF_LayerDef     *output )
{	
    return PF_Err_NONE;
}


//---------------------------------------------------------------------------//
// 概要   : パラメータの設定
// 関数名 : ParamsSetup
// 引数   : 
// 返り値 : 
//---------------------------------------------------------------------------//
static PF_Err ParamsSetup(  PF_InData       *in_data,
                            PF_OutData      *out_data,
                            PF_ParamDef     *params[],
                            PF_LayerDef     *output)
{
    
    PF_ParamDef def;
    PF_Err      err = PF_Err_NONE;

    AEFX_CLR_STRUCT(def);   // defを初期化 //

    def.param_type = PF_Param_CHECKBOX;
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_STRCPY(def.name, "white option");
    def.u.bd.value = def.u.bd.dephault = FALSE;
    def.u.bd.u.nameptr = "transparent"; /* this is strictly a pointer; don't STRCPY into it! */
    if (err == PF_ADD_PARAM(in_data, -1, &def)) return err;

    AEFX_CLR_STRUCT(def);


    PF_ADD_FLOAT_SLIDER("range",
                        0.0f,           //VALID_MIN,
                        100.0f,         //VALID_MAX,
                        0.0f,           //SLIDER_MIN,
                        10.0f,          //SLIDER_MAX,
                        1.00f,          //CURVE_TOLERANCE,  よくわかんない
                        1.0f,           //DFLT, デフォルト
                        1,              //DISP  会いたいをそのまま表示
                        0,              //PREC, パーセント表示？,
                        FALSE,          //WANT_PHASE,
                        PARAM_RANGE);   // ID

    PF_ADD_FLOAT_SLIDER("line weight",
                        0.0f,           //VALID_MIN,
                        1.0f,           //VALID_MAX,
                        0.0f,           //SLIDER_MIN,
                        1.0f,           //SLIDER_MAX,
                        1.0f,           //CURVE_TOLERANCE,  よくわかんない
                        0.0f,           //DFLT, デフォルト
                        1,              //DISP  会いたいをそのまま表示
                        0,              //PREC, パーセント表示？,
                        FALSE,          //WANT_PHASE,
                        PARAM_LINE_WEIGHT ); // ID

    // パラメータ数をセット //
    out_data->num_params = PARAM_NUM;

    
    return PF_Err_NONE;
}





//---------------------------------------------------------------------------//
// smoothing実行関数 
// PixelType		PF_Pixel8, PF_Pixel16
// PackedPixelType	KP_PIXEL32,	KP_PIXEL64
//---------------------------------------------------------------------------//
template<typename PixelType, typename PackedPixelType>
static PF_Err smoothing(PF_InData   *in_data,
						PF_OutData  *out_data,
                        PF_ParamDef *params[],
						PF_LayerDef *input,
						PF_LayerDef *output,
						PixelType	*in_ptr,
						PixelType	*out_ptr)
{
	PF_Err	err;

	PF_Rect extent_hint;
    BEGIN_PROFILE();
	

	// 白抜き & 領域情報取得
	preProcess<PixelType>(	in_ptr,
							input->rowbytes, input->height,
							&extent_hint,
							params[PARAM_WHITE_OPTION]->u.bd.value ? true : false );
	
    err = PF_COPY(input, output, NULL, NULL);

    
    int     in_width,in_height, out_width, out_height, i,j;
    long    in_target, out_target;
    unsigned int range = (unsigned int)(params[PARAM_RANGE]->u.fs_d.value * (getMaxValue<PixelType>() * 4)) / 100; 
    float       line_weight = (float)(params[PARAM_LINE_WEIGHT]->u.fs_d.value / 2.0 + 0.5),
                weight;
    bool        lack_flg;

    in_width    = GET_WIDTH(input);
    in_height   = GET_HEIGHT(input);
    out_width   = GET_WIDTH(output);
    out_height  = GET_HEIGHT(output);


    BlendingInfo<PixelType>    blend_info, *info;

    info = &blend_info;

    // 共通部分を初期化
    blend_info.input        = input;
    blend_info.output       = output;
    blend_info.in_ptr       = in_ptr;
    blend_info.out_ptr      = out_ptr;
    blend_info.range        = range;
    blend_info.LineWeight   = line_weight;
    
	// 領域情報を加工
	if( extent_hint.top == 0 )			extent_hint.top = 1;
	if( extent_hint.left == 0 )			extent_hint.left = 1;
	if( extent_hint.right == in_width )	extent_hint.right -= 1;
	if( extent_hint.bottom == in_height )	extent_hint.bottom -= 1;


    ///////////////// アンチ処理 //////////////////////////////////////////////////
    for( j=extent_hint.top; j<extent_hint.bottom; j++)
    {
        lack_flg = false;

        in_target   = j*in_width+extent_hint.left;
        out_target  = j*out_width+extent_hint.left;

        for( i=extent_hint.left; i<extent_hint.right; i++, in_target++, out_target++)
        {
            // 欠けである可能性あり
            if( lack_flg )
            {
                // フラグ落とす
                lack_flg = false;

                // 設定
                blend_info.i            = i;
                blend_info.j            = j;
                blend_info.in_target    = in_target;
                blend_info.out_target   = out_target;
                blend_info.flag         = 0;

                // 処理
                LackMode0304Execute( &blend_info );
            }

            // 可能性のある角のみに限定する //
			if( FAST_COMPARE_PIXEL(in_target, in_target+1))
            {
                unsigned char   mode_flg = 0;
                
                // 初期化 //

                blend_info.i            = i;
                blend_info.j            = j;
                blend_info.in_target    = in_target;
                blend_info.out_target   = out_target;
                blend_info.flag         = 0;

                memset( &blend_info.core, 0, sizeof(Cinfo)*4 ); // 0フィル

                // ここでアンチを処理 //
                if( ComparePixel(in_target, in_target+1))           (mode_flg |= 1<<0);
                if( ComparePixel(in_target, in_target-in_width))    (mode_flg |= 1<<1);
                if( ComparePixel(in_target, in_target+in_width))    (mode_flg |= 1<<2);
                if( ComparePixel(in_target, in_target-1))           (mode_flg |= 1<<3);


                if( mode_flg != 0 )
                {
                    // 次のピクセルがlackである可能性あり
                    if( i < input->width-2 && (mode_flg & 1<<0))
					{
                        lack_flg = true;
					}
                    
                    switch(mode_flg)
                    {
                    
                        case 3: ////// 上向きの角 /////////////////////////////
                            

                            // 8連結系モードとのバッティングを避ける
                            if( ComparePixelEqual(in_target-in_width,   in_target+1) &&     // 対角が同じで
                                ComparePixel     (in_target-in_width+1, in_target-in_width) &&  // 対角の角がそれぞれ違ういろ
                                ComparePixel     (in_target-in_width+1, in_target+1))
                            {
                                // 処理しない
                                break;
                            }
                    
                    
                            // カウント //
                            upMode_LeftCountLength<PixelType>( &blend_info);
                            
                            upMode_RightCountLength<PixelType>( &blend_info);
                            
                            upMode_TopCountLength<PixelType>( &blend_info);
                            
                            upMode_BottomCountLength<PixelType>( &blend_info);
                            
                    
                            ///// 補正処理 //////////
                            
                            /////// 水平方向 /////////////////////////
                            // start座標 (leftが長い場合しか行わない) //
                            if(blend_info.core[0].length - blend_info.core[1].length == 1)
                            {
                                blend_info.core[0].start -= 0.5f;
                                blend_info.core[1].start -= 0.5f;
                            }
                            
                            if( (blend_info.core[0].flg & CR_FLG_FILL ) || 
                                (blend_info.core[1].flg & CR_FLG_FILL ) )
                            {
                                weight = 0.5f;
                            }
                            else
                            {
                                weight = blend_info.LineWeight;
                            }
                            
                            // end を計算しなおす(Countが出力するend値はLenと同じで0.5がかけられていない)(補正時の再計算で沸けわからなくなるため) //
                            blend_info.core[0].end = blend_info.core[0].start - (blend_info.core[0].start - blend_info.core[0].end) * weight;
                            blend_info.core[1].end = blend_info.core[1].start + (blend_info.core[1].end   - blend_info.core[1].start) * weight;
                            
                            
                            /////// 垂直方向 /////////////////////////
                            // start座標 (bottomが長い場合しか行わない) //
                            if(blend_info.core[3].length - blend_info.core[2].length == 1)
                            {
                                blend_info.core[2].start += 0.5f;
                                blend_info.core[3].start += 0.5f;
                            }
                            
                            if( (blend_info.core[2].flg & CR_FLG_FILL ) || 
                                (blend_info.core[3].flg & CR_FLG_FILL ) )
                            {
                                weight = 0.5f;
                            }
                            else
                            {
                                weight = blend_info.LineWeight;
                            }
                            
                            // end を計算しなおす(Countが出力するend値はLenと同じで0.5がかけられていない)(補正時の再計算で沸けわからなくなるため) //
                            blend_info.core[2].end = blend_info.core[2].start - (blend_info.core[2].start - blend_info.core[2].end) * weight;
                            blend_info.core[3].end = blend_info.core[3].start + (blend_info.core[3].end   - blend_info.core[3].start) * weight;
                            
#if 0                       // 補正処理なし
                            {
                                blend_info.core[0].start    = (float)(i+1); // 画像の座標は左上が(0,0), でも論理線は右からだから+1
                                blend_info.core[0].end      = blend_info.core[0].start - (float)blend_info.core[0].length * 0.5f;
                                
                                blend_info.core[1].start    = (float)(i+1); // 画像の座標は左上が(0,0), でも論理線は右からだから+1
                                blend_info.core[1].end      = blend_info.core[1].start + (float)blend_info.core[1].length * 0.5f;
                                    
                                blend_info.core[2].start    = (float)(j);
                                blend_info.core[2].end      = blend_info.core[2].start - (float)blend_info.core[2].length * 0.5f;
                                
                                blend_info.core[3].start    = (float)(j);
                                blend_info.core[3].end      = blend_info.core[3].start + (float)blend_info.core[3].length * 0.5f;
                            }
#endif              
                    
                            if( blend_info.core[0].length >= 2 && // left >= 2 && bottom >= 2
                                blend_info.core[3].length >= 2)
                            {
                                // 欠けモード2
                                LackMode02Execute( &blend_info );
                            }
                            else if(blend_info.core[1].length > 0)  // Left > 0 && Right > 0
                            {   ////////// 水平方向のブレンド /////////////
                            
                                blend_info.mode = BLEND_MODE_UP_H;  // モード設定
                    
                                // ブレンド //
                                upMode_LeftBlending<PixelType>( &blend_info);
                               
                                upMode_RightBlending<PixelType>( &blend_info);
                    
                                if(blend_info.core[2].length > 1)
                                {   //// 同時に垂直方向も存在 //////
                                    upMode_TopBlending<PixelType>( &blend_info);
                                
                                    upMode_BottomBlending<PixelType>( &blend_info);
                                }
                                
                            }
                            else if(blend_info.core[2].length > 0)  // bottom > 0 && top > 0
                            {   //////////// 垂直方向のブレンド /////////////
                            
                                blend_info.mode = BLEND_MODE_UP_V;  // モード設定
                                
                            
                                upMode_TopBlending<PixelType>( &blend_info);
                                
                                upMode_BottomBlending<PixelType>( &blend_info);
                            }
                    
                            break;
                    
                    
                    
                    
                        case 5: //// 下向きの角 ///////////////////////////////////
                            // 8連結系モードとのバッティングを避ける
                            if( ComparePixelEqual(in_target+in_width,   in_target+1) &&     // 対角が同じで
                                ComparePixel     (in_target+in_width+1, in_target+in_width) &&  // 対角の角がそれぞれ違ういろ
                                ComparePixel     (in_target+in_width+1, in_target+1))
                            {
                                // 処理しない
                                break;
                            }
                    
                    
                            ////// カウント //////////
                            downMode_LeftCountLength<PixelType>( &blend_info);
                            
                            downMode_RightCountLength<PixelType>( &blend_info);
                            
                            downMode_TopCountLength<PixelType>( &blend_info);
                            
                            downMode_BottomCountLength<PixelType>( &blend_info);
                            
                            
                            
                            ///// 補正処理 //////////
                            /////// 水平方向 /////////////////////////
                            // start座標 (leftが長い場合しか行わない) //
                            if(blend_info.core[0].length - blend_info.core[1].length == 1)
                            {
                                blend_info.core[0].start -= 0.5f;
                                blend_info.core[1].start -= 0.5f;
                            }
                            
                            if( (blend_info.core[0].flg & CR_FLG_FILL ) || 
                                (blend_info.core[1].flg & CR_FLG_FILL ) )
                            {
                                weight = 0.5f;
                            }
                            else
                            {
                                weight = blend_info.LineWeight;
                            }
                            
                            // end を計算しなおす(Countが出力するend値はLenと同じで0.5がかけられていない)(補正時の再計算で沸けわからなくなるため) //
                            blend_info.core[0].end = blend_info.core[0].start - (blend_info.core[0].start - blend_info.core[0].end)  * weight;
                            blend_info.core[1].end = blend_info.core[1].start + (blend_info.core[1].end   - blend_info.core[1].start)  * weight;
                            
                            
                            /////// 垂直方向 /////////////////////////
                            // start座標 (bottomが長い場合しか行わない) //
                            if(blend_info.core[3].length - blend_info.core[2].length == 1)
                            {
                                blend_info.core[2].start += 0.5f;
                                blend_info.core[3].start += 0.5f;
                            }
                            
                            if( (blend_info.core[2].flg & CR_FLG_FILL ) || 
                                (blend_info.core[3].flg & CR_FLG_FILL ) )
                            {
                                weight = 0.5f;
                            }
                            else
                            {
                                weight = blend_info.LineWeight;
                            }
                    
                            // end を計算しなおす(Countが出力するend値はLenと同じで0.5がかけられていない)(補正時の再計算で沸けわからなくなるため) //
                            blend_info.core[2].end = blend_info.core[2].start - (blend_info.core[2].start - blend_info.core[2].end)  * weight;
                            blend_info.core[3].end = blend_info.core[3].start + (blend_info.core[3].end   - blend_info.core[3].start)  * weight;
                            
                            
#if 0                       // 補正処理なし                     
                            {
                                blend_info.core[0].start    = (float)(i+1); // 画像の座標は左上が(0,0), でも論理線は右からだから+1
                                blend_info.core[0].end      = blend_info.core[0].start - (float)blend_info.core[0].length * 0.5f;
                                
                                blend_info.core[1].start    = (float)(i+1); // 画像の座標は左上が(0,0), でも論理線は右からだから+1
                                blend_info.core[1].end      = blend_info.core[1].start + (float)blend_info.core[1].length * 0.5f;
                                    
                                blend_info.core[2].start    = (float)(j);
                                blend_info.core[2].end      = blend_info.core[2].start - (float)blend_info.core[2].length * 0.5f;
                                
                                blend_info.core[3].start    = (float)(j);
                                blend_info.core[3].end      = blend_info.core[3].start + (float)blend_info.core[3].length * 0.5f;
                            }
#endif              
                    
                            ////// ブレンド ///////
                            if( blend_info.core[0].length >= 2 && // left >= 2 && top >= 2
                                blend_info.core[2].length >= 2)
                            {
                                // 欠けモード1
                                LackMode01Execute( &blend_info );
                            }
                            else if(blend_info.core[1].length > 0)  // Left > 0 && Right > 0
                            {   ////////// 水平方向のブレンド /////////////
                            
                                blend_info.mode = BLEND_MODE_UP_H;  // モード設定
                    
                                // ブレンド //
                                downMode_LeftBlending<PixelType>( &blend_info);
                               
                                downMode_RightBlending<PixelType>( &blend_info);
                    
                                if(blend_info.core[3].length > 1)
                                {   //// 同時に垂直方向も存在 //////
                                    downMode_TopBlending<PixelType>( &blend_info);
                                
                                    downMode_BottomBlending<PixelType>( &blend_info);
                                }
                                
                            }
                            else if(blend_info.core[3].length > 0)  // bottom > 0 && top > 0
                            {   //////////// 垂直方向のブレンド /////////////
                            
                                blend_info.mode = BLEND_MODE_UP_V;  // モード設定
                                
                            
                                downMode_TopBlending<PixelType>( &blend_info);
                                
                                downMode_BottomBlending<PixelType>( &blend_info);
                            }

                            break;
                        //// 突起1 コ  2 п  3 ヒ  4 ш
                        case 7: ////// 突起1
                            Link8Mode01Execute(&blend_info);
                            break;
                    
                        case 11: ////// 突起2
                            Link8Mode02Execute(&blend_info);
                            break;
                        
                        case 13: ////// 突起4
                            Link8Mode04Execute(&blend_info);
                            break;
                    
                    
                    
                        case 15: ////// 四角のピクセル
                            Link8SquareExecute(&blend_info);
                            break;
                    
                        default:
                            break;
                    }
                    
					// 突起mode3
                    if(i < input->width-2)
                    {
                        // 初期化 //
                        blend_info.i            = i+1;
                        blend_info.j            = j;
                        blend_info.in_target    = in_target+1;
                        blend_info.out_target   = out_target+1;
                        blend_info.flag         = 0;
                        
                        mode_flg = 0;
                        if( ComparePixel(blend_info.in_target, blend_info.in_target-in_width))  (mode_flg |= 1<<0);
                        if( ComparePixel(blend_info.in_target, blend_info.in_target+in_width))  (mode_flg |= 1<<1);
                        if( ComparePixel(blend_info.in_target, blend_info.in_target+1))         (mode_flg |= 1<<2);
                    
                        if(3==mode_flg)
                        {
                            // 突起 3 ヒ
                            Link8Mode03Execute(&blend_info);
                        }
                    }
                }

            }

        }
    }
	
	DEBUG_PIXEL( out_ptr, output, extent_hint.left, extent_hint.top );
	DEBUG_PIXEL( out_ptr, output, extent_hint.left, extent_hint.bottom );
	DEBUG_PIXEL( out_ptr, output, extent_hint.right, extent_hint.top );
	DEBUG_PIXEL( out_ptr, output, extent_hint.right, extent_hint.bottom );


    END_PROFILE();

	return err;
}


//---------------------------------------------------------------------------//
// 概要   : レンダリング
// 関数名 : Render
// 引数   : 
// 返り値 : 
//---------------------------------------------------------------------------//
static PF_Err Render (  PF_InData       *in_data,
                        PF_OutData      *out_data,
                        PF_ParamDef     *params[],
                        PF_LayerDef     *output )
{
    PF_Err err = PF_Err_NONE;

	PF_LayerDef *input  = &params[PARAM_INPUT]->u.ld;

#if SK_STAGE_BETA
    //------ 期限制限部分 ------//
    switch(TimeFlag)
    {
        case    TIME_FLAG_INVALID:
        case    TIME_FLAG_OVER:
            err = PF_COPY(input, output, NULL, NULL);
            return  err;

        default:
            break;
    }
#endif
    

	PF_Pixel16	*in_ptr16, *out_ptr16;
	PF_GET_PIXEL_DATA16(output, NULL, &out_ptr16 );
	PF_GET_PIXEL_DATA16(input, NULL, &in_ptr16 );

	if( out_ptr16 != NULL && in_ptr16 != NULL )
	{
		// 16bpc or 32bpc
		err = smoothing<PF_Pixel16, KP_PIXEL64>(in_data, out_data, params,
												input, output, in_ptr16, out_ptr16 );
	}
	else
	{
		// 8bpc
		PF_Pixel8	*in_ptr8, *out_ptr8;
		PF_GET_PIXEL_DATA8(output, NULL, &out_ptr8 );
		PF_GET_PIXEL_DATA8(input, NULL, &in_ptr8 );
		
		err = smoothing<PF_Pixel8, KP_PIXEL32>(in_data, out_data, params,
												input, output, in_ptr8, out_ptr8 );
	}

	return err;


}








//---------------------------------------------------------------------------//
// ダイアログ作成
//---------------------------------------------------------------------------//
static PF_Err 
PopDialog (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err err = PF_Err_NONE;
 
	char str[256];
    memset( str, 0, 256 );

    sprintf(    out_data->return_msg, 
                 "%s, v%d.%d.%d\n%s\n",
                NAME, 
                MAJOR_VERSION, 
                MINOR_VERSION,
                BUILD_VERSION,
                str );

	return err;
}

