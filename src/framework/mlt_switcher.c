#include "mlt_switcher.h"
#include "mlt_frame.h"
#include "mlt_multitrack.h"
#include "mlt_log.h"
#include "mlt_transition.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_NB_FRAME_SWITCH 1000
#define DEFAULT_AUTO_SWITCH 0

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

            mlt_properties_set_int(properties, "auto_switch", DEFAULT_AUTO_SWITCH);
		    mlt_properties_set_int(properties, "nb_frame_switch", DEFAULT_NB_FRAME_SWITCH);
		    mlt_properties_set_int(properties, "current_track", 0);
		    mlt_properties_set_int(properties, "_frame_count", 0);

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


            mlt_properties_set_int(properties, "auto_switch", DEFAULT_AUTO_SWITCH);
		    mlt_properties_set_int(properties, "nb_frame_switch", DEFAULT_NB_FRAME_SWITCH);
		    mlt_properties_set_int(properties, "current_track", 0);
		    mlt_properties_set_int(properties, "_frame_count", 0);

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

mlt_service mlt_switcher_service( mlt_switcher self )
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
static void mlt_switcher_listener( mlt_multitrack tracks, mlt_switcher self )
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

static int producer_get_image( mlt_frame self, uint8_t **buffer, mlt_image_format *format, int *width, int *height, int writable )
{
	uint8_t *data = NULL;
	int size = 0;
	mlt_properties properties = MLT_FRAME_PROPERTIES( self );
	mlt_frame frame = mlt_frame_pop_service( self );
	mlt_properties frame_properties = MLT_FRAME_PROPERTIES( frame );
	mlt_properties_set( frame_properties, "rescale.interp", mlt_properties_get( properties, "rescale.interp" ) );
	mlt_properties_set_int( frame_properties, "resize_alpha", mlt_properties_get_int( properties, "resize_alpha" ) );
	mlt_properties_set_int( frame_properties, "distort", mlt_properties_get_int( properties, "distort" ) );
	mlt_properties_set_int( frame_properties, "consumer_deinterlace", mlt_properties_get_int( properties, "consumer_deinterlace" ) );
	mlt_properties_set( frame_properties, "deinterlace_method", mlt_properties_get( properties, "deinterlace_method" ) );
	mlt_properties_set_int( frame_properties, "consumer_tff", mlt_properties_get_int( properties, "consumer_tff" ) );
	mlt_properties_set( frame_properties, "consumer_color_trc", mlt_properties_get( properties, "consumer_color_trc" ) );
	// WebVfx uses this to setup a consumer-stopping event handler.
	mlt_properties_set_data( frame_properties, "consumer", mlt_properties_get_data( properties, "consumer", NULL ), 0, NULL, NULL );

	mlt_frame_get_image( frame, buffer, format, width, height, writable );
	mlt_frame_set_image( self, *buffer, 0, NULL );

	mlt_properties_set_int( properties, "width", *width );
	mlt_properties_set_int( properties, "height", *height );
	mlt_properties_set_int( properties, "format", *format );
	mlt_properties_set_double( properties, "aspect_ratio", mlt_frame_get_aspect_ratio( frame ) );
	mlt_properties_set_int( properties, "progressive", mlt_properties_get_int( frame_properties, "progressive" ) );
	mlt_properties_set_int( properties, "distort", mlt_properties_get_int( frame_properties, "distort" ) );
	mlt_properties_set_int( properties, "colorspace", mlt_properties_get_int( frame_properties, "colorspace" ) );
	mlt_properties_set_int( properties, "force_full_luma", mlt_properties_get_int( frame_properties, "force_full_luma" ) );
	mlt_properties_set_int( properties, "top_field_first", mlt_properties_get_int( frame_properties, "top_field_first" ) );
	mlt_properties_set( properties, "color_trc", mlt_properties_get( frame_properties, "color_trc" ) );
	mlt_properties_set_data( properties, "movit.convert.fence",
		mlt_properties_get_data( frame_properties, "movit.convert.fence", NULL ),
		0, NULL, NULL );
	mlt_properties_set_data( properties, "movit.convert.texture",
		mlt_properties_get_data( frame_properties, "movit.convert.texture", NULL ),
		0, NULL, NULL );
	mlt_properties_set_int( properties, "movit.convert.use_texture", mlt_properties_get_int( frame_properties, "movit.convert.use_texture" ) );
	int i;
	for ( i = 0; i < mlt_properties_count( frame_properties ); i++ )
	{
		char *name = mlt_properties_get_name( frame_properties, i );
		if ( name && !strncmp( name, "_movit ", 7 ) ) {
			mlt_properties_set_data( properties, name,
				mlt_properties_get_data_at( frame_properties, i, NULL ),
				0, NULL, NULL );
		}
	}

	data = mlt_frame_get_alpha( frame );
	if ( data )
	{
		mlt_properties_get_data( frame_properties, "alpha", &size );
		mlt_frame_set_alpha( self, data, size, NULL );
	};
	self->convert_image = frame->convert_image;
	self->convert_audio = frame->convert_audio;
	return 0;
}

