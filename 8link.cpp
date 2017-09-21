//---------------------------------------------------------------------------------------//
// 概要: 8連結系の処理
//---------------------------------------------------------------------------------------//

#include "util.h"
#include "define.h"
#include "8link.h"


//----------- マクロ -------------------------------------------------------------------//
#define MAX_LENGTH  (128)




// 8連結系のパターンは、縦割りに自分の領域にのみかける
// |----|
//      |----|
// それぞれ | |の間を受け持つ それのinside outside この場合上下を分けて考える



//-----------------------------------------------------------------------------------------------//
// 1本の列 あってる？ 使ってない？
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
static inline int CountLength(BlendingInfo<PixelType> *info, long target, int NextPixelStepIn, int Min, int Max, int LimitFromHere)
{
    int         width   = GET_WIDTH(info->input),
                height  = GET_HEIGHT(info->input);
    int         Length=0;
    int         Sign    = GET_SIGN(NextPixelStepIn);    // 符号
    int         LenDiff = Sign * 1; // -1 or +1
    int         range   = info->range;
    
    
    while(  Min < Length + LimitFromHere && Length + LimitFromHere < Max)
    {
        long t = target + Length*NextPixelStepIn;
    
        Length += LenDiff;

        if(ComparePixel( t, t + NextPixelStepIn))   // 違った色
            break;
    }

    return ABS(Length);
}

//-----------------------------------------------------------------------------------------------//
// 2本の列に同時にカウント
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
static inline int CountLengthTwoLines(	BlendingInfo<PixelType> *info, long target0, long target1, int NextPixelStepIn,
										int Min, int Max, int LimitFromHere,
										bool *t0_flg )
{
    int         width   = GET_WIDTH(info->input),
                height  = GET_HEIGHT(info->input);
    int         Length=0;
    int         Sign    = GET_SIGN(NextPixelStepIn);    // 符号
    int         LenDiff = Sign * 1; // -1 or +1
    int         range   = info->range;
    
    *t0_flg = false;    // t0の方が違う色でしたフラグ
    
    while(  Min < Length + LimitFromHere && Length + LimitFromHere < Max)
    {
        long    t0 = target0 + ABS(Length)*NextPixelStepIn,
                t1 = target1 + ABS(Length)*NextPixelStepIn;
    
        Length += LenDiff;

        if( ComparePixel( t0, t0 + NextPixelStepIn) )   
        {
            *t0_flg = true;
            break;
        }

        if( ComparePixel( t1, t1 + NextPixelStepIn) )   // 違った色
        {
            break;
        }
    }

    return ABS(Length);
}

//-----------------------------------------------------------------------------------------------//
// 外方向ブレンド
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
void BlendOutside(  BlendingInfo<PixelType> *info, // 
                    double length,          // このパターンの長さ
                    long blend_target,      // ブレンド先のターゲット(input)
                    long out_target,        // ブレンド先のターゲット(output)
                    int ref_offset,         // ブレンド参照先のターゲット(input)
                    int NextPixelStepIn,    // 次のピクセルへ移動するときこの値を足す(input)
                    int NextPixelStepOut,   // 次のピクセルへ移動するときこの値を足す(output)
                    bool ratio_invert,
                    bool no_line_weight )
{
    double  len; // 全体の底辺
    int     blend_count;            // ブレンドするピクセルの数 切り上げ
    double  pre_ratio       = 0.0;
    
    // line の太さ
    if( no_line_weight )
        len = (length * 0.5 ); 
    else
        len = (length * (double)info->LineWeight );
    
    blend_count     = CEIL(len);


    // 境界の逆方向からブレンドを行うので、その分移動 //
    blend_target    += (blend_count-1) * NextPixelStepIn;
    out_target      += (blend_count-1) * NextPixelStepOut;
    

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
        Blendingf(info->in_ptr, info->out_ptr, blend_target, blend_target+ref_offset, out_target, (float)r);

        pre_ratio = ratio;

        // 次のピクセルへ
        blend_target    -= NextPixelStepIn;
        out_target      -= NextPixelStepOut;
    }
}


