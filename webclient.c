#include <stdio.h>
#define _USE_BSD 1
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#define BUF_SIZE 20000
  
int get_request(char * url, char * port);
int isValidIP(char * ip);
int parseHeader(char * header);
char * splitKeyValue(char * line, int index);
void openFile();


FILE * fileptr;
char keys[][25] = {"Date: ", "Hostname: ", "Location: ", "Content-Type: "};
char status[4] = {0, 0, 0, 0};
char contentFileType[100];
char path[1000];


#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#define MAX_SEND_BUF 1000
#define MAX_RECV_BUF 1000



extern int errno;
int errexit(const char *format,...);
int connectTCP(const char *service,const char *host,char *url,int portnum);
int connectsock(const char *service,const char *host,char *url,int portnum,const char *transport);

/*------------------------------------------------------------------------------------
 * connectsock-Allocate and connect socket for TCP
 *------------------------------------------------------------------------------------
*/

int connectsock(const char *service,const char *host,char *url,int portnum,const char *transport)
{

/*
Arguments:
*service   - service associated with desired port
*host      - name of the host to which connection is desired
*transport - name of the transport protocol to use

*/ 

struct sockaddr_in sin; 
struct hostent *hptr;					 //an internet endpoint address
int s,type;               					 //socket descriptor and socket type 

memset(&sin,0,sizeof(sin));  
sin.sin_family=AF_INET;   				         //family name
  
sin.sin_port=htons(portnum);                                        //port number



/*
 * to determine the type of socket
 */

if(strcmp(transport,"udp")==0)         
type=SOCK_DGRAM;
else
type=SOCK_STREAM;

 char * ptr;
char getrequest[1024];        

 if (isValidIP(url)) { //when an IP address is given
  sprintf(getrequest, "GET / HTTP/1.0\nHOST: %s\n\n", url);
        } else { //when a host name is given
  if ((ptr = strstr(url, "/")) == NULL) {
   //when hostname does not contain a slash
   sprintf(getrequest, "GET / HTTP/1.0\nHOST: %s\n\n", url);
  } else {
   //when hostname contains a slash, it is a path to file
   strcpy(path, ptr);
          host = strtok(url, "/");
   sprintf(getrequest, "GET %s HTTP/1.0\nHOST: %s\n\n", path, url);
  }
 } 


hptr=gethostbyname(url);
bcopy((char*)hptr->h_addr, (char*)&sin.sin_addr.s_addr,hptr->h_length);

printf("%s\n",getrequest);

/* Allocate a socket */

s=socket(AF_INET,type,0);


if(s<0)
errexit("can't create socket : %s\n",strerror(errno));


/*connecting the socket to endpoint*/

if((connect(s,(struct sockaddr *) &sin,sizeof(sin)))<0)        //connect the socket 
errexit("can't connect to %s.%s: %s\n",host,service,strerror(errno));



 printf("Connection successful...\n\n\n");
 ptr = strtok(path, "/");
 strcpy(path, ptr);
 printf("path file=%s\n", path); 

 write(s, getrequest, strlen(getrequest));                     //send HTTP GET request    


return s;

}

/*
 * errexit- print and error message and exit
 */


int errexit(const char* format,...)
{
va_list args;

va_start(args,format);
vfprintf(stderr,format,args);
va_end(args);
exit(1);
}


/*------------------------------------------------------------------------
 * connectTCP-connect to a specified TCP service on specified host
 -------------------------------------------------------------------------*/

int connectTCP(const char *service,const char *host,char *url,int portnum)
{
/*
 Arguments:
 *service-service associated with desired port
 *host-name of the host to which connection is desired
 */

return connectsock(service,host,url,portnum,"tcp");
}


/*
 main - Client file
 */
 

int main(int argc,char *argv[])
{

char *host="localhost";

char *file_name;
ssize_t recv_bytes;
char recv_buff[1000];
int portnum;


char *service="time";                                               //default service port

int s,n;                                                            //socket descriptor


 int  ret; 
 
 char * url, * temp;
 int portNumber;
 char * fileName;
 char status_ok[] = "OK";
 char buffer[BUF_SIZE]; 
 char http_not_found[] = "HTTP/1.0 404 Not Found";
 char http_ok[] = "HTTP/1.0 200 OK";
 char location[] = "Location: ";
 char contentType[] = "Content-Type: ";
 int sPos, ePos;
 clock_t start,end,total;

 if (argc < 3) {
  printf("usage: [URL] [port number]\n");
  exit(1);  
 }

 url = argv[1];
 portNumber = atoi(argv[2]);

 //checking the protocol specified
 if ((temp = strstr(url, "http://")) != NULL) {
  url = url + 7;
 } else if ((temp = strstr(url, "https://")) != NULL) {
  url = url + 8;
 }

start=clock();                                            

s=connectTCP(service,host,url,portNumber); 

memset(&buffer, 0, sizeof(buffer));

strcpy(contentFileType,"text/html");



ssize_t rcvd_bytes;
int x;
int flag = 0;
int y=0;



while((rcvd_bytes = recv(s,buffer,BUF_SIZE,0))>0)
	{
    	/* Create a file or write to the file if it already exists. */
		if(y == 0)
		{
			printf("creating file\n");
			if((x = open(path, O_WRONLY|O_CREAT, 0644))<0)
   				printf("Error creating file\n");
		}
		y = 1;
		printf("Writing to file\n");
		printf("%s\n",buffer);

		/* Write to the file, the data received */
		if(write(x,buffer,rcvd_bytes)<0)
		{
     			printf("error in writing to file");
		}
		flag=1;
		close(x);
		close(s);
	}
	if(flag==0 && rcvd_bytes<=0)
	{
		printf("File not found\n");
	}

	
	end=clock();
	total=(double)(end-start);
	printf("Time required to fetch the file : %f\n", (double)end);

 	return 0;


}



int isValidIP(char * ip) {
 struct sockaddr_in addr;
 int valid = inet_pton(AF_INET, ip, &(addr.sin_addr));
 return valid != 0;
}











                                  




















