
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>

#include "epg.h"
#include "aribstr.h"


const uint16_t	EPG::EIT_PID[ EIT_PID_NUM] = {
	0x0012, 0x0026, 0x0027,
};

const EPG::CONTENT	EPG::contents[ CONTENT_NUM] = {
	{ "ニュース・報道"        , "news"       , },
	{ "スポーツ"              , "sports"     , },
	{ "情報"                  , "information", },
	{ "ドラマ"                , "drama"      , },
	{ "音楽"                  , "music"      , },
	{ "バラエティ"            , "variety"    , },
	{ "映画"                  , "cinema"     , },
	{ "アニメ・特撮"          , "anime"      , },
	{ "ドキュメンタリー・教養", "documentary", },
	{ "演劇"                  , "stage"      , },
	{ "趣味・実用"            , "hobby"      , },
	{ "福祉"                  , "etc"        , },
	{ "予備"                  , "etc"        , },
	{ "予備"                  , "etc"        , },
	{ "予備"                  , "etc"        , },
	{ "その他"                , "etc"        , },
};

EPG::EPG( std::string ch)
	: channel( ch)
{
	int32_t	i;

	sdt = new SDT( &descriptor_parser);

	for( i = 0; i < EIT_PID_NUM; i++) {
		eits[ EIT_PID[ i]] = new EIT( &descriptor_parser);
	}
}

EPG::~EPG()
{
	EITS::iterator	e;

	if( sdt) {
		delete sdt;
	}

	for( e = eits.begin(); e != eits.end(); e++) {
		if( e->second)	{
			delete e->second;
		}
	}
	eits.clear();
}

bool EPG::append( TSPacket *ts)
{
	int32_t		i;
	uint16_t	pid = ts->getPID();

	Section::STATUS	status;

	if( pid == SDT_PID) {
		return appendSDT( ts);
	}

	for( i = 0; i < EIT_PID_NUM; i++) {
		if( pid == EIT_PID[ i]) {
			return appendEIT( ts);
		}
	}

	return true;
}


bool EPG::appendSDT( TS::TSPacket *ts)
{
	Section::STATUS	status;
	uint32_t		index = 0;

	do {
		status = sdt->append( ts, &index);
		if( status == Section::SUCCESS || status == Section::INCLUDE_NEW_SECTION) {
			SDT::SERVICES::iterator		s;
			SDT::DESCRIPTORS::iterator	d;
			for( s = sdt->services.begin(); s != sdt->services.end(); s++) {
				if( s->EIT_schedule_flag != 0x01) {
					continue;
				}

				for( d = s->descriptors.begin(); d != s->descriptors.end(); d++) {
					if( (*d)->descriptor_tag == Service::TAG) {
						Service *des = (Service *)*d;
						if( des->service_type == 0x01) {
							SERVICE	tmp;

							tmp.transport_stream_id	= sdt->si_id.transport_stream_id;
							tmp.original_network_id	= sdt->original_network_id;
							tmp.service_id			= s->service_id;

							if( services.find( tmp) == services.end()) {
								std::string name = "";

								if( des->service_name_length != 0) {
									AribToString( &name, &des->service_name);
								}
								else if( des->service_provider_name_length != 0) {
									AribToString( &name, &des->service_provider_name);
								}
								services[ tmp] = name;
							}
						}
						break;
					}
				}
			}
		}
		else if( status == Section::CONTINUE) {
			return true;
		}
		else if( status == Section::ERROR_START_SECTION) {
			return true;
		}
		else {
			sdt->clear();
			return false;
		}
	} while( status == Section::INCLUDE_NEW_SECTION);

	return true;
}

bool EPG::appendEIT( TS::TSPacket *ts)
{
	Section::STATUS status;
	uint16_t		pid = ts->getPID();
	uint32_t		index = 0;

	do {
		status = eits[ pid]->append( ts, &index);
		if( status == Section::SUCCESS || status == Section::INCLUDE_NEW_SECTION) {
			appendEvnet((EIT*)eits[ pid]);
			if( status == Section::SUCCESS) {
				return true;
			}
		}
		else if( status == Section::CONTINUE) {
			return true;
		}
		else if( status == Section::ERROR_START_SECTION) {
			return true;
		}
		else if( status == Section::ERROR_TS_COUNTER) {
			eits[ pid]->clear();
			index = 0;
			fprintf( stderr, "TS COUNTER ERROR: ch = %s\n", channel.c_str());
			continue;
		}
		else {
			fprintf( stderr, "parse err: %d, ch = %s\n", status, channel.c_str());
			eits[ pid]->clear();
			return false;
			//return true;
		}
	} while( status == Section::INCLUDE_NEW_SECTION);

	return true;
}

