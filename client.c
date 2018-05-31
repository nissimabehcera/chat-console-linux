#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#define IP_SERVEUR_2 "127.0.1.1"
#define IP_SERVEUR "192.168.0.50"
#define PORT_CLIENT "2058"
#define PORT_SERVEUR "2059"

int valeur=0;
int sockfd;
int choix;


void handler(int a){
	if(a==SIGTERM){
		choix=0;
		pthread_exit(NULL);
		close(sockfd);
	}
	//valeur=1;
}
typedef struct{
	char ip_adresse[56];
	char pseudo[64];
	char message[600];
	unsigned int choix_user;//1) connexion  2)inscription
	char password[56];
	int reponse_serveur;
}data;

void receive_from_serveur(int *sockfd_desc){

	ssize_t valeur_retour;
	//data reponse_serveur;
	char retour[300];
	while(1){
		valeur_retour=recv(*sockfd_desc,(void *)&retour,sizeof (retour),0);
		if(valeur_retour==-1){
				perror("recv client");
				exit(EXIT_FAILURE);
		}
		printf("%s\n", retour);
}
}

static void purger(void){
    int c;
    while ((c = getchar()) != '\n' && c != EOF){}

}

static void clean (char *chaine){

    char *p = strchr(chaine, '\n');
    if(p){
        *p = 0;
    }
    else{
        purger();
    }
}
void connection_vers_serveur(int *sockfd){
	struct addrinfo hints,*res,*parcours;
	memset(&hints, 0, sizeof (hints));
	hints.ai_family =AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol=0;

	/*retour_getaddr=*/getaddrinfo(IP_SERVEUR_2, PORT_SERVEUR, &hints, &res);
	for (parcours=res; parcours!=NULL; parcours=parcours->ai_next)
	{
		*sockfd = socket(parcours->ai_family, parcours->ai_socktype, parcours->ai_protocol);
		if(*sockfd==-1){
			perror("socket");
			continue;
		}
		if(connect(*sockfd, parcours ->ai_addr, parcours ->ai_addrlen)!=-1){
			break;
		}
		close(*sockfd);
	}
	if (parcours == NULL) { /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
}
enum{Inscription,Connection};

int main(){

	
	char choix_user_tab[3];
	pthread_t thread;
	unsigned int choix_user_num;
	data login;
	memset(&login,0,sizeof login);
	connection_vers_serveur(&sockfd);
	do{
		printf("\n1)Se connecter\n2)S'inscrire\n3)Quitter l'application ");
		fgets(choix_user_tab,3,stdin);
		clean(choix_user_tab);
		choix_user_num=atoi(choix_user_tab);
		if(choix_user_num==3){
			return EXIT_SUCCESS;
		}
		
		printf("\nEntrez votre pseudo : ");
		fgets(login.pseudo,50,stdin);
		clean(login.pseudo);
		printf("\nEntrez votre mot de passe : ");
		fgets(login.password,50,stdin);
		clean(login.password);
		if(choix_user_num==1){
			login.choix_user=1;
		}else{
			login.choix_user=2;
		}
		if(send(sockfd,(void *)&login,sizeof login,0)==-1){
			perror("send serveur");
			exit(EXIT_FAILURE);
		}
		if(recv(sockfd,(void *)&login, sizeof login,0)<=0){
			perror("recv client");
			exit(EXIT_FAILURE);
		}
		
		if(choix_user_num==1 && login.reponse_serveur==0){
			printf("\nIdentifiants incorrect, veuillez vous reconnecter\n");
		}
		else if(choix_user_num==1 && login.reponse_serveur==1){
			printf("\nConnexion effectuée avec succès\n");
			break;
		}
		else if(choix_user_num==2 && login.reponse_serveur==0){
			printf("\npseudo deja pris,veuillez vous réinscrire \n");
		}
		else if(choix_user_num==2 && login.reponse_serveur==1){
			printf("\nInscription effectuée avec succès\n");
			break;
		}
		
	}while(1);
		login.choix_user=0;

	if(pthread_create(&thread,NULL,(void *)&receive_from_serveur,&sockfd)!=0){
		perror("pthread_create ");
		exit(EXIT_FAILURE);
	}
	
	while(1){
		
		fgets(login.message,600,stdin);
		clean(login.message);
		if(send(sockfd,(void *)&login,sizeof login,0)==-1){
			perror("send client");
			exit(EXIT_FAILURE);
		}
		
	}
	pthread_exit(NULL);
	return 0;
}
