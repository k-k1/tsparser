
#include <stdio.h>
#include "psi.h"

using namespace TS;
using namespace TS::PSI;
using namespace TS::Description;

Section::Section( int32_t id, DescriptorParser *des_parser)
{
	check_id			= id;
	descriptor_parser	= des_parser;
	ts_packet			= NULL;

	clear();
}

Section::Section( const Section &src)
{
	copy( &src, false);
}

Section::~Section()
{
	clear();
}

void Section::copy(  const Section *src, bool recursive)
{
	table_id					= src->table_id;
	section_syntax_indicator	= src->section_syntax_indicator;
	private_indicator			= src->private_indicator;
	reserved_1					= src->reserved_1;
	section_length				= src->section_length;
	CRC_32						= src->CRC_32;

	section_continue			= src->section_continue;
	erase_buffer				= src->erase_buffer;
	check_crc					= src->check_crc;
	descriptor_parser			= src->descriptor_parser;

	check_id					= src->check_id;
	ts_pid						= src->ts_pid;
	prev_ts_counter				= src->prev_ts_counter;
	parse_end					= src->parse_end;

	if( !erase_buffer) {
		section_buffer			= section_buffer;
	}
	ts_packet					= NULL;
}

Section::STATUS Section::append( const TSPacket *p, uint32_t *payload_index)
{
	uint8_t		*payload, *begin, *end;
	uint32_t	payload_len;
	uint32_t	i;
	int			pointer_field = -1;
	uint8_t		*sec_buf;
	uint16_t	sec_len;
	uint32_t	len = 0;
	STATUS		status;

	if( !(p->getHeader()->getAdaptationFieldControl() & 0x01)) {
		return NO_SECTION;
	}
	
	payload_len = p->getPayload( &payload);
	begin = payload;
	end = payload + payload_len;

	if( p->getHeader()->getPayloadUnitStartIndicator()) {
		if( *payload_index == 0) {
			pointer_field = payload[ 0];
			payload++;
			payload_len--;
		}
		else {
			pointer_field = *payload_index;
		}
	}
	
	if( !section_continue) {
		if( pointer_field == -1) {
			return ERROR_START_SECTION;
		}
		if( payload_len < pointer_field) {
			return ERROR_PAYLOAD_LENGTH;
		}
		payload += pointer_field;
		payload_len -= pointer_field;

		if( !checkID( payload[ 0])) {
			return ERROR_TABLE_ID;
		}

		clear();
		
		if( !section_buffer.create()) {
			return ERROR_ALLOCATE_MEMORY;
		}
	}

	if( (status = checkTSPacket( p)) != SUCCESS) {
		return status;
	}
	
	if( section_buffer.size() < SECTION_PARSE_SIZE) {
		len = SECTION_PARSE_SIZE - section_buffer.size();
		if( len > payload_len) {
			section_buffer.append( payload, payload_len);
			section_continue = true;
			return CONTINUE;
		}
		section_buffer.append( payload, len);
		payload += len;
		payload_len -= len;
		parseHeader( section_buffer);
	}
	
	len = section_length + SECTION_PARSE_SIZE - section_buffer.size();
	if( len > payload_len) {
		section_continue = true;
		len = payload_len;
	}
	else {
		section_continue = false;
	}
	if( !section_buffer.append( payload, len)) {
		return ERROR_SECTION_OVERFLOW;
	}
	payload += len;
	payload_len -= len;
	
	if( section_continue) {
		return CONTINUE;
	}

	if( (status = parse( section_buffer)) != SUCCESS) {
		return status;
	}
	
	if( section_syntax_indicator) {
		if( section_buffer.length() < 4) {
			return ERROR_PARSE_SECTION;
		}
		CRC_32 = 0;
		for( i = 0; i < CRC32_SIZE; i++) {
			CRC_32 |= section_buffer[ i] << ((CRC32_SIZE - 1 - i) * 8);
		}
		section_buffer.reset();
		CRC crc;
		if( check_crc && crc( section_buffer, section_buffer.size())) {
			return ERROR_CRC;
		}
	}
	
	if( erase_buffer) {
		section_buffer.clear();
	}
	else {
		section_buffer.reset();
	}
	parse_end = true;

	if( payload_len > 0 && payload[ 0] != 0xFF) {
		*payload_index = payload - begin;
		return INCLUDE_NEW_SECTION;
	}
	return SUCCESS;
}

