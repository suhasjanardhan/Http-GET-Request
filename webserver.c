#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>



#define BUF_SIZE 1024
#define URL_SIZE 128


static char* HTTP_ACCEPTED =
  "HTTP/1.0 200 OK\n"
  "\n";


static char* FILE_NOT_FOUND = 
  "HTTP/1.0 404 Not Found\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>FILE NOT FOUND</h1>\n"
  "  <p>The requested file was not found on this server.</p>\n"
  " </body>\n"
  "</html>\n";


int sock;      /* Server socket should be global */
void appendpath(char* path, const char* file_url,const char*dir);



int main(int argc, char* argv[]) 
{
     	int port;  /* Port we listen on */
     	int accept_socket;
     	char *dir;
        char *host;
     	
     	struct sockaddr_in serv_name;   /* Socket address structure */
     	size_t len = sizeof(serv_name); /* size of the socket address structure */

     	

     	/* Handle any command line arguments */
     	if (argc > 1) 
    	{
          	port = atoi(argv[3]);
	  	dir = argv[2];
                host=argv[1];
     	} 

     	/* Set socket descriptor */
     	if (0 > (sock = socket(AF_INET, SOCK_STREAM, 0))) 
     	{
          	perror("Error creating socket\n");
          	exit(1); 
     	}
     
     	/* set name as the physical address of the socket descriptor */
     	bzero(&serv_name, sizeof(serv_name)); /* initialize */
     	serv_name.sin_family = AF_INET;       /* specify internet socket */     
     	serv_name.sin_port = htons(port);     /* establish listening port */
	serv_name.sin_addr.s_addr=htons(INADDR_ANY); 

	/* to set the socket options- to reuse the given port multiple times */
	int num=1;

	if(setsockopt(sock,SOL_SOCKET,SO_REUSEPORT,(const char*)&num,sizeof(num))<0)
	{
	printf("setsockopt(SO_REUSEPORT) failed\n");
	exit(0);
	}


     	/* connect socket to descriptor */
     	if (0 > bind(sock, (struct sockaddr*)&serv_name, sizeof(serv_name))) 
     	{
          	perror("Error binding listening socket\n");
          	exit(1);
     	}

     	listen(sock, 10);  /* listen for connections on the socket */
	while(1)
	{
     	printf("Listening to client.\n");

     	/* accept  connection from client */
     	accept_socket = accept(sock, (struct sockaddr*)NULL,NULL);
     
     	printf("Connected to client\n");




	time_t timenow;
	struct tm * timeinfo;
	time (&timenow);
	timeinfo = localtime(&timenow);
     	int count;
     	char buffer[BUF_SIZE];
     	char request_str[URL_SIZE];
     	char filename[URL_SIZE];
     	int fd;
	char * header; 
        char *temp;  
	header = (char *)malloc(BUF_SIZE*sizeof(char)); 
     	count = read(accept_socket, buffer, BUF_SIZE-1);
     	buffer[count] = (char)0;
	printf("\n\n%s\n\n",buffer);

     	
     	if (count == 0)
     	{
          	close(accept_socket);
          	return 0;
     	}

     	/* Check for a HTTP GET Method */
     	if (1 > sscanf(buffer, "GET %s", request_str)) 
     	{
          	send(accept_socket,  FILE_NOT_FOUND,strlen( FILE_NOT_FOUND),0);
     	} 
     	else 
     	{
          	printf("Requested File: %s\n", request_str);

		
          	appendpath(filename, request_str,dir);
          	
            
          
	            
		   
          	{
               		printf("FilePath: %s\n", filename);

			sprintf(header, "Date: %sHostname: %s:%d\nLocation: %s\n\n", asctime(timeinfo), host, port, dir);
               		/* Opening the file */
               		fd = open(filename, O_RDONLY);
               		
               	       if (fd < 0) 
               		{
				printf("Requested URL %s is not found on this server.\n",filename);
                    		send(accept_socket, FILE_NOT_FOUND,strlen(FILE_NOT_FOUND),0);
               		} 
               		else 
               		{		                    			                                      
		    		size_t readbytes,sendbytes;
				while((readbytes = read(fd,buffer,1024))>0)
				{	
					send(accept_socket, HTTP_ACCEPTED,strlen(HTTP_ACCEPTED),0);
					send(accept_socket, header,strlen(header),0); 	
					printf("%s\n",buffer);
					if((sendbytes =send(accept_socket,buffer,readbytes,0)) < readbytes)
					{
						perror("send() failed");
						exit(-1);
					}
				}
                    		close(fd);
               		}
          	}
     	}

     	close(accept_socket);
     
}
  	close(sock);
     	return 0;
}


void appendpath(char* path, const char* file_url,const char* dir) 
{
 	  	bzero(path, URL_SIZE);

  	/* Begin with the root path. */
  	strncpy(path, dir, URL_SIZE);

    	strncat(path, file_url, URL_SIZE);
  	return;
}
