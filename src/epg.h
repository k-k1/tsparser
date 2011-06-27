#pragma once

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "TS/ts_packet.h"
#include "TS/psi.h"
#include "ARIB/si.h"

using namespace TS;
using namespace TS::PSI;
using namespace TS::PSI::ARIB;
using namespace TS::Description::ARIB;

class EPG
{
private:
	struct CONTENT {
		const char *jpn;
		const char *eng;
	};

	struct SERVICE {
		uint16_t				transport_stream_id;
		uint16_t				original_network_id;
		uint16_t				service_id;

		struct Less : public std::binary_function< SERVICE, SERVICE, bool> {
			bool operator()( const SERVICE &a, const SERVICE &b) const {
				if( a.original_network_id < b.original_network_id) {
					return true;
				}
				else if( a.transport_stream_id < a.transport_stream_id) {
					return true;
				}
				else if( a.service_id < b.service_id) {
					return true;
				}
				else {
					return false;
				}
			}
		};
	};

	struct EXTEND {
		struct ITEM {
			std::string				description;
			std::string				item;
			
			void clear() {
				description.clear();
				item.clear();
			}
		};

		typedef std::list< ITEM>	ITEMS;

		ITEMS						items;
		std::string					text;
		
		void clear() {
			items.clear();
			text.clear();
		}
	};

	struct EVENT {
		typedef std::vector< uint8_t>	CATEGORYS;

		uint16_t				event_id;
		time_t					start_time;
		time_t					end_time;
		std::string				title;
		std::string				desc;
		CATEGORYS				categorys;

		uint8_t					table_id;

		bool					extend_flag;
		EXTEND					extend;

		EVENT() {
			extend_flag = false;
		}
#if 0
		struct Less : public std::binary_function< EVENT, EVENT, bool> {
			bool operator()( const EVENT &a, const EVENT &b) const {
				return (a.start_time < b.start_time);
			}
		};
#endif
	};

	typedef std::map< uint8_t , EIT*>			EITS;

	typedef std::map< SERVICE, std::string, SERVICE::Less>	SERVICES;
//	typedef std::set< EVENT, EVENT::Less>					EVENTS;
	typedef std::map< uint16_t, EVENT>						EVENTS;
	typedef std::map< SERVICE, EVENTS, SERVICE::Less>		EPG_DATA;

	static const int32_t		EIT_PID_NUM	= 3;
	static const uint16_t		SDT_PID		= 0x0011;
	static const uint16_t		EIT_PID[ EIT_PID_NUM];
	static const int32_t		CONTENT_NUM = 16;
		
	static const CONTENT		contents[ CONTENT_NUM];

	AribDescriptorParser		descriptor_parser;
	SDT							*sdt;
	EITS						eits;

	SERVICES					services;
//	EVENTS						events;
	EPG_DATA					epg_data;

	const	std::string			channel;

public:
	EPG( std::string ch);
	~EPG();

	bool	append( TSPacket *ts);
	bool	write( const char *xml_file);

private:
	bool	appendSDT( TSPacket *ts);
	bool	appendEIT( TSPacket *ts);

	void	appendEvnet( EIT *p);

	const std::string	createChannelID( const SERVICE *s);

	void	convertCharactor( std::string *str);

	bool	writeHeader( int fd);
	bool	writeChannel( int fd);
	bool	writeFooter( int fd);
};