bool Section::finish()
{
	return parse_end;
}

Section::STATUS Section::getTSPacket( TSPacket **packet, uint8_t *counter, AdaptationField *af, bool next)
{
	STATUS		status;
	uint8_t		afc = 0;

	TSPacket	*tmp = NULL;
	Header		*h = new Header();
	uint32_t	payload_len = TSPacket::TS_PACKET_SIZE;

	if( next && !ts_packet) {
		next = false;
	}

	if( ts_packet) {
		delete ts_packet;
		ts_packet = NULL;
	}

	if( !next) {
		status = createBytes();
		if( status != SUCCESS) {
#if 0
			if( !af) {
				return status;
			}
#else
			return status;
#endif
		}
		section_buffer.reset();
	}

	payload_len -= h->size();
	if( af) {
		// アダプテーション有り
		afc |= 0x02;
		payload_len -= af->getAdaptationFieldLength();
	}
	if( section_buffer.size() > 0) {
		// ペイロード有り
		if( !next) {
			payload_len -= 1;		// pointer_field
		}

		if( payload_len > 0) {
			afc |= 0x01;
		}
	}

	h->setPid( ts_pid);
	h->setPayloadUnitStartIndicator( next ? 0x00 : 0x01);
	h->setAdaptationFieldControl( afc);
	h->setContinuityCounter( *counter);

	if( (afc & 0x01)) {
		uint8_t payload[ TSPacket::TS_PACKET_SIZE];
		if( payload_len > section_buffer.length()) {
			payload_len = section_buffer.length();
		}
		if( !next) {
			payload[ 0] = 0x00;
			memcpy( &payload[ 1], section_buffer, payload_len);
			section_buffer += payload_len;
			payload_len += 1;
		}
		else {
			memcpy( &payload[ 0], section_buffer, payload_len);
			section_buffer += payload_len;
		}
		ts_packet = TSPacket::create( h, af, payload, payload_len);
//		section_buffer += payload_len;
		(*counter)++;
		(*counter) &= 0x0f;
	}
	else {
		ts_packet = TSPacket::create( h, af, NULL, 0);
	}

	*packet = ts_packet;

	return SUCCESS;
}

void Section::clear()
{
	table_id					= 0x00;
	section_syntax_indicator	= 0x00;
	private_indicator			= 0x00;
	reserved_1					= 0x00;
	section_length				= 0x00;

	CRC_32						= 0x00;

	ts_pid				= 0;
	prev_ts_counter		= 0;

	section_continue	= false;
	erase_buffer		= true;
	check_crc			= true;
	parse_end			= false;

	if( ts_packet) {
		delete ts_packet;
	}
	ts_packet			= NULL;

	section_buffer.clear();
}

int32_t Section::parseDescriptors( const uint8_t *data, const uint16_t len, DESCRIPTORS *list)
{
	int16_t					index = 0;
	uint16_t				size = 0;

	DescriptorParser		parser;
	DescriptorParser		*p = descriptor_parser;
	Descriptor				*des;

	if( !p) {
		p = &parser;
	}

	list->clear();
	while( len - index > 0) {
		des = p->parse( &data[ index], len - index, &size);
		if( !des) {
			fprintf( stderr, "sec_len = %d, des_loop_len = %d, parse_index = %d, tag = 0x%02X, des_len = %d\n",
					 section_length, len, index, data[ index], data[ index + 1]);

			DESCRIPTORS::iterator d;
			for( d = list->begin(); d != list->end(); d++) {
				fprintf( stderr, "0x%02X, %d\n", (*d)->descriptor_tag, (*d)->descriptor_length);
			}
			fprintf( stderr, "\n");
			fprintf( stderr, "Descriptor err\n");
			clearDescriptors( list);
			return -1;
		}
		index += size;
		list->push_back( des);
	}

	return index;
}

