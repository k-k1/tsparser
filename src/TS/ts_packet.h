#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include "utils.h"

namespace TS {

enum PID {
	PID_PAT		= 0x0000,
	PID_CAT		= 0x0001,
	PID_TSDT	= 0x0002,
	PID_NIT		= 0x0010,
	PID_SDT		= 0x0011,
	PID_EIT		= 0x0012,
	PID_TDT		= 0x0014,
	PID_TOT		= 0x0014,

	PID_EIT1	= 0x0026,
	PID_EIT2	= 0x0027,
};

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

	int					getBytes( uint8_t *buf, uint32_t size) const;
	int					parse( const uint8_t *buf, uint32_t len);

	uint32_t			size() const;

	uint8_t				getTransportErrorIndicator() const;
	uint8_t				getPayloadUnitStartIndicator() const;
	uint8_t				getTransportPriority() const;
	uint16_t			getPid() const;
	uint8_t				getTransportScramblingControl() const;
	uint8_t				getAdaptationFieldControl() const;
	uint8_t				getContinuityCounter() const;

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
	const static int	PCR_SIZE	= 6;

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
	AdaptationField( const AdaptationField &src);
	virtual ~AdaptationField();

	int					getBytes( uint8_t *buf, uint32_t size) const;
	int					parse( uint8_t *buf, uint32_t len);

	uint32_t			size() const;

	uint8_t				getAdaptationFieldLength() const;				//  8bit
	uint8_t				getDiscontinuityIndicator() const;				//  1bit
	uint8_t				getRandomAccessIndicator()  const;				//  1bit
	uint8_t				getElementaryStreamPriorityIndicator() const;	//  1bit
	uint8_t				getPCRFlag()  const;								//  1bit
	uint8_t				getOPCRFlag()  const;								//  1bit
	uint8_t				getSplicingPointFlag() const;					//  1bit
	uint8_t				getTransportPrivateDataFlag() const;			//  1bit
	uint8_t				getAdaptationFieldExtensionFlag() const;		//  1bit

	// PCR
	uint64_t			getProgramClockReferenceBase() const;			// 33bit
	uint16_t			getProgramClockReferenceExtension() const;		//  9bit
	uint64_t			getProgramClockReference() const;

	AdaptationField&	operator=( const AdaptationField &src);

private:
	void			copy( const AdaptationField *src);
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
	uint16_t			getPID() const;
	Header*				getHeader() const;
	AdaptationField*	getAdaptationField() const;
	uint32_t			getPayloadLength() const;
	uint32_t			getPayload( uint8_t **p) const;

	int					getBytes( uint8_t *buf, uint32_t size) const;

	static TSPacket*	parse( uint8_t *buf, uint32_t len, uint32_t *read_len);
	static TSPacket*	create( Header *h, AdaptationField *af, uint8_t *p, uint32_t p_len);

private:
	static int			sync( uint8_t *buf, uint32_t len) ;
};



inline uint32_t Header::size() const
{
	return TS_HEADER_SZIE;
}

inline uint8_t Header::getTransportErrorIndicator() const
{
	return Bits::get( header[ 1], 7, 0x01);
}

inline void Header::setTransportErrorIndicator( uint8_t val)
{
	Bits::set( &header[ 1], val, 7, 0x01);
}

inline uint8_t Header::getPayloadUnitStartIndicator() const
{
	return Bits::get( header[ 1], 6, 0x01);
}

inline void Header::setPayloadUnitStartIndicator( uint8_t val)
{
	Bits::set( &header[ 1], val, 6, 0x01);
}

inline uint8_t Header::getTransportPriority() const
{
	return Bits::get( header[ 1], 5, 0x01);
}

inline void Header::setTransportPriority( uint8_t val)
{
	Bits::set( &header[ 1], val, 5, 0x01);
}

inline uint16_t Header::getPid() const
{
	return (Bits::get( header[ 1], 0, 0x1f) << 8) + header[ 2];
}

inline void Header::setPid( uint16_t val)
{
	Bits::set( &header[ 1], (uint8_t)(val >> 8), 0, 0x1f);
	header[ 2] = val & 0xff;
}

inline uint8_t Header::getTransportScramblingControl() const
{
	return Bits::get( header[ 3], 6, 0x03);
}

inline void Header::setTransportScramblingControl( uint8_t val)
{
	Bits::set( &header[ 3], val, 6, 0x03);
}

inline uint8_t Header::getAdaptationFieldControl() const
{
	return Bits::get( header[ 3], 4, 0x03);
}

inline void Header::setAdaptationFieldControl( uint8_t val)
{
	Bits::set( &header[ 3], val, 4, 0x03);
}

inline uint8_t Header::getContinuityCounter() const
{
	return Bits::get( header[ 3], 0, 0x0f);
}

inline void Header::setContinuityCounter( uint8_t val)
{
	Bits::set( &header[ 3], val, 0, 0x0f);
}



inline uint32_t AdaptationField::size() const
{
	return getAdaptationFieldLength() + 1;
}

inline uint8_t AdaptationField::getAdaptationFieldLength() const
{
	if( adaptation_field) {
		return adaptation_field[ 0];
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getDiscontinuityIndicator() const
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 7, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getRandomAccessIndicator() const
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 6, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getElementaryStreamPriorityIndicator() const
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 5, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getPCRFlag() const
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 4, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getOPCRFlag() const
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 3, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getSplicingPointFlag() const
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 2, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getTransportPrivateDataFlag() const
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 1, 0x01);
	}
	else {
		return 0;
	}
}

inline uint8_t AdaptationField::getAdaptationFieldExtensionFlag() const
{
	if( adaptation_field) {
		return Bits::get( adaptation_field[ 1], 0, 0x01);
	}
	else {
		return 0;
	}
}

inline uint64_t	 AdaptationField::getProgramClockReferenceBase() const
{
	uint64_t base = 0;
	if( getPCRFlag() && field_data && field_data_length >= PCR_SIZE) {
		base =	((uint64_t)field_data[ 0] << 25) + (field_data[ 1] << 17) +
				(field_data[ 2] <<  9) + (field_data[ 3] <<  1) +
				((field_data[ 4] >>  7) & 0x01);
	}
	return base;
}

inline uint16_t	 AdaptationField::getProgramClockReferenceExtension() const
{
	uint16_t ext = 0;
	if( getPCRFlag() && field_data && field_data_length >= PCR_SIZE) {
		ext = 	((field_data[ 4] & 0x01) << 8) + field_data[ 5];
	}
	return ext;
}

inline uint64_t AdaptationField::getProgramClockReference() const
{
	uint64_t base	= getProgramClockReferenceBase();
	uint16_t ext	= getProgramClockReferenceExtension();
	return (base * 300 + ext);
}

}

