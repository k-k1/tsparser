
#include <stdio.h>
#include "descriptor.h"

using namespace TS::Description;

Descriptor* DescriptorParser::parse( const uint8_t *data, const uint16_t len, uint16_t *parse_size)
{
	uint8_t		des_tag;
	uint8_t		des_length;
	int16_t		ret;
	int16_t		index = 0;
	Descriptor	*p = NULL;
	
	*parse_size = 0;

	if( len < 2) {
		return NULL;
	}

	des_tag		= data[ index++];
	des_length	= data[ index++];
	if( des_length > len - index) {
		return NULL;
	}

	p = create( des_tag, des_length);
	if( !p) {
		return p;
	}

	ret = p->parse( &data[ index], len - index);
	if( ret < 0) {
		fprintf( stderr, "Descriptor parse error tag = 0x%02X, len = %d\n", des_tag, des_length);
		delete p;
		p = NULL;
	}
	else if( ret != des_length) {
		fprintf( stderr, "des overflow tag = 0x%02X, len = %d, ret = %d\n", des_tag, des_length, ret);
		for( int i = 0; i < des_length;) {
			for( int j = 0; j < 16 && i < des_length; j++, i++) {
				fprintf( stderr, "%02X ", data[ index + i]);
			}
			fprintf( stderr, "\n");
		}
		delete p;
		p = NULL;
	}
	else {
		index += ret;
		*parse_size = index;
	}

	return p;
}

Descriptor* DescriptorParser::clone( const Descriptor *des)
{
	int16_t		ret;
	Descriptor	*p = NULL;

	p = create( des->descriptor_tag, des->descriptor_length);
	if( !p) {
		return p;
	}

	ret = p->parse( des->descriptor.begin(), des->descriptor.size());
	if( ret < 0) {
		fprintf( stderr, "Descriptor clone error tag = 0x%02X", des->descriptor_tag);
		delete p;
		p = NULL;
	}
	else if( ret != des->descriptor_length) {
		fprintf( stderr, "clone des overflow tag = 0x%02X, len = %d, ret = %d\n", des->descriptor_tag, des->descriptor_length, ret);
		delete p;
		p = NULL;
	}
	return p;
}

Descriptor* DescriptorParser::create( const uint8_t tag, const uint8_t length)
{
	Descriptor *p = NULL;

	switch( tag) {
	case ConditionalAccess::TAG:
		p = new ConditionalAccess( length);
		break;
	default:
		p = new Descriptor( tag, length);
	}
		
	return p;
}

	

Descriptor::Descriptor(  const uint8_t tag, const uint8_t length)
	: descriptor_tag( tag), descriptor_length( length)
{
	descriptor.clear();
}

Descriptor::~Descriptor()
{
	descriptor.clear();
}

int16_t Descriptor::parse( const uint8_t *data, const uint16_t len)
{
	if( !descriptor.create( descriptor_length)) {
		return -1;
	}
	descriptor.append( data, descriptor_length);
	return descriptor_length;
}

int16_t Descriptor::getBytes( TS::SectionBuffer &buf)
{
	if( descriptor.size() != descriptor_length) {
		return -1;
	}

	if( !buf.append( descriptor_tag)) {
		return -1;
	}
	if( !buf.append( descriptor_length)) {
		return -1;
	}
	if( !buf.append( descriptor.begin(), descriptor.size())) {
		return -1;
	}
	buf += descriptor.size() + 2;

	return descriptor.size() + 2;
}


ConditionalAccess::ConditionalAccess( const uint8_t length)
	: Descriptor( TAG, length)
{
}

ConditionalAccess::~ConditionalAccess()
{
	private_data.clear();
}

const TS::SectionBuffer* ConditionalAccess::getPrivateData()
{
	return &private_data;
}

int16_t ConditionalAccess::parse( const uint8_t *data, const uint16_t len)
{
	int16_t			index = 0;

	CA_system_ID	= (data[ index + 0] << 8) + data[ index + 1];
	reserved		= (data[ index + 2] >> 5) & 0x07;
	CA_PID			= ((data[ index + 2] & 0x1f) << 8) + data[ index + 3];
	index += 4;

	if( !private_data.create( descriptor_length - index)) {
		return -1;
	}
	private_data.append( &data[ index], descriptor_length - index);
	index = descriptor_length;

	return Descriptor::parse( data, len);
//	return index;
}



