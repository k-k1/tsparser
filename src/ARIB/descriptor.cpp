
#include <stdio.h>
#include "descriptor.h"

using namespace TS::Description;
using namespace TS::Description::ARIB;

Descriptor* AribDescriptorParser::create( const uint8_t tag, const uint8_t length)
{
	Descriptor *p = NULL;

	switch( tag) {
	case NetworkName::TAG:
		p = new NetworkName( length);
		break;
	case ServiceList::TAG:
		p = new ServiceList( length);
		break;
	case Service::TAG:
		p = new Service( length);
		break;
	case ShortEvent::TAG:
		p = new ShortEvent( length);
		break;
	case ExtendedEvent::TAG:
		p = new ExtendedEvent( length);
		break;
	case Component::TAG:
		p = new Component( length);
		break;
	case Content::TAG:
		p = new Content( length);
		break;
	case AudioComponent::TAG:
		p = new AudioComponent( length);
		break;
	case EventGroup::TAG:
		p = new EventGroup( length);
		break;
	default:
		p = DescriptorParser::create( tag, length);
	}

	return p;
}


NetworkName::NetworkName( const uint8_t length)
	: Descriptor( TAG, length)
{
}

NetworkName::~NetworkName()
{
}
int16_t NetworkName::parse( const uint8_t *data, const uint16_t len)
{
	int16_t	index = 0;

	if( descriptor_length > 0) {
		name.assign( (char *)&data[ index], descriptor_length);
		index += descriptor_length;
	}
	else {
		name = "";
	}

	return Descriptor::parse( data, len);
//	return index;
}


ServiceList::ServiceList( const uint8_t length)
	: Descriptor( TAG, length)
{
}

ServiceList::~ServiceList()
{
	services.clear();
}
int16_t ServiceList::parse( const uint8_t *data, const uint16_t len)
{
	int16_t	index = 0;
	SERVICE	s;

	services.clear();

	while( descriptor_length > index && descriptor_length - index >= 3) {
		s.service_id	= (data[ index] << 8) + data[ index + 1];
		s.service_type	=  data[ index + 2];
		index += 3;
		
		services.push_back( s);
	}

	return Descriptor::parse( data, len);
//	return index;
}



Service::Service( const uint8_t length)
	: Descriptor( TAG, length)
{
}

Service::~Service()
{
}
int16_t Service::parse( const uint8_t *data, const uint16_t len)
{
	int16_t	index = 0;

	service_type					= data[ index++];
	service_provider_name_length	= data[ index++];
	if( service_provider_name_length > 0) {
		if( descriptor_length - index >= service_provider_name_length) {
			service_provider_name.assign( (char *)&data[ index], service_provider_name_length);
			index += service_provider_name_length;
		}
		else {
			return -1;
		}
	}
	else {
		service_provider_name = "";
	}

	service_name_length				= data[ index++];
	if( service_name_length > 0) {
		if( descriptor_length - index >= service_name_length) {
			service_name.assign( (char *)&data[ index], service_name_length);
			index += service_name_length;
		}
		else {
			return -1;
		}
	}
	else {
		service_name = "";
	}

	return Descriptor::parse( data, len);
//	return index;
}

ShortEvent::ShortEvent( const uint8_t length)
	: Descriptor( TAG, length)
{
}

ShortEvent::~ShortEvent()
{
}
int16_t ShortEvent::parse( const uint8_t *data, const uint16_t len)
{
	int16_t	index = 0;

	ISO_639_language_code.assign( (char *)&data[ index], 3);
	index += 3;
	event_name_length =  data[ index++];
	if( event_name_length > 0) {
		if( descriptor_length - index >= event_name_length) {
			event_name_char.assign( (char *)&data[ index], event_name_length);
			index += event_name_length;
		}
		else {
			return -1;
		}
	}
	else {
		event_name_char = "";
	}

	text_length				= data[ index++];
	if( text_length > 0) {
		if( descriptor_length - index >= text_length) {
			text_char.assign( (char *)&data[ index], text_length);
			index += text_length;
		}
		else {
			return -1;
		}
	}
	else {
		text_char = "";
	}

	return Descriptor::parse( data, len);
//	return index;
}


ExtendedEvent::ExtendedEvent( const uint8_t length)
	: Descriptor( TAG, length)
{
}

ExtendedEvent::~ExtendedEvent()
{
	items.clear();
}
int16_t ExtendedEvent::parse( const uint8_t *data, const uint16_t len)
{
	int16_t			index = 0;
	int16_t			item_index = 0;
	const uint8_t	*buf;
	ITEM			item;

	items.clear();

	descriptor_number		= (data[ index  ] >> 4) & 0x0f;
	last_descriptor_number	= (data[ index++] >> 0) & 0x0f;
	ISO_639_language_code.assign( (char *)&data[ index], 3);
	index += 3;
	length_of_items			=  data[ index++];

	buf = &data[ index];
	while( length_of_items > item_index) {
		item.item_description_length = buf[ item_index++];
		if( item.item_description_length > 0) {
			item.item_description_char.assign( (char *)&buf[ item_index], item.item_description_length);
			item_index += item.item_description_length;
		}
		else {
			item.item_description_char = "";
		}
		item.item_length = buf[ item_index++];
		if( item.item_length > 0) {
			item.item_char.assign( (char *)&buf[ item_index], item.item_length);
			item_index += item.item_length;
		}
		else {
			item.item_char = "";
		}
		items.push_back( item);
	}
	index += item_index;
	
	text_length				= data[ index++];
	if( text_length > 0) {
		text_char.assign( (char *)&data[ index], text_length);
		index += text_length;
	}
	else {
		text_char = "";
	}

	return Descriptor::parse( data, len);
//	return index;
}