bool Section::cloneDescriptors( DESCRIPTORS *dst, const DESCRIPTORS *src)
{
	DescriptorParser		parser;
	DescriptorParser		*p = descriptor_parser;
	Descriptor				*des;

	DESCRIPTORS::const_iterator	src_it;

	if( !p) {
		p = &parser;
	}

	dst->clear();
	for( src_it = src->begin(); src_it != src->end(); src_it++) {
		des = p->clone( *src_it);
		if( !des) {
			return false;
		}
		dst->push_back( des);
	}
	return true;
}

void Section::clearDescriptors( DESCRIPTORS *list)
{
	DESCRIPTORS::iterator i;
	for( i = list->begin(); i != list->end(); i++) {
		if( *i) {
			delete *i;
			*i = NULL;
		}
	}
	list->clear();
}

Section::STATUS Section::checkTSPacket( const TSPacket *p)
{
	uint16_t	pid		= p->getPID();
	uint8_t		counter	= p->getHeader()->getContinuityCounter();

	if( section_continue) {
		if( pid != ts_pid) {
			return ERROR_TS_PID;
		}
		if( counter != ((prev_ts_counter + 1) & 0x0f)) {
			fprintf( stderr, "prev = %2d, cur = %2d, cal = %2d\n", prev_ts_counter, counter, (prev_ts_counter + 1)& 0x0f);
			return ERROR_TS_COUNTER;
		}
	}
	else {
		ts_pid = pid;
	}
	prev_ts_counter = counter;
	return SUCCESS;
}

void Section::parseHeader( SectionBuffer &sec)
{
	table_id					=  sec[ 0];
	section_syntax_indicator	= (sec[ 1] >> 7) & 0x01;
	private_indicator			= (sec[ 1] >> 6) & 0x01;
	reserved_1					= (sec[ 1] >> 4) & 0x03;
	section_length				= ((sec[ 1] & 0x0f) << 8) + sec[ 2];
	sec += SECTION_PARSE_SIZE;
}

Section::STATUS Section::parse( SectionBuffer &sec)
{
	erase_buffer = false;
	return SUCCESS;
}

bool Section::checkID( uint8_t id)
{
	if( check_id != -1) {
		if( check_id != id) {
			return false;
		}
	}
	return true;
}

Section::STATUS Section::createBytes()
{
	STATUS			status;
	SectionBuffer	buf;
	CRC				crc;

	if( !erase_buffer) {
		return SUCCESS;
	}

	if( !buf.create()) {
		return ERROR_ALLOCATE_MEMORY;
	}
	status = getBytes( buf);
	if( status != SUCCESS) {
		return status;
	}
	section_length = buf.size() + CRC32_SIZE;

	if( !section_buffer.create()) {
		return ERROR_ALLOCATE_MEMORY;
	}
	if( !section_buffer.reserve( SECTION_PARSE_SIZE)) {
		return ERROR_GETBYTES_OVERFLOW;
	}
	section_buffer[ 0] = table_id;
	Bits::set( &section_buffer[ 1], section_syntax_indicator      ,  7, 0x01);
	Bits::set( &section_buffer[ 1], private_indicator             ,  6, 0x01);
	Bits::set( &section_buffer[ 1], private_indicator             ,  4, 0x03);
	Bits::set( &section_buffer[ 1], (uint8_t)(section_length >> 8),  0, 0x0f);
	section_buffer[ 2] = section_length & 0xff;
	section_buffer += SECTION_PARSE_SIZE;

	if( !section_buffer.append( buf.begin(), buf.size())) {
		return ERROR_GETBYTES_OVERFLOW;
	}

	uint32_t	crc32 = crc( section_buffer.begin(), section_buffer.size());
	uint8_t		crc_buf[ CRC32_SIZE];
	int i;
	for( i = 0; i < CRC32_SIZE; i++) {
		crc_buf[ i] = (uint8_t)(crc32 >> ((CRC32_SIZE - 1 - i) * 8));
		if( !section_buffer.append( (uint8_t)(crc32 >> ((CRC32_SIZE - 1 - i) * 8)))) {
			return ERROR_GETBYTES_OVERFLOW;
		}
	}
	return SUCCESS;
}