static int producer_get_audio( mlt_frame self, void **buffer, mlt_audio_format *format, int *frequency, int *channels, int *samples )
{
	mlt_properties properties = MLT_FRAME_PROPERTIES( self );
	mlt_frame frame = mlt_frame_pop_audio( self );
	mlt_properties frame_properties = MLT_FRAME_PROPERTIES( frame );
	mlt_properties_set( frame_properties, "consumer_channel_layout", mlt_properties_get( properties, "consumer_channel_layout" ) );
	mlt_properties_set( frame_properties, "producer_consumer_fps", mlt_properties_get( properties, "producer_consumer_fps" ) );
	mlt_frame_get_audio( frame, buffer, format, frequency, channels, samples );
	mlt_frame_set_audio( self, *buffer, *format, mlt_audio_format_size( *format, *samples, *channels ), NULL );
	mlt_properties_set_int( properties, "audio_frequency", *frequency );
	mlt_properties_set_int( properties, "audio_channels", *channels );
	mlt_properties_set_int( properties, "audio_samples", *samples );
	return 0;
}

static void destroy_data_queue( void *arg )
{
	if ( arg != NULL )
	{
		// Assign the correct type
		mlt_deque queue = arg;

		// Iterate through each item and destroy them
		while ( mlt_deque_peek_front( queue ) != NULL )
			mlt_properties_close( mlt_deque_pop_back( queue ) );

		// Close the deque
		mlt_deque_close( queue );
	}
}

/** Get the next frame.
 *
 * TODO : Fix it.
 *
 * \private \memberof mlt_switcher_s
 * \param parent the producer interface to the switcher
 * \param[out] frame a frame by reference
 * \param track the 0-based track index
 * \return true on error
 */

