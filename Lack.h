//---------------------------------------------------------------------------//
// コメント     : 2pixel以上の欠けているような状態の角の処理
//---------------------------------------------------------------------------//


#ifndef __LACK_H
#define __LACK_H

template<typename PixelType>
void LackMode01Execute( BlendingInfo<PixelType> *info );

template<typename PixelType>
void LackMode02Execute( BlendingInfo<PixelType> *info );

template<typename PixelType>
void LackMode0304Execute( BlendingInfo<PixelType> *info );


#endif


