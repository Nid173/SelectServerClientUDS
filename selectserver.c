/*
 ** selectserver.c -- a cheezy multiperson chat server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

#define MAXDATASIZE 100
#define SOCK_PATH "echo_socket"

#define PORT "3490" // port we're listening on

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



typedef struct ClientI
{
    int id;
    int Overhead[100];//change
    int counter;
    int averageOverhead;
}Client[MAXDATASIZE-1];

void getAvrg(Client* ClientInfo){
    //printf(" avrage of ClientInfo->id: %d  is updating\n",(*ClientInfo)->id);
    if((*ClientInfo)->averageOverhead == 0){
        (*ClientInfo)->averageOverhead=(*ClientInfo)->Overhead[(*ClientInfo)->counter-1];
    }else{
        (*ClientInfo)->averageOverhead=((*ClientInfo)->Overhead[(*ClientInfo)->counter-1]+(*ClientInfo)->averageOverhead)/2;
    }
    //printf("(func) new averageOverhead is '%d'\n",(*ClientInfo)->averageOverhead);
    return;
}
int compare (const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}

//void whois(Client *ClientInfo,int fdmax,int s2) // change to send to usdclient
//    {
//        printf("----------Client----------\n");
//        printf("ID :%d \n",(*ClientInfo)->id);
//        printf("Overheads :");
//        for(int i=0 ; i<100 ; i++){
//        printf("%d,",(*ClientInfo)->Overhead[i]);
//        }
//        printf("\nCounter :%d\n",(*ClientInfo)->counter);
//        printf("Average :%d\n",(*ClientInfo)->averageOverhead);
//        }

char* getClient(Client *ClientInfo,int lock)
{
    char Clientbuff[1024];
    char tmpbuff[1024];
    int n;
    sprintf(Clientbuff, "Client %d : ID = %d \n",(*ClientInfo)->id-6, (*ClientInfo)->id);
    if(lock==1){
        strcat(Clientbuff, "Overheads: ");
    for(n=0;n<99;n++){
        sprintf(tmpbuff, "%d, ",(*ClientInfo)->Overhead[n]);
        strcat(Clientbuff,tmpbuff);
        memset(tmpbuff, '\0', 1024);
        }
    }
    sprintf(tmpbuff, "\nAverage OverHead: %d \n ----------------------------\n",(*ClientInfo)->averageOverhead);
    strcat(Clientbuff,tmpbuff);
    return Clientbuff;
}
//----------------
char* getworst(Client *ClientInfo,int fdmax)
{
    printf("i'm in getworst fun\n");
    int  c, d, position,i=0,j=0;
    int nId , nRec;
    char buf[1024];
    Client Clienttmp[MAXDATASIZE-1];
    for(i=7;i<=fdmax;i++){
        Clienttmp[i]->id=ClientInfo[i]->id;
        Clienttmp[i]->averageOverhead=ClientInfo[i]->averageOverhead;
    }
    for ( c = 7 ; c < j ; c++ )
    {
        position = c;

        for ( d = c + 1 ; d < j; d++ )
        {
            if ( Clienttmp[position]->averageOverhead > Clienttmp[d]->averageOverhead )
                position = d;
        }
        if ( position != c )
        {
            nId=Clienttmp[c]->id;
            nRec=Clienttmp[c]->averageOverhead;

            Clienttmp[c]->id=Clienttmp[position]->id;
            Clienttmp[c]->averageOverhead = Clienttmp[position]->averageOverhead;

            Clienttmp[position]->id = nId;
            Clienttmp[position]->averageOverhead=nRec;
        }
    }
    if(j%2==0)j=(j/2);
    else j=(j/2); //50% of the worst
    strcpy(buf,getClient(&Clienttmp[7],0));
    for(i=7;i<j;i++)
    {
        strcat(buf,getClient(&Clienttmp[i],0));
    }
    
    return buf;
}

int main(void)
{
    
    int s, s2,t, len;
    struct sockaddr_un local, remote;
    char str[100];
    
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    
    Client ClientInfo[MAXDATASIZE-1]; // struct of clients
//    Client ClientBtS[MAXDATASIZE-1];
    
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    
    char buf[256],buf2[1024];    // buffer for client data
    int nbytes;
    
    char remoteIP[INET6_ADDRSTRLEN];
    
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;
    
    struct addrinfo hints, *ai, *p;
    
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    
    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    // uds stuff
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        
        break;
    }
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        printf("p==NULL\n");
        exit(2);
    }
    
    freeaddrinfo(ai); // all done with this
    
    if (bind(s, (struct sockaddr *)&local, len) == -1) {
        printf("bind s\n");
        perror("bind");
        exit(1);
    }
    //uds listen
    if (listen(s, 5) == -1) {
        perror("listen");
        exit(1);
    }
    
    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }
    
    // add the listener to the master set
    FD_SET(listener, &master);
    FD_SET(s,&master);
    // keep track of the biggest file descriptor
    if(s>listener){
        fdmax = s;
    }else{
        fdmax = listener; // so far, it's this one
    }
    printf("3\n" );
    // main loop
    for(;;) {
        printf("4\n");
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *)&remoteaddr,
                                   &addrlen);
                    printf("5\n");
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        
                        
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                            
                            //init client in the structure
                            ClientInfo[newfd]->id=newfd;
                            ClientInfo[newfd]->averageOverhead=0;
                            ClientInfo[newfd]->counter=0;
                            ClientInfo[newfd]->Overhead[0]=0;
                        }
                        
                        printf("selectserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr*)&remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                        
                    }
                }
                else if(i==s){
                    int done, n;
                    printf("Waiting for a connection...\n");
                    t = sizeof(remote);
                    if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
                        perror("accept");
                        exit(1);
                    }
                    
                    printf("Connected.\n");
                    
                    FD_SET(s2,&master);
                    if(s2>fdmax)
                        fdmax=s2;
                }else if(i==s2){
                    //recieved
                    if ((nbytes = recv(s2, buf2, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", ClientInfo[i]->id);
                        } else {
                            perror("recv");
                            close(i); // bye!
                            FD_CLR(i, &master); // remove from master set
                            
                        }
                        
                    }
                    printf("im here");
                    //printf("%s \n");
                    if(strcmp(buf2,"whois\n")==0)
                    {
                        printf("got whois\n");
                        // edit and call strcmp
                        memset(buf2,'\0',1024);
                        if(fdmax<7){
                            if (send(s2, "No Client\n", 11, 0) == -1)  //we need to check if all the data passed
                                perror("send");
                        }else{
                            for(j=7;j<=fdmax;j++)
                            {
                                memset(buf2,'\0',1024);
                                strcpy(buf2,getClient(&ClientInfo[j],1));
                                if (send(s2, buf2,sizeof(buf2), 0) == -1)  //we need to check if all the data passed
                                    perror("send");
                                recv(s2,buf2,sizeof(buf2),0);
                                
                            }
                            memset(buf2,'\0',1024);
                            if (send(s2,"ok",2, 0) == -1)  //we need to check if all the data passed
                                perror("send");
                        }
                        
                    }else if(strcmp(buf2,"getworst\n")==0){
                        // edit and call strcmp
                        printf("got getworst\n");
                        memset(buf2,'\0',1024);
            
                        
                        strcpy(buf2,getworst(&ClientInfo,fdmax));
                        printf("%s\n",buf2);
                        if (send(s2, buf2,sizeof(buf2), 0) == -1)  //we need to check if all the data passed
                            perror("send");
                        recv(s2,buf2,sizeof(buf2),0);
                        
                    
                    memset(buf2,'\0',1024);
                    if (send(s2,"ok",2, 0) == -1)  //we need to check if all the data passed
                        perror("send");
                        
                    }else if(strcmp(buf2,"grep ARGS\n")==0){
                        // edit and call strcmp
                        printf("got grep ARGS");
                        //add grep function edit ... change the args to int and do 
                    }
                    else{
                        memset(buf2,'\0',1024);
                        if (send(s2,"ok",2, 0) == -1)  //we need to check if all the data passed
                            perror("send");
                        
                    }
                    //                                         if (send(s2, buf2, MAXDATASIZE-1, 0) == -1)  //we need to check if all the data passed
                    //                                             perror("send");
                    
                    // metabel bel data whois(i)
                } else {
                    
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", ClientInfo[i]->id);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        //recieved successfuly
                        printf("im in begin");
                        
                        if(strcmp(buf,"beginT")==0)
                        {
                            printf("got beginT \n");
                            //																printf("%s\n",buf);
                            memset(buf,'\0',256);
                            
                            if (recv(i,buf,sizeof(buf), 0) == -1)  //we need to check if all the data passed
                                perror("recv");
                            
                            
                            ClientInfo[i]->Overhead[ClientInfo[i]->counter]=atol(buf); // put time from client in overhead
                            //                                                                printf("Overhead[%d] is updated to '%d'\n",ClientInfo[i]->counter,ClientInfo[i]->Overhead[ClientInfo[i]->counter]);
                            if(ClientInfo[i]->counter==99) ClientInfo[i]->counter=0;
                            (ClientInfo[i]->counter)++;
                            getAvrg(&ClientInfo[i]);
                            //avrArr[i]=ClientInfo[i]->averageOverhead;
                            //                                                                printf("averageOverhead is updated to '%d'\n",ClientInfo[i]->averageOverhead);
                            /*WHO IS                for(int t=5;t<=fdmax;t++){
                             whois(&ClientInfo[t]);
                             }
                             */
                        }
                        
//                        if (send(i, buf, MAXDATASIZE-1, 0) == -1)  //we need to check if all the data passed
//                            perror("send");
                        // we got some data from a client
                        /*			printf("Server: recieved '%s' Client'%d'='%d'\n",buf,ClientInfo[i].id ,ClientInfo[i].Overhead);
                         
                         //ClientInfo[i].averageOverhead=getAvrg(ClientInfo[i].Overhead,ClientInfo[i].averageOverhead); //get average from last and the newest time
                         printf("overhead=%d",ClientInfo[i].Overhead);
                         printf("ClientInfo[%d].Overhead[ClientInfo[%d].averageOverhead]=%d\n",i,i,ClientInfo[i].averageOverhead);
                         //metabel ba clients*/
                        
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}
