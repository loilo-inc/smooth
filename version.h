//---------------------------------------------------------------------------//
// 作成した人   : 杉山浩二 <sugiyama@dd.namco.co.jp>
// 作成日       : 2004年 8/19 19:50
// コメント     : エフェクトのバージョン
//---------------------------------------------------------------------------//

#ifndef __VERSION_H_
#define __VERSION_H_

#ifdef	_DEBUG
#define SK_STAGE_DEVELOP	(1)
#define SK_STAGE_BETA		(0)
#else
#define SK_STAGE_BETA		(0)
#define	SK_STAGE_RELEASE	(1)
#endif


#define NAME                "smooth"
#define HP                  "http://loilo.tv/"

#define MAJOR_VERSION       (1)
#define MINOR_VERSION       (4)
#define BUILD_VERSION       (0)


#endif
