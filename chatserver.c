//Ajustado del código de Ricardo Romo https://ricardoromo.co/2019/crea-una-servidor-de-sala-de-chat-en-c-servidor/
//El siguiente cliente puede interactuar con otros clientes a través de un server no de forma local. Debes ingresar la IP destino y el puerto, así como el nombre del cliente 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 250

// Global variables
int sockfd = 0;
char *nombre;
volatile sig_atomic_t flag = 0;

void reset_stdout(); //vacía buffer de escritura
void emisor(); //maneja los mensajes de entrada 
void receptor(); //maneja los mensajes de entrada 

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}
void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length && i < 250; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}


void reset_stdout()
{
	printf("%s", "> ");
	fflush(stdout);
}


int main(int argc, char **argv)
{

	char *ip = argv[1];
	int port = atoi(argv[2]); //Convierte string a int
	nombre = argv[3];
	
	struct sockaddr_in server_addr;

	/* Configuracion del Socket  */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	// Conectando al servidor
	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1)
	{
		printf("ERROR: conectando\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, argv[3], 32, 0);

	printf("=== Bienvenido %s ===\n",argv[3]);

	pthread_t hilo_emisor;
	if (pthread_create(&hilo_emisor, NULL, (void *)emisor, NULL) != 0)
	{
		printf("ERROR: pthread  - Hilo del emisor\n");
		return EXIT_FAILURE;
	}

	pthread_t hilo_receptor;
	if (pthread_create(&hilo_receptor, NULL, (void *)receptor, NULL) != 0)
	{
		printf("ERROR: pthread - Hilo del receptor\n");
		return EXIT_FAILURE;
	}
	while (1)
	{
		if(flag){
			printf("\nBye\n");
			break;
		}
	}
	
	close(sockfd);
	return EXIT_SUCCESS;
}

void emisor()
{
	char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};


	while (1)
	{
		reset_stdout();
    	fgets(message, LENGTH, stdin);
    	str_trim_lf(message, LENGTH);

    	if (strcmp(message, "bye") == 0) {
			break;
    	} 
		else {
      		sprintf(buffer, "%s: %s\n", nombre, message);
      		send(sockfd, buffer, strlen(buffer), 0);
    	}

		bzero(message, LENGTH);
    	bzero(buffer, LENGTH + 32);
  	}
  	catch_ctrl_c_and_exit(2);
}

void receptor()
{
	char message[LENGTH] = {};
	while (1)
	{
		int receive = recv(sockfd, message, LENGTH, 0); //recibe los mensajes del servidor
		if (receive > 0)
		{
			printf("%s", message); //Imprime los mensajes del servidor
			reset_stdout();
		}
		else if (receive == 0)
		{
			break;
		}
		else
		{
			//
		}
		memset(message, 0, sizeof(message)); //resetea el buffer de mensajes
	}
	printf("Bye desde el Server");
	catch_ctrl_c_and_exit(2);
}