Section::STATUS Section::getBytes( SectionBuffer &buf)
{
	return ERROR_NOT_IMPLEMENT_GETBYTES;
}


ProgramSpecificInfomation::ProgramSpecificInfomation( int32_t id, DescriptorParser *des_parser)
	: Section( id, des_parser)
{
	psi_id.transport_stream_id	= 0x00;
	reserved_2					= 0x00;
	version_number				= 0x00;
	current_next_indicator		= 0x00;
	section_number				= 0x00;
	last_section_number			= 0x00;
}

ProgramSpecificInfomation::ProgramSpecificInfomation( const ProgramSpecificInfomation &src)
	: Section( src)
{
	copy( &src, false);
}

ProgramSpecificInfomation::~ProgramSpecificInfomation()
{
}

void ProgramSpecificInfomation::copy( const Section *src, bool recursive)
{
	if( recursive) {
		Section::copy( src, recursive);
	}

	const ProgramSpecificInfomation *psi = (const ProgramSpecificInfomation *)src;

	psi_id.transport_stream_id	= psi->psi_id.transport_stream_id;
	reserved_2					= psi->reserved_2;
	version_number				= psi->version_number;
	current_next_indicator		= psi->current_next_indicator;
	section_number				= psi->section_number;
	last_section_number			= psi->last_section_number;
}

ProgramSpecificInfomation::STATUS ProgramSpecificInfomation::parse( SectionBuffer &sec)
{
	if( section_syntax_indicator != 0) {
		if( sec.length() < PSI_HEADER_SIZE) {
			return ERROR_PARSE_SECTION;
		}

		psi_id.transport_stream_id	= (sec[ 0] << 8) + sec[ 1];
		reserved_2					= (sec[ 2] >> 6) & 0x03;
		version_number				= (sec[ 2] >> 1) & 0x1f;
		current_next_indicator		= (sec[ 2] >> 0) & 0x01;
		section_number				=  sec[ 3];
		last_section_number			=  sec[ 4];
		
		sec += PSI_HEADER_SIZE;
	}

	return SUCCESS;
}

ProgramSpecificInfomation::STATUS ProgramSpecificInfomation::getBytes( SectionBuffer &buf)
{
	uint16_t id = psi_id.transport_stream_id;

	if( !buf.reserve( PSI_HEADER_SIZE)) {
		return ERROR_GETBYTES_OVERFLOW;
	}

	buf[ 0] = (uint8_t)((id >> 8) & 0xff);
	buf[ 1] = (uint8_t)(id & 0xff);

	Bits::set( &buf[ 2], reserved_2            , 6, 0x03);
	Bits::set( &buf[ 2], version_number        , 1, 0x1f);
	Bits::set( &buf[ 2], current_next_indicator, 0, 0x01);

	buf[ 3] = section_number;
	buf[ 4] = last_section_number;

	buf += PSI_HEADER_SIZE;

	return SUCCESS;
}



PAT::PAT( DescriptorParser *des_parser)
	: ProgramSpecificInfomation( ID, des_parser)
{
	network_PID = 0;
	program_map_PID.clear();
}

PAT::PAT( const PAT &src)
	: ProgramSpecificInfomation( src)
{
	copy( &src, false);
}

