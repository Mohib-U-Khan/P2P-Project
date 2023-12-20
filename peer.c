/*******************************************************************************
* Title                 :   P2P Project
* Filename              :   peer.c
* Author                :   Thomas Pazhaidam, Mohib Khan, Edwin Chen
* Origin Date           :   11/11/2023
* Version               :   0.0.1
* Notes                 :   None
*******************************************************************************/
/** \file peer.c
 * \brief This module contains functionality of peers for a peer to peer application
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#define BUFFLEN 100
#define PACKETSIZE 101
#define	NAMESIZE	10
/******************************************************************************
* Module Typedefs
*******************************************************************************/
struct PDU {
    char type;
    char data[BUFFLEN];
};

struct pdu {
 	char type;
	char peerName[NAMESIZE];
	char contentName[NAMESIZE];
	struct	sockaddr_in addr;
 };
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
enum MsgType {
    REGISTER = 114, DOWNLOAD = 100, SEARCH = 115, DEREGISTER = 116, DATA = 99,
    ONLINE = 111, ACK = 97, ERR = 101, QUIT = 113
};
/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Request user to specify peer name from terminal
*******************************************************************************/
void GetPeerName(char *PeerName)
{
    int outLen = 0;
    //memset(&PeerName, 0, sizeof(PeerName));
    printf("Specify peer name (max 10 characters): \n");
    outLen = read(0, PeerName, 10);
    PeerName[outLen-1] = 0;
}

/******************************************************************************
* Request list of online content on server
*******************************************************************************/
void RequestOnlineContent(int Socket)
{
    struct PDU msg, resp;
    memset(&msg, 0, sizeof(msg));
    memset(&resp, 0, sizeof(resp));
    msg.type = ONLINE;

    write(Socket, &msg, sizeof(msg));
    read(Socket, &resp, sizeof(resp));

    printf("Content available: \n");
    printf("%s\n", resp.data);
}

/******************************************************************************
* Initialize TCP Connection and returns tcp server socket and server response
*******************************************************************************/
int InitializeTCPUpload(int UdpSocket, char* PeerName, char* Filename, struct PDU *Response, struct sockaddr_in reg_addr)
{
	//printf("function start"); 
	fflush(stdout);   
	// Make tcp connection
	int 	tcp_socket, alen;
	struct	sockaddr_in server;
    struct  in_addr ip_addr_struct;
	struct 	PDU msg;
	char 	tcpAddr[15], tcpPort[6], send_data[100];
	//printf("socket start"); 
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "Can't create a socket\n");
    exit(1);
	}
    bzero((char*)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    // server.sin_port = htons(0);
    // server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    server.sin_port = reg_addr.sin_port;
    server.sin_addr.s_addr = reg_addr.sin_addr.s_addr;
    
    //get current ip
    // struct ifreq ifr;
    // char array[] = "enp0s3";
    // strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
    // ioctl(tcp_socket, SIOCGIFADDR, &ifr);
    
    // if(bind(tcp_socket,(struct sockaddr*)&server,sizeof(server))==-1)
    // {
    //     fprintf(stdin, "cant bind name to socket.\n");
    //     return -1;
    // }
    alen = sizeof(struct sockaddr_in);
    
	//printf("get socket name"); 
	fflush(stdout);  
	//getsockname(tcp_socket, (struct sockaddr*)&server, &alen);
    ip_addr_struct.s_addr = server.sin_addr.s_addr;
    sprintf(tcpAddr, "%s", inet_ntoa(ip_addr_struct));
    printf("Here: %s\n", tcpAddr);
    //printf("%s",tcpAddr);
    sprintf(tcpPort, "%d", server.sin_port);
    //printf("%d",server.sin_port);
    
    memset(&msg, 0, sizeof(msg));

    msg.type = REGISTER;
    
    char data[BUFFLEN] = {0};
    strcpy(data, PeerName);
    strcat(data, ";");
    strcat(data, Filename);
    strcat(data, ";");
    strcat(data, tcpAddr);
    strcat(data, ";");
    strcat(data, tcpPort);
    strcpy(msg.data, data);
    printf("Here is the data: %s\n", msg.data);

    write(UdpSocket, &msg, sizeof(msg));
    fflush(stdout);  
    read(UdpSocket, Response, sizeof(Response));
    fflush(stdout);  
    return tcp_socket;
}

