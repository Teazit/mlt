#ifndef MLT_SWITCHER_H
#define MLT_SWITCHER_H

#include "mlt_producer.h"

struct mlt_switcher_s
{
	struct mlt_producer_s parent;
	mlt_service producer;
};

#define MLT_SWITCHER_PRODUCER( switcher )		( &( switcher )->parent )
#define MLT_SWITCHER_SERVICE( switcher )		MLT_PRODUCER_SERVICE( MLT_SWITCHER_PRODUCER( switcher ) )
#define MLT_SWITCHER_PROPERTIES( switcher )	MLT_SERVICE_PROPERTIES( MLT_SWITCHER_SERVICE( switcher ) )

extern mlt_switcher mlt_switcher_init( );
extern mlt_switcher mlt_switcher_new( );
extern mlt_service mlt_switcher_service( mlt_switcher self );
extern mlt_producer mlt_switcher_producer( mlt_switcher self );
extern mlt_properties mlt_switcher_properties( mlt_switcher self );
extern mlt_multitrack mlt_switcher_multitrack( mlt_switcher self );
extern int mlt_switcher_connect( mlt_switcher self, mlt_service service );
extern void mlt_switcher_refresh( mlt_switcher self );
extern int mlt_switcher_set_track( mlt_switcher self, mlt_producer producer, int index );
extern int mlt_switcher_insert_track( mlt_switcher self, mlt_producer producer, int index );
extern int mlt_switcher_remove_track( mlt_switcher self, int index );
extern mlt_producer mlt_switcher_get_track( mlt_switcher self, int index );
extern void mlt_switcher_close( mlt_tractor self );

/* TODO : Add method relative to the switch
*/

#endif
