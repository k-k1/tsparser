#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include "utils.h"

namespace TS {

class Header {
public:
	static const int		TS_HEADER_SZIE	= 4;
	static const uint8_t	SYNC			= 0x47;

private:
	uint8_t				header[ TS_HEADER_SZIE];

#ifdef PACKET_DEBUG
	uint8_t				sync_byte;								//  8bit
	uint8_t				transport_error_indicator;				//  1bit
	uint8_t				payload_unit_start_indicator;			//  1bit
	uint8_t				transport_priority;						//  1bit
	uint16_t			pid;									// 13bit
	uint8_t				transport_scrambling_control;			//  2bit
	uint8_t				adaptation_field_control;				//  2bit
	uint8_t				continuity_counter;						//  4bit
#endif

public:
	Header();
	virtual ~Header();

	int					getBytes( uint8_t *buf, uint32_t size);
	int					parse( const uint8_t *buf, uint32_t len);

	uint32_t			size();

	uint8_t				getTransportErrorIndicator();
	uint8_t				getPayloadUnitStartIndicator();
	uint8_t				getTransportPriority();
	uint16_t			getPid();
	uint8_t				getTransportScramblingControl();
	uint8_t				getAdaptationFieldControl();
	uint8_t				getContinuityCounter();

	void				setTransportErrorIndicator( uint8_t val);
	void				setPayloadUnitStartIndicator( uint8_t val);
	void				setTransportPriority( uint8_t val);
	void				setPid( uint16_t val);
	void				setTransportScramblingControl( uint8_t val);
	void				setAdaptationFieldControl( uint8_t val);
	void				setContinuityCounter( uint8_t val) ;
};

class AdaptationField {
private:
	uint8_t				*adaptation_field;

#ifdef PACKET_DEBUG
	uint8_t				adaptation_field_length;				//  8bit
	uint8_t				discontinuity_indicator;				//  1bit
	uint8_t				random_access_indicator;				//  1bit
	uint8_t				elementary_stream_priority_indicator;	//  1bit
	uint8_t				PCR_flag;								//  1bit
	uint8_t				OPCR_flag;								//  1bit
	uint8_t				splicing_point_flag;					//  1bit
	uint8_t				transport_private_data_flag;			//  1bit
	uint8_t				adaptation_field_extension_flag;		//  1bit
#endif

	uint8_t				*field_data;
	uint32_t			field_data_length;
	
public:
	AdaptationField();
	AdaptationField( AdaptationField &src);
	virtual ~AdaptationField();

	int					getBytes( uint8_t *buf, uint32_t size);
	int					parse( uint8_t *buf, uint32_t len);

	uint32_t			size();

	uint8_t				getAdaptationFieldLength();					//  8bit
	uint8_t				getDiscontinuityIndicator();					//  1bit
	uint8_t				getRandomAccessIndicator();					//  1bit
	uint8_t				getElementaryStreamPriorityIndicator();	//  1bit
	uint8_t				getPCRFlag();									//  1bit
	uint8_t				getOPCRFlag();									//  1bit
	uint8_t				getSplicingPointFlag();						//  1bit
	uint8_t				getTransportPrivateDataFlag();				//  1bit
	uint8_t				getAdaptationFieldExtensionFlag();			//  1bit


	AdaptationField&	operator=( AdaptationField &src);

private:
	void			copy( AdaptationField &dst);
	void			clear();
};


class TSPacket {
public:
	static const int TS_PACKET_SIZE = 188;

private:
	Header				*header;
	AdaptationField		*adaptation_field;
    uint8_t				*payload;
	uint32_t			payload_length;

private:
	TSPacket( Header *h, AdaptationField *af, uint8_t *p, uint32_t p_len);

public:
	virtual ~TSPacket();

public:
	uint16_t			getPID();
	Header*				getHeader();
	AdaptationField*	getAdaptationField();
	uint32_t			getPayloadLength();
	uint32_t			getPayload( uint8_t **p);

	int					getBytes( uint8_t *buf, uint32_t size);

	static TSPacket*	parse( uint8_t *buf, uint32_t len, uint32_t *read_len);
	static TSPacket*	create( Header *h, AdaptationField *af, uint8_t *p, uint32_t p_len);

private:
	static int			sync( uint8_t *buf, uint32_t len) ;
};



inline uint32_t Header::size()
{
	return TS_HEADER_SZIE;
}

inline uint8_t Header::getTransportErrorIndicator()
{
	return Bits::get( header[ 1], 7, 0x01);
}

inline void Header::setTransportErrorIndicator( uint8_t val)
{
	Bits::set( &header[ 1], val, 7, 0x01);
}

inline uint8_t Header::getPayloadUnitStartIndicator()
{
	return Bits::get( header[ 1], 6, 0x01);
}

inline void Header::setPayloadUnitStartIndicator( uint8_t val)
{
	Bits::set( &header[ 1], val, 6, 0x01);
}

inline uint8_t Header::getTransportPriority()
{
	return Bits::get( header[ 1], 5, 0x01);
}

inline void Header::setTransportPriority( uint8_t val)
{
	Bits::set( &header[ 1], val, 5, 0x01);
}

inline uint16_t Header::getPid()
{
	return (Bits::get( header[ 1], 0, 0x1f) << 8) + header[ 2];
}

inline void Header::setPid( uint16_t val)
{
	Bits::set( &header[ 1], (uint8_t)(val >> 8), 0, 0x1f);
	header[ 2] = val & 0xff;
}

inline uint8_t Header::getTransportScramblingControl()
{
	return Bits::get( header[ 3], 6, 0x03);
}

inline void Header::setTransportScramblingControl( uint8_t val)
{
	Bits::set( &header[ 3], val, 6, 0x03);
}

inline uint8_t Header::getAdaptationFieldControl()
{
	return Bits::get( header[ 3], 4, 0x03);
}

inline void Header::setAdaptationFieldControl( uint8_t val)
{
	Bits::set( &header[ 3], val, 4, 0x03);
}

inline uint8_t Header::getContinuityCounter()
{
	return Bits::get( header[ 3], 0, 0x0f);
}

inline void Header::setContinuityCounter( uint8_t val)
{
	Bits::set( &header[ 3], val, 0, 0x0f);
}



inline uint32_t AdaptationField::size()
{
	return getAdaptationFieldLength() + 1;
}

inline uint8_t AdaptationField::getAdaptationFieldLength()
{
	if( adaptation_field) {
		return adaptation_field[ 0];
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getDiscontinuityIndicator()
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 7, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getRandomAccessIndicator()
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 6, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getElementaryStreamPriorityIndicator()
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 5, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getPCRFlag()
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 4, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getOPCRFlag()
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 3, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getSplicingPointFlag()
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 2, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getTransportPrivateDataFlag()
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 1, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getAdaptationFieldExtensionFlag()
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 0, 0x01);
	}
	else {
		return 0;
	}
}

}

