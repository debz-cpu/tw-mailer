#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <string>

////////////////////////////////////////////////

////////////////////////////////////////////////
#define BUF 1024

////////////////////////////////////////////////
/*Global variables to handle the socket and the abort signal*/
////////////////////////////////////////////////
int abortRequested = 0;
int create_socket = -1;
int new_socket = -1;

////////////////////////////////////////////////
void *clientCommunication(void *data);
void signalHandler(int sig);
bool readline(int sock, std::string &line);
void handleSend(int sock);
void handleList(int sock);
void handleRead(int sock);
void handleDel(int sock);



int main(int argc, char *argv[])
{

   // exactly 3 words: program, port, spool directory
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <port> <mail-spool-directoryname>" << std::endl;
      return EXIT_FAILURE;
   }

   // convert port text -> number
   int port;
   try {
      port = std::stoi(argv[1]);
   } catch (const std::exception &) {
      std::cerr << "Error: port must be a number, got: " << argv[1] << "\n";
      return EXIT_FAILURE;
   }

   if(port<1||port >65535){
    std::cerr << "Error: port must be between 1 and 65535\n";
    return EXIT_FAILURE;
   } 
   std::string spoolDir = argv[2];


   socklen_t addrlen;
   struct sockaddr_in address, cliaddress;
   int reuseValue = 1;



////////////////////////////////////////////////////////////////////////////
   // SIGNAL HANDLER
   // SIGINT (Interrup: ctrl+c)
   // https://man7.org/linux/man-pages/man2/signal.2.html

   if (signal(SIGINT, signalHandler) == SIG_ERR)
   {
      perror("signal can not be registered");
      return EXIT_FAILURE;
   }


   ////////////////////////////////////////////////////////////////////////////
   // CREATE A SOCKET
   // https://man7.org/linux/man-pages/man2/socket.2.html
   // https://man7.org/linux/man-pages/man7/ip.7.html
   // https://man7.org/linux/man-pages/man7/tcp.7.html
   // IPv4, TCP (connection oriented), IP (same as client)
   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("Socket error"); // errno set by socket()
      return EXIT_FAILURE;
   }

   ////////////////////////////////////////////////////////////////////////////
   // SET SOCKET OPTIONS
   // https://man7.org/linux/man-pages/man2/setsockopt.2.html
   // https://man7.org/linux/man-pages/man7/socket.7.html
   // socket, level, optname, optvalue, optlen
   if (setsockopt(create_socket,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  &reuseValue,
                  sizeof(reuseValue)) == -1)
   {
      perror("set socket options - reuseAddr");
      return EXIT_FAILURE;
   }

   if (setsockopt(create_socket,
                  SOL_SOCKET,
                  SO_REUSEPORT,
                  &reuseValue,
                  sizeof(reuseValue)) == -1)
   {
      perror("set socket options - reusePort");
      return EXIT_FAILURE;
   }


   ////////////////////////////////////////////////////////////////////////////
   // INIT ADDRESS
   // Attention: network byte order => big endian
   memset(&address, 0, sizeof(address));
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(port);


   ////////////////////////////////////////////////////////////////////////////
   // ASSIGN AN ADDRESS WITH PORT TO SOCKET
   if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
   {
      perror("bind error");
      return EXIT_FAILURE;
   }
