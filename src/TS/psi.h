#pragma once

#include <inttypes.h>
#include <string.h>
#include <list>
#include <map>
#include "ts_packet.h"
#include "descriptor.h"
#include "utils.h"

namespace TS {
	namespace PSI {

class Section {
protected:
	static const uint32_t	SECTION_PARSE_SIZE	= 3;
	static const uint32_t	CRC32_SIZE			= 4;

public:
	typedef	std::list< TS::Description::Descriptor *>	DESCRIPTORS;
	enum STATUS {
		SUCCESS,
		CONTINUE,
		INCLUDE_NEW_SECTION,
		NO_SECTION,

		ERROR_TS_PID,
		ERROR_TS_COUNTER,
		ERROR_TABLE_ID,
		ERROR_PAYLOAD_LENGTH,
		ERROR_START_SECTION,
		ERROR_ALLOCATE_MEMORY,
		ERROR_SECTION_OVERFLOW,
		ERROR_PARSE_SECTION,
		ERROR_CRC,

		ERROR_NOT_IMPLEMENT_GETBYTES,
		ERROR_GETBYTES_DESCRIPTOR,
		ERROR_GETBYTES_OVERFLOW,
	};
		
	uint8_t				table_id;					//  8bit
	uint8_t				section_syntax_indicator;	//  1bit
	uint8_t				private_indicator;			//  1bit
	uint8_t				reserved_1;					//  2bit
	uint16_t			section_length;				// 12bit

	uint32_t			CRC_32;						// 32bit;

protected:
	bool				section_continue;
	bool				erase_buffer;
	bool				check_crc;

	 TS::Description::DescriptorParser	*descriptor_parser;

private:
	bool				parse_end;
	int32_t				check_id;
	uint16_t			ts_pid;
	uint8_t				prev_ts_counter;
	SectionBuffer		section_buffer;

	TSPacket*			ts_packet;

public:
	Section( int32_t id = -1, TS::Description::DescriptorParser *des_parser = NULL);
	virtual ~Section();

protected:
	Section( const Section &src);

public:

	STATUS				append( const TSPacket *p, uint32_t *payload_index);
	bool				finish();
	virtual void		clear();
	STATUS				getTSPacket( TSPacket **packet, uint8_t *counter, AdaptationField *af, bool next = true);

protected:
	virtual void		copy( const Section *src, bool recursive = true);

	int32_t				parseDescriptors( const uint8_t *data, const uint16_t len, DESCRIPTORS *list);
	bool				cloneDescriptors( DESCRIPTORS *dst, const DESCRIPTORS *src);
	void				clearDescriptors( DESCRIPTORS *list);


private:
	STATUS				checkTSPacket( const TSPacket *p);
	void				parseHeader( SectionBuffer &sec);
	virtual STATUS		parse( SectionBuffer &sec);
	virtual bool		checkID( uint8_t id);
	STATUS				createBytes();
	virtual STATUS		getBytes( SectionBuffer &buf);
};


class ProgramSpecificInfomation : public Section
{
protected:
	static const uint32_t	PSI_HEADER_SIZE	= 5;

public:
	union ID {
		uint16_t	transport_stream_id;		// PAT
		uint16_t	reserved;					// CAT, TSDT
		uint16_t	program_number;				// PMT
		//uint16_t	table_id_extension;			// private section
	} psi_id;									// 16bit
	
	uint8_t			reserved_2;					//  2bit
	uint8_t			version_number;				//  5bit
	uint8_t			current_next_indicator;		//  1bit
	uint8_t			section_number;				//  8bit
	uint8_t			last_section_number;		//  8bit

	ProgramSpecificInfomation( int32_t id = -1, TS::Description::DescriptorParser *des_parser = NULL);
	virtual ~ProgramSpecificInfomation();

protected:
	ProgramSpecificInfomation( const ProgramSpecificInfomation &src);

	virtual STATUS	parse( SectionBuffer &sec);
	virtual STATUS	getBytes( SectionBuffer &buf);
	//void			copy( const ProgramSpecificInfomation *src);
	virtual void	copy( const Section *src, bool recursive = true);
};


// PAT
class PAT : public ProgramSpecificInfomation
{
public:
	static const int32_t	ID = 0x00;
	typedef	std::map< uint16_t, uint16_t>	PROGRAM_MAP;

	uint16_t	network_PID;
	PROGRAM_MAP	program_map_PID;

	PAT( TS::Description::DescriptorParser *des_parser = NULL);
	PAT( const PAT &src);
	virtual ~PAT();

public:
	void			clear();
	PAT&			operator=( const PAT &src);

private:
	STATUS			parse( SectionBuffer &sec);
	STATUS			getBytes( SectionBuffer &buf);
	void			copy( const Section *src, bool recursive = true);
};


// PMT
class PMT : public ProgramSpecificInfomation
{
public:
	static const int32_t	ID					= 0x02;
	static const uint32_t	PMS_HEADER_SIZE		= 4;
	static const uint32_t	ELEMENT_HEADER_SIZE	= 5;

	struct ELEMENT {
		uint8_t			stream_type;					//  8bit
		uint8_t			reserved_1;						//  3bit
		uint16_t		elementary_PID;					// 13bit
		uint8_t			reserved_2;						//  4bit
		uint16_t		ES_info_length;					// 12bit
		DESCRIPTORS		descriptors;
	};
	typedef std::list< ELEMENT>	ELEMENTS;

	uint8_t				reserved_3;						//  3bit
	uint16_t			PCR_PID;						// 13bit
	uint8_t				reserved_4;						//  4bit
	uint16_t			program_info_length;			// 12bit
	DESCRIPTORS			program_info_descriptors;
	ELEMENTS			elements;

	PMT( TS::Description::DescriptorParser *des_parser = NULL);
	PMT( const PMT &src);
	virtual ~PMT();

	void			clear();
	void			eraseElement( uint8_t stream_type);
	PMT&			operator=( const PMT &src);

private:
	STATUS			parse( SectionBuffer &sec);
	STATUS			getBytes( SectionBuffer &buf);
	void			clearPMT();
	void			copy( const Section *src, bool recursive = true);

};

// CAT
class CAT : public ProgramSpecificInfomation
{
public:
	static const int32_t	ID = 0x01;

	DESCRIPTORS				descriptors;

	CAT( TS::Description::DescriptorParser *des_parser = NULL);
	virtual ~CAT();

	void			clear();

private:
    STATUS			parse( SectionBuffer &sec);
    void			clearCAT();
};

// TSDT
class TSDT : public ProgramSpecificInfomation
{
public:
	static const int32_t	ID = 0x03;

	DESCRIPTORS				descriptors;

	TSDT( TS::Description::DescriptorParser *des_parser = NULL);
	virtual ~TSDT();

	void			clear();

private:
    STATUS			parse( SectionBuffer &sec);
    void			clearTSDT();
};


	}
}

