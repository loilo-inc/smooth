//---------------------------------------------------------------------------------------//
// 概要: 上角系の処理
//---------------------------------------------------------------------------------------//


#include "util.h"
#include "define.h"

#include "upMode.h"

#include <stdio.h>
#include <math.h>



//---------------------------------------------------------------------------------------//
// 名前:        upMode_????CountLength
// 概要:        アンチを掛ける長さをカウント
// 引数:        info : カウント情報の構造体
// 返り値:      なし
//---------------------------------------------------------------------------------------//

///// Left /////////////
template<typename PixelType>
void upMode_LeftCountLength(BlendingInfo<PixelType> *info)
{
    PF_LayerDef *output         = info->output;

    long        count_target = 0;
    int         len = 1,
                width           = GET_WIDTH(info->input),
                height          = output->height;
    u_int       *flg            = &info->core[0].flg;

    while(1)
    {
        count_target = info->in_target - (len-1);   // 検査するターゲットを変更
        
        // べた塗り系か？ //
        if( ComparePixel(count_target, count_target-1))
        {   ///////////// 下側走査 ///////////////////////////////////

            info->core[0].start = (float)(info->i+1);   // 画像の座標は左上が(0,0), でも論理線は右からだから+1
            info->core[0].end   = (float)(info->i+1) - (float)len;

            // 特殊処理するよフラグなりなんなり立てる //
            (*flg) |= CR_FLG_FILL;

            break;
        }


        count_target = info->in_target - width - (len - 1); // 検査するターゲットを変更

        if( ComparePixel(count_target, count_target-1))
        {   ///////////// 上側走査 ///////////////////////////////////

            info->core[0].start = (float)(info->i+1);   // 画像の座標は左上が(0,0), でも論理線は右からだから+1
            info->core[0].end   = (float)(info->i+1) - (float)len;

            if(  width-2 > info->i && info->i > 2 &&
                 height-2 > info->j && info->j > 2)
            {
                //// end値修正が必要か？ ////
                //// 線上の1つ上の角のレングスを調べ、その差が１だったら補正 ////
                if( !(info->flag & SECOND_COUNT) &&
                    ComparePixel(count_target-1, count_target-1 - width))       // 角かどうか調べる //
                {
                    // そこでカウント //
                    BlendingInfo<PixelType>    sc_info;

                    sc_info             = *info;            // コピーして初期化

                    sc_info.i           = info->i-len;
                    sc_info.j           = info->j-1;
                    sc_info.out_target  = 
                    sc_info.in_target   = sc_info.j*width + sc_info.i;
                    sc_info.flag        = SECOND_COUNT;

                    upMode_LeftCountLength<PixelType>(&sc_info);
                    
                    
                    if(sc_info.core[0].length - len == 1)
                    {
                        info->core[0].end   -= 0.5f;    // 半ピクセル左にあるものと補正 // 

                    }
                    
#if 0
                    if(len - sc_info.core[0].length == 1)
                    {
                        info->core[0].end   -= 0.5f;    // 半ピクセル右にあるものと補正 // 
                    }
#endif

                }
            }

            break;
        }

        
        len++;

        // x = 0の点まで行ったら終り //
        if(info->i - len <= 1)
        {
            len = info->i - 1;

            info->core[0].start = (float)(info->i+1);   // 画像の座標は左上が(0,0), でも論理線は右からだから+1
            info->core[0].end   = (float)(info->i+1) - (float)len;
            break;
        }
    }

    info->core[0].length = len;
}