PAT::~PAT()
{
	program_map_PID.clear();
}

void PAT::copy( const Section *src, bool recursive)
{
	if( recursive) {
		ProgramSpecificInfomation::copy( src, recursive);
	}

	const PAT *pat = (const PAT *)src;

	network_PID		= pat->network_PID;
	program_map_PID	= pat->program_map_PID;
}

PAT& PAT::operator=( const PAT &src)
{
	clear();
	copy( &src);
	return *this;
}

void PAT::clear()
{
	program_map_PID.clear();
	ProgramSpecificInfomation::clear();
}

PAT::STATUS PAT::parse( SectionBuffer &sec)
{
	uint32_t	i;
	uint16_t	prog_num, pid;
	
	STATUS state = ProgramSpecificInfomation::parse( sec);
	if( state != SUCCESS) {
		return state;
	}

	for( i = 0; i < sec.length() - CRC32_SIZE; i += 4) {
		prog_num	=  (sec[ i + 0] << 8) + sec[ i + 1];
		pid			= ((sec[ i + 2] << 8) + sec[ i + 3]) & 0x1fff;
		if( prog_num == 0x0000) {
			network_PID = pid;
		}
		else {
			program_map_PID[ prog_num] = pid;
		}
	}
	
	sec += i;

	erase_buffer = true;
	return SUCCESS;
}

PAT::STATUS PAT::getBytes( SectionBuffer &buf)
{
	STATUS	status;
	PROGRAM_MAP::iterator	it;

	status = ProgramSpecificInfomation::getBytes( buf);
	if( status != SUCCESS) {
		return status;
	}

	if( network_PID != 0x0000) {
		if( !buf.reserve( 4)) {
			return ERROR_GETBYTES_OVERFLOW;
		}

		buf[ 0] = 0x00;
		buf[ 1] = 0x00;
		buf[ 2] = (uint8_t)((network_PID >> 8) & 0xff);
		buf[ 3] = (uint8_t)(network_PID & 0x1f);
		buf += 4;
	}

	for( it = program_map_PID.begin(); it != program_map_PID.end(); it++) {
		if( !buf.reserve( 4)) {
			return ERROR_GETBYTES_OVERFLOW;
		}

		buf[ 0] = (uint8_t)((it->first >> 8) & 0xff);
		buf[ 1] = (uint8_t)(it->first & 0xff);
		buf[ 2] = (uint8_t)((it->second >> 8) & 0x1f);
		buf[ 3] = (uint8_t)(it->second & 0xff);
		buf += 4;
	}

	return SUCCESS;
}





PMT::PMT( DescriptorParser *des_parser)
	: ProgramSpecificInfomation( ID, des_parser)
{
}

PMT::PMT( const PMT &src)
	: ProgramSpecificInfomation( src)
{
	copy( &src);
}

PMT::~PMT()
{
	clearPMT();
}

void PMT::copy( const Section *src, bool recursive)
{
	if( recursive) {
		ProgramSpecificInfomation::copy( src, recursive);
	}

	const PMT *pmt = (const PMT *)src;

	reserved_3			= pmt->reserved_3;
	PCR_PID				= pmt->PCR_PID;
	reserved_4			= pmt->reserved_4;
	program_info_length	= pmt->program_info_length;

	cloneDescriptors( &program_info_descriptors, &pmt->program_info_descriptors);

	ELEMENT						e;
	ELEMENTS::const_iterator	i;
	for( i = pmt->elements.begin(); i != pmt->elements.end(); i++) {
		e.stream_type			= i->stream_type;
		e.reserved_1			= i->reserved_1;
		e.elementary_PID		= i->elementary_PID;
		e.reserved_2			= i->reserved_2;
		e.ES_info_length		= i->ES_info_length;

		this->cloneDescriptors( &e.descriptors, &i->descriptors);
		elements.push_back( e);
	}
}

