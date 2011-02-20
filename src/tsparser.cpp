
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <map>
#include <set>
#include "TS/ts_packet.h"
#include "TS/psi.h"
#include "ARIB/si.h"
#include "aribstr.h"

using namespace TS;
using namespace TS::PSI;
using namespace TS::PSI::ARIB;
using namespace TS::Description;
using namespace TS::Description::ARIB;

typedef std::map< uint16_t, Section*>	SECTIONS;


void printPAT( Section *sec)
{
	PAT *pat = (PAT *)sec;

	fprintf( stdout, "\n[ PAT ]\n");
	fprintf( stdout, "table id                           = %5u\n", pat->table_id);
	fprintf( stdout, "section syntax indicator           = %5u\n", pat->section_syntax_indicator);
	fprintf( stdout, "section length                     = %5u\n", pat->section_length);
	fprintf( stdout, "transport stream id                = %5u\n", pat->psi_id.transport_stream_id);
	fprintf( stdout, "version number                     = %5u\n", pat->version_number);
	fprintf( stdout, "current next indicator             = %5u\n", pat->current_next_indicator);
	fprintf( stdout, "section number                     = %5u\n", pat->section_number);
	fprintf( stdout, "last section number                = %5u\n", pat->last_section_number);
	fprintf( stdout, "network PID                        = %5u\n", pat->network_PID);

	PAT::PROGRAM_MAP::iterator it;
	for( it = pat->program_map_PID.begin(); it != pat->program_map_PID.end(); it++) {
		fprintf( stdout, "  program number = 0x%04X, program map PID = 0x%04X\n", it->first, it->second);
	}
}

void printCAT( Section *sec)
{
	CAT *cat = (CAT *)sec;

	fprintf( stdout, "\n[ CAT ]\n");
	fprintf( stdout, "table id                           = %5u\n", cat->table_id);
	fprintf( stdout, "section syntax indicator           = %5u\n", cat->section_syntax_indicator);
	fprintf( stdout, "section length                     = %5u\n", cat->section_length);
	fprintf( stdout, "version number                     = %5u\n", cat->version_number);
	fprintf( stdout, "current next indicator             = %5u\n", cat->current_next_indicator);
	fprintf( stdout, "section number                     = %5u\n", cat->section_number);
	fprintf( stdout, "last section number                = %5u\n", cat->last_section_number);

	CAT::DESCRIPTORS::iterator d;
	for( d = cat->descriptors.begin(); d != cat->descriptors.end(); d++) {
		fprintf( stdout, "  descriptor_tag                   = 0x%02X\n", (*d)->descriptor_tag);
		fprintf( stdout, "  descriptor_length                = %5u\n", (*d)->descriptor_length);
	}
}

void printTSDT( Section *sec)
{
	TSDT *tsdt = (TSDT *)sec;

	fprintf( stdout, "\n[ TSDT ]\n");
	fprintf( stdout, "table id                           = %5u\n", tsdt->table_id);
	fprintf( stdout, "section syntax indicator           = %5u\n", tsdt->section_syntax_indicator);
	fprintf( stdout, "section length                     = %5u\n", tsdt->section_length);
	fprintf( stdout, "version number                     = %5u\n", tsdt->version_number);
	fprintf( stdout, "current next indicator             = %5u\n", tsdt->current_next_indicator);
	fprintf( stdout, "section number                     = %5u\n", tsdt->section_number);
	fprintf( stdout, "last section number                = %5u\n", tsdt->last_section_number);

	TSDT::DESCRIPTORS::iterator d;
	for( d = tsdt->descriptors.begin(); d != tsdt->descriptors.end(); d++) {
		fprintf( stdout, "  descriptor_tag                   = 0x%02X\n", (*d)->descriptor_tag);
		fprintf( stdout, "  descriptor_length                = %5u\n", (*d)->descriptor_length);
	}
}