///// Right /////////////
template<typename PixelType>
void upMode_RightCountLength(BlendingInfo<PixelType> *info)
{
    PF_LayerDef *output         = info->output;

    long        count_target = 0;
    int         len = 0,
                width           = GET_WIDTH(info->input),
                height          = output->height;
    u_int       *flg            = &info->core[1].flg;

    // 始めの1回は左の方だけ検査 //
    count_target = info->in_target+width;   // 検査するターゲットを変更
    
    if( ComparePixel(count_target, count_target+1) &&
        ComparePixelEqual( info->in_target+1, info->in_target+1+width ))
    {
        info->core[1].length = 0;
        return;
    }
    else
    {
        len++;
        
        if((info->i+1) + len >= (width-1))
        {
            len = width-1 - (info->i+1);

            info->core[1].start     = (float)(info->i+1);   // 画像の座標は左上が(0,0), でも論理線は右からだから+1
            info->core[1].end       = (float)(info->i+1) + (float)len;
            info->core[1].length    = len;
            return;
        }
    }

    while(1)
    {
        count_target = info->in_target+len; // 検査するターゲットを変更
        // べた塗り系か？ //
        if( ComparePixel(count_target, count_target+1))
        {
            info->core[1].start = (float)(info->i+1);   // 画像の座標は左上が(0,0), でも論理線は右からだから+1
            info->core[1].end   = (float)(info->i+1 + len);
                
            // 特殊処理するよフラグなりなんなり立てる //
            (*flg) |= CR_FLG_FILL;
            
            break;
        }



        count_target = info->in_target+width+len;   // 検査するターゲットを変更
        
        if( ComparePixel(count_target, count_target+1))
        {
            info->core[1].start     = (float)(info->i+1);   // 画像の座標は左上が(0,0), でも論理線は右からだから+1
            info->core[1].end       = (float)(info->i+1 + len);

            if(  width-2 > info->i && info->i > 2 &&
                 height-2 > info->j && info->j > 2)
            {
                //// end値修正が必要か？ ////
                //// 線上の1つ下の角のレングスを調べ、その差が１だったら補正 ////
                // 角かどうか調べる //
                if( !(info->flag & SECOND_COUNT) &&
                    ComparePixel(count_target, count_target+1))     // 角か？
                {
                    // そこでカウント //
                    BlendingInfo<PixelType>    sc_info;

                    sc_info             = *info;            // コピーして初期化

                    sc_info.i           = info->i+len;
                    sc_info.j           = info->j+1;
                    sc_info.out_target  =
                    sc_info.in_target   = sc_info.j*width + sc_info.i;
                    sc_info.flag        = SECOND_COUNT;

                    upMode_RightCountLength<PixelType>(&sc_info);

                    if(len - sc_info.core[1].length == 1 && sc_info.core[1].length != 0)
                    {
                        info->core[1].end   -= 0.5f;    // 半ピクセル右にあるものと補正 // 
                    }
                    

                }
            }

            break;
        }
        
        
        len++;

        // x = 0の点まで行ったら終り //
        if((info->i+1) + len >= (width-1))
        {
            len = width-1 - (info->i+1);
            info->core[1].start = (float)(info->i+1);   // 画像の座標は左上が(0,0), でも論理線は右からだから+1
            info->core[1].end   = (float)(info->i+1) + (float)len;
            break;
        }
    }

    info->core[1].length = len;
}