static int producer_get_frame( mlt_producer parent, mlt_frame_ptr frame, int track ){
    mlt_switcher self = parent->child;

    mlt_properties switcher_properties = mlt_switcher_properties(self);

    // Try to obtain the multitrack associated to the switcher
    mlt_multitrack multitrack = mlt_properties_get_data( switcher_properties, "multitrack", NULL );

    // No concurrent access to our properties switcher when we get a frame
    mlt_properties_lock(switcher_properties);
    int frame_count = mlt_properties_get_int(switcher_properties, "_frame_count");
    int nb_frame_switch = mlt_properties_get_int(switcher_properties, "nb_frame_switch");
    int current_track = mlt_properties_get_int(switcher_properties, "_current_track");
    int next_track = current_track;
    int use_auto_switch = mlt_properties_get_int(switcher_properties, "auto_switch");
    ++ frame_count;
    if(frame_count >= nb_frame_switch){
        frame_count = 0;
        if(use_auto_switch && multitrack){
            ++next_track;
            if(next_track >= multitrack->count){
                next_track = 0;
            }
        }
    }
    mlt_properties_set_int(switcher_properties, "_frame_count", frame_count);
    mlt_properties_set_int(switcher_properties, "_current_track", next_track);
    mlt_properties_unlock(switcher_properties);

    // We only respond to the first track requests
    if(track == 0 && self->producer != NULL){
        // Try to obtain the multitrack associated to the switcher
		// Or a specific producer
		mlt_producer producer = mlt_properties_get_data( switcher_properties, "producer", NULL );

        if(multitrack != NULL){
            mlt_frame temp = NULL;
            int done = 0, count = 0, image_count = 0, rotated_index = 0;

            // Determine whether this switcher feeds to the consumer or stops here
		    int global_feed = mlt_properties_get_int( switcher_properties, "global_feed" );

		    // The output frame will hold the 'global' data feeds (ie: those which are targetted for the final frame)
			mlt_deque data_queue = mlt_deque_init( );

			// Used to garbage collect all frames
			char label[64];

			// Get the id of the tractor
			char *id = mlt_properties_get( switcher_properties, "_unique_id" );
			if ( !id ) {
				mlt_properties_set_int64( switcher_properties, "_unique_id", (int64_t) switcher_properties );
				id = mlt_properties_get( switcher_properties, "_unique_id" );
			}

			// Will be used to store the frame properties object
			mlt_properties frame_properties = NULL;

			// We'll store audio and video frames to use here
			mlt_frame audio = NULL;
			mlt_frame video = NULL;
			mlt_frame first_video = NULL;

			// Temporary properties
			mlt_properties temp_properties = NULL;

			// Get the multitrack's producer
			mlt_producer target = MLT_MULTITRACK_PRODUCER( multitrack );
			mlt_producer_seek( target, mlt_producer_frame( parent ) );
			mlt_producer_set_speed( target, mlt_producer_get_speed( parent ) );

			// We will create one frame and attach everything to it
			*frame = mlt_frame_init( MLT_PRODUCER_SERVICE( parent ) );

			// Get the properties of the frame
			frame_properties = MLT_FRAME_PROPERTIES( *frame );

			for(int multitrack_index = 0; !done; ++multitrack_index){
			    if(multitrack_index < multitrack->count)
			        rotated_index = (multitrack_index + current_track + 1) % multitrack->count;
			    else
			        rotated_index = multitrack_index;

			    // Get a frame from the producer
				mlt_service_get_frame( self->producer, &temp, rotated_index );

                // Get the temporary properties
				temp_properties = MLT_FRAME_PROPERTIES( temp );

				// Pass all unique meta properties from the producer's frame to the new frame
				mlt_properties_lock( temp_properties );
				int props_count = mlt_properties_count( temp_properties );
				int j;
				for ( j = 0; j < props_count; j ++ )
				{
					char *name = mlt_properties_get_name( temp_properties, j );
					if ( !strncmp( name, "meta.", 5 ) && !mlt_properties_get( frame_properties, name ) )
						mlt_properties_set( frame_properties, name, mlt_properties_get_value( temp_properties, j ) );
				}
				mlt_properties_unlock( temp_properties );

				// Copy the format conversion virtual functions
				if ( ! (*frame)->convert_image && temp->convert_image )
					(*frame)->convert_image = temp->convert_image;
				if ( ! (*frame)->convert_audio && temp->convert_audio )
					(*frame)->convert_audio = temp->convert_audio;

				// Check for last track
				done = mlt_properties_get_int( temp_properties, "last_track" );

				// Handle fx only tracks
				if ( mlt_properties_get_int( temp_properties, "fx_cut" ) )
				{
					int hide = ( video == NULL ? 1 : 0 ) | ( audio == NULL ? 2 : 0 );
					mlt_properties_set_int( temp_properties, "hide", hide );
				}

				// We store all frames with a destructor on the output frame
				snprintf( label, sizeof(label), "mlt_tractor %s_%d", id, count ++ );
				mlt_properties_set_data( frame_properties, label, temp, 0, ( mlt_destructor )mlt_frame_close, NULL );

				// We want to append all 'final' feeds to the global queue
				if ( !done && mlt_properties_get_data( temp_properties, "data_queue", NULL ) != NULL )
				{
					// Move the contents of this queue on to the output frames data queue
					mlt_deque sub_queue = mlt_properties_get_data( MLT_FRAME_PROPERTIES( temp ), "data_queue", NULL );
					mlt_deque temp = mlt_deque_init( );
					while ( global_feed && mlt_deque_count( sub_queue ) )
					{
						mlt_properties p = mlt_deque_pop_back( sub_queue );
						if ( mlt_properties_get_int( p, "final" ) )
							mlt_deque_push_back( data_queue, p );
						else
							mlt_deque_push_back( temp, p );
					}
					while( mlt_deque_count( temp ) )
						mlt_deque_push_front( sub_queue, mlt_deque_pop_back( temp ) );
					mlt_deque_close( temp );
				}

				// Now do the same with the global queue but without the conditional behaviour
				if ( mlt_properties_get_data( temp_properties, "global_queue", NULL ) != NULL )
				{
					mlt_deque sub_queue = mlt_properties_get_data( MLT_FRAME_PROPERTIES( temp ), "global_queue", NULL );
					while ( mlt_deque_count( sub_queue ) )
					{
						mlt_properties p = mlt_deque_pop_back( sub_queue );
						mlt_deque_push_back( data_queue, p );
					}
				}

				// Pick up first video and audio frames
				if ( !done && !mlt_frame_is_test_audio( temp ) && !( mlt_properties_get_int( temp_properties, "hide" ) & 2 ) )
				{
					// Order of frame creation is starting to get problematic
					if ( audio != NULL )
					{
						mlt_deque_push_front( MLT_FRAME_AUDIO_STACK( temp ), producer_get_audio );
						mlt_deque_push_front( MLT_FRAME_AUDIO_STACK( temp ), audio );
					}
					audio = temp;
				}
				if ( !done && !mlt_frame_is_test_card( temp ) && !( mlt_properties_get_int( temp_properties, "hide" ) & 1 ) )
				{
					if ( video != NULL )
					{
						mlt_deque_push_front( MLT_FRAME_IMAGE_STACK( temp ), producer_get_image );
						mlt_deque_push_front( MLT_FRAME_IMAGE_STACK( temp ), video );
					}
					video = temp;
					if ( first_video == NULL )
						first_video = temp;

					mlt_properties_set_int( MLT_FRAME_PROPERTIES( temp ), "image_count", ++ image_count );
					image_count = 1;
				}
			}

			// Now stack callbacks
			if ( audio != NULL )
			{
				mlt_frame_push_audio( *frame, audio );
				mlt_frame_push_audio( *frame, producer_get_audio );
			}

			if ( video != NULL )
			{
				mlt_properties video_properties = MLT_FRAME_PROPERTIES( first_video );
				mlt_frame_push_service( *frame, video );
				mlt_frame_push_service( *frame, producer_get_image );
				if ( global_feed )
					mlt_properties_set_data( frame_properties, "data_queue", data_queue, 0, NULL, NULL );
				mlt_properties_set_data( video_properties, "global_queue", data_queue, 0, destroy_data_queue, NULL );
				mlt_properties_set_int( frame_properties, "width", mlt_properties_get_int( video_properties, "width" ) );
				mlt_properties_set_int( frame_properties, "height", mlt_properties_get_int( video_properties, "height" ) );
				mlt_properties_pass_list( frame_properties, video_properties, "meta.media.width, meta.media.height" );
				mlt_properties_set_int( frame_properties, "progressive", mlt_properties_get_int( video_properties, "progressive" ) );
				mlt_properties_set_double( frame_properties, "aspect_ratio", mlt_properties_get_double( video_properties, "aspect_ratio" ) );
				mlt_properties_set_int( frame_properties, "image_count", image_count );
				mlt_properties_set_data( frame_properties, "_producer", mlt_frame_get_original_producer( first_video ), 0, NULL, NULL );
			}
			else
			{
				destroy_data_queue( data_queue );
			}

			mlt_frame_set_position( *frame, mlt_producer_frame( parent ) );
			mlt_properties_set_int( MLT_FRAME_PROPERTIES( *frame ), "test_audio", audio == NULL );
			mlt_properties_set_int( MLT_FRAME_PROPERTIES( *frame ), "test_image", video == NULL );

        }else if(producer != NULL){
            mlt_producer_seek( producer, mlt_producer_frame( parent ) );
			mlt_producer_set_speed( producer, mlt_producer_get_speed( parent ) );
			mlt_service_get_frame( self->producer, frame, track );
        }else{
            mlt_log( MLT_PRODUCER_SERVICE( parent ), MLT_LOG_ERROR, "switcher without a multitrack!!\n" );
			mlt_service_get_frame( self->producer, frame, track );
        }
        // Prepare the next frame
		mlt_producer_prepare_next( parent );

		// Indicate our found status
		return 0;
    }else{
        // Generate a test card
		*frame = mlt_frame_init( MLT_PRODUCER_SERVICE( parent ) );
		return 0;
    }
}

/** Close the switcher and free its resources.
 *
 * \public \memberof mlt_switcher_s
 * \param self a switcher
 */

void mlt_switcher_close( mlt_switcher self ){
    if(self != NULL && mlt_properties_dec_ref( MLT_SWITCHER_PROPERTIES( self ) ) <= 0){
        self->parent.close = NULL;
		mlt_producer_close( &self->parent );
		free( self );
    }
}

void mlt_switcher_set_current_track(mlt_switcher self, int track){
    mlt_properties switcher_properties = mlt_switcher_properties(self);
    mlt_properties_lock(switcher_properties);

    mlt_properties_set_int(switcher_properties, "_current_track", track);
    mlt_properties_set_int(switcher_properties, "_frame_count", 0);

    mlt_properties_unlock(switcher_properties);
}