//-----------------------------------------------------------------------------------------------//
// 内方向ブレンド
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
void BlendInside(   PixelType TempPixel[2][MAX_LENGTH],
					int index,
                    BlendingInfo<PixelType> *info,
                    double length,              // このパターンの長さ
                    long blend_target,          // ブレンド先のターゲット(input)
                    int ref_offset,             // ブレンド参照先のターゲット(input)
                    int NextPixelStepIn,        // 次のピクセルへ移動するときこの値を足す(input)
                    bool ratio_invert,
                    bool no_line_weight )
{
    double  len;  // 全体の底辺
    int     blend_count;            // ブレンドするピクセルの数 切り上げ
    double  pre_ratio       = 0.0;
    
    // line の太さ
    if( no_line_weight )
        len = (length * 0.5 ); 
    else
        len = (length * (double)info->LineWeight );
    
    blend_count = CEIL(len);


    // 境界の逆方向からブレンドを行うので、その分移動 //
    blend_target    += (blend_count-1) * NextPixelStepIn;
    

    int t;
    for(t=0;t<blend_count;t++)
    {
        double  l           = len - (float)(((int)CEIL(len)-1) - t);    // このピクセルでの底辺  (int)CEIL(len)-1 は (1.000...1 ～ 2.0) -> 1としたいため
        double  ratio       = (l * l * 1.0 * 0.5) / len;                    // 高さ1.0として考える !! ここ違います (Outside比)!!
        double  r;

        r = ratio_invert ? 1.0 - (ratio - pre_ratio) : (ratio - pre_ratio);

        // ブレンド !! ここが違います (Outside比) !!
        BlendingPixelf(&info->in_ptr[blend_target], &info->in_ptr[blend_target+ref_offset], &TempPixel[index][blend_count-1-t], (float)r);

        pre_ratio = ratio;

        // 次のピクセルへ
        blend_target    -= NextPixelStepIn;
    }
}