///// top /////////////
template<typename PixelType>
void upMode_TopCountLength( BlendingInfo<PixelType> *info)
{
    PF_LayerDef *output         = info->output;

    long        count_target = 0;
    int         len = 0,
                width           = GET_WIDTH(info->input),
                height          = output->height;
    u_int       *flg            = &info->core[2].flg;

    //----------------------------------------------------------//
    //              上側(top_len)の長さカウント             //
    //----------------------------------------------------------//
    // 始めの1回は左の方だけ検査 //
    count_target = info->in_target-1;   // 検査するターゲットを変更
    
    if( ComparePixel(count_target, count_target-width) &&
        ComparePixelEqual( info->in_target-width, info->in_target-1-width ))
    {
        info->core[2].length = 0;
        info->core[2].start = (float)(info->j);
        info->core[2].end   = info->core[2].start;
        return;
    }
    else    
    {
        len++;
    
        if(info->j - len <= 1)
        {
            len = info->j-1;    
            
            info->core[2].start     = (float)(info->j);
            info->core[2].end       = (float)(info->j - len);
            info->core[2].length    = len;
            return;
        }
    
    }
    

    while(1)
    {
        count_target = info->in_target - (len * width); // 検査するターゲットを変更
        
        ///////////// べた塗り系か？ //
        if( ComparePixel(count_target, count_target-width))
        {
            info->core[2].start = (float)(info->j);
            info->core[2].end   = (float)(info->j - len);

            // べた塗りです //
            //DEBUG_PIXEL( info->in_target, DEBUG_COL_BLUE);
                
            (*flg) |= CR_FLG_FILL;
            
            break;
        }



        count_target = info->in_target - ( len * width) - 1;    // 検査するターゲットを変更
        
        ///////// 1ピクセルずつ調べていって違う色になった？ //
        if( ComparePixel(count_target, count_target-width))
        {
            info->core[2].start     = (float)(info->j);
            info->core[2].end       = (float)(info->j - len);
            
            if(  width-2 > info->i && info->i > 2 &&
                 height-2 > info->j && info->j > 2)
            {
                //// end値修正が必要か？ ////
                //// 線上の1つ下の角のレングスを調べ、その差が１だったら補正 ////
                // 角かどうか調べる 非突起 //
                if( !(info->flag & SECOND_COUNT) &&
                      ComparePixel(count_target, count_target+1))
                {
                    // そこでカウント //
                    BlendingInfo<PixelType>    sc_info;

                    sc_info             = *info;            // コピーして初期化

                    sc_info.i           = info->i-1;
                    sc_info.j           = info->j-len;
                    sc_info.out_target  =
                    sc_info.in_target   = sc_info.j*width + sc_info.i;
                    sc_info.flag        = SECOND_COUNT;

                    upMode_TopCountLength<PixelType>(&sc_info);
                    
                    if(len - sc_info.core[2].length == 1 && sc_info.core[2].length != 0)
                    {
                        info->core[2].end   += 0.5f;    // 半ピクセル右にあるものと補正 // 
                        
                    }
                    
                }
            }

            break;
        }
        
        
        len++;

        // y = 0の点(画像の一番上)まで行ったら終了 //
        if(info->j - len <= 1)
        {
            len = info->j-1;    
            
            info->core[2].start = (float)(info->j);
            info->core[2].end   = (float)(info->j - len);

            break;
        }
    }

    // コピー //
    info->core[2].length = len;
}





//// Bottom ////////////////////
template<typename PixelType>
void upMode_BottomCountLength( BlendingInfo<PixelType> *info)
{
    PF_LayerDef *output         = info->output;

    long        count_target = 0;
    int         len = 1,
                width           = GET_WIDTH(info->input),
                height          = output->height;
    u_int       *flg            = &info->core[3].flg;

    //----------------------------------------------------------//
    //              下側(bottom_len)の長さカウント              //
    //----------------------------------------------------------//
    while(1)
    {
        count_target = info->in_target + (len-1)*width; // 検査するターゲットを変更
        
        // べた塗り系か？ //
        if( ComparePixel(count_target, count_target+width))
        {
            info->core[3].start     = (float)(info->j);
            info->core[3].end       = (float)(info->j + len);

            // べた塗りです //
            //DEBUG_PIXEL( info->in_target, DEBUG_COL_BLUE);
                    
            (*flg) |= CR_FLG_FILL;

            break;
        }


        count_target = info->in_target + (len-1)*width + 1; // 検査するターゲットを変更

        if( ComparePixel(count_target, count_target+width))
        {
            info->core[3].start     = (float)(info->j);
            info->core[3].end       = (float)(info->j + len);
            
            if(  width-2 > info->i && info->i > 2 &&
                 height-2 > info->j && info->j > 2)
            {
                //// end値修正が必要か？ ////
                //// 線上の1つ下の角のレングスを調べ、その差が１だったら補正 ////
                // 角かどうか調べる //
                if( !(info->flag & SECOND_COUNT) &&
                    ComparePixel(count_target+width, count_target+width+1)) // 角か？
                {
                    // そこでカウント //
                    BlendingInfo<PixelType>    sc_info;

                    sc_info             = *info;            // コピーして初期化
                    
                    sc_info.i           = info->i+1;
                    sc_info.j           = info->j+len;
                    sc_info.out_target  =
                    sc_info.in_target   = sc_info.j*width + sc_info.i;
                    sc_info.flag        = SECOND_COUNT;

                    upMode_BottomCountLength<PixelType>(&sc_info);
                    
                    //fprintf(debug_fp, "bot  : %d\n", sc_info.core[3].length);
                    //fprintf(debug_fp, "len  : %d\n", len);

                    if( sc_info.core[3].length - len == 1)
                    {
                        info->core[3].end   += 0.5f;    // 半ピクセル右にあるものと補正 // 
                        
                        //fprintf(debug_fp, "<%d, %d>\n", info->i, info->j);
                    }
                    
                    //fprintf(debug_fp, "---------------\n");
                }
            }

            break;
        }

        

        len++;

        // x = 0の点まで行ったら終り //
        if(info->j + len >= height-1)
        {
            len = height-1 - info->j;
            
            info->core[3].start     = (float)(info->j);
            info->core[3].end       = (float)(info->j + len);
            break;
        }
    }

    info->core[3].length = len;
}






