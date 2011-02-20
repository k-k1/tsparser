#pragma once

#include <inttypes.h>
#include <list>
#include "../TS/psi.h"
#include "../TS/utils.h"
#include "descriptor.h"
#include "converter.h"

namespace TS {
	namespace PSI {
		namespace ARIB {

class ServiceInformation : public TS::PSI::Section
{
protected:
	static const uint32_t		SI_PARSE_SIZE	= 5;

public:
	union ID {
		uint16_t		network_id;				// NIT, TLV-NIT
		uint16_t		bouquet_id;				// BAT
		uint16_t		transport_stream_id;	// SDT
		uint16_t		service_id;				// EIT, PCAT
		uint16_t		original_network_id;	// BIT, NBIT
		uint16_t		original_service_id;	// LDT
		uint16_t		table_id_extension;		// AMT
	} si_id;									// 16bit

	uint8_t				reserved_2;				//  2bit
	uint8_t				version_number;			//  5bit
	uint8_t				current_next_indicator;	//  1bit
	uint8_t				section_number;			//  8bit
	uint8_t				last_section_number;	//  8bit

	ServiceInformation( int32_t id = -1, TS::Description::DescriptorParser *des_parser = NULL);
	virtual ~ServiceInformation();

protected:
	STATUS			parse( SectionBuffer &sec);
	virtual STATUS	parseSection( SectionBuffer &sec);
};

class NIT : public ServiceInformation
{
private:
	static const uint32_t		STREAM_HEADER_SIZE		= 6;
	
public:
	struct STREAM {
		uint16_t		transport_stream_id;			// 16bit
		uint16_t		original_network_id;			// 16bit
		uint8_t			reserved_future_use;			//  4bit
		uint16_t		transport_descriptors_length;	// 12bit
		DESCRIPTORS		descriptors;
	};
	typedef std::list< STREAM>	STREAMS;
	
	uint8_t				reserved_future_use1;			//  4bit
	uint16_t			network_descriptors_length;		// 12bit
	DESCRIPTORS			networks;
	uint8_t				reserved_future_use2;			//  4bit
	uint16_t			transport_stream_loop_length;	// 12bit
	STREAMS				streams;
	
public:
	NIT( TS::Description::DescriptorParser *des_parser = NULL);
	virtual ~NIT();

	void	clear();
	
private:
	STATUS	parseSection( SectionBuffer &sec);
	bool	checkID( uint8_t id);

	void	clearNIT();
};


// SDT 0x42, 0x46
class SDT : public ServiceInformation
{
private:
	static const uint32_t		SD_PARSE_SIZE		= 3;
	static const uint32_t		SERVICE_HEADER_SIZE	= 5;

public:
	struct SERVICE {
		uint16_t		service_id;						// 16bit
		uint8_t			reserved_future_use;			//  3bit
		uint8_t			EIT_user_defined_flags;			//  3bit
		uint8_t			EIT_schedule_flag;				//  1bit
		uint8_t			EIT_present_following_flag;		//  1bit
		uint8_t			running_status;					//  3bit
		uint8_t			free_CA_mode;					//  1bit
		uint16_t		descriptors_loop_length;		// 12bit
		DESCRIPTORS		descriptors;
	};
	typedef	std::list< SERVICE>	SERVICES;

	uint16_t			original_network_id;			// 16bit
	uint8_t				reserved_future_use;			//  8bit
	SERVICES			services;
	
public:
	SDT( TS::Description::DescriptorParser *des_parser = NULL);
	virtual ~SDT();

	void	clear();
	
private:
	STATUS	parseSection( SectionBuffer &sec);
	bool	checkID( uint8_t id);

	void	clearSDT();
};


// EIT
class EIT : public ServiceInformation
{
private:
	static const uint32_t	EI_PARSE_SIZE		= 6;
	static const uint32_t	EVENT_HEADER_SIZE	= 12;

public:
	struct EVENT {
	public:
		uint16_t		event_id;						// 16bit
		time_t			start_time;						// 40bit
		int32_t			duration;						// 24bit sec
		uint8_t			running_status;					//  3bit
		uint8_t			free_CA_mode;					//  1bit
		uint16_t		descriptors_loop_length;		// 12bit
		DESCRIPTORS		descriptors;
	};
	typedef	std::list< EVENT>	EVENTS;

	uint16_t			transport_stream_id;			// 16bit
	uint16_t			original_network_id; 			// 16bit
	uint8_t				segment_last_section_number;	//  8bit
	uint8_t				last_table_id;					//  8bit
	EVENTS				events;
	
public:
	EIT( TS::Description::DescriptorParser *des_parser = NULL);
	virtual ~EIT();

	void	clear();
	
private:
	STATUS	parseSection( SectionBuffer &sec);
	bool	checkID( uint8_t id);

	void	clearEIT();
};

		}
	}
}

/*
network_information_section(){
	reserved_future_use 4 bslbf
	network_descriptors_length 12 uimsbf

bouquet_association_section(){
	reserved_future_use 4 bslbf
	bouquet_descriptors_length 12 uimsbf

service description section(){
	original_network_id 16 uimsbf
	reserved_future_use 8 bslbf

event_information_section(){
	transport_stream_id 16 uimsbf
	original_network_id 16 uimsbf
	segment_last_section_number 8 uimsbf
	last_table_id 8 uimsbf

partial_content_announcement_section(){
	transport_stream_id 16 uimsbf
	original_network_id 16 uimsbf
	content_id 32 uimsbf
	num_of_content_version 8 uimsbf

broadcaster_information _section(){
	reserved_future_use 3 bslbf
	broadcast_view_propriety 1 bslbf
	first_descriptors_length 12 uimsbf

network_board_information_section()

linked_description_section(){
	transport_stream_id 16 uimsbf
	original_network_id 16 uimsbf

TLV_network_information_table( ){
	reserved_future_use 4 bslbf
	network_descriptors_length 12 bslbf

address_map_table() {
	num_of_service_id 10 uimsbf
	reserved_future_use 6 bslbf

*/
