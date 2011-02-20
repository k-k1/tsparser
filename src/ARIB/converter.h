#pragma once

#include <inttypes.h>
#include <time.h>
#include <stdlib.h>


namespace TS {
	namespace PSI {
		namespace ARIB {

class Converter
{
public:
	static time_t	date( const uint8_t *d) {
		tm t;
		int Y, M, D;
		int K = 0;
		char str[ 3];
		const uint16_t mdj = (d[ 0] << 8) + d[ 1];
		
		Y = (int)((mdj - 15078.2) / 365.25);
		M = (int)((mdj - 14956.1 - (int)(Y * 365.25)) / 30.6001);
		D = mdj - 14956 - (int)(Y * 365.25) - (int)(M * 30.6001);
		if( M == 14 || M == 15) {
			K = 1;
		}
		Y += K;
		M = M - 1 - K * 12;
		
		t.tm_year	= Y;
		t.tm_mon	= M - 1;
		t.tm_mday	= D;

		sprintf( str, "%02X", d[ 2]);
		t.tm_hour = atoi( str);
		sprintf( str, "%02X", d[ 3]);
		t.tm_min = atoi( str);
		sprintf( str, "%02X", d[ 4]);
		t.tm_sec = atoi( str);
		
		return mktime( &t);
	}
	
	static int32_t	time( const uint8_t *d) {
		int32_t buf;
		int32_t time = 0;
		char str[ 3];
		
		sprintf( str, "%02X", d[ 0]);
		buf = atoi( str);
		time += buf * 3600;
		
		sprintf( str, "%02X", d[ 1]);
		buf = atoi( str);
		time += buf * 60;
		
		sprintf( str, "%02X", d[ 2]);
		buf = atoi( str);
		time += buf;
		
		return time;
	}

};

		}
	}
}

