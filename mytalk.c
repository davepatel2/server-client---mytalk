#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <pwd.h>
#define LEN 100
#define MAXLEN 1000
#define OKLEN 2
#define LISTENLEN 128

extern void start_windowing(void);
void print_usage_and_exit();
extern void stop_windowing(void);
void isinclude(int *v, int *a, int *n, int *client ,int *server ,char **argv, int argc);
void chat_sc(int sockfiled, int n, int v);
extern int read_from_input(char *buf, size_t len);
extern int write_to_output(const char *buf, size_t len);
extern int has_hit_eof(void);
extern void stop_windowing(void);
extern void update_input_buffer(void);
extern int has_hit_eof(void);
extern int fprint_to_output(const char *fmt, ...);
int print_add_server(struct sockaddr_in client, char* name, int a);
char* print_add_client(struct sockaddr_in client, int a);

void print_usage_and_exit()
{
    fprintf(stderr, "usage: mytalk [ -v ] [ -a ] [ -N ] [ hostname ] port \n");
    exit(-1);
}

void isinclude(int *v, int *a, int *n, int *client ,int *server ,char **argv, int argc)
{
    int ind = 0;
    int maxarg = 2;
   
    while( ind != argc){
        char *current = argv[ind];
        if(strcmp(current ,"-v") == 0){
            *v = 1; 
            maxarg++;
            
        }
        if(strcmp(current ,"-a") == 0){
            *a = 1; 
            maxarg++;
           
        }
        if(strcmp(current,"-N") == 0){
            *n = 1; 
            maxarg++;
            
        }
        ind++;   
    }
    if( argc < maxarg){
        print_usage_and_exit();
    }
    if( argc == maxarg){
        *server = 1;
        
    }
    if( argc == maxarg + 1){
        *client = 1;
        
    }

    
    
    
}

int print_add_server(struct sockaddr_in client, char* name, int a){
            struct hostent *host;
            char *hostname; 
            char response;
            int bol;
            
            host = gethostbyaddr(&client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), AF_INET);
            hostname = host -> h_name;
            
            if( a == 0){
                printf("Mytalk request from %s@%s  Accept (y/n)? ", name, hostname);
                scanf(" %c", &response);

                if( response == 'y'){
                    bol = 1;
                    return bol;
                }
                if( response == 'n'){
                    bol = 0;
                    return bol;
                }
                perror("not a valid input, terminating");
                exit(-1);
            }
            bol = 1;
            return bol;
          
}

char* print_add_client(struct sockaddr_in client, int a){
        if( a == 0){
            struct hostent *host;
            char *hostname; 
            char hostwperiod[MAXLEN];
            
            
            host = gethostbyaddr(&client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), AF_INET);
            hostname = host -> h_name;
            strcpy(hostwperiod, hostname);
            hostwperiod[strlen(hostname)] = '.';
            write(STDOUT_FILENO, "Waiting for response from ", 28);
            write(STDOUT_FILENO, hostwperiod, strlen(hostname)+1);
            return hostname;
        }
        return NULL;
            
}

