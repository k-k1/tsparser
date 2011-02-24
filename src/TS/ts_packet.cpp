
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ts_packet.h"

using namespace TS;

Header::Header()
{
	memset( header, 0x00, TS_HEADER_SZIE);
	header[ 0] = SYNC;
}

Header::~Header()
{
}

int Header::getBytes( uint8_t *buf, uint32_t size) const
{
	if( size < TS_HEADER_SZIE) {
		return -1;
	}

	memcpy( buf, header, TS_HEADER_SZIE);

	return TS_HEADER_SZIE;
}

int Header::parse( const uint8_t *buf, uint32_t len)
{
	if( len < TS_HEADER_SZIE) {
		return -1;
	}

	memcpy( header, buf, TS_HEADER_SZIE);

#ifdef PACKET_DEBUG
	sync_byte						= (buf[ 0] >> 0) & 0xff;
	transport_error_indicator		= (buf[ 1] >> 7) & 0x01;
	payload_unit_start_indicator	= (buf[ 1] >> 6) & 0x01;
	transport_priority				= (buf[ 1] >> 5) & 0x01;
	pid								= ((buf[ 1] & 0x1f) << 8) + buf[ 2];
	transport_scrambling_control	= (buf[ 3] >> 6) & 0x03;
	adaptation_field_control		= (buf[ 3] >> 4) & 0x03;
	continuity_counter				= (buf[ 3] >> 0) & 0x0f;
#endif

	return TS_HEADER_SZIE;
}

AdaptationField::AdaptationField()
{
	adaptation_field	= NULL;

	field_data			= NULL;
	field_data_length	= 0;

#ifdef PACKET_DEBUG
	adaptation_field_length					= 0x00;
	discontinuity_indicator					= 0x00;
	random_access_indicator					= 0x00;
	elementary_stream_priority_indicator	= 0x00;
	PCR_flag								= 0x00;
	OPCR_flag								= 0x00;
	splicing_point_flag						= 0x00;
	transport_private_data_flag				= 0x00;
	adaptation_field_extension_flag			= 0x00;
#endif
}

AdaptationField::AdaptationField( const AdaptationField &src)
{
	copy( &src);
}

AdaptationField::~AdaptationField()
{
	clear();
}

void AdaptationField::clear()
{
	if( adaptation_field) {
		delete[] adaptation_field;
	}
	adaptation_field	= NULL;
	field_data			= NULL;
	field_data_length	= 0;
}

AdaptationField& AdaptationField::operator=( const AdaptationField &src)
{
	clear();
	copy( &src);
	return *this;
}

void AdaptationField::copy( const AdaptationField *src)
{
	const uint8_t af_len = src->getAdaptationFieldLength();

	if( src->adaptation_field) {
		adaptation_field = new uint8_t[ af_len + 1];
		memcpy( adaptation_field, src->adaptation_field, af_len + 1);

		if( src->field_data_length > 0) {
			field_data_length 	= src->field_data_length;
			field_data			= &adaptation_field[ 2];
		}
	}

#ifdef PACKET_DEBUG
	adaptation_field_length					= src->adaptation_field_length;
	discontinuity_indicator					= src->discontinuity_indicator;
	random_access_indicator					= src->random_access_indicator;
	elementary_stream_priority_indicator	= src->elementary_stream_priority_indicator;
	PCR_flag								= src->PCR_flag;
	OPCR_flag								= src->OPCR_flag;
	splicing_point_flag						= src->splicing_point_flag;
	transport_private_data_flag				= src->transport_private_data_flag;
	adaptation_field_extension_flag			= src->adaptation_field_extension_flag;
#endif
}

int AdaptationField::getBytes( uint8_t *buf, uint32_t size) const
{
	int	 index = 0;
	const uint8_t af_len = getAdaptationFieldLength();

	if( size < af_len + 1) {
		return -1;
	}

	memcpy( buf, adaptation_field, af_len + 1);

	return af_len + 1;
}

int AdaptationField::parse( uint8_t *buf, uint32_t len)
{
	int			index = 0;
	uint8_t		af_len;

	if( len < 1) {
		return -1;
	}

	af_len = buf[ 0];
	index++;
	if( len < af_len + index) {
		return -1;
	}

	clear();
	adaptation_field = new uint8_t[ af_len + index];
	memcpy( adaptation_field, buf, af_len + index);

#ifdef PACKET_DEBUG
	adaptation_field_length					= af_len;
#endif

	if( af_len > 0) {
#ifdef PACKET_DEBUG
		discontinuity_indicator					= (buf[ index] >> 7) & 0x01;
		random_access_indicator					= (buf[ index] >> 6) & 0x01;
		elementary_stream_priority_indicator	= (buf[ index] >> 5) & 0x01;
		PCR_flag								= (buf[ index] >> 4) & 0x01;
		OPCR_flag								= (buf[ index] >> 3) & 0x01;
		splicing_point_flag						= (buf[ index] >> 2) & 0x01;
		transport_private_data_flag				= (buf[ index] >> 1) & 0x01;
		adaptation_field_extension_flag			= (buf[ index] >> 0) & 0x01;
#endif
		index++;

		field_data_length = af_len - 1;
		if( field_data_length > 0) {
			field_data = &adaptation_field[ index];
		}
		index += field_data_length;
	}

	return index;
}