//-----------------------------------------------------------------------------------------------//
// 実際の共通実行関数
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
static void Link8Execute(   BlendingInfo<PixelType> *info, 
                            int RefPixelStepIn,                             // 1
                            int RefPixelStepOut,                            // 1
                            int NextPixelStepIn,                            // width
                            int NextPixelStepOut,                           // width
                            int Min, int Max, int LimitFromHere,            // 0, height-1, info->j
                            int AreaMin, int AreaMax, int AreaPosition,     // 1, width-2, info->i
                            int mode )                                      
{
    PixelType	*in_ptr     = info->in_ptr,
                *out_ptr    = info->out_ptr;
    long        in_target   = info->in_target,
                out_target  = info->out_target;


    int         i;
    PF_LayerDef *input      = info->input,
                *output     = info->output;
    PixelType   TempPixel[2][MAX_LENGTH];
    int         TempLength;
    int         Length[2];  // 0 : 左or上  1 : 右or下
    int         range       = info->range;
    bool        inside_flg[2] = { false, false };
    bool        flag;

    //----------- 左右(上下)それぞれカウント --------------------------//
    // 左側
    Length[0] = CountLengthTwoLines(info, in_target, in_target-RefPixelStepIn, NextPixelStepIn, Min, Max, LimitFromHere, &flag);
                
    // 右側
    Length[1] = CountLengthTwoLines(info, in_target, in_target+RefPixelStepIn, NextPixelStepIn, Min, Max, LimitFromHere, &flag);

    // クランプ
    Length[0] = MIN(MAX_LENGTH, Length[0]);
    Length[1] = MIN(MAX_LENGTH, Length[1]);

    // 最大Length
    TempLength = MAX(Length[0], Length[1]);

        

    //------------ 両脇も閉局面？ -----------------------------------//
    bool ForceInsideFlag = false;
    if( AreaMin < AreaPosition && AreaPosition < AreaMax)
    {
        if( ComparePixel(in_target-RefPixelStepIn, in_target-RefPixelStepIn*2) &&
            ComparePixel(in_target+RefPixelStepIn, in_target+RefPixelStepIn*2))
        {   // 必ず内側 //
            ForceInsideFlag = true;
        }
    }
    
    //------------ 左(上)側 -----------------------------------------//
    // 外側？内側？
    if(ComparePixelEqual(in_target, in_target - NextPixelStepIn - RefPixelStepIn) && !ForceInsideFlag)
    {   
        bool flg = false;

        //---- 外側 ----------------------------//
        
        // ブレンドしようとする対象も閉局面だった場合には処理をしない(外だけ)
        if( AreaMin < AreaPosition && AreaPosition < AreaMax)
        {
            if(ComparePixelEqual(in_target-RefPixelStepIn, in_target-RefPixelStepIn*2))
            {
                flg = true;
            }
        }
        else
        {
            flg = true;
        }

        if(flg)
        {
            BlendOutside(   info, 
                            Length[0],
                            info->in_target-RefPixelStepIn,
                            info->out_target-RefPixelStepOut,
                            RefPixelStepIn,
                            NextPixelStepIn,
                            NextPixelStepOut,
                            true,
                            true );
        }

    }
    else
    {   
        //----- 内側 ---------------------------//
        BlendInside(    TempPixel,
						0,
                        info,
                        Length[0],
                        info->in_target,
                        -RefPixelStepIn,
                        NextPixelStepIn,
                        true,
                        true );
        
        inside_flg[0] = true;
    }



    //------------ 右(下)側 -----------------------------------------//
    if(ComparePixelEqual(in_target, in_target - NextPixelStepIn + RefPixelStepIn) && !ForceInsideFlag)
    {   
        bool flg = false;

        //---- 外側 ----------------------------//
        
        // ブレンドしようとする対象も閉局面だった場合には処理をしない(外だけ)
        if( AreaMin < AreaPosition && AreaPosition < AreaMax)
        {
            if(ComparePixelEqual(in_target+RefPixelStepIn, in_target+RefPixelStepIn*2))
            {
                flg = true;
            }
        }
        else
        {
            flg = true;
        }

        if(flg)
        {
            BlendOutside(   info,
                            Length[1],
                            info->in_target+RefPixelStepIn,
                            info->out_target+RefPixelStepOut,
                            -RefPixelStepIn,
                            NextPixelStepIn,
                            NextPixelStepOut,
                            true,
                            true );
        }
    }
    else
    {
        //---- 内側 ----------------------------//
        BlendInside(    TempPixel, 1,
                        info,
                        Length[1], 
                        info->in_target,
                        RefPixelStepIn,
                        NextPixelStepIn,
                        true,
                        true );

        inside_flg[1] = true;
    }



    //------------------------------------------------------------
    // どちらもinsideな処理でかつその両方の色が違う場合
	//   Γ
	//__Γ____ みたいな3色が交わったパターン
	// 
    if( ComparePixel( in_target-RefPixelStepIn, in_target+RefPixelStepIn) &&
        inside_flg[0] == true &&
        inside_flg[1] == true)
    {
        bool blend_flg = false;
		bool f[2] = {false, false};

		f[0] = ComparePixel( info->in_target - NextPixelStepIn, info->in_target - NextPixelStepIn + RefPixelStepIn);
		f[1] = ComparePixel( info->in_target - NextPixelStepIn, info->in_target - NextPixelStepIn - RefPixelStepIn);

        //------------------------------------------------------------
        // upmode,dowmmodeとのバッティングを避ける必要あり
        // 2,4は半分だめ,3は全部ダメ，1は全部やる
		// だたし、バッティングするような角になっていない場合はやる
        //------------------------------------------------------------

		// どちらも同系色なので、バッティングするような角になっていない
		if( f[0] == false && f[1] == false )
		{
			blend_flg = true;
		}

        switch( mode )
        {
            case 2:
            case 4:
                {
                    if( ( mode == 2 && f[0] == false && f[1] == true ) ||
                        ( mode == 4 && f[0] == false && f[1] == true ))
                    {
                        blend_flg = true;
                    }
                }
                break;
            
            default:
            case 3:
                break;

            case 1:
                blend_flg = true;
                break;
        }

        if( blend_flg )
        {
            // 突起の上(右)の色と同系と考えられる方とブレンド 
			// 1つのpixelとブレンドするのはノイズになりやすい
            double len = (double)MIN( Length[0], Length[1]);

			if( ComparePixelEqual( info->in_target - NextPixelStepIn, info->in_target + RefPixelStepIn) )
			{
				BlendLine<PixelType>(
					info,
					len,
					in_target,
					out_target,
					RefPixelStepIn,	// 
					NextPixelStepIn,
					NextPixelStepOut,
					true,
					true );
			}
			else if( ComparePixelEqual( info->in_target - NextPixelStepIn, info->in_target - RefPixelStepIn) ) 
			{
				BlendLine<PixelType>(
					info,
					len,
					in_target,
					out_target,
					-RefPixelStepIn, // 
					NextPixelStepIn,
					NextPixelStepOut,
					true,
					true );
			}

        }
    }
    else
    {
        //------------ 左右の内側を平均化し出力 ---------------------//
        int total_len, len[2];

        total_len = CEIL((float)TempLength * 0.5f);

        len[0] = CEIL((float)Length[0] * 0.5f);
        len[1] = CEIL((float)Length[1] * 0.5f);
        
        for(i=0; i<total_len; i++)
        {

            // 両方あるとき
            if( (i < len[0] && inside_flg[0] == true) &&
                (i < len[1] && inside_flg[1] == true) )
            {
                out_ptr[out_target + i * NextPixelStepOut].red     = (TempPixel[0][i].red     +  TempPixel[1][i].red)     / 2;
                out_ptr[out_target + i * NextPixelStepOut].green   = (TempPixel[0][i].green   +  TempPixel[1][i].green)   / 2;
                out_ptr[out_target + i * NextPixelStepOut].blue    = (TempPixel[0][i].blue    +  TempPixel[1][i].blue)    / 2;
                out_ptr[out_target + i * NextPixelStepOut].alpha   = (TempPixel[0][i].alpha   +  TempPixel[1][i].alpha)   / 2;
            }
            else if( i < len[0] && inside_flg[0] == true)
            {   // 0側だけ
                BlendingPixelf( &in_ptr[in_target + i * NextPixelStepIn],
                                &TempPixel[0][i],
                                &out_ptr[out_target + i * NextPixelStepOut],
                                0.5f );
            }
            else if( i < len[1] && inside_flg[1] == true)
            {   // 1側だけ
                BlendingPixelf( &in_ptr[in_target + i * NextPixelStepIn],
                                &TempPixel[1][i],
                                &out_ptr[out_target + i * NextPixelStepOut],
                                0.5f );
            }
        }
    }




    //---------------------------------------------------------------------------//
    // upmode、downmodeで対応されないパターン
    // |_
    //  _|
    // |
    // こういうやつ
    //---------------------------------------------------------------------------//
    if( ComparePixelEqual( in_target, in_target + NextPixelStepIn - RefPixelStepIn) &&
        ComparePixelEqual( in_target, in_target + NextPixelStepIn + RefPixelStepIn) )
    {
        switch( mode )
        {
            default:
            case 3:
                // 上下どっちもやらない
                break;

            case 1:
                // 上下どっちもやる
                {
                    int len[2] = {0,0};
                    // カウント
                    // 下
                    len[0] = CountLengthTwoLines(   info,
                                                    in_target-RefPixelStepIn,
                                                    in_target-RefPixelStepIn + NextPixelStepIn, 
                                                    -RefPixelStepIn, 
                                                    AreaMin,
                                                    AreaMax,
                                                    AreaPosition,
                                                    &flag );

                    // ブレンド
                    BlendLine<PixelType>(  info,
                                len[0],
                                in_target-RefPixelStepIn,
                                out_target-RefPixelStepOut,
                                -RefPixelStepIn+NextPixelStepIn,
                                -RefPixelStepIn,
                                -RefPixelStepOut,
                                true,
                                true );

                    // カウント
                    // 上
                    len[1] = CountLengthTwoLines(   info,
                                                    in_target+RefPixelStepIn,
                                                    in_target+RefPixelStepIn + NextPixelStepIn, 
                                                    RefPixelStepIn, 
                                                    AreaMin,
                                                    AreaMax,
                                                    AreaPosition,
                                                    &flag );
                    // ブレンド
                    BlendLine<PixelType>(  info,
                                len[1],
                                in_target+RefPixelStepIn,
                                out_target+RefPixelStepOut,
                                RefPixelStepIn+NextPixelStepIn,
                                RefPixelStepIn,
                                RefPixelStepOut,
                                true,
                                true );
                }
                break;

            case 2:
            case 4:
                // 右だけ
                {
                    int len;

                    // カウント
                    len = CountLengthTwoLines(  info,
                                                in_target+RefPixelStepIn,
                                                in_target+RefPixelStepIn + NextPixelStepIn, 
                                                RefPixelStepIn, 
                                                AreaMin,
                                                AreaMax,
                                                AreaPosition,
                                                &flag );
                    // ブレンド
                    BlendLine<PixelType>(  info,
                                len,
                                in_target+RefPixelStepIn,
                                out_target+RefPixelStepOut,
                                RefPixelStepIn+NextPixelStepIn,
                                RefPixelStepIn,
                                RefPixelStepOut,
                                true,
                                true );

                }
                break;
        }
    }
}