Component::Component( const uint8_t length)
	: Descriptor( TAG, length)
{
}

Component::~Component()
{
}

int16_t Component::parse( const uint8_t *data, const uint16_t len)
{
	int16_t	index = 0;
	int16_t	text_len;
	
	reserved_future_use	= (data[ index  ] >> 4) & 0x0f;
	stream_content		= (data[ index++] >> 0) & 0x0f;
	component_type		=  data[ index++];
	component_tag		=  data[ index++];

	ISO_639_language_code.assign( (char *)&data[ index], 3);
	index += 3;
	
	text_len = descriptor_length - index;
	if( text_len > 0) {
		text_char.assign( (char *)&data[ index], text_len);
		index += text_len;
	}
	else {
		text_char = "";
	}

	return Descriptor::parse( data, len);
//	return index;
}


Content::Content( const uint8_t length)
	: Descriptor( TAG, length)
{
}

Content::~Content()
{
	contents.clear();
}
int16_t Content::parse( const uint8_t *data, const uint16_t len)
{
	int16_t			index = 0;
	CONTENT			content;

	contents.clear();

	while( descriptor_length > index) {
		content.content_nibble_level_1	= (data[ index] >> 4) & 0x0f;
		content.content_nibble_level_2	= (data[ index] >> 0) & 0x0f;
		index++;

		content.user_nibble_1			= (data[ index] >> 4) & 0x0f;
		content.user_nibble_2			= (data[ index] >> 0) & 0x0f;
		index++;

		contents.push_back( content);
	}

	return Descriptor::parse( data, len);
//	return index;
}


AudioComponent::AudioComponent( const uint8_t length)
	: Descriptor( TAG, length)
{
}

AudioComponent::~AudioComponent()
{
}

int16_t AudioComponent::parse( const uint8_t *data, const uint16_t len)
{
	int16_t	index = 0;
	int16_t	text_len;
	
	reserved_future_use_1	= (data[ index  ] >> 4) & 0x0f;
	stream_content			= (data[ index++] >> 0) & 0x0f;
	component_type			=  data[ index++];
	component_tag			=  data[ index++];
	stream_type				=  data[ index++];
	simulcast_group_tag		=  data[ index++];
	ES_multi_lingual_flag	= (data[ index  ] >> 7) & 0x01;
	main_component_flag		= (data[ index  ] >> 6) & 0x01;
	quality_indicator		= (data[ index  ] >> 4) & 0x03;
	sampling_rate			= (data[ index  ] >> 1) & 0x07;
	reserved_future_use_2	= (data[ index++] >> 0) & 0x01;

	ISO_639_language_code.assign( (char *)&data[ index], 3);
	index += 3;
	if( ES_multi_lingual_flag) {
		ISO_639_language_code_2.assign( (char *)&data[ index], 3);
		index += 3;
	}
	
	text_len = descriptor_length - index;
	if( text_len > 0) {
		text_char.assign( (char *)&data[ index], text_len);
		index += text_len;
	}
	else {
		text_char = "";
	}

	return Descriptor::parse( data, len);
//	return index;
}


EventGroup::EventGroup( const uint8_t length)
	: Descriptor( TAG, length)
{
}

EventGroup::~EventGroup()
{
	events.clear();
	network_events.clear();
	private_data.clear();
}

const TS::SectionBuffer* EventGroup::getPrivateData()
{
	return &private_data;
}
int16_t EventGroup::parse( const uint8_t *data, const uint16_t len)
{
	int16_t			index = 0;
	int				i;
	EVENT			e;
	NETWORK_EVENT	n;

	group_type	= (data[ index] >> 4) & 0x0f;
	event_count	= (data[ index] >> 0) & 0x0f;
	index++;

	for( i = 0; i < event_count; i++) {
		e.service_id	= (data[ index + 0] << 8) + data[ index + 1];
		e.event_id		= (data[ index + 2] << 8) + data[ index + 3];
		index += 4;
		events.push_back( e);
	}

	if( group_type == 4 || group_type == 5) {
		while( descriptor_length > index) {
			n.original_network_id	= (data[ index + 0] << 8) + data[ index + 1];
			n.transport_stream_id	= (data[ index + 2] << 8) + data[ index + 3];
			n.event.service_id		= (data[ index + 4] << 8) + data[ index + 5];
			n.event.event_id		= (data[ index + 6] << 8) + data[ index + 7];
			index += 8;
			network_events.push_back( n);
		}
	}
	else {
		if( !private_data.create( descriptor_length - index)) {
			return -1;
		}
		private_data.append( &data[ index], descriptor_length - index);
		index = descriptor_length;
	}

	return Descriptor::parse( data, len);
}