PMT& PMT::operator=( const PMT &src)
{
	clear();
	copy( &src);
	return *this;
}

void PMT::clear()
{
	clearPMT();
	ProgramSpecificInfomation::clear();
}

PMT::STATUS PMT::parse( SectionBuffer &sec)
{
	int32_t		size = 0;
	ELEMENT		e;
	
	clearPMT();
	
	STATUS state = ProgramSpecificInfomation::parse( sec);
	if( state != SUCCESS) {
		return state;
	}

	if( sec.length() < PMS_HEADER_SIZE) {
		return ERROR_PARSE_SECTION;
	}
	reserved_3					= (sec[ 0] >> 5) & 0x07;
	PCR_PID						= ((sec[ 0] & 0x1f) << 8) + sec[ 1];
	reserved_4					= (sec[ 2] >> 4) & 0x0f;
	program_info_length			= ((sec[ 2] & 0x0f) << 8) + sec[ 3];
	sec += PMS_HEADER_SIZE;

	if( sec.length() < program_info_length) {
		return ERROR_PARSE_SECTION;
	}
	size = parseDescriptors( sec, program_info_length, &program_info_descriptors);
	if( size == -1) {
		return ERROR_PARSE_SECTION;
	}
	sec += size;
	
	while( sec.length() > ELEMENT_HEADER_SIZE) {
		e.stream_type			= (sec[ 0] >> 0) & 0xff;
		e.reserved_1			= (sec[ 1] >> 5) & 0x07;
		e.elementary_PID		= ((sec[ 1] & 0x1f) << 8) + sec[ 2];
		e.reserved_2			= (sec[ 3] >> 4) & 0x0f;
		e.ES_info_length		= ((sec[ 3] & 0x0f) << 8) + sec[ 4];
		sec += ELEMENT_HEADER_SIZE;
		if( sec.length() < e.ES_info_length) {
			return ERROR_PARSE_SECTION;
		}

		size = parseDescriptors( sec, e.ES_info_length, &e.descriptors);
		if( size == -1) {
			return ERROR_PARSE_SECTION;
		}
		sec += size;
		elements.push_back( e);
	}
	
	erase_buffer = true;
	return SUCCESS;
}

void PMT::clearPMT()
{
	ELEMENTS::iterator		i;
	
	clearDescriptors( &program_info_descriptors);
	
	for( i = elements.begin(); i != elements.end(); i++) {
		clearDescriptors( &i->descriptors);
	}
	elements.clear();
}

void PMT::eraseElement( uint8_t stream_type)
{
	ELEMENTS::iterator	e = elements.begin();

	while( e != elements.end()) {
		if( e->stream_type == stream_type) {
			e = elements.erase( e);
		}
		else {
			e++;
		}
	}

	/*
	for( e = elements.begin(); e != elements.end(); e++) {
		if( e->stream_type == stream_type) {
			e = elements.erase( e);
			continue;
		}
	}
	*/
}