/******************************************************************************
* Register new content
*******************************************************************************/
void RegisterContent(int Socket, char* PeerName, struct sockaddr_in reg_addr)
{
    struct PDU msg, resp;

    printf("File name to register: ");
    fflush(stdout);
    char Filename[25] = {0};
    
    int outLen = 0;
    outLen = read(0, Filename, sizeof(Filename));
    Filename[outLen-1] = 0;
    struct stat info;
    if(access(Filename, F_OK) != 0)
    {
        printf("Unable to find file. \n");
	fflush(stdout);
        return;
    }
    
    fflush(stdout);
    int tcpSocket = InitializeTCPUpload(Socket, PeerName, Filename, &resp, reg_addr);
    if(tcpSocket < 0)
    {
        return;
    }
    if(resp.type == (char)ERR)
    {
        printf("Can't register data. \n");
	    fflush(stdout);
    }
    else if(resp.type == (char)ACK)
    {
       memset(&resp, 0, sizeof(resp));
       memset(&msg, 0, sizeof(msg));

       //listen(tcpSocket, 5);
       struct sockaddr_in client;
       int clientlen = sizeof(client);
       int newTcpSocket=0;
       //newTcpSocket = accept(tcpSocket, (struct sockaddr *)&client, &clientlen);
    //    if((newTcpSocket)<0)
    //    {
    //         fprintf(stdout, "can't accept client\n");
    //         fflush(stdout); 
    //         return;
    //    }
    //    printf("created socket.\n"); 
    //    fflush(stdout); 
    //    switch(fork()){
    //     case 0:
    //         (void) close(tcpSocket);
    //         read(newTcpSocket, &resp, sizeof(resp));
    //         if(resp.type == (char)DOWNLOAD)
    //         {
    //             FILE *file;
    //             file = fopen(Filename, "rb");
    //             msg.type = (char)DATA;
    //             while(fgets(msg.data, BUFFLEN, file)>0){
    //                 sleep(0.01);
    //                 write(newTcpSocket, &msg, sizeof(msg));
    //             }
    //             fclose(file);
    //         }
    //         (void) close(newTcpSocket);
    //         exit(0);
    //     default:
    //         (void) close(newTcpSocket);
    //         break;
    //     case -1:
    //         fprintf(stderr, "fork: error \n");
    //    }
    }
}

/******************************************************************************
* Initialize TCP Connection and returns tcp client socket
*******************************************************************************/
int InitializeTCPDownload(char* Address, char* Port)
{
 // Make tcp connection
 int tcp_socket;
 struct sockaddr_in server;
 struct hostent *host;
 //char *serverHost = "0";
 printf("This is the address: %s\n", Address);
 printf("This is the port: %s\n", Port);
printf("initializing download\n");
 if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
 fprintf(stderr, "Can't create a socket\n");
 exit(1);
 }
 
 bzero((char*)&server, sizeof(struct sockaddr_in));
 server.sin_family = AF_INET; 
 int port = atoi(Port);
 
 server.sin_port = port; 
 printf("%d", server.sin_port);
 if (host = gethostbyname(Address)){
 bcopy(host->h_addr, (char *)&server.sin_addr, host->h_length);
 }
 else if (inet_aton(Address, (struct in_addr *) &server.sin_addr)){
 fprintf(stderr, "Can't get server's address\n");
 } 

 if (connect(tcp_socket, (struct sockaddr *)&server, sizeof(server)) == -1){
 fprintf(stderr, "cant connect to server: %s\n", host->h_name);
 }
printf("done init download\n");
 return tcp_socket;
}

/******************************************************************************
* Download Content
*******************************************************************************/
void DownloadContent(int Socket, char *SelfPeerName, struct sockaddr_in reg_addr) 
{
   int n;
   printf("Enter name of content to download: \n");
   int outLen = 0;
   char Filename[50] = {0};
   outLen = read(0, Filename, sizeof(Filename));
   Filename[outLen-1] = 0;
   char peerName[50] = {0};

   printf("Enter name of peer to download from: \n");
   outLen = read(0, peerName, sizeof(peerName));
   peerName[outLen-1]=0;

   struct PDU msg;
   memset(&msg, 0, PACKETSIZE);
   msg.type = (char) SEARCH;
   strcpy(msg.data, peerName);
   strcat(msg.data, ";");
   strcat(msg.data, Filename);

   write(Socket, &msg, sizeof(msg));

   struct PDU resp;
   read(Socket, &resp, sizeof(resp));

   char temp[BUFFLEN] = {0};
   strcpy(temp, resp.data);
   
   //memset(peerName, 0, sizeof(peerName));
   char *contentName = strtok(temp, ";");
   char *tmpPeerName = strtok(NULL, ";");
   char *addr = strtok(NULL, ";");
   char *port = strtok(NULL, ";");

   int tcpSocket = InitializeTCPDownload(addr, port);

   memset(&msg, 0, PACKETSIZE);
   memset(&resp, 0, PACKETSIZE);

   msg.type = (char) DOWNLOAD;
   strcpy(msg.data, SelfPeerName);
   strcat(msg.data, ";");
   strcat(msg.data, contentName);
    printf("Message_Data: %s\n", msg.data);
   write(tcpSocket, &msg, sizeof(msg));

   FILE *file = 0;
   file = fopen(contentName, "wb");

   int contentExists = 1;
   while (1) {
		n = recv(tcpSocket, &resp, PACKETSIZE, 0);
		resp.data[BUFFLEN] = '\0';
		if (resp.type == ERR){
			printf("ERROR: %s\n", resp.data);
			remove(contentName);
			return -1;
		}
		fprintf(file, "%s", resp.data);	// Write to file
		if (resp.type  == 'F'){		// When final pdu received, break loop
			break;
		}
	
	}
	fclose(file);

   if(contentExists)
   {
        RegisterContent(Socket, SelfPeerName, reg_addr);
   }
}

