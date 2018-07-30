#include "MltSwitcher.h"
#include "MltMultitrack.h"
#include "MltPlaylist.h"
using namespace Mlt;

Switcher::Switcher():
    instance(mlt_switcher_new()){}

Switcher::Switcher(Profile& profile):
    instance(mlt_switcher_new()){
        set_profile(profile);
}

Switcher::Switcher( Service &switcher ):
    instance(NULL){
        if(switcher.type() == switcher_type){
            instance = (mlt_switcher)switcher.get_service();
            inc_ref();
        }
}

Switcher::Switcher( mlt_switcher switcher ):
    instance(switcher){
        inc_ref();
}

Switcher::Switcher( Switcher &switcher ):
    Mlt::Producer(switcher),
    instance(switcher.get_switcher()){
        inc_ref();
}

Switcher::Switcher( Profile& profile, char *id, char *arg):
    instance(NULL){
        Producer producer(profile, id, arg);
        if(producer.is_valid() && producer.type() == switcher_type){
            instance = (mlt_switcher)producer.get_producer();
            inc_ref();
        }else if(producer.is_valid()){
            instance = mlt_switcher_new( );
		    set_profile( profile );
		    set_track( producer, 0 );
        }
}

Switcher::~Switcher(){
    mlt_switcher_close(instance);
}

mlt_switcher Switcher::get_switcher(){
    return instance;
}

mlt_producer Switcher::get_producer(){
    return mlt_switcher_producer(get_switcher());
}

Multitrack* Switcher::multitrack(){
    return new Multitrack(mlt_switcher_multitrack(get_switcher()));
}

void Switcher::refresh(){
    mlt_switcher_refresh(get_switcher());
}

int Switcher::set_track(Producer& producer, int index){
    return mlt_switcher_set_track(get_switcher(), producer.get_producer(), index);
}

int Switcher::insert_track(Producer& producer, int index){
    return mlt_switcher_insert_track(get_switcher(), producer.get_producer(), index);
}

int Switcher::remove_track(int index){
    return mlt_switcher_remove_track(get_switcher(), index);
}

Producer* Switcher::track(int index){
    mlt_producer producer = mlt_switcher_get_track(get_switcher(), index);
    return producer != NULL ? new Producer(producer) : NULL;
}

int Switcher::count(){
    return mlt_multitrack_count(mlt_switcher_multitrack(get_switcher()));
}

bool Switcher::locate_cut(Producer* producer, int &track, int &cut){
    bool found = false;

    for ( track = 0; producer != NULL && !found && track < count( ); track ++ )
	{
		Playlist playlist( ( mlt_playlist )mlt_switcher_get_track( get_switcher( ), track ) );
		for ( cut = 0; !found && cut < playlist.count( ); cut ++ )
		{
			Producer *clip = playlist.get_clip( cut );
			found = producer->get_producer( ) == clip->get_producer( );
			delete clip;
		}
	}

	track --;
	cut --;

	return found;
}

int Switcher::connect(Producer &producer){
    return mlt_switcher_connect(get_switcher(), producer.get_service());
}

void Switcher::set_current_track(int index){
    mlt_switcher_set_current_track(get_switcher(), index);
}