void EPG::appendEvnet( EIT *p)
{
	ShortEvent		*se;
	Content			*c;
	ExtendedEvent	*ee;
	SERVICE			s;

	s.transport_stream_id	= p->transport_stream_id;
	s.original_network_id	= p->original_network_id;
	s.service_id			= p->si_id.service_id;

	EIT::EVENTS::iterator it;
	EIT::DESCRIPTORS::iterator d;

	for( it = p->events.begin(); it != p->events.end(); it++) {
#if 0
		for( d = it->descriptors.begin(); d != it->descriptors.end(); d++) {
			if( (*d)->descriptor_tag == ShortEvent::TAG) {
				break;
			}
		}
		if( d == it->descriptors.end()) {
			continue;
		}
#endif

		EXTEND::ITEM	item, item_tmp;
		std::string		ext_text;
		
		EVENT			*e = &epg_data[ s][ it->event_id];
		bool			ext_flag = e->extend_flag;
		
		e->event_id		= it->event_id;
		e->start_time	= it->start_time;
		if( it->duration == 0) {
			// とりあえず5分後
			e->end_time	= it->start_time + 5 * 60;
		}
		else {
			e->end_time	= it->start_time + it->duration;
		}
		
		for( d = it->descriptors.begin() ; d != it->descriptors.end(); d++) {
			if( (*d)->descriptor_tag == ShortEvent::TAG) {
				se = (ShortEvent*)*d;
				AribToString( &e->title, &se->event_name_char);
				AribToString( &e->desc , &se->text_char);
			}
			else if( (*d)->descriptor_tag == Content::TAG) {
				c = (Content*)*d;
				Content::CONTENTS::iterator c_it;
				for( c_it = c->contents.begin(); c_it != c->contents.end(); c_it++) {
					e->categorys.push_back( c_it->content_nibble_level_1);
				}
			}
			else if( (*d)->descriptor_tag == ExtendedEvent::TAG) {
				ExtendedEvent::ITEMS::iterator	i_it;
				ee = (ExtendedEvent*)*d;
				
				if( e->extend_flag) {
					e->extend.clear();
					e->extend_flag = false;
				}

				for( i_it = ee->items.begin(); i_it != ee->items.end(); i_it++) {
					if( !i_it->item_description_char.empty()) {
						
						if( !item.description.empty()) {
							ext_flag = true;
							AribToString( &item_tmp.description, &item.description);
							AribToString( &item_tmp.item       , &item.item);
							e->extend.items.push_back( item_tmp);
							item.clear();
						}

						item.description	= i_it->item_description_char;
						item.item			= i_it->item_char;
					}
					else {
						if( !i_it->item_char.empty()) {
							item.item += i_it->item_char;
						}
					}
				}
				// とりあえず格納
				ext_text += ee->text_char;
			}
		}

		if( !item.description.empty()) {
			ext_flag = true;
			AribToString( &item_tmp.description, &item.description);
			AribToString( &item_tmp.item       , &item.item);
			e->extend.items.push_back( item_tmp);
			item.clear();
		}
		if( !ext_text.empty()) {
			ext_flag = true;
			AribToString( &e->extend.text, &ext_text);
		}
		e->extend_flag = ext_flag;

		e->table_id =  p->table_id;

#if 0
		if( !e.title.empty()) {	
			epg_data[ s].insert( e);
		}
#endif
	}
}

const std::string	EPG::createChannelID( const SERVICE *s)
{
	std::ostringstream	id;

	if( channel.compare( "BS") == 0) {
		id << "BS" << s->service_id;
	}
	else if( channel.compare( "CS") == 0) {
		id << "CS" << s->service_id;
	}
	else {
		id << channel;
	}

	return id.str();
}