/******************************************************************************
* Search for content
*******************************************************************************/
void SearchContent(int Socket, char *SelfPeerName)
{
    printf("Item to search for: \n");
    char content[50] = {0};
    int outLen = read(0, content, sizeof(content));
    content[outLen-1]=0;

    struct PDU msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = (int) SEARCH;
    strcpy(msg.data, SelfPeerName);
    strcat(msg.data, ";");
    strcat(msg.data, content);

    write(Socket, &msg, sizeof(msg));

    struct PDU resp;
    read(Socket, &resp, sizeof(msg));

    char *tok = strtok(resp.data, ";");
    printf("Content Name: %s\n", tok);
    tok = strtok(NULL, ";");
    printf("Peer Name: %s\n", tok);
    tok = strtok(NULL, ";");
    printf("Address: %s\n", tok);
    tok = strtok(NULL, ";");
    printf("Port: %s\n", tok);
}

/******************************************************************************
* Deregister Content
*******************************************************************************/
void DeregisterContent(int Socket, char *SelfPeerName)
{
    printf("Item to deregister: \n");
    char content[50] = {0};
    int outLen = read(0, content, sizeof(content));
    content[outLen-1]=0;

    struct PDU msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = (int) DEREGISTER;
    strcpy(msg.data, SelfPeerName);
    strcat(msg.data, ";");
    strcat(msg.data, content);

    write(Socket, &msg, sizeof(msg));

    struct PDU resp;
    read(Socket, &resp, sizeof(msg));

    if(resp.type == (char) ACK)
    {
        printf("Content deregistered. \n");
    }
    else if(resp.type == (char) ERR)
    {
        printf("Unable to deregister content. \n");
    }
}