//-----------------------------------------------------------------------------------------------//
// 名前 : 2 пモード メイン
// 概要 : 上記モードのメイン
// 1ピクセル内で2つの推理線が影響していることが考えられるので
// 縦に2つに分割し、それぞれのピクセル値を平均化することによって、結果を得る
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
void Link8Mode02Execute(BlendingInfo<PixelType> *info)
{

    int         in_width    = GET_WIDTH(info->input),
                in_height   = GET_HEIGHT(info->input),
                out_width   = GET_WIDTH(info->output),
                out_height  = GET_HEIGHT(info->output);
    
    Link8Execute(   info, 
                    1,
                    1,
                    in_width,
                    out_width,
                    0, in_height-1, info->j,
                    1, in_width-2, info->i,
                    2 );

}

//-----------------------------------------------------------------------------------------------//
// 名前 : 4 ш モード メイン
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
void Link8Mode04Execute(BlendingInfo<PixelType> *info)
{
    int         in_width    = GET_WIDTH(info->input),
                in_height   = GET_HEIGHT(info->input),
                out_width   = GET_WIDTH(info->output),
                out_height  = GET_HEIGHT(info->output);
    
    Link8Execute(   info, 
                    1,
                    1,
                    -in_width,
                    -out_width,
                    0, in_height-1, info->j,
                    1, in_width-2, info->i,
                    4 );
}



