#include "mlt_switcher.h"
#include "mlt_frame.h"
#include "mlt_multitrack.h"
#include "mlt_log.h"
#include "mlt_transition.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Forward references to static methods.
*/

static int producer_get_frame( mlt_producer parent, mlt_frame_ptr frame, int track );
static void mlt_switcher_listener( mlt_multitrack tracks, mlt_switcher self );

/** Construct a switcher without a multitrack.
 *
 * Sets the resource property to "<switcher>", the mlt_type to "mlt_producer",
 * and mlt_service to "switcher".
 *
 * \public \memberof mlt_switcher_s
 * \return the new switcher
 */

mlt_switcher mlt_switcher_init( )
{
	mlt_switcher self = calloc( 1, sizeof( struct mlt_switcher_s ) );
	if ( self != NULL )
	{
		mlt_producer producer = &self->parent;
		if ( mlt_producer_init( producer, self ) == 0 )
		{
			mlt_properties properties = MLT_PRODUCER_PROPERTIES( producer );

			mlt_properties_set( properties, "resource", "<switcher>" );
			mlt_properties_set( properties, "mlt_type", "mlt_producer" );
			mlt_properties_set( properties, "mlt_service", "switcher" );
			mlt_properties_set_int( properties, "in", 0 );
			mlt_properties_set_int( properties, "out", -1 );
			mlt_properties_set_int( properties, "length", 0 );

			producer->get_frame = producer_get_frame;
			producer->close = ( mlt_destructor )mlt_switcher_close;
			producer->close_object = self;
		}
		else
		{
			free( self );
			self = NULL;
		}
	}
	return self;
}

/** Construct a switcher as well as a multitrack.
 *
 * Sets the resource property to "<switcher>", the mlt_type to "mlt_producer",
 * and mlt_service to "switcher".
 *
 * \public \memberof mlt_switcher_s
 * \return the new switcher
 */

mlt_switcher mlt_switcher_new( )
{
    mlt_switcher self = calloc( 1, sizeof( struct mlt_switcher_s ) );
	if ( self != NULL )
	{
		mlt_producer producer = &self->parent;
		if ( mlt_producer_init( producer, self ) == 0 )
		{
			mlt_properties properties = MLT_PRODUCER_PROPERTIES( producer );
			mlt_multitrack multitrack = mlt_multitrack_init( );

			mlt_properties_set( properties, "resource", "<switcher>" );
			mlt_properties_set( properties, "mlt_type", "mlt_producer" );
			mlt_properties_set( properties, "mlt_service", "switcher" );
			mlt_properties_set_int( properties, "in", 0 );
			mlt_properties_set_int( properties, "out", -1 );
			mlt_properties_set_int( properties, "length", 0 );

			mlt_properties_set_data( properties, "multitrack", multitrack, 0, (mlt_destructor)mlt_multitrack_close, NULL);
			mlt_events_listen( MLT_MULTITRACK_PROPERTIES(multitrack), self, "producer-changed", (mlt_listener)mlt_switcher_listener);

			producer->get_frame = producer_get_frame;
			producer->close = ( mlt_destructor )mlt_switcher_close;
			producer->close_object = self;
		}
		else
		{
			free( self );
			self = NULL;
		}
	}
	return self;
}

/** Get the service object associated to the switcher.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 * \return the parent service object
 * \see MLT_SWITCHER_SERVICE
 */

mlt_service mlt_switcher_service( mlt_tractor self )
{
	return MLT_PRODUCER_SERVICE( &self->parent );
}

/** Get the producer object associated to the switcher.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 * \return the parent producer object
 * \see MLT_SWITCHER_PRODUCER
 */

mlt_producer mlt_switcher_producer( mlt_switcher self )
{
	return self != NULL ? &self->parent : NULL;
}

/** Get the properties object associated to the switcher.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 * \return the switcher's property list
 * \see MLT_SWITCHER_PROPERTIES
 */

