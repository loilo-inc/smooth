//---------------------------------------------------------------------------------------//
// 概要: 上角系の処理
//---------------------------------------------------------------------------------------//


#ifndef	__UPMODE_H
#define	__UPMODE_H


// 左側の長さカウント //
template<typename PixelType>
void upMode_LeftCountLength(BlendingInfo<PixelType> *info);	// 左

template<typename PixelType>
void upMode_RightCountLength(BlendingInfo<PixelType> *info);	// 右

template<typename PixelType>
void upMode_TopCountLength( BlendingInfo<PixelType> *info);		// 上

template<typename PixelType>
void upMode_BottomCountLength( BlendingInfo<PixelType> *info);	// 下



// ブレンド //
template<typename PixelType>
void upMode_LeftBlending( 	BlendingInfo<PixelType>	*info);		// 左ブレンド

template<typename PixelType>
void upMode_RightBlending( 	BlendingInfo<PixelType>	*info);		// 右ブレンド

template<typename PixelType>
void upMode_TopBlending( 	BlendingInfo<PixelType>	*info);		// 上ブレンド

template<typename PixelType>
void upMode_BottomBlending(	BlendingInfo<PixelType>	*info);		// 下ブレンド

#endif
