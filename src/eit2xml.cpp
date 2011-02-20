
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "epg.h"


void printUsage( void)
{
	const char *usage = 
		"Usage:\n"
		"  eit2xml < BS | CS > < TS file > < XML file >\n"
		"  eit2xml < channel > < TS file > < XML file >\n" 
		"      BS   :         BS\n"
		"      CS   :         CS\n"
		"    channel:         channel id";

	fprintf( stdout, "%s\n", usage);
}


int main( int argc, char **argv)
{
	int ts_fd, xml_fd;
	char *channel;
	char *ts_file;
	char *xml_file;
	
	const uint32_t buf_size = TS::TSPacket::TS_PACKET_SIZE * 20;

	uint8_t		buf[ buf_size];
	uint32_t	index = 0;
	uint32_t	read_len;
	int32_t		offset = 0;
	ssize_t		file_read_size;
	bool		err = false;
	TSPacket	*ts = NULL;
	EPG			*epg = NULL;

	if( argc != 4) {
		printUsage();
		return 1;
	}
	
	channel = argv[ 1];
	ts_file = argv[ 2];
	xml_file = argv[ 3];

	if( strcmp( ts_file, "-") == 0) {
		ts_fd = 0;
	}
	else {
		ts_fd = open( ts_file, O_RDONLY);
		if( ts_fd == -1) {
			fprintf( stderr, "Cannot open %s.\n", ts_file);
			return 1;
		}
	}

	epg = new EPG( channel);

	while( (file_read_size = read( ts_fd, buf + offset, buf_size - offset) + offset) > 0 ) {
		index = 0;
		while( index + TS::TSPacket::TS_PACKET_SIZE <= file_read_size) {
			if( ts) {
				delete ts;
			}

			ts = TS::TSPacket::parse( &buf[ index], file_read_size - index, &read_len);
			if( !ts) {
				if( file_read_size - index <= TS::TSPacket::TS_PACKET_SIZE) {
					break;
				}
				index += TS::TSPacket::TS_PACKET_SIZE;
			}
			else {
				index += read_len;
				if( !epg->append( ts)) {
					//err = true;
					//break;
				}
			}
		}
		if( err) {
			break;
		}

		offset = file_read_size - index;
		if( offset > 0) {
			memmove( buf, buf + index, offset);
		}
	}

EXIT:
	if( ts) {
		delete ts;
		ts = NULL;
	}
	if( epg) {
		epg->write( xml_file);
		delete epg;
	}

	if( ts_fd != 0) {
		close( ts_fd);
	}

	return 0;
}

