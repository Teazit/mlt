#ifndef MLTPP_SWITCHER_H
#define MLTPP_SWITCHER_H

#include "MltConfig.h"

#include <framework/mlt.h>

#include "MltProducer.h"

namespace Mlt{
    class Producer;
    class Multitrack;
    class Profile;

    class MLTPP_DECLSPEC Switcher : public Producer{
        private:
            mlt_switcher instance;
        public:
            Switcher( );
			Switcher( Profile& profile );
			Switcher( Service &switcher );
			Switcher( mlt_switcher switcher );
			Switcher( Switcher &switcher );
			Switcher( Profile& profile, char *id, char *arg = NULL );
			virtual ~Switcher( );
			virtual mlt_switcher get_switcher( );
			mlt_producer get_producer( );
			Multitrack *multitrack( );
			void refresh( );
			int set_track( Producer &producer, int index );
			int insert_track( Producer &producer, int index );
			int remove_track( int index );
			Producer *track( int index );
			int count( );
			bool locate_cut( Producer *producer, int &track, int &cut );
			int connect( Producer &producer );;

			/* TODO : Add method relative to the switch
            */
    };
}

#endif