//-----------------------------------------------------------------------------------------------//
// 名前 : 1 コ モード メイン
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
void Link8Mode01Execute(BlendingInfo<PixelType> *info)
{
    int         in_width    = GET_WIDTH(info->input),
                in_height   = GET_HEIGHT(info->input),
                out_width   = GET_WIDTH(info->output),
                out_height  = GET_HEIGHT(info->output);
    
    Link8Execute(   info, 
                    -in_width,
                    -out_width,
                    -1,
                    -1,
                    0, in_width-1, info->i,
                    1, in_height-2, info->j,
                    1 );
}


//-----------------------------------------------------------------------------------------------//
// 名前 : 3 ヒ モード メイン
//-----------------------------------------------------------------------------------------------//
template<typename PixelType>
void Link8Mode03Execute(BlendingInfo<PixelType> *info)
{
    int         in_width    = GET_WIDTH(info->input),
                in_height   = GET_HEIGHT(info->input),
                out_width   = GET_WIDTH(info->output),
                out_height  = GET_HEIGHT(info->output);
    
    Link8Execute(   info, 
                    -in_width,
                    -out_width,
                    1,
                    1,
                    0, in_width-1, info->i,
                    1, in_height-2, info->j,
                    3 );
}














//-------------------------------------------------------------------------------------------------//
// ■モード


