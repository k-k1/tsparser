#pragma once

#include <inttypes.h>
#include <list>
#include <string>
#include "../TS/descriptor.h"
#include "../TS/utils.h"


namespace TS {
	namespace Description {
		namespace ARIB {

class AribDescriptorParser : public TS::Description::DescriptorParser
{
protected:
	TS::Description::Descriptor*	create( const uint8_t tag, const uint8_t length);
};


class NetworkName : public TS::Description::Descriptor
{
public:
	static const uint8_t	TAG	= 0x40;

	std::string				name;

	NetworkName( const uint8_t length);
	virtual ~NetworkName();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};


class ServiceList : public TS::Description::Descriptor
{
public:
	static const uint8_t	TAG = 0x41;
	struct SERVICE {
		uint16_t				service_id;
		uint8_t					service_type;
	};

	typedef	std::list< SERVICE>	SERVICES;

	SERVICES				services;

	ServiceList( const uint8_t length);
	virtual ~ServiceList();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};


class Service : public TS::Description::Descriptor
{
public:
	static const uint8_t	TAG = 0x48;

	uint8_t					service_type;
	uint8_t					service_provider_name_length;
	std::string				service_provider_name;
	uint8_t					service_name_length;
	std::string				service_name;

	Service( const uint8_t length);
	virtual ~Service();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};

class ShortEvent : public TS::Description::Descriptor
{
public:
	static const uint8_t	TAG	= 0x4D;

	std::string				ISO_639_language_code;
	uint8_t					event_name_length;
	std::string				event_name_char;
	uint8_t					text_length;
	std::string				text_char;

	ShortEvent( uint8_t length);
	virtual ~ShortEvent();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};

class ExtendedEvent : public TS::Description::Descriptor
{
public:
	struct ITEM {
		uint8_t				item_description_length;
		std::string			item_description_char;
		uint8_t				item_length;
		std::string			item_char;
	};

	typedef	std::list< ITEM>	ITEMS;

	static const uint8_t	TAG = 0x4E;

	uint8_t					descriptor_number;
	uint8_t					last_descriptor_number;
	std::string				ISO_639_language_code;
	uint8_t					length_of_items;
	ITEMS					items;
	uint8_t					text_length;
	std::string				text_char;
	
	ExtendedEvent( const uint8_t length);
	virtual ~ExtendedEvent();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};


class Component : public TS::Description::Descriptor
{
public:
	static const uint8_t	TAG	= 0x50;

	uint8_t					reserved_future_use;
	uint8_t					stream_content;
	uint8_t					component_type;
	uint8_t					component_tag;
	std::string				ISO_639_language_code;
	std::string				text_char;

	Component( uint8_t length);
	virtual ~Component();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};


class Content : public TS::Description::Descriptor
{
public:
	struct CONTENT {
		uint8_t				content_nibble_level_1;
		uint8_t				content_nibble_level_2;
		uint8_t				user_nibble_1;
		uint8_t				user_nibble_2;
	};

	typedef	std::list< CONTENT>	CONTENTS;

	static const uint8_t	TAG = 0x54;

	CONTENTS				contents;
	
	Content( const uint8_t length);
	virtual ~Content();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};


class AudioComponent : public TS::Description::Descriptor
{
public:
	static const uint8_t	TAG	= 0xC4;

	uint8_t					reserved_future_use_1;
	uint8_t					stream_content;
	uint8_t					component_type;
	uint8_t					component_tag;
	uint8_t					stream_type;
	uint8_t					simulcast_group_tag;
	uint8_t					ES_multi_lingual_flag;
	uint8_t					main_component_flag;
	uint8_t					quality_indicator;
	uint8_t					sampling_rate;
	uint8_t					reserved_future_use_2;
	std::string				ISO_639_language_code;
	std::string				ISO_639_language_code_2;
	std::string				text_char;

	AudioComponent( uint8_t length);
	virtual ~AudioComponent();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};


class EventGroup : public TS::Description::Descriptor
{
public:
	struct EVENT {
		uint16_t			service_id;
		uint16_t			event_id;
	};

	struct NETWORK_EVENT {
		uint16_t			original_network_id;
		uint16_t			transport_stream_id;
		EVENT				event;
	};

	typedef	std::list< EVENT>			EVENTS;
	typedef	std::list< NETWORK_EVENT>	NETWORK_EVENTS;

	static const uint8_t	TAG	= 0xD6;

	uint8_t					group_type;			//  4bit
	uint8_t					event_count;		//  4bit
	EVENTS					events;
	NETWORK_EVENTS			network_events;

private:
	SectionBuffer			private_data;
	
public:
	EventGroup( const uint8_t length);
	virtual ~EventGroup();

	const SectionBuffer*	getPrivateData();

private:
	int16_t		parse( const uint8_t *data, const uint16_t len);
};

		}
	}
}

