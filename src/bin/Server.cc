/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "Parameters.h"
#include "Logger.h"
#include "Swirl.h"

using namespace std;
using namespace srl;

void s_error(const char *msg)
{
    perror(msg);
    exit(1);
}

static const char* parseAndClassifyString(char *txt)
{
    TreePtr tree;
    std::ostringstream stream;
    std::string str;
    const char* result;

    // classify all predicates in this sentence
    tree = Swirl::parse(txt);

    // dump extended CoNLL format
    Swirl::serialize(tree, txt, stream);
    str = stream.str();
    result = str.c_str();
    return result;
}


void handle (int sock)
{
   int n;
   char buffer[MAX_INPUT];
    const char* processed;
      

    bzero(buffer, MAX_INPUT);
     n = read(sock, buffer, MAX_INPUT - 1);
     if (n < 0) s_error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);

     // parse
     processed = parseAndClassifyString(buffer);

    cout << processed << endl;
    cout << strlen(processed) << endl;
     // respond
     n = write(sock, processed, strlen(processed));
     if (n < 0) s_error("ERROR writing to socket");
}


int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

    int idx = -1;

    try{
        idx = Parameters::read(argc, argv);
    } catch(...){
        CERR << "Exiting..." << endl;
        exit(-1);
    }

    int verbosity = 0;
    Parameters::get("verbose", verbosity);
    Logger::setVerbosity(verbosity);

    //String classifierType = "ab";
    //Parameters::get("classifier", classifierType);

    bool caseSensitive = true; // default: case sensitive
    if(Parameters::contains("case-insensitive")) caseSensitive = false;

    if(idx > argc - 2){
        CERR << "wrong args" << endl;
        exit(-1);
    }

    if(! Swirl::initialize(argv[idx + 0], argv[idx + 1], caseSensitive)){ 
        cerr << "Failed to initialize SRL system!\n";
        exit(1);
    } 
    cout << "Ready!" << endl;

    // server code
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        s_error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));

     portno = 2718;
     Parameters::get("port", portno);

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              s_error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);


     while (1) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             s_error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             s_error("ERROR on fork");
         if (pid == 0)  {
             close(sockfd);
             handle(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } /* end of while */
     close(sockfd);
     return 0; /* we never get here */
}

