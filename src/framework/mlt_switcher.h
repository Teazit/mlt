#ifndef MLT_SWITCHER_H
#define MLT_SWITCHER_H

#include "mlt_producer.h"
/*
    mpicard : Here you will find some choice i have made, before really documenting them.

    -> As MLT's dev' do, I will use the mlt_properties for all my variable.
       So you will find :
            - "auto_switch" : boolean, to use or not the auto_switch
            - "nb_frame_switch" : int, the number of frame to get before
            switching.
            - "_current_track" : int, the displayed track
            - "_frame_count" : int, the number of frame requested. Reset to 0
            everytime it hit the value of "nb_frame_switch", and change the
            "_current_track".
*/

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
extern void mlt_switcher_close( mlt_switcher self );
extern void mlt_switcher_set_current_track(mlt_switcher self, int track);

#endif