mlt_properties mlt_switcher_properties( mlt_switcher self )
{
	return MLT_PRODUCER_PROPERTIES( &self->parent );
}

/** Get the multitrack a switcher is pulling.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 * \return a multitrack or NULL if there is none
 */

mlt_multitrack mlt_switcher_multitrack( mlt_switcher self )
{
	return mlt_properties_get_data( MLT_SWITCHER_PROPERTIES( self ), "multitrack", NULL );
}

/** Ensure the switcher in/out points match the multitrack.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 */

void mlt_switcher_refresh( mlt_switcher self )
{
	mlt_multitrack multitrack = mlt_switcher_multitrack( self );
	mlt_properties multitrack_props = MLT_MULTITRACK_PROPERTIES( multitrack );
	mlt_properties properties = MLT_SWITCHER_PROPERTIES( self );
	mlt_events_block( multitrack_props, properties );
	mlt_events_block( properties, properties );
	mlt_multitrack_refresh( multitrack );
	mlt_properties_set_position( properties, "in", 0 );
	mlt_properties_set_position( properties, "out", mlt_properties_get_position( multitrack_props, "out" ) );
	mlt_events_unblock( properties, properties );
	mlt_events_unblock( multitrack_props, properties );
	mlt_properties_set_position( properties, "length", mlt_properties_get_position( multitrack_props, "length" ) );
}

/* The listener to changes made to the internal multitrack.
*/
static void mlt_switcher_listener( mlt_multitrack tracks, mlt_tractor self )
{
	mlt_switcher_refresh( self );
}

/** Connect the switcher.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 * \param producer a producer
 * \return true on error
 */

int mlt_switcher_connect( mlt_switcher self, mlt_service producer )
{
	int ret = mlt_service_connect_producer( MLT_SWITCHER_SERVICE( self ), producer, 0 );

	// This is the producer we're going to connect to
	if ( ret == 0 )
		self->producer = producer;

    return ret;
}

/** Set the producer for a specific track.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 * \param producer a producer
 * \param index the 0-based track index
 * \return true on error
 */

int mlt_switcher_set_track( mlt_switcher self, mlt_producer producer, int index )
{
	return mlt_multitrack_connect( mlt_switcher_multitrack( self ), producer, index );
}

/** Insert a producer before a specific track.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 * \param producer a producer
 * \param index the 0-based track index
 * \return true on error
 */

int mlt_switcher_insert_track( mlt_switcher self, mlt_producer producer, int index ){
    return mlt_multitrack_insert( mlt_switcher_multitrack( self ), producer, index );
}

/** Remove a track by its index.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 * \param index the 0-based track index
 * \return true on error
 */

int mlt_switcher_remove_track( mlt_switcher self, int index ){
    return mlt_multitrack_disconnect( mlt_switcher_multitrack( self ), index );
}

/** Get the producer for a specific track.
 *
 * \public \memberof mlt_switcherr_s
 * \param self a switcher
 * \param index the 0-based track index
 * \return the producer for track \p index
 */

mlt_producer mlt_switcher_get_track( mlt_switcher self, int index )
{
	return mlt_multitrack_track( mlt_switcher_multitrack( self ), index );
}

/** Get the next frame.
 *
 * TODO : Write it.
 *
 * \private \memberof mlt_switcher_s
 * \param parent the producer interface to the switcher
 * \param[out] frame a frame by reference
 * \param track the 0-based track index
 * \return true on error
 */

static int producer_get_frame( mlt_producer parent, mlt_frame_ptr frame, int track ){
    return 1;
}

/** Close the switcher and free its resources.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 */

void mlt_switcher_close( mlt_switcher self ){
    if(self != NULL && mlt_properties_dec_ref( MLT_TRACTOR_PROPERTIES( self ) ) <= 0){
        self->parent.close = NULL;
		mlt_producer_close( &self->parent );
		free( self );
    }
}