/******************************************************************************
* Quit Application
*******************************************************************************/
int Quit(int Socket, char *SelfPeerName)
{
    struct PDU msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = (char) QUIT;
    strcpy(msg.data, SelfPeerName);
    write(Socket, &msg, sizeof(msg));

    struct PDU resp;
    memset(&resp, 0, sizeof(resp));   
    read(Socket, &resp, sizeof(resp));

    if(resp.type == (char)QUIT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/******************************************************************************
* Handle Client
*******************************************************************************/
void handle_client(int sd)
{
	struct PDU rpdu;
	struct PDU spdu;
	char	fileName[NAMESIZE];
	char	fileNotFound[] = "FILE NOT FOUND\n";
	int 	n;
	FILE	*file;
	
	if ((n = recv(sd, &rpdu, PACKETSIZE, 0)) == -1){
		fprintf(stderr, "Content Server recv: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
    printf("%s\n", rpdu.data);
    char temp[BUFFLEN] = {0};
    strcpy(temp, rpdu.data);
    
    //memset(peerName, 0, sizeof(peerName));
    char *peerName = strtok(temp, ";");
    char *contentName = strtok(NULL, ";");
	if (rpdu.type == DOWNLOAD){
		memcpy(fileName, contentName, NAMESIZE);
		char filePath[NAMESIZE+2];	// Add current directory to file name
		snprintf(filePath, sizeof(filePath), "%s%s", "./", fileName);
		
		file = fopen(filePath, "r");	
		if (file == NULL) {			// File does not exist
			spdu.type = ERR;
           
			memcpy(spdu.data, fileNotFound, sizeof(fileNotFound));
			write(sd, &spdu, sizeof(spdu));
		}
		else {
			printf("Sending file...\n");
			struct stat fileInfo;
			stat(fileName, &fileInfo);
			sendFile(sd, file, fileInfo.st_size);
			printf("Successfuly sent file\n\n");
			fclose(file);
		}
	}
}

/******************************************************************************
* Send File
*******************************************************************************/
void sendFile(int sd, FILE *p, int fileByteSize)
{
	struct	PDU packet;
	char	fileData[BUFFLEN] = {0};
	int	n, bytesSent, totalBytesSent = 0;

	while((n = fread(fileData, sizeof(char), BUFFLEN, p)) > 0) {	// NULL if EOF reached or error occurs
		if (totalBytesSent + BUFFLEN >= fileByteSize)
			packet.type = 'F';
		else
			packet.type = 'C';
		
		memcpy(packet.data, fileData, BUFFLEN);
		
		if((bytesSent = send(sd, &packet, sizeof(packet), 0)) == -1){
		  fprintf(stderr, "Error sending data\n");
		  exit(1);
		}
		totalBytesSent += n;
		bzero(fileData, BUFFLEN);	//Erase data
	}
    printf("%d\n", totalBytesSent);
} 

/******************************************************************************
* main
*******************************************************************************/
int main ( int argc, char **argv )
{
	char	*host = "localhost";
	int	port = 3000;
	char	now[100];		/* 32-bit integer to hold time	*/ 
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct sockaddr_in sin, reg_addr, client;	/* an Internet endpoint address		*/
    int reg_len, client_len, ret_sel;
	int	s, n, type, new_sd, sd;	/* socket descriptor and socket type	*/

	switch (argc) {
	case 1:
		break;
	case 2:
		host = argv[1];
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "usage: UDPtime [host [port]]\n");
		exit(1);
	}

	memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;                                                                
        sin.sin_port = htons(port);
                                                                                        
    /* Map host name to IP address, allowing for dotted decimal */
        if ( phe = gethostbyname(host) ){
                memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
        }
        else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
        printf("%d\n",sin.sin_addr.s_addr);                                                                      
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "Can't create socket \n");
	
                                                                                
    /* Connect the socket */
        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Can't connect to %s \n", host);

    /* Setup TCP socket */
    	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    		fprintf(stderr, "Can't create a socket\n");
    		exit(EXIT_FAILURE);
    	}
    	bzero((char*)&reg_addr, sizeof(struct sockaddr_in));

        struct ifreq ifr;
        char array[] = "enp0s3";

    	reg_addr.sin_family = AF_INET;
        strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
        ioctl(sd, SIOCGIFADDR, &ifr);
    	reg_addr.sin_port = htons(0);
    	reg_addr.sin_addr.s_addr = inet_addr(inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
        fflush(stdout);
        printf("%s\n", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
        fflush(stdout);
    	if (bind(sd,(struct sockaddr*)&reg_addr, sizeof(reg_addr)) == -1){
    		fprintf(stderr, "Can't bind name to socket\n");
    		exit(EXIT_FAILURE);
    	}
    	
    	reg_len = sizeof(struct sockaddr_in);
    	getsockname(sd,(struct sockaddr*)&reg_addr,&reg_len);
        printf("%d\n", reg_addr.sin_port);
    	
    /* Queue up to 10 connection requests */
    	if(listen(sd,10) < 0){
    		fprintf(stderr, "Listening failed\n");
    		exit(EXIT_FAILURE);
    	}
    
    /* Listen to multiple sockets */
       fd_set rfds, afds;

    char peerName[10] = {0};
    GetPeerName(peerName);
    
    while(1)
    {
        FD_ZERO(&afds);
       	FD_SET(0,&afds);	// Listening on stdin
       	FD_SET(sd, &afds);	// Listening on server TCP socket
		memcpy(&rfds, &afds, sizeof(rfds));
		
	    	if (ret_sel = select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0){
	    		printf("Select() Error\n");
	    		exit(EXIT_FAILURE);
	    	}
	    	
		if(FD_ISSET(sd, &rfds)) {	// Check server TCP socket
			client_len = sizeof(client);
			new_sd = accept(sd,(struct sockaddr*)&client,&client_len);
			if (new_sd >= 0) {		// New Accepted TCP socket
				handle_client(new_sd);	// Handle download request
				close(new_sd);
				
			}
		}
		if(FD_ISSET(fileno(stdin), &rfds)) {
			char input[10] = {0};

        printf("Enter the letter associated with the action you would like to complete: \n");
        printf("Register content (r). \n");
        printf("List content (o). \n");
        printf("Download content (d). \n");
        printf("Search for content (s).\n");
        printf("Deregister content (t).\n");
        printf("Quit (q).\n");
        printf("\nMake a selection: ");
	fflush(stdout);
        read(0, input, 2);

        switch((int) input[0])
        {
            case REGISTER:
                RegisterContent(s, peerName, reg_addr);
                break;
            case ONLINE:
                RequestOnlineContent(s);
                break;
            case DOWNLOAD:
                DownloadContent(s, peerName, reg_addr);
                break;
            case SEARCH:
                SearchContent(s, peerName);
                break;
            case DEREGISTER:
                DeregisterContent(s, peerName);
                break;
            case 113:
                if(Quit(s, peerName))
                {
                    goto Exit;
                }     
        }
			
		}
        
    }
Exit:
    close(s);
    exit(0);
}