//---------------------------------------------------------------------------------------//
//名前:     upMode_????Blending()
//概要:     実際にアンチをかける
//引数:     info : カウント情報の構造体
//返り値:   なし
//---------------------------------------------------------------------------------------//

/// Left //////////////////////////////
template<typename PixelType>
void upMode_LeftBlending( BlendingInfo<PixelType>  *info)
{
    long        t;
    int         in_width    = GET_WIDTH(info->input);
    float       start       = info->core[0].start;
    float       end         = info->core[0].end;

    // 通常Length の値は半分にしてつかうけど今回は単位が1/2ピクセル単位なので2倍、len*(1/2)*2=len
    // でそのまま使える
#if 0
    if(flg & CR_FLG_FILL)
    {
        //----------------------//
        //      べた塗り        //
        //----------------------//
        int t;

        blend_target = target;

        for(t=0;t<len;t++)
        {
            Blendingf(in_ptr, out_ptr, blend_target, blend_target-width, 0.8f);
            blend_target--;
        }
    
    }
#endif
    {
        //--------------//
        //      通常    //
        //--------------//
        long    blend_target = 0, out_target = 0;
        int     blend_count;
        float   pre_ratio,  // 一つ前の三角の面積比
                ratio,      // 現行の面積比
                l,          // 底辺の長さ
                len;        // 全体の底辺の長さ
        int     end_p;      // 開始ピクセル(名前がendなのはinfo->end系だから)
        
        end_p = (int)end;

        len = start - end;

        // 何ピクセルブレンドするのか？切り上げ //
        blend_count = CEIL((float)(info->i+1) - end);
        
        // 前回の割合 初期化 //
        pre_ratio = 0.0f;
        
        // ブレンドするターゲットの初期化 //
        blend_target    = info->in_target   - (blend_count-1);
        out_target      = info->out_target  - (blend_count-1);

        // 底辺×高さ÷２ = 底辺×((底辺/全体の底辺)×全体の高さ)÷２ = l * l * 0.5 * 0.5 / len //
        // 左から計算してく //
        for(t=0;t<blend_count;t++)
        {
            l       = (float)(end_p+1+t) - end;
            ratio   = (l * l * 0.5f * 0.5f) / len;

            // ブレンド //
            Blendingf(	info->in_ptr,
						info->out_ptr,
						blend_target,
						blend_target - in_width, 
						out_target,
						1.0f-(ratio - pre_ratio));
            
            pre_ratio = ratio;
            
            blend_target++;
            out_target++;
        }
    }
}