void chat_sc(int sockfiled, int n, int v){
    
    int incominglen; 
    int outgoinglen;
    int finish = 0;
    fd_set readfds;
    char buff[LEN];
    
    if( n == 0){
        start_windowing();
    }
    while( finish == 0){
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfiled, &readfds);
        select(sockfiled + 1, &readfds, NULL, NULL, NULL);
        
        if(FD_ISSET(sockfiled, &readfds)){
            
            outgoinglen = recv(sockfiled, buff, sizeof(buff), 0);
            if(outgoinglen == 0){
                
                break;
            }
            
            write_to_output(buff, outgoinglen);
        }

        update_input_buffer();
        if(has_hit_eof()){
            
            break;
        }
        if(FD_ISSET(STDIN_FILENO, &readfds)){ 
            
            incominglen = read_from_input(buff, LEN);
            
            
            outgoinglen = send(sockfiled, buff, incominglen, 0);
        }
        
    }
    
    close(sockfiled);
    
    fprint_to_output("Connection closed. ^C to terminate");
    pause();
    if( n == 0 ){
        stop_windowing();
    }
    


}
int main(int argc, char *argv[]){
   
    int v=0, a=0, n=0, client = 0, server = 0;
    isinclude(&v, &a, &n, &client, &server, argv, argc);
    
    
    if( client != 0){
       
        struct hostent *hostent; 
        int sockfiled;
        struct sockaddr_in sa;
        char *port = argv[argc - 1];
        char *hostname = argv[argc - 2];
        char *nameofhost;
        char buff[3]; 
        char *verb;

        struct passwd *user;

        char* username;
        user = getpwuid(getuid());
        username = user -> pw_name;
            

        
        hostent = gethostbyname(hostname); 
        
        
        if( v != 0){
            verb = "making socket\n";
            write(STDOUT_FILENO, verb, strlen(verb));
        }
        sockfiled = socket(AF_INET, SOCK_STREAM, 0);

        sa.sin_family = AF_INET;
        int portnum = atoi(port);
        sa.sin_port = htons(portnum);
        sa.sin_addr.s_addr = *(uint32_t*)hostent-> h_addr_list[0];
        nameofhost = print_add_client(sa, a); 

        if( v != 0){
            verb = "...connecting\n";
            write(STDOUT_FILENO, verb, strlen(verb));
        }
        if(connect(sockfiled, (struct sockaddr *) &sa, sizeof(sa)) == -1){
            perror("\nbad connection\n");
            exit(-1);
        }
        
        send(sockfiled, username, strlen(username), 0);
        recv(sockfiled, buff, sizeof(buff), 0);
        
        if( v != 0){
            verb = "checking for response...\n";
            write(STDOUT_FILENO, verb, strlen(verb));
        }
        if( strcmp(buff, "ok") != 0){ 
            printf("\n%s declined connection\n", nameofhost); 
        }else{
            if( v != 0){
                verb = "Accepted. Chat Enabled\n";
                write(STDOUT_FILENO, verb, strlen(verb));
            }
            printf("\n");
            
            chat_sc(sockfiled, n, v);
        }
        
    }
    
    if( server != 0){
       
        
        int sockfiled;
        int newsock;
        char *port = argv[argc - 1];
        int portnum = atoi(port);
        struct sockaddr_in sa;
        struct sockaddr_in client; 
        sockfiled = socket(AF_INET, SOCK_STREAM, 0);
        char buff[MAXLEN + 1]; 
        char *verb;
        
        socklen_t slen;
        
        int answer;

        sa.sin_family = AF_INET;
        
        sa.sin_port = htons(portnum);
        sa.sin_addr.s_addr = htonl(INADDR_ANY);

        if( v != 0){
                verb = "Binding server\n";
                write(STDOUT_FILENO, verb, strlen(verb));
            }
        if(bind(sockfiled, (struct sockaddr *)&sa, sizeof(sa)) != 0){
            perror("failed bind");
            exit(-1);
        }
        
        if( v != 0){
                verb = "listening...\n";
                write(STDOUT_FILENO, verb, strlen(verb));
        }
        if(listen(sockfiled, 128) != 0){
            perror("failed listen");
            exit(-1);
        }
            
        slen = sizeof(client);
    
        newsock = accept(sockfiled, (struct sockaddr *)&client, &slen);
        

        if(newsock != -1){
            int len;
            len = recv(newsock, buff, MAXLEN, 0);
            buff[len] = '\0'; 
            
            
            
            answer = print_add_server(client, buff, a);
            
            
        }else{
            perror("failed accept");
            exit(-1);
        }

        if( answer == 1){
            if( v != 0){
                verb = "Sending confirmation. Chat enabled\n\n";
                write(STDOUT_FILENO, verb, strlen(verb));
            }
            send(newsock, "ok", OKLEN, 0);
            
            chat_sc(newsock, n, v);
        }
        
    }
    

    return 0;
}