void printPMT( Section *sec)
{
	PMT *pmt = (PMT *)sec;
	PMT::DESCRIPTORS::iterator d;

	fprintf( stdout, "\n[ PMT ]\n");
	fprintf( stdout, "table id                           = %5u\n", pmt->table_id);
	fprintf( stdout, "section syntax indicator           = %5u\n", pmt->section_syntax_indicator);
	fprintf( stdout, "section length                     = %5u\n", pmt->section_length);
	fprintf( stdout, "program number                     = %5u\n", pmt->psi_id.program_number);
	fprintf( stdout, "version number                     = %5u\n", pmt->version_number);
	fprintf( stdout, "current next indicator             = %5u\n", pmt->current_next_indicator);
	fprintf( stdout, "section number                     = %5u\n", pmt->section_number);
	fprintf( stdout, "last section number                = %5u\n", pmt->last_section_number);
	fprintf( stdout, "PCR_PID                            = 0x%04X\n", pmt->PCR_PID);
	fprintf( stdout, "program_info_length                = %5u\n", pmt->program_info_length);

	for( d = pmt->program_info_descriptors.begin(); d != pmt->program_info_descriptors.end(); d++) {
		fprintf( stdout, "descriptor_tag                     = 0x%02X\n", (*d)->descriptor_tag);
		fprintf( stdout, "descriptor_length                  = %5u\n", (*d)->descriptor_length);

		if( (*d)->descriptor_tag == ConditionalAccess::TAG) {
			ConditionalAccess *ca = (ConditionalAccess *)*d;
			fprintf( stdout, "CA_system_ID                       = %5u\n", ca->CA_system_ID);
			fprintf( stdout, "CA_PID                             = 0x%04X\n", ca->CA_PID);
		}
	}

	PMT::ELEMENTS::iterator e;
	for( e = pmt->elements.begin(); e != pmt->elements.end(); e++) {
		fprintf( stdout, "  stream_type                      = 0x%02X\n", e->stream_type);
		fprintf( stdout, "  elementary_PID                   = 0x%04X\n", e->elementary_PID);
		fprintf( stdout, "  ES_info_length                   = %5u\n", e->ES_info_length);

		PMT::DESCRIPTORS::iterator d;
		for( d = e->descriptors.begin(); d != e->descriptors.end(); d++) {
			fprintf( stdout, "    descriptor_tag                 = 0x%02X\n", (*d)->descriptor_tag);
			fprintf( stdout, "    descriptor_length              = %5u\n", (*d)->descriptor_length);	

			if( (*d)->descriptor_tag == ConditionalAccess::TAG) {
				ConditionalAccess *ca = (ConditionalAccess *)*d;
				fprintf( stdout, "    CA_system_ID                   = %5u\n", ca->CA_system_ID);
				fprintf( stdout, "    CA_PID                         = 0x%04X\n", ca->CA_PID);
			}
		}
	}
}
	

