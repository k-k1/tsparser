#pragma once

#include <inttypes.h>
#include <list>
#include <string>
#include "utils.h"


namespace TS {
	namespace Description {

class Descriptor;

class DescriptorParser
{
public:
	Descriptor*	parse( const uint8_t *data, const uint16_t len, uint16_t *parse_size);

protected:
	virtual Descriptor*	create( const uint8_t tag, const uint8_t length);
};


class DescriptorList : public TS::List< Descriptor>
{

};


class Descriptor
{
	friend class DescriptorParser;

public:
	static const uint8_t	TAG	= 0x00;
	const uint8_t			descriptor_tag;
	const uint8_t			descriptor_length;

private:
	TS::SectionBuffer		descriptor;

protected:
	Descriptor( const uint8_t tag, const uint8_t length);

public:
	virtual ~Descriptor();

	virtual int16_t		getBytes( SectionBuffer &buf);

protected:
	virtual int16_t		parse( const uint8_t *data, const uint16_t len);

};

class ConditionalAccess : public Descriptor
{
public:
	static const uint8_t	TAG	= 0x09;

	uint16_t				CA_system_ID;		// 16bit
	uint8_t					reserved;			//  3bit
	uint16_t				CA_PID;				// 13bit

private:
	TS::SectionBuffer		private_data;
	
public:
	ConditionalAccess( const uint8_t length);
	virtual ~ConditionalAccess();

	const SectionBuffer*	getPrivateData();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};

	}
}