/// Right ///////////////////////////////////////
template<typename PixelType>
void upMode_RightBlending(BlendingInfo<PixelType>  *info)
{
    long        t;
    long        length      = info->core[1].length;
    float       start       = info->core[1].start;
    float       end         = info->core[1].end;
    int         in_width    = GET_WIDTH(info->input);

#if 0
    else if(flg & CR_FLG_FILL)
    {
        //----------------------//
        //      べた塗り        //
        //----------------------//
    }
#endif
    {
        //--------------//
        //      通常    //
        //--------------//
        if(length > 0)
        {
            long    blend_target = 0, out_target = 0;
            int     blend_count;
            float   pre_ratio,  // 一つ前の三角の面積比
                    ratio,      // 現行の面積比
                    l,          // 底辺の長さ
                    len;        // 全体の底辺の長さ
            int     end_p;      // 開始ピクセル(名前がendなのはinfo->end系だから)
            
            end_p = (int)(end-0.000001);    // 4.0fなどの丁度のところでおかしくなるのの苦肉の策

            len = end - start;

            // 何ピクセルブレンドするのか？切り上げ //
            blend_count = CEIL(end - (float)(info->i+1));
            
            
            // 前回の割合 初期化 //
            pre_ratio = 0.0f;

            // ブレンドするターゲットの初期化 //
            blend_target    = info->in_target   + blend_count;
            out_target      = info->out_target  + blend_count;
            

            // 右から計算してく //
            for(t=0;t<blend_count;t++)
            {
                l       = end - (float)(end_p-t);
                ratio   = (l * l * 0.5f * 0.5f) / len;

                // ブレンド //
                Blendingf(	info->in_ptr,
							info->out_ptr,
							blend_target, 
							blend_target+in_width, 
							out_target, 
							1.0f - (ratio - pre_ratio));
                
                pre_ratio = ratio;

                blend_target--;
                out_target--;
            }
        }
    }
}



/// Top /////////////////////////////////////////
template<typename PixelType>
void upMode_TopBlending( BlendingInfo<PixelType> *info)
{
    long        t;
    long        length      = info->core[2].length;
    float       start       = info->core[2].start;
    float       end         = info->core[2].end;
    int         in_width    = GET_WIDTH(info->input);
    int         out_width   = GET_WIDTH(info->output);


    // 通常Length の値は半分にしてつかうけど今回は単位が半ピクセル単位なので2倍、len*(1/2)*2=len
    // でそのまま使える
    
    
#if 0
    else if(flg->top & CR_FLG_FILL)
    {
        //----------------------//
        //      べた塗り        //
        //----------------------//
    }
#endif
    {    ////////// 通常 //////////////////////////////
        if(length > 0)
        {
            long    blend_target = 0, out_target = 0;
            int     blend_count;
            float   pre_ratio,  // 一つ前の三角の面積比
                    ratio,      // 現行の面積比
                    l,          // 底辺の長さ
                    len;        // 全体の底辺の長さ
            int     end_p;      // 開始ピクセル(名前がendなのはinfo->end系だから)
            
            end_p = (int)(end); // 4.0fなどの丁度のところでおかしくなるのの苦肉の策

            len = start - end;

            // 何ピクセルブレンドするのか？切り上げ //
            blend_count = CEIL((float)(info->j) - end);
            
            // 前回の割合 初期化 //
            pre_ratio = 0.0f;

            // ブレンドするターゲットの初期化 //
            blend_target    = info->in_target   - blend_count*in_width;
            out_target      = info->out_target  - blend_count*out_width;

            // 上から計算してく //
            for(t=0;t<blend_count;t++)
            {
                l       = (float)(end_p+1+t) - end;
                ratio   = (l * l * 0.5f * 0.5f) / len;

                // ブレンド //
                Blendingf(	info->in_ptr,
							info->out_ptr,
							blend_target,
							blend_target-1, 
							out_target, 
							1.0f - (ratio - pre_ratio));
                
                pre_ratio = ratio;

                blend_target+= in_width;
                out_target  += out_width;
            }

        }
    }
}




