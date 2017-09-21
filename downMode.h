//---------------------------------------------------------------------------------------//
// 概要: 下角系の処理
//---------------------------------------------------------------------------------------//


#ifndef	__DOWNMODE_H
#define	__DOWNMODE_H


// 長さカウント //
template<typename PixelType>
void downMode_LeftCountLength(BlendingInfo<PixelType> *info);	// 左

template<typename PixelType>
void downMode_RightCountLength(BlendingInfo<PixelType> *info);	// 右

template<typename PixelType>
void downMode_TopCountLength( BlendingInfo<PixelType> *info);		// 上

template<typename PixelType>
void downMode_BottomCountLength( BlendingInfo<PixelType> *info);	// 下




// ブレンド //
template<typename PixelType>
void downMode_LeftBlending( 	BlendingInfo<PixelType>	*info);		// 左ブレンド

template<typename PixelType>
void downMode_RightBlending( 	BlendingInfo<PixelType>	*info);		// 右ブレンド

template<typename PixelType>
void downMode_TopBlending( 		BlendingInfo<PixelType>	*info);		// 上ブレンド

template<typename PixelType>
void downMode_BottomBlending(	BlendingInfo<PixelType>	*info);		// 下ブレンド


#endif
