
#include <stdio.h>
#include "si.h"

using namespace TS::PSI;
using namespace TS::PSI::ARIB;
using namespace TS::Description;
using namespace TS::Description::ARIB;

ServiceInformation::ServiceInformation( int32_t id,  DescriptorParser *des_parser)
	: Section( id, des_parser)
{
	si_id.transport_stream_id	= 0x00;
	reserved_2					= 0x00;
	version_number				= 0x00;
	current_next_indicator		= 0x00;
	section_number				= 0x00;
	last_section_number			= 0x00;
}

ServiceInformation::~ServiceInformation()
{
}

ServiceInformation::STATUS ServiceInformation::parse( SectionBuffer &sec)
{
	if( section_syntax_indicator != 0) {
		if( sec.length() < SI_PARSE_SIZE) {
			return ERROR_PARSE_SECTION;
		}

		si_id.transport_stream_id	= (sec[ 0] << 8) + sec[ 1];
		reserved_2					= (sec[ 2] >> 6) & 0x03;
		version_number				= (sec[ 2] >> 1) & 0x1f;
		current_next_indicator		= (sec[ 2] >> 0) & 0x01;
		section_number				=  sec[ 3];
		last_section_number			=  sec[ 4];
		
		sec += SI_PARSE_SIZE;
	}

	return SUCCESS;
}


NIT::NIT( DescriptorParser *des_parser)
	: ServiceInformation( -1, des_parser)
{
	reserved_future_use1			= 0x00;
	network_descriptors_length		= 0x00;
	reserved_future_use2			= 0x00;
	transport_stream_loop_length	= 0x00;
}

NIT::~NIT()
{
	clearNIT();
}

void NIT::clear()
{
	clearNIT();
	ServiceInformation::clear();
}

NIT::STATUS NIT::parse( SectionBuffer &sec)
{
	int32_t		size = 0;
	STREAM		s;
	
	clearNIT();
	
	STATUS state = ServiceInformation::parse( sec);
	if( state != SUCCESS) {
		return state;
	}

	if( sec.length() < 2) {
		return ERROR_PARSE_SECTION;
	}
	reserved_future_use1			= (sec[ 0] >> 4) & 0x0f;
	network_descriptors_length		= ((sec[ 0] & 0x0f) << 8) + sec[ 1];
	sec += 2;
	if( sec.length() < network_descriptors_length) {
		return ERROR_PARSE_SECTION;
	}
	size = parseDescriptors( sec, network_descriptors_length, &networks);
	if( size == -1) {
		return ERROR_PARSE_SECTION;
	}
	sec += size;
	
	if( sec.length() < 2) {
		return ERROR_PARSE_SECTION;
	}
	reserved_future_use2			= (sec[ 0] >> 4) & 0x0f;
	transport_stream_loop_length	= ((sec[ 0] & 0x0f) << 8) + sec[ 1];
	sec += 2;
	
	while( sec.length() >= STREAM_HEADER_SIZE) {
		s.transport_stream_id	 		= (sec[ 0] << 8) + sec[ 1];
		s.original_network_id			= (sec[ 2] << 8) + sec[ 3];
		s.reserved_future_use			= (sec[ 4] >> 4) & 0x0f;
		s.transport_descriptors_length	= ((sec[ 4] & 0x0f) << 8) + sec[ 5];
		sec += STREAM_HEADER_SIZE;
		if( sec.length() < s.transport_descriptors_length) {
			return ERROR_PARSE_SECTION;
		}

		size = parseDescriptors( sec, s.transport_descriptors_length, &s.descriptors);
		if( size == -1) {
			return ERROR_PARSE_SECTION;
		}
		sec += size;
		streams.push_back( s);
	}
	
	erase_buffer = true;
	return SUCCESS;
}

bool NIT::checkID( uint8_t id)
{
	if( id == 0x40 || id <= 0x41) {
		return true;
	}
	else {
		return false;
	}
}

void NIT::clearNIT()
{
	STREAMS::iterator		i;
	
	clearDescriptors( &networks);
	
	for( i = streams.begin(); i != streams.end(); i++) {
		clearDescriptors( &i->descriptors);
	}
	streams.clear();
}


SDT::SDT( DescriptorParser *des_parser)
	: ServiceInformation( -1, des_parser)
{
	original_network_id	= 0x00;
	reserved_future_use	= 0x00;
}

SDT::~SDT()
{
	clearSDT();
}

void SDT::clear()
{
	clearSDT();
	ServiceInformation::clear();
}

SDT::STATUS SDT::parse( SectionBuffer &sec)
{
	int32_t		size = 0;
	SERVICE		s;
	
	clearSDT();
	
	STATUS state = ServiceInformation::parse( sec);
	if( state != SUCCESS) {
		return state;
	}
	
	if( sec.length() < SD_PARSE_SIZE) {
		return ERROR_PARSE_SECTION;
	}
	original_network_id			= (sec[ 0] << 8) + sec[ 1];
	reserved_future_use			=  sec[ 2];
	sec += SD_PARSE_SIZE;
	
	while( sec.length() >= SERVICE_HEADER_SIZE) {
		s.service_id			 		= (sec[ 0] << 8) + sec[ 1];
		s.reserved_future_use			= (sec[ 2] >> 5) & 0x07;
		s.EIT_user_defined_flags		= (sec[ 2] >> 2) & 0x07;
		s.EIT_schedule_flag				= (sec[ 2] >> 1) & 0x01;
		s.EIT_present_following_flag	= (sec[ 2] >> 0) & 0x01;
		s.running_status				= (sec[ 3] >> 5) & 0x07;
		s.free_CA_mode					= (sec[ 3] >> 4) & 0x01;
		s.descriptors_loop_length		= ((sec[ 3] & 0x0f) << 8) + sec[ 4];
		sec += SERVICE_HEADER_SIZE;
		if( sec.length() < s.descriptors_loop_length) {
			return ERROR_PARSE_SECTION;
		}

		size = parseDescriptors( sec, s.descriptors_loop_length, &s.descriptors);
		if( size == -1) {
			return ERROR_PARSE_SECTION;
		}
		sec += size;
		services.push_back( s);
	}
	
	erase_buffer = true;
	return SUCCESS;
}

