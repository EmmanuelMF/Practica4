///Universidad La Salle
///Facultad de ingenieria
///Sistemas Distribuidos
///Práctica 4
///Marisol Gonzalez Campos
///Emmanuel Macedo Franco
///Eduardo Méndez Méndez
/* -------------------------------------------------------------------------- */
/* chat client                                                                */
/*                                                                            */
/* client program that works with datagram type sockets sending entries typed */
/* by a user and in order for the server  to receive them and forward them to */
/* all of the nodes connected.                                                */
/*                                                                            */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* libraries needed for the program execution                                 */
/* -------------------------------------------------------------------------- */
#include <netinet/in.h>                /* TCP/IP library                      */
#include <sys/socket.h>                /* sockets library                     */
#include <sys/types.h>                 /* shared data types                   */
#include <stdio.h>                     /* standard input/output               */
#include <unistd.h>                    /* unix standard functions             */
#include <string.h>                    /* text handling functions             */
#include <pthread.h>                   /* libraries for thread handling       */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>
#include <string.h>
/* -------------------------------------------------------------------------- */
/* global definitions                                                         */
/* -------------------------------------------------------------------------- */
#define LINELENGTH 4096
#define  BUFFERSIZE 4096               /* buffer size                         */
#define min(a,b) ((a) < (b) ? a : b)

int    sfda;
char   text[LINELENGTH], filecont[LINELENGTH], path[LINELENGTH];

struct stat archivo;               /*ESTRUTURA INFO ARCHIVO               */

/* -------------------------------------------------------------------------- */
/* global variables and structures                                            */
/* -------------------------------------------------------------------------- */
struct data
  {
    int  data_type;                              /* type of data sent         */
    int  chat_id;                                /* chat id sent by server    */
    char data_text[BUFFERSIZE-(sizeof(int)*2)];  /* data sent                 */
  };

/* ------------------------------------------------------------------------- */
/* print_message()                                                           */
/*                                                                           */
/* this function will constantly look for a new message to print coming from */
/* the server                                                                */
/* void *ptr  - pointer that will receive the parameters for the thread      */
/*                                                                           */
/* ------------------------------------------------------------------------- */
void *print_message(void *ptr)
  {
    int    *sock_desc;                 /* pointer for parameter              */
    int    read_char;                  /* read characters                    */
    char   text1[BUFFERSIZE];          /* reading buffer                     */

    sock_desc = (int *)ptr;
    while(1)
      {
        read_char = recvfrom(*sock_desc,text1,BUFFERSIZE,0,NULL,NULL);
        text1[read_char] = '\0';
        printf("%s\n",text1);
      }
}


/*----------------------------------------------------------------------------*/
/*Declaracion de funciones*/
int Send_file();