//---------------------------------------------------------------------------//
template<typename PixelType>
void Link8SquareBlendOutside(   BlendingInfo<PixelType> *info,
                                long in_target,
                                long out_target,
                                int ref_offset,
                                int next_pixel_step_in,
                                int next_pixel_step_out,
                                int min, int max, int limit_from_here )
{
    int count;
    bool no_line_weight;

    count = CountLengthTwoLines(    info, in_target, in_target+ref_offset, next_pixel_step_in,
                                    min, max, limit_from_here,
                                    &no_line_weight );

    BlendLine<PixelType>(  info,
                (double)count,
                in_target,
                out_target,
                ref_offset,
                next_pixel_step_in,
                next_pixel_step_out, 
                true,
                no_line_weight );

}



//---------------------------------------------------------------------------//
// 概要   : 
// 関数名 : 
// 引数   : 
// 返り値 : 
//---------------------------------------------------------------------------//
template<typename PixelType>
void Link8SquareExecute( BlendingInfo<PixelType> *info )
{
    PF_LayerDef *input      = info->input;
    PixelType	*in_ptr     = info->in_ptr;
    int         in_width    = GET_WIDTH(info->input),
                out_width   = GET_WIDTH(info->output),
                in_height   = GET_HEIGHT(info->input);
    int         range       = info->range;
    int         i;
    unsigned int flg=0, link_count=0;

    // まず8連結状態を調べる
    if( ComparePixelEqual( info->in_target, info->in_target-in_width -1 )) flg |= (1<<0);   // パターン0
    if( ComparePixelEqual( info->in_target, info->in_target-in_width +1 )) flg |= (1<<1);   // パターン1
    if( ComparePixelEqual( info->in_target, info->in_target+in_width +1 )) flg |= (1<<2);   // パターン2
    if( ComparePixelEqual( info->in_target, info->in_target+in_width -1 )) flg |= (1<<3);   // パターン3


    // 自分自身のPixel ----------------------------------
    {
        PixelType   temp_pixel[4];
        int         ref_tbl[4];
        int         sum_color[4];
    
        // 初期化
        ref_tbl[0] = -in_width -1;
        ref_tbl[1] = -in_width +1;
        ref_tbl[2] =  in_width +1;
        ref_tbl[3] =  in_width -1;

        for( i=0; i<4 ;i++)
            sum_color[i] = 0;


        // 処理
        for( i=0; i<4 ;i++)
        {
            temp_pixel[i]   = in_ptr[info->in_target];
        
            if( !(flg & ( 1<<i)) )
            {
                BlendingPixelf( &in_ptr[info->in_target],
                                &in_ptr[info->in_target + ref_tbl[i]],
                                &temp_pixel[i],
                                0.5f );
            }
            
            // ついでに合計しとく
            sum_color[0] += temp_pixel[i].red;
            sum_color[1] += temp_pixel[i].green;
            sum_color[2] += temp_pixel[i].blue;
            sum_color[3] += temp_pixel[i].alpha;
        }


        // 出力
        info->out_ptr[ info->out_target ].red      = sum_color[0] / 4;
        info->out_ptr[ info->out_target ].green    = sum_color[1] / 4;
        info->out_ptr[ info->out_target ].blue     = sum_color[2] / 4;
        info->out_ptr[ info->out_target ].alpha    = sum_color[3] / 4;

    }



    // ← ----------------------------------------------
    // カウント
    if( (flg & 0x9) != 0x9 )    // 両方に連結していた場合には他のパターンで処理する パターン0&3でない
    {

        if( flg & (1<<0) )              // パターン0のみ
        {
            Link8SquareBlendOutside(    info,
                                        info->in_target-1,
                                        info->out_target-1,
                                        -in_width,
                                        -1,
                                        -1,
                                        1, in_width-2, info->i );
        }
        else if( flg & (1<<3) )         // パターン3のみ
        {
            Link8SquareBlendOutside(    info,
                                        info->in_target-1,
                                        info->out_target-1,
                                        in_width,
                                        -1,
                                        -1,
                                        1, in_width-2, info->i );
        }
    }


    // ↑ ----------------------------------------------
    if( (flg & 0x3) != 0x3 )    // 両方に連結していた場合には他のパターンで処理する パターン0&1でない
    {

        if( flg & (1<<0) )  // パターン0のみ
        {
            Link8SquareBlendOutside(    info,
                                        info->in_target-in_width,
                                        info->out_target-out_width,
                                        -1,
                                        -in_width,
                                        -out_width,
                                        1, in_height-2, info->j );
        }
        else if( flg & (1<<1) )         // パターン1のみ
        {
            Link8SquareBlendOutside(    info,
                                        info->in_target-in_width,
                                        info->out_target-out_width,
                                        1,
                                        -in_width,
                                        -out_width,
                                        1, in_height-2, info->j );
        }
    }
    
    // → ----------------------------------------------
    if( (flg & 0x6) != 0x6 )    // 両方に連結していた場合には他のパターンで処理する パターン1&2でない
    {
        if( flg & (1<<1) )  // パターン1のみ
        {
            Link8SquareBlendOutside(    info,
                                        info->in_target+1,
                                        info->out_target+1,
                                        -in_width,
                                        1,
                                        1,
                                        1, in_width-2, info->i );
        }
        else if( flg & (1<<2) )         // パターン2のみ
        {
            Link8SquareBlendOutside(    info,
                                        info->in_target+1,
                                        info->out_target+1,
                                        in_width,
                                        1,
                                        1,
                                        1, in_width-2, info->i );
        }
    }

    // ↓ ----------------------------------------------
    if( (flg & 0xc) != 0xc )    // 両方に連結していた場合には他のパターンで処理する パターン2&3でない
    {

        if( flg & (1<<2) )  // パターン2のみ
        {
            Link8SquareBlendOutside(    info,
                                        info->in_target+in_width,
                                        info->out_target+out_width,
                                        1,
                                        in_width,
                                        out_width,
                                        1, in_height-2, info->j );
        }
        else if( flg & (1<<3) )         // パターン3のみ
        {
            Link8SquareBlendOutside(    info,
                                        info->in_target+in_width,
                                        info->out_target+out_width,
                                        -1,
                                        in_width,
                                        out_width,
                                        1, in_height-2, info->j );
        }
    }
}




// 明示的インスタンス化
template void Link8Mode01Execute<PF_Pixel8>(BlendingInfo<PF_Pixel8> *pInfo);
template void Link8Mode01Execute<PF_Pixel16>(BlendingInfo<PF_Pixel16> *pInfo);

template void Link8Mode02Execute<PF_Pixel8>(BlendingInfo<PF_Pixel8> *pInfo);
template void Link8Mode02Execute<PF_Pixel16>(BlendingInfo<PF_Pixel16> *pInfo);

template void Link8Mode03Execute<PF_Pixel8>(BlendingInfo<PF_Pixel8> *pInfo);
template void Link8Mode03Execute<PF_Pixel16>(BlendingInfo<PF_Pixel16> *pInfo);

template void Link8Mode04Execute<PF_Pixel8>(BlendingInfo<PF_Pixel8> *pInfo);
template void Link8Mode04Execute<PF_Pixel16>(BlendingInfo<PF_Pixel16> *pInfo);

template void Link8SquareExecute<PF_Pixel8>( BlendingInfo<PF_Pixel8> *info );
template void Link8SquareExecute<PF_Pixel16>( BlendingInfo<PF_Pixel16> *info );