bool SDT::checkID( uint8_t id)
{
	if( id == 0x42 || id <= 0x46) {
		return true;
	}
	else {
		return false;
	}
}

void SDT::clearSDT()
{
	SERVICES::iterator		i;
	
	for( i = services.begin(); i != services.end(); i++) {
		clearDescriptors( &i->descriptors);
	}
	services.clear();
}



EIT::EIT( DescriptorParser *des_parser)
	: ServiceInformation( -1, des_parser)
{
	transport_stream_id			= 0x00;
	original_network_id			= 0x00;
	segment_last_section_number	= 0x00;
	last_table_id				= 0x00;
}

EIT::~EIT()
{
	clearEIT();
}

void EIT::clear()
{
	clearEIT();
	ServiceInformation::clear();
}

EIT::STATUS EIT::parse( SectionBuffer &sec)
{
	int32_t		size = 0;
	EVENT		e;
	
	clearEIT();
	
	STATUS state = ServiceInformation::parse( sec);
	if( state != SUCCESS) {
		return state;
	}
	
	if( sec.length() < EI_PARSE_SIZE) {
		return ERROR_PARSE_SECTION;
	}
	transport_stream_id 		= (sec[ 0] << 8) + sec[ 1];
	original_network_id			= (sec[ 2] << 8) + sec[ 3];
	segment_last_section_number	=  sec[ 4];
	last_table_id				=  sec[ 5];
	sec += EI_PARSE_SIZE;
	
	while( sec.length() >= EVENT_HEADER_SIZE) {
		e.event_id					= (sec[ 0] << 8) + sec[ 1];
		e.start_time				= Converter::date( &sec[ 2]);
		e.duration					= Converter::time( &sec[ 7]);
		e.running_status			= (sec[ 10] >> 5) & 0x07;
		e.free_CA_mode				= (sec[ 10] >> 4) & 0x01;
		e.descriptors_loop_length	= ((sec[ 10] & 0x0f) << 8) + sec[ 11];
		sec += EVENT_HEADER_SIZE;
		if( sec.length() < e.descriptors_loop_length) {
			return ERROR_PARSE_SECTION;
		}

		size = parseDescriptors( sec, e.descriptors_loop_length, &e.descriptors);
		if( size == -1) {
			return ERROR_PARSE_SECTION;
		}
		sec += size;
		events.push_back( e);
	}
	
	erase_buffer = true;
	return SUCCESS;
}

bool EIT::checkID( uint8_t id)
{
	if( id >= 0x4E && id <= 0x6F) {
		return true;
	}
	else {
		return false;
	}
}

void EIT::clearEIT()
{
	EVENTS::iterator		i;
	
	for( i = events.begin(); i != events.end(); i++) {
		clearDescriptors( &i->descriptors);
	}
	events.clear();
}



TDT::TDT( DescriptorParser *des_parser)
	: Section( -1, des_parser)
{
}

TDT::~TDT()
{
}

TDT::STATUS TDT::parse( SectionBuffer &sec)
{
	int32_t		size = 0;
	
	if( sec.length() < TD_PARSE_SIZE) {
		return ERROR_PARSE_SECTION;
	}
	
	JST_time				= Converter::date( &sec[ 0]);
	sec += TD_PARSE_SIZE;
	
	erase_buffer = true;
	return SUCCESS;
}

bool TDT::checkID( uint8_t id)
{
	return (id == TDT_ID);
}




TOT::TOT( DescriptorParser *des_parser)
	: TDT( des_parser)
{
}

TOT::~TOT()
{
	clearTOT();
}

void TOT::clear()
{
	clearTOT();
	Section::clear();
}

TOT::STATUS TOT::parse( SectionBuffer &sec)
{
	int32_t		size = 0;
	
	clearTOT();
	
	STATUS state = TDT::parse( sec);
	if( state != SUCCESS) {
		return state;
	}
	if( table_id == TDT_ID) {
		return SUCCESS;
	}
	
	if( sec.length() < TO_PARSE_SIZE) {
		return ERROR_PARSE_SECTION;
	}
	
	reserved_2				= (sec[ 0] >> 4) & 0x0f;
	descriptors_loop_length	= ((sec[ 0] & 0x0f) << 8) + sec[ 1];
	sec += TO_PARSE_SIZE;

	size = parseDescriptors( sec, descriptors_loop_length, &descriptors);
	if( size == -1) {
		return ERROR_PARSE_SECTION;
	}
	sec += size;
	
	erase_buffer = true;
	return SUCCESS;
}

bool TOT::checkID( uint8_t id)
{
	if( TDT::checkID( id)) {
		return true;
	}
	else if( id == TOT_ID) {
		return true;
	}
	else {
		return false;
	}
}

void TOT::clearTOT()
{
	clearDescriptors( &descriptors);
}

