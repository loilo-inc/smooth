//---------------------------------------------------------------------------------------//
// 概要: 8連結系の処理
//---------------------------------------------------------------------------------------//

#ifndef __8LINK_H
#define __8LINK_H



template<typename PixelType>
void Link8Mode01Execute(BlendingInfo<PixelType> *pInfo);

template<typename PixelType>
void Link8Mode02Execute(BlendingInfo<PixelType> *pInfo);

template<typename PixelType>
void Link8Mode03Execute(BlendingInfo<PixelType> *pInfo);

template<typename PixelType>
void Link8Mode04Execute(BlendingInfo<PixelType> *pInfo);

template<typename PixelType>
void Link8SquareExecute( BlendingInfo<PixelType> *info );

#endif