/// Bottom /////////////////////////////////////////
template<typename PixelType>
void upMode_BottomBlending( BlendingInfo<PixelType> *info)
{
    long        t;
    float       start       = info->core[3].start;
    float       end         = info->core[3].end;
    int         in_width    = GET_WIDTH(info->input);
    int         out_width   = GET_WIDTH(info->output);

#if 0
    else if(flg & CR_FLG_FILL)
    {
        //----------------------//
        //      べた塗り        //
        //----------------------//
    }
#endif
    {   ///////// 通常 /////////////////////
        long    blend_target = 0, out_target = 0;
        int     blend_count;
        float   pre_ratio,  // 一つ前の三角の面積比
                ratio,      // 現行の面積比
                l,          // 底辺の長さ
                len;        // 全体の底辺の長さ
        int     end_p;      // 開始ピクセル(名前がendなのはinfo->end系だから)
        
        end_p = (int)(end - 0.00001);

        len = end - start;

        // 何ピクセルブレンドするのか？切り上げ //
        blend_count = CEIL(end -(float)(info->j));
        


        // 前回の割合 初期化 //
        pre_ratio = 0.0f;

        // ブレンドするターゲットの初期化 //
        blend_target    = info->in_target + (blend_count-1)*in_width;   // info->targetが最終処理ピクセルなので-1
        out_target      = info->out_target + (blend_count-1)*out_width; // info->targetが最終処理ピクセルなので-1

        // 下から計算してく //
        for(t=0;t<blend_count;t++)
        {
            l       = end - (float)(end_p-t);
            ratio   = (l * l * 0.5f * 0.5f) / len;


            // ブレンド //
            Blendingf(	info->in_ptr,
						info->out_ptr, 
						blend_target,
						blend_target+1,
						out_target, 
						1.0f-(ratio - pre_ratio));
            
            pre_ratio = ratio;

            blend_target-= in_width;
            out_target  -= out_width;
        }
        
    }
}


// 明示的インスタンス化宣言
template void upMode_LeftCountLength<PF_Pixel8>(BlendingInfo<PF_Pixel8> *info);
template void upMode_LeftCountLength<PF_Pixel16>(BlendingInfo<PF_Pixel16> *info);

template void upMode_RightCountLength<PF_Pixel8>(BlendingInfo<PF_Pixel8> *info);
template void upMode_RightCountLength<PF_Pixel16>(BlendingInfo<PF_Pixel16> *info);

template void upMode_TopCountLength<PF_Pixel8>(BlendingInfo<PF_Pixel8> *info);
template void upMode_TopCountLength<PF_Pixel16>(BlendingInfo<PF_Pixel16> *info);

template void upMode_BottomCountLength<PF_Pixel8>(BlendingInfo<PF_Pixel8> *info);
template void upMode_BottomCountLength<PF_Pixel16>(BlendingInfo<PF_Pixel16> *info);


template void upMode_LeftBlending<PF_Pixel8>( BlendingInfo<PF_Pixel8> *info);
template void upMode_LeftBlending<PF_Pixel16>(BlendingInfo<PF_Pixel16> *info);

template void upMode_RightBlending<PF_Pixel8>( BlendingInfo<PF_Pixel8> *info);
template void upMode_RightBlending<PF_Pixel16>(BlendingInfo<PF_Pixel16> *info);

template void upMode_TopBlending<PF_Pixel8>( BlendingInfo<PF_Pixel8> *info);
template void upMode_TopBlending<PF_Pixel16>(BlendingInfo<PF_Pixel16> *info);

template void upMode_BottomBlending<PF_Pixel8>( BlendingInfo<PF_Pixel8> *info);
template void upMode_BottomBlending<PF_Pixel16>(BlendingInfo<PF_Pixel16> *info);