void printEIT( Section *sec)
{
	EIT *eit = (EIT *)sec;

	fprintf( stdout, "\n[ EIT ]\n");
	fprintf( stdout, "table id                           = 0x%02X\n", eit->table_id);
	fprintf( stdout, "section syntax indicator           = %5u\n", eit->section_syntax_indicator);
	fprintf( stdout, "private indicator                  = %5u\n", eit->private_indicator);
	fprintf( stdout, "section length                     = %5u\n", eit->section_length);
	fprintf( stdout, "service_id                         = %5u\n", eit->si_id.service_id);
	fprintf( stdout, "version number                     = %5u\n", eit->version_number);
	fprintf( stdout, "current next indicator             = %5u\n", eit->current_next_indicator);
	fprintf( stdout, "section number                     = %5u\n", eit->section_number);
	fprintf( stdout, "last section number                = %5u\n", eit->last_section_number);
	fprintf( stdout, "transport_stream_id                = %5u\n", eit->transport_stream_id);
	fprintf( stdout, "original_network_id                = %5u\n", eit->original_network_id);
	fprintf( stdout, "segment_last_section_number        = %5u\n", eit->segment_last_section_number);
	fprintf( stdout, "last_table_id                      = %5u\n", eit->last_table_id);

	EIT::EVENTS::iterator e;
	for( e = eit->events.begin(); e != eit->events.end(); e++) {
		fprintf( stdout, "  event_id                         = %5u\n", e->event_id);
		fprintf( stdout, "  start_time                       = %s", asctime( localtime( &e->start_time)));
		fprintf( stdout, "  duration                         = %5d\n", e->duration);
		fprintf( stdout, "  running_status                   = %5u\n", e->running_status);
		fprintf( stdout, "  free_CA_mode                     = %5u\n", e->free_CA_mode);
		fprintf( stdout, "  descriptors_loop_length          = %5u\n", e->descriptors_loop_length);					

		EIT::DESCRIPTORS::iterator d;
		for( d = e->descriptors.begin(); d != e->descriptors.end(); d++) {
			fprintf( stdout, "    descriptor_tag                 = 0x%02X\n", (*d)->descriptor_tag);
			fprintf( stdout, "    descriptor_length              = %5u\n", (*d)->descriptor_length);	
#if 1
			if( (*d)->descriptor_tag == ShortEvent::TAG) {
				ShortEvent *des = (ShortEvent*)*d;
				std::string str;
				AribToString( &str, &des->event_name_char);
				fprintf( stdout, "    event_name                     = %s\n", str.c_str());
				AribToString( &str, &des->text_char);
				fprintf( stdout, "    text                           = %s\n", str.c_str());

			}
			if( (*d)->descriptor_tag == ExtendedEvent::TAG) {
				ExtendedEvent *des = (ExtendedEvent *)*d;
				fprintf( stdout, "    descriptor_number              = %d\n", des->descriptor_number);
				fprintf( stdout, "    last_descriptor_number         = %d\n", des->last_descriptor_number);
				fprintf( stdout, "    length_of_items                = %d\n", des->length_of_items);

				ExtendedEvent::ITEMS::iterator it;
				std::string str;
				for( it = des->items.begin(); it != des->items.end(); it++) {
					AribToString( &str, &it->item_description_char);
					fprintf( stdout, "    item_description               = %s\n", str.c_str());
					AribToString( &str, &it->item_char);
					fprintf( stdout, "    item                           = %s\n", str.c_str());
				}
				AribToString( &str, &des->text_char);
				fprintf( stdout, "    text_char                      = %s\n", str.c_str());
			}
			if( (*d)->descriptor_tag == Component::TAG) {
				Component *des = (Component *)*d;
				std::string str;
				fprintf( stdout, "    stream_content                 = 0x%02X\n", des->stream_content);
				fprintf( stdout, "    component_type                 = 0x%02X\n", des->component_type);
				fprintf( stdout, "    component_tag                  = 0x%02X\n", des->component_tag);
				AribToString( &str, &des->text_char);
				fprintf( stdout, "    text                           = %s\n", str.c_str());
			}
			if( (*d)->descriptor_tag == Content::TAG) {
				Content *des = (Content *)*d;
				Content::CONTENTS::iterator it;
				for( it = des->contents.begin(); it != des->contents.end(); it++) {
					fprintf( stdout, "    content_nibble_level_1         = %d\n", it->content_nibble_level_1);
					fprintf( stdout, "    content_nibble_level_2         = %d\n", it->content_nibble_level_2);
					fprintf( stdout, "    user_nibble_1                  = %d\n", it->user_nibble_1);
					fprintf( stdout, "    user_nibble_2                  = %d\n", it->user_nibble_2);
				}
			}
			if( (*d)->descriptor_tag == AudioComponent::TAG) {
				AudioComponent *des = (AudioComponent *)*d;
				std::string str;
				fprintf( stdout, "    stream_content                 = 0x%02X\n", des->stream_content);
				fprintf( stdout, "    component_type                 = 0x%02X\n", des->component_type);
				fprintf( stdout, "    component_tag                  = 0x%02X\n", des->component_tag);
				fprintf( stdout, "    stream_type                    = 0x%02X\n", des->stream_type);
				fprintf( stdout, "    simulcast_group_tag            = 0x%02X\n", des->simulcast_group_tag);
				fprintf( stdout, "    ES_multi_lingual_flag          = %2d\n", des->ES_multi_lingual_flag);
				fprintf( stdout, "    main_component_flag            = %2d\n", des->main_component_flag);
				fprintf( stdout, "    quality_indicator              = 0x%02X\n", des->quality_indicator);
				fprintf( stdout, "    sampling_rate                  = 0x%02X\n", des->sampling_rate);
				fprintf( stdout, "    component_tag                  = 0x%02X\n", des->component_tag);
				fprintf( stdout, "    ISO_639_language_code          = %s\n", des->ISO_639_language_code.c_str());
				if( des->ES_multi_lingual_flag) {
					fprintf( stdout, "    ISO_639_language_code_2        = %s\n", des->ISO_639_language_code_2.c_str());
				}
				AribToString( &str, &des->text_char);
				fprintf( stdout, "    text                           = %s\n", str.c_str());
			}
			if( (*d)->descriptor_tag == EventGroup::TAG) {
				EventGroup *des = (EventGroup *)*d;
				EventGroup::EVENTS::iterator			e;
				EventGroup::NETWORK_EVENTS::iterator	n;
				fprintf( stdout, "    group_type                     = %X\n", des->group_type);
				fprintf( stdout, "    event_count                    = %d\n", des->event_count);
				for( e = des->events.begin(); e != des->events.end(); e++) {
					fprintf( stdout, "    service_id                     = %d\n", e->service_id);
					fprintf( stdout, "    event_id                       = %d\n", e->event_id);
				}
				if( des->group_type == 4 || des->group_type == 5) {
					for( n = des->network_events.begin(); n != des->network_events.end(); n++) {
						fprintf( stdout, "    original_network_id            = %d\n", n->original_network_id);
						fprintf( stdout, "    transport_stream_id            = %d\n", n->transport_stream_id);
						fprintf( stdout, "    service_id                     = %d\n", n->event.service_id);
						fprintf( stdout, "    event_id                       = %d\n", n->event.event_id);
					}
				}
				else {
					fprintf( stdout, "    private_data_length            = %d\n", des->getPrivateData()->size());
				}
			}
#endif
		}
	}
}