/* -------------------------------------------------------------------------- */
/* main ()                                                                    */
/*                                                                            */
/* main function of the system                                                */
/* -------------------------------------------------------------------------- */
main()
  {
    struct sockaddr_in sock_write;     /* structure for the write socket      */
    struct data message;               /* message to sendto the server        */
    char   text1[BUFFERSIZE];          /* reading buffer                      */
    char   *auxptr;                    /* auxiliar char pointer               */
    int    read_char;                  /* read characters                     */
    int    i;                          /* counter                             */
    int    sfd;                        /* socket descriptor                   */
    int    chat_id;                    /* identificator in the chat session   */
    int    iret1;                      /* thread return value                 */
    pthread_t thread1;                 /* thread id                           */
    int x;
    /* ---------------------------------------------------------------------- */
    /* structure of the socket that the server will use to send an answer     */
    /* ---------------------------------------------------------------------- */
    sock_write.sin_family = AF_INET;
    sock_write.sin_port   = 10101;
    sock_write.sin_addr.s_addr = inet_addr("200.13.89.15");
    for (i=0;i<=7;++i)
      sock_write.sin_zero[i]='\0';

    /* ---------------------------------------------------------------------- */
    /* Instrucctions for publiching the socket                                */
    /* ---------------------------------------------------------------------- */
    sfd = socket(AF_INET,SOCK_DGRAM,0);

    /* ---------------------------------------------------------------------- */
    /* request and sending of an alias                                        */
    /*                                                                        */
    /* if data_type = 0 we are telling the server that we are logging for the */
    /* first tim and providing an alias. we will receive a integer with a nu- */
    /* meric id that we will send in every next message                       */
    /* ---------------------------------------------------------------------- */
    printf("Please provide an alias: ");
    message.chat_id = 0;
    message.data_type = 0;             /* data_type 0 is used to send alias   */
    message.data_text[0] = '\0';
    fgets(message.data_text, BUFFERSIZE-(sizeof(int)*2), stdin);
    for(auxptr = message.data_text; *auxptr != '\n'; ++auxptr);
      *auxptr = '\0';

    /* sending of information to log in chat room                             */
    sendto(sfd,(struct data *)&(message),sizeof(struct data),0,(struct sockaddr *)&(sock_write),sizeof(sock_write));
    read_char = recvfrom(sfd,(int *)&(chat_id),sizeof(int),0,NULL,NULL);
    if (chat_id == -1)                 /* client rejected                     */
      {
        printf("Client could not join. Too many participants in room\n");
        close(sfd);
        return(0);
      }

     /* Creation of reading thread                                            */
     iret1 = pthread_create( &thread1, NULL, print_message, (void *)(&sfd));

    /* ---------------------------------------------------------------------- */
    /* text typed by the user isread and sent to the server.  The client then */
    /* waits for  an answer and  displays it. The  cycle  continues until the */
    /* word "exit" is written.                                                */
    /* ---------------------------------------------------------------------- */
    while (strcmp(message.data_text,"exit") != 0)
      {
        printf("Teclee un mensaje, teclee %cremote_dir%c para ver el directorio activo en el servidor,y poder recibir archivo o teclee %cexit%c para salir\n",34 , 34, 34, 34);
        printf("$ ");

        /* assembling of the message to send                                  */
        message.data_type = 1;         /* data_type 1 is used to send message */
        fgets(message.data_text, BUFFERSIZE-(sizeof(int)*2), stdin);
        for(auxptr = message.data_text; *auxptr != '\n'; ++auxptr);
	  *auxptr = '\0';
        message.chat_id = chat_id;
        ///checa si se quiere enviar un archivo
        if(strcmp(message.data_text,"remote_dir")==0 )
        {
            message.data_type = 2;         /* data_type 2 is used to recieve a file */
            sendto(sfd,(struct data *)&(message),sizeof(struct data),0,(struct sockaddr *)&(sock_write),sizeof(sock_write));
			x=Send_file();

        }
        sendto(sfd,(struct data *)&(message),sizeof(struct data),0,(struct sockaddr *)&(sock_write),sizeof(sock_write));
      }
    close(sfd);
    return(0);
  }

  int Send_file()
{
    struct sockaddr_in estrsock;       /* socket structure                   */
    int    i;                          /* counter                            */
    int    sfd_in;                     /* descriptor for the read port        */
    char   *auxptr;
    text[0] = '\0';
    char buf[20];
    size_t nbytes;
    size_t bytes_read;
    int pfd;
    long int carleidos=0;
    long int cont=0;


    /* ---------------------------------------------------------------------- */
    /* definition of the socket                                               */
    /*                                                                        */
    /* AF_INET      - TCP Socket Family                                       */
    /* 10073        - Number of port in  which  the server will  be listening */
    /*                for incoming messages                                   */
    /* 200.13.89.15 - Server Address. This is  the address that  will be pub- */
    /*                lished to receive  information. It is the server's add- */
    /*                ress                                                    */
    /*                                                                        */
    /* This process will publish  the combination of  200.13.89.15 plus  port */
    /* 10101 in order to make the OS listen through it and  bring information */
    /* to the process                                                         */
    /* ---------------------------------------------------------------------- */
    strsock.sin_family = AF_INET;      /* AF_INET = TCP Socket                */
    strsock.sin_port   = 10102;        /* Port Number to Publish              */
    /* Address of the server's computer in the case of the server             */
    strsock.sin_addr.s_addr = inet_addr("200.13.89.14");
    for (i=0;i<=7;++i)
      strsock.sin_zero[i]='\0';

    /* ---------------------------------------------------------------------- */
    /* Instruccions to request and publish the socket                         */
    /* ---------------------------------------------------------------------- */

    /* ---------------------------------------------------------------------- */
    /* Creation of the Socket                                                 */
    /*                                                                        */
    /* #include <sys/socket.h>                                                */
    /* int socket(int domain, int type, int protocol);                        */
    /* Returns: file (socket) descriptor if OK, -1 on error                   */
    /* ---------------------------------------------------------------------- */
    sfda = socket(AF_INET,SOCK_STREAM,0);
    if (sfda == -1)
      {
        perror("Problem creating the socket");
        fprintf(stderr,"errno = %d\n", errno);
        exit(1);
      }

    /* ---------------------------------------------------------------------- */
    /* Asociation of an O.S. socket with the socket data structure of the pro */
    /* gram.                                                                  */
    /*                                                                        */
    /* #include <sys/socket.h>                                                */
    /* int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen)*/
    /* Returns: 0 if OK, -1 on error                                          */
    /* ---------------------------------------------------------------------- */
    if (bind(sfda, (struct sockaddr *)&(strsock), sizeof(strsock)) == -1)
      {
        perror("Problem binding the socket");
        fprintf(stderr,"errno = %d\n", errno);
        exit(1);
      }

      /* ---------------------------------------------------------------------- */
    /* Creation of a listener  process for the socket. This is needed only in */
    /* connection oriented sockets.                                           */
    /*                                                                        */
    /* #include <sys/socket.h>                                                */
    /* int listen(int sockfd, int buffersize)                                 */
    /* Returns: 0 if OK, -1 on error                                          */
    /* ---------------------------------------------------------------------- */
    if (listen(sfda,LINELENGTH) == -1)
      {
        perror("Problem creating listener process for the socket");
        fprintf(stderr,"errno = %d\n", errno);
        exit(1);
      }
    /* ---------------------------------------------------------------------- */
    /* Initialization of the connection. Binding of the socket to a file poin */
    /* ter                                                                    */
    /*                                                                        */
    /* #include <sys/socket.h>                                                */
    /* int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen)   */
    /* Returns: 0 if OK, -1 on error                                          */
    /* ---------------------------------------------------------------------- */
    i = sizeof(strsock);
    /* ptrsock = (struct sockaddr *)&(strsock);                               */
    sfd_in=accept(sfd, (struct sockaddr *)&(strsock),(int *)&i);
    if (sfd_in == -1)
      {
        perror("Problem starting the accept process of the socket");
        fprintf(stderr,"errno = %d\n", errno);
        exit(1);
      }

        strcpy(path, "/home/cib_700_10/pract4");

        fprintf(stdout, "Nombre de archivo :[%s]\n",text);

        strcat(path,text);

        fprintf(stdout, "ruta de guardado :[%s]\n",path);

        read_char = read(sfd_in,&tam,sizeof(long int));
        fprintf(stdout, "Tamaño de archivo: [%ld]\n", tam);

        int exito;
        exito=open(path,O_CREAT | O_RDWR, 0700); //O_RDWR | O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (exito == -1)
        {
            perror("Cannot open output file\n"); break;
        }
        else{
        while(cont<tam){
            read_char = read(sfd_in,filecont,LINELENGTH-1);
            filecont[read_char] = '\0';
            fprintf(stdout, "Cont Envio :[%s]\n",filecont);
            cont += read_char;
            write(exito,filecont,read_char);}
        close(exito);

        fprintf(stdout, "teclee %c Nombre.ftype%c del archivo que desea recibir", 34, 34);
        fgets(text, LINELENGTH, stdin);
        for(auxptr = text; *auxptr != '\n'; ++auxptr);
            *auxptr = '\0';
        write(sfda, text, strlen(text));


        strcpy(message.data_text,"File sent");
        read_char = read(sfd_in,text,LINELENGTH-1);
        text[read_char] = '\0';
        close(sfd);
		}
      return 0;
}