///////////////////////////////////////////////////////////////////////////
   // ALLOW CONNECTION ESTABLISHING
   // Socket, Backlog (= count of waiting connections allowed)
   if (listen(create_socket, 5) == -1)
   {
      perror("listen error");
      return EXIT_FAILURE;
   }

   while (!abortRequested)
   {
      /////////////////////////////////////////////////////////////////////////
      // ignore errors here... because only information message
      // https://linux.die.net/man/3/printf
      printf("Waiting for connections...\n");

      /////////////////////////////////////////////////////////////////////////
      // ACCEPTS CONNECTION SETUP
      // blocking, might have an accept-error on ctrl+c
      addrlen = sizeof(struct sockaddr_in);
      if ((new_socket = accept(create_socket,
                               (struct sockaddr *)&cliaddress,
                               &addrlen)) == -1)
      {
         if (abortRequested)
         {
            perror("accept error after aborted");
         }
         else
         {
            perror("accept error");
         }
         break;
      }

/////////////////////////////////////////////////////////////////////////
// START CLIENT
// ignore printf error handling
      printf("Client connected from %s:%d...\n",
             inet_ntoa(cliaddress.sin_addr),
             ntohs(cliaddress.sin_port));
      clientCommunication(&new_socket); // returnValue can be ignored
      new_socket = -1;
   }

   // frees the descriptor
   if (create_socket != -1)
   {
      if (shutdown(create_socket, SHUT_RDWR) == -1)
      {
         perror("shutdown create_socket");
      }
      if (close(create_socket) == -1)
      {
         perror("close create_socket");
      }
      create_socket = -1;
   }

   return EXIT_SUCCESS;
}
/////////////////////////////////////////////////////////
bool readline(int sock, std::string &line){
   line.clear();
   while(true){
      char c;
      ssize_t n = recv(sock, &c, 1, 0);
      if(n<=0){
         return false;
      }
      if(c =='\n'){
         return true;
      }else if(c =='\r'){
         continue;
      }else{

         line+=c;
      }

   }
}
///////////////////////////////////////////////////////////////
void handleSend(int sock){
   printf("SEND command received\n");
   send(sock, "OK\n",3,0);
}
///////////////////////////////////////////////////////////////
void handleList(int sock){
   printf("LIST command received\n");
   send(sock, "OK\n", 3, 0);
}
///////////////////////////////////////////////////////////////
void handleRead(int sock){
   printf("READ command received\n");
   send(sock, "OK\n", 3, 0);
}
///////////////////////////////////////////////////////////////
void handleDel(int sock)
{
   printf("DEL command received\n");
   send(sock, "OK\n", 3, 0);
}

///////////////////////////////////////////////////////////////
void *clientCommunication(void *data)
{
   int *current_socket = (int *)data;
   std::string line;

   do   
   {
      /////////////////////////////////////////////////////////////////////////
      // RECEIVE
      if(!readline(*current_socket, line)){
         printf("client closed remote socket\n");
         break;
      }
      if (line == "QUIT")
      {
         break; 
      }else if (line == "SEND"){
         handleSend(*current_socket);
      }else if(line =="LIST"){
         handleList(*current_socket);
      }else if (line == "READ"){
         handleRead(*current_socket);
      }else if (line == "DEL"){
         handleDel(*current_socket);
      }else{
         printf("Unknown command: %s\n", line.c_str());
         if (send(*current_socket, "ERR\n", 4, 0) == -1)
         {
            perror("send answer failed");
            return NULL;
         }
      }
   } while (!abortRequested);

   // closes/frees the descriptor if not already
   if (*current_socket != -1)
   {
      if (shutdown(*current_socket, SHUT_RDWR) == -1)
      {
         perror("shutdown new_socket");
      }
      if (close(*current_socket) == -1)
      {
         perror("close new_socket");
      }
      *current_socket = -1;
   }

   return NULL;
}

void signalHandler(int sig)
{
   if (sig == SIGINT)
   {
      printf("abort Requested... "); // ignore error
      abortRequested = 1;
      /////////////////////////////////////////////////////////////////////////
      // With shutdown() one can initiate normal TCP close sequence ignoring
      // the reference count.
      // https://beej.us/guide/bgnet/html/#close-and-shutdownget-outta-my-face
      // https://linux.die.net/man/3/shutdown
      if (new_socket != -1)
      {
         if (shutdown(new_socket, SHUT_RDWR) == -1)
         {
            perror("shutdown new_socket");
         }
         if (close(new_socket) == -1)
         {
            perror("close new_socket");
         }
         new_socket = -1;
      }

      if (create_socket != -1)
      {
         if (shutdown(create_socket, SHUT_RDWR) == -1)
         {
            perror("shutdown create_socket");
         }
         if (close(create_socket) == -1)
         {
            perror("close create_socket");
         }
         create_socket = -1;
      }
   }
   else
   {
      exit(sig);
   }



}