void printNIT( Section *sec)
{
	NIT *nit = (NIT *)sec;

	fprintf( stdout, "\n[ NIT ]\n");
	fprintf( stdout, "table id                           = 0x%02X\n", nit->table_id);
	fprintf( stdout, "section syntax indicator           = %5u\n", nit->section_syntax_indicator);
	fprintf( stdout, "private indicator                  = %5u\n", nit->private_indicator);
	fprintf( stdout, "section length                     = %5u\n", nit->section_length);
	fprintf( stdout, "network_id                         = %5u\n", nit->si_id.network_id);
	fprintf( stdout, "version number                     = %5u\n", nit->version_number);
	fprintf( stdout, "current next indicator             = %5u\n", nit->current_next_indicator);
	fprintf( stdout, "section number                     = %5u\n", nit->section_number);
	fprintf( stdout, "last section number                = %5u\n", nit->last_section_number);
	fprintf( stdout, "network_descriptors_length         = %5u\n", nit->network_descriptors_length);

	NIT::DESCRIPTORS::iterator d;
	for( d = nit->networks.begin(); d != nit->networks.end(); d++) {
		fprintf( stdout, "  descriptor_tag                   = 0x%02X\n", (*d)->descriptor_tag);
		fprintf( stdout, "  descriptor_length                = %5u\n", (*d)->descriptor_length);
		if( (*d)->descriptor_tag == NetworkName::TAG) {
			NetworkName *des = (NetworkName *)*d;
			std::string str;
			AribToString( &str, &des->name);
			fprintf( stdout, "  name                             = %s\n", str.c_str());
		}
	}

	fprintf( stdout, "transport_stream_loop_length       = %5u\n", nit->transport_stream_loop_length);
	NIT::STREAMS::iterator s;
	for( s = nit->streams.begin(); s != nit->streams.end(); s++) {
		fprintf( stdout, "  transport_stream_id              = %5u\n", s->transport_stream_id);
		fprintf( stdout, "  original_network_id              = %5u\n", s->original_network_id);
		fprintf( stdout, "  transport_descriptors_length     = %5u\n", s->transport_descriptors_length);
		for( d = s->descriptors.begin(); d != s->descriptors.end(); d++) {
			fprintf( stdout, "    descriptor_tag                 = 0x%02X\n", (*d)->descriptor_tag);
			fprintf( stdout, "    descriptor_length              = %5u\n", (*d)->descriptor_length);
			if( (*d)->descriptor_tag == ServiceList::TAG) {
				ServiceList *des = (ServiceList *)*d;
				ServiceList::SERVICES::iterator it;
				for( it = des->services.begin(); it != des->services.end(); it++) {
					fprintf( stdout, "    service_id                     = %5u\n", it->service_id);
					fprintf( stdout, "    service_type                   = %5u\n", it->service_type);
				}
			}
		}
	}
}

