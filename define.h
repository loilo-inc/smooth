//---------------------------------------------------------------------------------------//
// 概要: smooth内でいろいろ使う関数など
// 更新: 2003 4/21
// 
//---------------------------------------------------------------------------------------//

#ifndef __DEFINE_H
#define __DEFINE_H

// 型 //
typedef unsigned char   u_char;
typedef unsigned int    u_int;
typedef unsigned short  u_short;
typedef unsigned long   u_long;



// AfterEffectsはARGB


struct Cinfo
{
    int     length;     // カウントした画素数
    float   start, end; // 実際にブレンドする座標値(その該当する方向のものしか有効でない)
    u_int   flg;        // 特殊処理用のフラグ

    Cinfo()  {length = 0; flg = 0;} // コンストラクタ
};

// ↑のフラグ //
#define CR_FLG_FILL         (1<<0) // べた塗りモード


// 合成関数用の情報用構造体 //
template <typename PixelType>
struct BlendingInfo
{
    PF_LayerDef         *input, *output;    // 入出力画像
	PixelType			*in_ptr, *out_ptr;	// 画像データへのポインタ
    int                 i,j;                // 現在処理中の座標値
    long                in_target,          // 処理中の座標値の1次配列インデックス  (inputに対する)
                        out_target;         //                                      (outputに対する)
    Cinfo               core[4];            // 処理すべき長さ、フラグなどのコアの情報 0:left 1:right 2:top 3:bottom
    int                 flag;               // 制御用のフラグ(補正用のカウントなど)
    unsigned int        range;              // 同じ色とみなす範囲
    int                 mode;               // 処理するモード

    float               LineWeight;         // ラインの太さ
};

// ↑のフラグ
#define SECOND_COUNT    (1<<0)      // 補正用の2度目のカウント

// ↑のMode //
enum
{
    BLEND_MODE_UP_H = 0,            // 上向きの角の横方向
    BLEND_MODE_UP_V,                // 上向きの角の縦方向
    BLEND_MODE_DOWN_H,              // 下向きの角の横方向
    BLEND_MODE_DOWN_V,              // 下向きの角の縦方向
};


#endif
