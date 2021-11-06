#ifndef _ULT_H
#define _ULT_H
#include"stdio.h"
#include "types.h"



void Systick_Init (uint8 SYSCLK);
void Delay_s( uint32 time_s );
void Delay_us(uint32 time_us);
void Delay_ms(uint32 time_ms);


uint16 ATOI(char* str,uint16 base); 			/* Convert a string to integer number */
uint32 ATOI32(char* str,uint16 base); 			/* Convert a string to integer number */
void itoa(uint16 n,uint8* str, uint8 len);
int ValidATOI(char* str, int base, int* ret); 		/* Verify character string and Convert it to (hexa-)decimal. */
char C2D(u_char c); 					/* Convert a character to HEX */

uint16 swaps(uint16 i);
uint32 swapl(uint32 l);

void replacetochar(char * str, char oldchar, char newchar);

void mid(int8* src, int8* s1, int8* s2, int8* sub);
void inet_addr_(unsigned char* addr,unsigned char *ip);

char* inet_ntoa(unsigned long addr);			/* Convert 32bit Address into Dotted Decimal Format */
char* inet_ntoa_pad(unsigned long addr);

//unsigned long inet_addr(unsigned char* addr);		/* Converts a string containing an (Ipv4) Internet Protocol decimal dotted address into a 32bit address */

char VerifyIPAddress_orig(char* src);			/* Verify dotted notation IP address string */
char VerifyIPAddress(char* src, uint8 * ip);
unsigned long GetDestAddr(SOCKET s);			/* Output destination IP address of appropriate channel */
unsigned int GetDestPort(SOCKET s);			/* Output destination port number of appropriate channel */
unsigned short htons( unsigned short hostshort);	/* htons function converts a unsigned short from host to TCP/IP network byte order (which is big-endian).*/
unsigned long htonl(unsigned long hostlong);		/* htonl function converts a unsigned long from host to TCP/IP network byte order (which is big-endian). */
unsigned long ntohs(unsigned short netshort);		/* ntohs function converts a unsigned short from TCP/IP network byte order to host byte order (which is little-endian on Intel processors). */
unsigned long ntohl(unsigned long netlong);		/* ntohl function converts a u_long from TCP/IP network order to host byte order (which is little-endian on Intel processors). */
u_char CheckDestInLocal(u_long destip);			/* Check Destination in local or remote */

SOCKET getSocket(unsigned char status, SOCKET start); 	/* Get handle of socket which status is same to 'status' */
unsigned short checksum(unsigned char * src, unsigned int len);		/* Calculate checksum of a stream */

#endif