void printSDT( Section *sec)
{
	SDT *sdt = (SDT *)sec;
	std::string str;

	fprintf( stdout, "\n[ SDT ]\n");
	fprintf( stdout, "table id                           = 0x%02X\n", sdt->table_id);
	fprintf( stdout, "section syntax indicator           = %5u\n", sdt->section_syntax_indicator);
	fprintf( stdout, "private indicator                  = %5u\n", sdt->private_indicator);
	fprintf( stdout, "section length                     = %5u\n", sdt->section_length);
	fprintf( stdout, "transport_stream_id                = %5u\n", sdt->si_id.transport_stream_id);
	fprintf( stdout, "version number                     = %5u\n", sdt->version_number);
	fprintf( stdout, "current next indicator             = %5u\n", sdt->current_next_indicator);
	fprintf( stdout, "section number                     = %5u\n", sdt->section_number);
	fprintf( stdout, "last section number                = %5u\n", sdt->last_section_number);
	fprintf( stdout, "original_network_id                = %5u\n", sdt->original_network_id);

	SDT::SERVICES::iterator s;
	for( s = sdt->services.begin(); s != sdt->services.end(); s++) {
		fprintf( stdout, "  service_id                       = %5u\n", s->service_id);
		fprintf( stdout, "  EIT_user_defined_flags           = %d\n", s->EIT_user_defined_flags);
		fprintf( stdout, "  EIT_schedule_flag                = %d\n", s->EIT_schedule_flag);
		fprintf( stdout, "  EIT_present_following_flag       = %d\n", s->EIT_present_following_flag);
		fprintf( stdout, "  running_status                   = %5u\n", s->running_status);
		fprintf( stdout, "  free_CA_mode                     = %5u\n", s->free_CA_mode);
		fprintf( stdout, "  descriptors_loop_length          = %5u\n", s->descriptors_loop_length);
	
		SDT::DESCRIPTORS::iterator d;
		for( d = s->descriptors.begin(); d != s->descriptors.end(); d++) {
			fprintf( stdout, "    descriptor_tag                 = 0x%02X\n", (*d)->descriptor_tag);
			fprintf( stdout, "    descriptor_length              = %5u\n", (*d)->descriptor_length);
			if( (*d)->descriptor_tag == Service::TAG) {
				Service *p = (Service *)*d;
				fprintf( stdout, "    service_type                   = %d\n", p->service_type);
#if 1
				AribToString( &str, &p->service_provider_name);
				fprintf( stdout, "    service_provider_name          = %s\n", str.c_str());
				AribToString( &str, &p->service_name);
				fprintf( stdout, "    service_name                   = %s\n", str.c_str());
#endif
			}
		}
	}
}