TSPacket::TSPacket( Header *h, AdaptationField *af, uint8_t *p, uint32_t p_len)
{
	header				= h;
	adaptation_field	= af;
	
	if( p_len > 0) {
		payload = new uint8_t[ p_len];
		memcpy( payload, p, p_len);
		payload_length = p_len;
	}
	else {
		payload			= NULL;
		payload_length	= 0;
	}
}

TSPacket::~TSPacket()
{
	if( header) {
		delete header;
		header = NULL;
	}
	if( adaptation_field) {
		delete adaptation_field;
		adaptation_field = NULL;
	}
	if( payload) {
		delete[] payload;
		payload = NULL;
	}
	payload_length = 0;
}

uint16_t TSPacket::getPID() const
{
	return header->getPid();
}

Header* TSPacket::getHeader() const
{
	return header;
}

AdaptationField* TSPacket::getAdaptationField() const
{
	return adaptation_field;
}
uint32_t TSPacket::getPayloadLength() const
{
	return payload_length;
}

uint32_t TSPacket::getPayload( uint8_t **p) const
{
	*p = payload;
	return payload_length;
}

int TSPacket::getBytes( uint8_t *buf, uint32_t size) const
{
	int	index = 0;
	int w;

	if( size < TS_PACKET_SIZE) {
		return -1;
	}

	w = header->getBytes( &buf[ index], size - index);
	if( w != Header::TS_HEADER_SZIE) {
		return -1;
	}
	index += w;

	if( adaptation_field) {
		w = adaptation_field->getBytes( &buf[ index], size - index);
		if( w < 0) {
			return -1;
		}
		index += w;
	}

	if( index + payload_length > TS_PACKET_SIZE) {
		return -1;
	}
	memcpy( &buf[ index], payload, payload_length);
	index += payload_length;

	if( index < TS_PACKET_SIZE) {
		memset( &buf[ index], 0xff, TS_PACKET_SIZE - index);
		index += (TS_PACKET_SIZE - index);
	}

	return index;
}


TSPacket* TSPacket::parse( uint8_t *buf, uint32_t len, uint32_t *read_len)
{
	TSPacket		*p	= NULL;
	Header			*h	= NULL;
	AdaptationField	*af	= NULL;
	int index			= 0;
	int sync_index		= 0;
	int payload_len		= 0;
	int read;
	
	*read_len = 0;

	if( (index = TSPacket::sync( buf, len)) == -1) {
		goto ERR;
	}
	if( sync_index != 0) {
		fprintf( stderr, "sync index = %d\n", sync_index);
	}

	if( len - index < TS_PACKET_SIZE) {
		goto ERR;
	}
	sync_index = index;

	h = new Header();
	if( !h) {
		goto ERR;
	}
	if( (read = h->parse( &buf[ index], len - index)) == -1) {
		goto ERR;
	}
	index += read;

	if( h->getTransportErrorIndicator()) {
		//fprintf( stderr, "transport_error_indicator = 1, PID = 0x%04X\n", h->getPid());
		goto ERR;
	}

	if( h->getAdaptationFieldControl() == 0x02 || h->getAdaptationFieldControl() == 0x03) {
		af	= new AdaptationField();
		if( !af) {
			goto ERR;
		}
		if( (read = af->parse( &buf[ index], len - index)) == -1) {
			goto ERR;
		}
		index += read;
	}
	payload_len = TS_PACKET_SIZE - (index - sync_index);
	if( payload_len < 0) {
		payload_len = 0;
	}

	p = new TSPacket( h, af, &buf[ index], payload_len);
	if( !p) {
		goto ERR;
	}
	index += payload_len;
	*read_len = index;

	return p;

ERR:
	if( h) {
		delete h;
	}
	if( af) {
		delete af;
	}
	if( p) {
		delete p;
	}

	return NULL;

}

TSPacket* TSPacket::create( Header *h, AdaptationField *af, uint8_t *p, uint32_t p_len)
{
	TSPacket *ts = new TSPacket( h, af, p, p_len);

	return ts;
}

int TSPacket::sync( uint8_t *buf, uint32_t len)
{
	uint32_t	i;
	int			sync_index = -1;
	for( i = 0; i < len; i++) {
		if( buf[ i] == Header::SYNC) {
			sync_index = (int)i;
			break;
		}
	}
	return sync_index;
}