bool EPG::write( const char *xml_file)
{
	char				start_time[ 64];
	char				end_time[ 64];
	int					fd;
	int					i;
	int					category;

	std::string			str;
	std::ostringstream	buf;

	EVENTS				*p;
	EVENT				*e;
	SERVICES::iterator	s;
	EVENTS::iterator	e_it;

	if( strcmp( xml_file, "-") == 0) {
		fd = 1;
	}
	else {
		fd = open( xml_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if( fd == -1) {
			fprintf( stderr, "%s is not opened.\n", xml_file);
			return false;
		}
	}

#if 0
	for( s = services.begin(); s != services.end(); s++) {
		fprintf( stderr, "on = %d, ts = %d, sid = %d, %s\n",
				  s->first.original_network_id, s->first.transport_stream_id,
				  s->first.service_id, s->second.c_str()
				 );
	}
#endif

#if 1
	if( channel.compare( "BS") != 0 && channel.compare( "CS") != 0 && services.size() > 1) {
		SERVICES::iterator index = services.begin();
		std::advance( index, 1);
		services.erase( index, services.end());
	}
#endif

	if( !writeHeader( fd)) {
		goto ERR;
	}
	if( !writeChannel( fd)) {
		goto ERR;
	}

	for( s = services.begin(); s != services.end(); s++) {
		p = &epg_data[ s->first];
		for( e_it = p->begin(); e_it != p->end(); e_it++) {
			e = &e_it->second;
			if( e->title.empty()) {
				continue;
			}

			strftime( start_time, sizeof( start_time), "%Y%m%d%H%M%S", localtime( &e->start_time));
			strftime(   end_time, sizeof(   end_time), "%Y%m%d%H%M%S", localtime( &e->end_time));

			if( e->categorys.size() == 0) {
				category = CONTENT_NUM - 1;
			}
			else {
				category = e->categorys[ 0];
			}
		
			buf.str( "");
			buf << "\t<programme "
				<< "start=\"" << start_time << " +0900\" "
				<< "stop=\"" << end_time << " +0900\" "
				<< "channel=\"" << createChannelID( &s->first) << "\" "
#if 0
				<< "sid=\"" << s->first.service_id << "\" "
				<< "eid=\"" << e->event_id << "\" "
#endif
				<< ">\n";
			if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
				goto ERR;
			}

			str = e->title;
			convertCharactor( &str);
			buf.str( "");
			buf << "\t\t<title lang=\"ja_JP\">" << str << "</title>\n";
			if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
				goto ERR;
			}

			str = e->desc;
			convertCharactor( &str);
			buf.str( "");
			buf << "\t\t<desc lang=\"ja_JP\">" << str << "</desc>\n";
			if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
				goto ERR;
			}

			buf.str( "");
			buf << "\t\t<category lang=\"ja_JP\">" << contents[ category].jpn << "</category>\n";
			if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
				goto ERR;
			}

			buf.str( "");
			buf << "\t\t<category lang=\"en\">" << contents[ category].eng << "</category>\n";
			if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
				goto ERR;
			}

			if( e->extend_flag) {
				EXTEND::ITEMS::const_iterator item;

				buf.str( "");
				buf << "\t\t<desc lang=\"ja_JP\">";
				for( item = e->extend.items.begin(); item != e->extend.items.end(); item++) {
					str = item->description;
					convertCharactor( &str);
					buf << str << "\n";

					str = item->item;
					convertCharactor( &str);
					buf << str << "\n";
				}
				if( !e->extend.text.empty()) {
					str = item->item;
					convertCharactor( &str);
					buf << str << "\n";
				}

				buf << "</desc>\n";
				if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
					goto ERR;
				}
			}

			buf.str( "");
			buf << "\t</programme>\n";
			if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
				goto ERR;
			}
		}
	}
	if( !writeFooter( fd)) {
		goto ERR;
	}

	if( fd != 1) {
		close( fd);
	}
	return true;

ERR:
	if( fd != 1) {
		close( fd);
	}
	return false;
}

bool EPG::writeHeader( int fd)
{
	const char *header;
	header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"	\
		     "<!DOCTYPE tv SYSTEM \"xmltv.dtd\">\n\n"			\
		     "<tv generator-info-name=\"eit2xml\" generator-info-url=\"http://localhost/\">\n";

	if( ::write( fd, header, strlen( header)) != strlen( header)) {
		return false;
	}

	return true;
}

bool EPG::writeChannel( int fd)
{
	std::ostringstream buf;
	std::string ch_id;
	std::string	str;

	SERVICES::iterator	s;
	for( s = services.begin(); s != services.end(); s++) {
		ch_id = createChannelID( &s->first);
		buf.str( "");
		buf << " \t<channel id=\"" << ch_id << "\">\n";
		if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
			return false;
		}

		str = s->second;
		convertCharactor( &str);
		buf.str( "");
		buf << "\t\t<display-name lang=\"ja_JP\">" << str << "</display-name>\n";
		if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
			return false;
		}
#if 0
		buf.str( "");
		buf << "\t\t<sid>" << s->first.service_id << "</sid>";
		if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
			return false;
		}
#endif
		buf.str( "");
		buf << "\t</channel>\n";
		if( ::write( fd, buf.str().c_str(), buf.str().length()) != buf.str().length()) {
			return false;
		}
	}
	return true;
}

bool EPG::writeFooter( int fd)
{
	const char *footer = "</tv>\n";
	if( ::write( fd, footer, strlen( footer)) != strlen( footer)) {
		return false;
	}
	return true;
}

void EPG::convertCharactor(std::string *str)
{
	const char *rep[][2] = {
		{ "&" , "&amp;" , },
		{ "'" , "&apos;", },
		{ "\"", "&quot;", },
		{ "<" , "&lt;"  , },
		{ ">" , "&gt;"  , },
		{ NULL, NULL    , },
	};

	const char *from, *to;
	int 		i;

	std::string::size_type pos;

	for( i = 0, from = rep[ i][ 0], to = rep[ i][ 1]; from && to; i++, from = rep[ i][ 0], to = rep[ i][ 1]) {
		pos = 0;
		while( (pos = str->find( from, pos)) != std::string::npos) {
			str->replace( pos, strlen( from), to);
			pos += strlen( to);
		}
	}
}