int main( int argc, char **argv)
{
	int fd;
	char *filename;
	
	const uint32_t buf_size = TS::TSPacket::TS_PACKET_SIZE * 10;

	uint16_t	pid;
	uint8_t		buf[ buf_size];
	uint32_t	index = 0;
	uint32_t	read_len;
	uint32_t	offset = 0;
	ssize_t		file_read_size;
	bool		output_ts = false;

	TSPacket			*ts = NULL;
	Section::STATUS		status;
	SECTIONS			secs;
	SECTIONS::iterator	sec_it;

	AribDescriptorParser	des_parser;
	
	uint32_t payload_index = 0;

	if( argc <2) {
		fprintf( stderr, "required TS file\n");
		return 1;
	}

	filename = argv[ 1];
#if 0
	if( argc >= 3) {
		output_ts = true;
	}
#endif

	if( strcmp( filename, "-") == 0) {
		fd = 0;
	}
	else {
		fd = open( filename, O_RDONLY);
	}
	if( fd == -1) {
		fprintf( stderr, "%s is not opened.\n", filename);
		return 1;
	}

	secs[ 0x0000] = new PAT( &des_parser);
	secs[ 0x0001] = new CAT( &des_parser);
	secs[ 0x0002] = new TSDT( &des_parser);
	secs[ 0x0010] = new NIT( &des_parser);
	secs[ 0x0011] = new SDT( &des_parser);
	secs[ 0x0012] = new EIT( &des_parser);
	secs[ 0x0026] = new EIT( &des_parser);
	secs[ 0x0027] = new EIT( &des_parser);

	int32_t	t_err_pid;

	int ofd = -1;
	uint8_t	write_buf[ 1024];
	bool	save_flag = false;
	uint16_t			pmt_pid = 0;
	uint8_t				pat_counter = 0;
	uint8_t				pmt_counter = 0;
	std::set< uint16_t>	save_pid;

	save_pid.insert( 0x0000);
	save_pid.insert( 0x0001);
	save_pid.insert( 0x0010);
	save_pid.insert( 0x0011);

	while( (file_read_size = read( fd, buf + offset, buf_size - offset) + offset) > 0 ) {
		index = 0;
		while( index + TS::TSPacket::TS_PACKET_SIZE <= file_read_size) {
			if( ts) {
				delete ts;
			}

			ts = TS::TSPacket::parse( &buf[ index], file_read_size, &read_len);
			if( !ts) {
				fprintf( stderr, "TS Packet parse error\n");
				index += TS::TSPacket::TS_PACKET_SIZE;
				continue;
			}
//			index += read_len;
			pid = ts->getPID();
			sec_it = secs.find( pid);
			if( sec_it != secs.end()) {
				payload_index = 0;
				do {
					status = sec_it->second->append( ts, &payload_index);
					if( status == Section::SUCCESS || status == Section::INCLUDE_NEW_SECTION) {
						if( sec_it->first == 0x0000) {
							//printPAT( sec_it->second);

							PAT *pat = (PAT *)sec_it->second;
							PAT::PROGRAM_MAP::iterator it;
							SECTIONS::iterator pmt;
#if 0
							for( it = pat->program_map_PID.begin(); it != pat->program_map_PID.end(); it++) {
								pmt = secs.find( it->second);
								if( pmt == secs.end()) {
									secs[ it->second] = new PMT( &des_parser);
									save_pid.insert( it->second);
									fprintf( stderr, "new pid 0x%04X, program number %u\n", it->second, it->first);
									if( pmt_pid == 0) {
										pmt_pid = it->second;
									}
								}
							}
#else
							save_pid.insert( pat->network_PID);
							it = pat->program_map_PID.begin();
							std::advance( it, 1);
							pat->program_map_PID.erase( it, pat->program_map_PID.end());
							it = pat->program_map_PID.begin();
							if( secs.find( it->second) == secs.end()) {
								secs[ it->second] = new PMT( &des_parser);
								save_pid.insert( it->second);
								fprintf( stderr, "new pid 0x%04X, program number %u\n", it->second, it->first);
								pmt_pid = it->second;
							}
#endif
						}
						else if( sec_it->first == 0x0001) {
							//printCAT( sec_it->second);
						}
						else if( sec_it->first == 0x0002) {
							//printTSDT( sec_it->second);
						}
						else if( sec_it->first == 0x0010) {
							//printNIT( sec_it->second);
						}
						else if( sec_it->first == 0x0011) {
							///printSDT( sec_it->second);
						}
						else if( (sec_it->first == 0x0012 || sec_it->first == 0x0026 || sec_it->first == 0x0027)) {
							printEIT( sec_it->second);
						}
						else {
							if( sec_it->second->table_id == PMT::ID) {
								//printPMT( sec_it->second);

								PMT *pmt = (PMT *)sec_it->second;
								PMT::DESCRIPTORS::iterator	d;
								PMT::ELEMENTS::iterator	e;

								//if( pmt_pid == sec_it->first) {
									if( save_pid.find( pmt->PCR_PID) == save_pid.end()) {
										save_pid.insert( pmt->PCR_PID);
										fprintf( stderr, "new pid 0x%04X, PCR, cur pid 0x%04X\n", pmt->PCR_PID, sec_it->first);
									}

									for( d = pmt->program_info_descriptors.begin(); d != pmt->program_info_descriptors.end(); d++) {
										if( (*d)->descriptor_tag == ConditionalAccess::TAG) {
											ConditionalAccess *ca = (ConditionalAccess *)*d;
											if( save_pid.find( ca->CA_PID) == save_pid.end()) {
												//secs[ ca->CA_PID] = new CAT( &des_parser);
												if( ca->CA_PID != 0x1FFF) {
													save_pid.insert( ca->CA_PID);
													fprintf( stderr, "new pid 0x%04X, CA ID %u, cur pid 0x%04X\n", ca->CA_PID, ca->CA_system_ID, sec_it->first);
												}
											}
										}
									}

									pmt->eraseElement( 0x0D);

									for( e = pmt->elements.begin(); e != pmt->elements.end(); e++) {
										if( save_pid.find( e->elementary_PID) == save_pid.end()) {
											save_pid.insert( e->elementary_PID);
											fprintf( stderr, "new pid 0x%04X, stream_type 0x%02X, cur pid 0x%04X\n", e->elementary_PID, e->stream_type, sec_it->first);
										}

										for( d = e->descriptors.begin(); d != e->descriptors.end(); d++) {
											if( (*d)->descriptor_tag == ConditionalAccess::TAG) {
												ConditionalAccess *ca = (ConditionalAccess *)*d;
												if( save_pid.find( ca->CA_PID) == save_pid.end()) {
													//secs[ ca->CA_PID] = new CAT( &des_parser);
													if( ca->CA_PID != 0x1FFF) {
														save_pid.insert( ca->CA_PID);
														fprintf( stderr, "new pid 0x%04X, CA ID %u, cur pid 0x%04X\n", ca->CA_PID, ca->CA_system_ID, sec_it->first);
													}
												}
											}
										}
									}
									save_flag = true;
								//}
							}
						}
					}
					else if( status != Section::CONTINUE) {
						fprintf( stderr, "Section parse error = %d, PID = 0x%04X\n", status, ts->getPID());
					}
				} while( status == Section::INCLUDE_NEW_SECTION);
			}
#if 0
			if( ofd == -1 && output_ts) {
				ofd = open( "output.ts", O_WRONLY | O_CREAT | O_TRUNC, 0644);
			}

			if( ofd > 0) {
				if( ts->getPID() == 0x0000) {
					PAT *pat = (PAT *)secs[ 0x0000];
					if( pat->finish()) {
						TSPacket *tmp_ts;
						status = pat->getTSPacket( &tmp_ts, &pat_counter, ts->getAdaptationField());
						if( status != Section::SUCCESS) {
							fprintf( stderr, "PAT TS Packet rebuild error: %d\n", status);
						}
						else {
							int w = tmp_ts->getBytes( write_buf, 1024);
							if( w > 0) {
								::write( ofd, &write_buf, w);
							}
						}
					}
				}
				else if( ts->getPID() == pmt_pid) {
					PMT *pmt = (PMT *)secs[ pmt_pid];
					if( pmt->finish()) {
						TSPacket *tmp_ts;
						status = pmt->getTSPacket( &tmp_ts, &pmt_counter, ts->getAdaptationField());
						if( status != Section::SUCCESS) {
							fprintf( stderr, "PMT TS Packet rebuild error: %d\n", status);
						}
						else {
							int w = tmp_ts->getBytes( write_buf, 1024);
							if( w > 0) {
								::write( ofd, &write_buf, w);
							}
						}
					}
				}
				else if( save_pid.find( ts->getPID()) != save_pid.end()) {
					int w = ts->getBytes( write_buf, 1024);
					if( memcmp( write_buf, &buf[ index], w) != 0) {
						fprintf( stderr, "TS Packet rebuild error: PID = 0x%04X\n", ts->getPID());
						int i;
						for( i = 0; i < w; i++) {
							if( write_buf[ i] != buf[ index + i]) {
								fprintf( stderr, "TS Packet rebuild error index = %d\n", i);
								break;
							}
						}
					}
					if( w > 0) {
						::write( ofd, &write_buf, w);
					}
				}
			}
#endif
			index += read_len;
		}
		offset = file_read_size - index;
		memmove( buf, buf + index, offset);
	}
	
EXIT:
	if( fd > 0) {
		close( fd);
	}
	if( ofd > 0) {
		close( ofd);
	}

	for( sec_it = secs.begin(); sec_it != secs.end(); sec_it++) {
		delete sec_it->second;
	}
	secs.clear();

	return 0;
}
