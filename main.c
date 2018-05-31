	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <signal.h>
	#include <string.h>
	#include "mysql/include/mysql.h"
	#include "mysql/include/my_global.h"
	#include <arpa/inet.h>
	#define PORT_SERVEUR "2059"

	typedef struct{
		
		char adresse_ip[56];
		char pseudo[64];
		char message[600];
		long choix_user;
		char password[56];
		long reponse_serveur;
	} data;

	int main()
	{
		MYSQL *connexion;
		MYSQL_RES *resultat;
		MYSQL_ROW row;
		char *serveur="localhost";
		char *user="";
		char *password="";
		char *database="mysql";

		connexion=mysql_init(NULL);
		//connexionect to database
		if(!mysql_real_connect(connexion,serveur,user,password,database,0,NULL,0)){
			fprintf(stderr, "%s\n", mysql_error(connexion));
			exit(1);
		}

		//send SQL query
		if(mysql_query(connexion,"CREATE DATABASE if not exists chat")){
			fprintf(stderr, "%s\n", mysql_error(connexion));
			exit(EXIT_FAILURE);
		}
		if(mysql_query(connexion,"USE chat")){
			fprintf(stderr, "%s\n", mysql_error(connexion));
			exit(EXIT_FAILURE);
		}
		if(mysql_query(connexion,"CREATE TABLE if not exists user(\
							id INT AUTO_INCREMENT PRIMARY KEY,\
							port_user INT NOT NULL,\
							adresse_ip VARCHAR(50) NOT NULL,\
							pseudo VARCHAR(60) NOT NULL UNIQUE,\
							password VARCHAR(50) NOT NULL,\
							date_inscription DATETIME NOT NULL\
							)ENGINE=InnoDB DEFAULT CHARSET=utf8")){
			
			fprintf(stderr, "user%s\n", mysql_error(connexion));
			exit(EXIT_FAILURE);
		}
		if(mysql_query(connexion,"CREATE TABLE if not exists messages_user(\
							id INT AUTO_INCREMENT PRIMARY KEY,\
							message TEXT NOT NULL,\
							date_message DATETIME NOT NULL,\
							pseudo_user VARCHAR(60) NOT NULL,\
							FOREIGN KEY(pseudo_user) REFERENCES user(pseudo)\
							)ENGINE=InnoDB DEFAULT CHARSET=utf8")){
			fprintf(stderr, "message%s\n", mysql_error(connexion));
			exit(EXIT_FAILURE);
		}
		
		int sockfd, new_fd,nbytes,j,i;
		struct addrinfo hints, *servinfo,*parcours;
		struct sockaddr_storage their_adr;
		char ipstr[INET6_ADDRSTRLEN];
		int port/*,retour_getaddr*/;
		data data;//,reponse_serveur;
		socklen_t addrlen;
		char requete[300];
		//char buf[100]; // buffer données client
		fd_set master; // master file descriptor list
		fd_set read_fds; // temp file descriptor list for select()
		int fdmax; // maximum file descriptor number
		FD_ZERO(&master); // clear the master and tempsets
		FD_ZERO(&read_fds);
		// Création socket et attachement
		printf("fd_set%d\n",sizeof(fd_set) );
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol=0;
		
		/*retour_getaddr =*/getaddrinfo("127.0.1.1", PORT_SERVEUR, &hints, & servinfo);
		printf("servinfo= %d\n",sizeof(*servinfo) );
	    for (parcours = servinfo; parcours != NULL; parcours = parcours->ai_next) {
	       
	       sockfd = socket(parcours->ai_family, parcours->ai_socktype,
	               parcours->ai_protocol);
	       if (sockfd == -1)
	           continue;
	       printf("ai_addr= %d  ai_addrlen= %d\n  addrlen= %d",sizeof(parcours->ai_addr), sizeof(parcours->ai_addrlen),sizeof (addrlen));
	       if (bind(sockfd, parcours->ai_addr, parcours->ai_addrlen) == 0)
	           break;                  /* Success */

	       close(sockfd);
	   }

	    if (parcours == NULL) {               /* No address succeeded */
	       	fprintf(stderr, "Could not bind\n");
	        exit(EXIT_FAILURE);
	    }

	    freeaddrinfo(servinfo);
		signal(SIGCHLD, SIG_IGN);

		listen(sockfd, 5);
		FD_SET(sockfd, &master); // Ajout sockfd à ensemble
		fdmax = sockfd; // Garde valeur max socket
		while(1){

			read_fds = master; // ensemble socket attente lecture
			if (select(fdmax+1, &read_fds, NULL, NULL, NULL)== -1){
				perror("select");
				exit(4);
			}
			for(i = 0; i <= fdmax; i++) {
				if (FD_ISSET(i, &read_fds)) {
					if (i == sockfd){
						addrlen = sizeof(their_adr);
						new_fd = accept(sockfd,(struct sockaddr *)&their_adr, &addrlen);
						
						if (new_fd == -1){
							perror("accept");
							exit(EXIT_FAILURE);
						}
						else{ // Ajout new_fd à ensemble
							FD_SET(new_fd, &master);
							if (new_fd > fdmax){
								fdmax = new_fd;
							}
							printf("Nouvelle connexion au serveur.\n");
						if (their_adr.ss_family == AF_INET) {
						    struct sockaddr_in *s = (struct sockaddr_in *)&their_adr;
						    port = ntohs(s->sin_port);
						    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
						} else {
						    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&their_adr;
						    port = ntohs(s->sin6_port);
						    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
						}

						printf("Peer IP address: %s\n", ipstr);
						printf("Peer port number : %d\n",port);
					}
						
					} else { // gestion données client i
						if ((nbytes = recv(i, (void *)&data, sizeof data, 0)) <= 0){ // erreur ou connexionexion fermée par client
							if (nbytes == 0){
								
								printf("connexion %d fermée.\n", i);
								close(i);
								FD_CLR(i, &master); // Supprime ensemble
							}
							else{
								perror("recv");
								exit(EXIT_FAILURE);
							}
						}else { 
						// Données reçu du client
							if(data.choix_user==1 ||data.choix_user==2){//connexion ou inscription
								printf("bobo\n");
								snprintf(requete,300,"SELECT pseudo,password FROM user WHERE pseudo='%s' AND password='%s'",data.pseudo,data.password);
								printf("test%s\n", requete);
								if(mysql_query(connexion,requete)){
									fprintf(stderr, "%s\n", mysql_error(connexion));
									exit(EXIT_FAILURE);
								}
								resultat=mysql_store_result(connexion);

								int num_rows = mysql_num_rows(resultat);
								
								if(num_rows==0){
									if(data.choix_user==1){
										data.reponse_serveur=0;
									}else{
										data.reponse_serveur=1;
										snprintf(requete,300,"INSERT INTO user(id,port_user,adresse_ip,pseudo,password,date_inscription)VALUES(NULL,%d,'%s','%s','%s',now())",port,ipstr,data.pseudo,data.password);
										printf("requete1%s\n",requete);
										if(mysql_query(connexion,requete)){
											fprintf(stderr, "%s\n", mysql_error(connexion));
											exit(EXIT_FAILURE);
										}
									}
									
								}else{
									if(data.choix_user==1){
										
										data.reponse_serveur=1;
									}else{
										data.reponse_serveur=0;
									}
									
								}
								send(i,(void *)&data,sizeof data,0);
							}
							if(strcmp(data.message,"")!=0){
								//int last_id_user=mysql_insert_id(connexion);
								char test[strlen(data.message)*2+1];
								mysql_real_escape_string_quote(connexion,test,data.message,strlen(data.message),'\'');
								snprintf(requete,300,"INSERT INTO messages_user (id,message,date_message,pseudo_user) VALUES(NULL,'%s',now(),'%s')",/*data.message*/test,data.pseudo);
								
								if(mysql_query(connexion,requete)){
									fprintf(stderr, "%s\n", mysql_error(connexion));
									exit(EXIT_FAILURE);
								}
								int last_id_msg=mysql_insert_id(connexion);
								snprintf(requete,300,"SELECT pseudo_user,message FROM messages_user WHERE id=%d",last_id_msg);
								
								if(mysql_query(connexion,requete)){
									fprintf(stderr, "%s\n", mysql_error(connexion));
									exit(EXIT_FAILURE);
								}
						
								resultat=mysql_store_result(connexion);
								int num_rows = mysql_num_rows(resultat);
								row=mysql_fetch_row(resultat);
								
								if(num_rows>0){
									
								for (j = 0; j <=fdmax ; j++){
									if(FD_ISSET(j,&master)){
										if(j!=sockfd && j!=i){
											char retour[300];
											strcpy(retour,row[0]);
											strcat(retour," : ");
											strcat(retour,row[1]);
		
											if(send(j,(void *)&retour,sizeof retour,0)==-1){
												perror("send serveur");
												exit(EXIT_FAILURE);
											}
										}
									}
								}	
							}	
						}
					}
						
					} // Fin bloc ELSE client
					//nbytes=0;
				} // Fin bloc IF FD_ISSET
			}
			  // Fin boucle FOR sur i
		} // Fin boucle WHILE
		mysql_close(connexion);
		return 0;
	}