PMT::STATUS PMT::getBytes( SectionBuffer &buf)
{
	STATUS	status;

	status = ProgramSpecificInfomation::getBytes( buf);
	if( status != SUCCESS) {
		return status;
	}

	DESCRIPTORS::iterator	d;
	SectionBuffer			des;
	int32_t					len;
	uint16_t				d_len;

	if( !des.create()) {
		return ERROR_ALLOCATE_MEMORY;
	}
	d_len = 0;
	for( d = program_info_descriptors.begin(); d != program_info_descriptors.end(); d++) {
		len = (*d)->getBytes( des);
		if( len == -1) {
			return ERROR_GETBYTES_DESCRIPTOR;
		}
		d_len += len;
	}
	program_info_length = d_len;

	if( !buf.reserve( PMS_HEADER_SIZE)) {
		return ERROR_GETBYTES_OVERFLOW;
	}
	Bits::set( &buf[ 0], reserved_3                         , 5, 0x07);
	Bits::set( &buf[ 0], (uint8_t)(PCR_PID >> 8)            , 0, 0x1f);
	buf[ 1] = (uint8_t)(PCR_PID & 0xff);
	Bits::set( &buf[ 2], reserved_4                         , 4, 0x0f);
	Bits::set( &buf[ 2], (uint8_t)(program_info_length >> 8), 0, 0x0f);
	buf[ 3] = (uint8_t)(program_info_length & 0xff);
	buf += PMS_HEADER_SIZE;

	if( !buf.append( des.begin(), des.size())) {
		return ERROR_GETBYTES_OVERFLOW;
	}
	buf += des.size();

	ELEMENTS::iterator	e;

	for( e = elements.begin(); e != elements.end(); e++) {
		des.wipe();
		d_len = 0;
		for( d = e->descriptors.begin(); d != e->descriptors.end(); d++) {
			len = (*d)->getBytes( des);
			if( len == -1) {
				return ERROR_GETBYTES_DESCRIPTOR;
			}
			d_len += len;
		}
		e->ES_info_length = d_len;

		if( !buf.reserve( ELEMENT_HEADER_SIZE)) {
			return ERROR_GETBYTES_OVERFLOW;
		}
		buf[ 0] = e->stream_type;
		Bits::set( &buf[ 1], e->reserved_1                    , 5, 0x07);
		Bits::set( &buf[ 1], (uint8_t)(e->elementary_PID >> 8), 0, 0x1f);
		buf[ 2] = (uint8_t)(e->elementary_PID & 0xff);
		Bits::set( &buf[ 3], e->reserved_2                    , 4, 0x0f);
		Bits::set( &buf[ 3], (uint8_t)(e->ES_info_length >> 8), 0, 0x0f);
		buf[ 4] = (uint8_t)(e->ES_info_length & 0xff);
		buf += ELEMENT_HEADER_SIZE;

		if( !buf.append( des.begin(), des.size())) {
			return ERROR_GETBYTES_OVERFLOW;
		}
		buf += des.size();
	}

	return SUCCESS;
}



CAT::CAT( DescriptorParser *des_parser)
	: ProgramSpecificInfomation( ID, des_parser)
{
}

CAT::~CAT()
{
	clearCAT();
}

void CAT::clear()
{
	clearCAT();
	ProgramSpecificInfomation::clear();
}

CAT::STATUS CAT::parse( SectionBuffer &sec)
{
	int32_t         size = 0;

	clearCAT();
	
	STATUS state = ProgramSpecificInfomation::parse( sec);
	if( state != SUCCESS) {
		return state;
	}

	while( sec.length() > CRC32_SIZE) {
		size = parseDescriptors( sec, sec.length() - CRC32_SIZE, &descriptors);
		if( size == -1) {
			return ERROR_PARSE_SECTION;
		}
		sec += size;
	}

	erase_buffer = true;
	return SUCCESS;
}

void CAT::clearCAT()
{
	clearDescriptors( &descriptors);
}





TSDT::TSDT( DescriptorParser *des_parser)
	: ProgramSpecificInfomation( ID, des_parser)
{
}

TSDT::~TSDT()
{
	clearTSDT();
}

void TSDT::clear()
{
	clearTSDT();
	ProgramSpecificInfomation::clear();
}

TSDT::STATUS TSDT::parse( SectionBuffer &sec)
{
	int32_t         size = 0;

	clearTSDT();
	
	STATUS state = ProgramSpecificInfomation::parse( sec);
	if( state != SUCCESS) {
		return state;
	}

	while( sec.length() > CRC32_SIZE) {
		size = parseDescriptors( sec, sec.length() - CRC32_SIZE, &descriptors);
		if( size == -1) {
			return ERROR_PARSE_SECTION;
		}
		sec += size;
	}

	erase_buffer = true;
	return SUCCESS;
}

void TSDT::clearTSDT()
{
	clearDescriptors( &descriptors);
}
