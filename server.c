#define _XOPEN_SOURCE 700


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
	char buffer[BUFSIZ];
	char protoname[] = "tcp";

	struct protoent *protoent;
	int enable = 1;
	int server_sockfd, client_sockfd;
	socklen_t client_len;
	ssize_t nbytes_read;
	struct sockaddr_in client_address, server_address;
	unsigned short server_port = 12345u;
	
	char *filename_html;

	for (int i = 1; i < argc; ++i) {
		char *endptr;
		long val = strtol(argv[i], &endptr, 10);

		if (*endptr == '\0' && argv[i][0] != '\0') {
			if (val <= 65535) {
				server_port = (unsigned short)val;
			} else {
				printf("The port is outside the valid range; Selected port: %lu", val);
				return 1;
			}
		} else {
			filename_html = argv[i];
		}
	}
	// if (argc == 1) {
	// 	server_port = strtol(argv[1], NULL, 10);
	// }

	protoent = getprotobyname(protoname); // Indicamos que queremos una estructura con el protcolo 'tcp'
	if (protoent == NULL) {
		perror("getprotobyname");
		exit(EXIT_FAILURE);
	}

	server_sockfd = socket(
		AF_INET,
		SOCK_STREAM,
		protoent->p_proto
	);
	if (server_sockfd == -1){
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0){
		perror("setsocket(SO_REUSEADDR) failed");
		exit(EXIT_FAILURE);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(server_port);
	if (bind(
			server_sockfd,
			(struct sockaddr*)&server_address,
			sizeof(server_address)
		) == -1 
	) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_sockfd, 5) == -1){
		perror("listen");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "listen on port %u\n\n HTML file: %s", server_port, filename_html);

	while(1) {
		client_len = sizeof(client_address);
		client_sockfd = accept(
			server_sockfd, 
			(struct sockaddr*)&client_address,
			&client_len
		);
        if (client_sockfd == -1) {
			perror("accept");
		}

		// Read HTTP request complete
		memset(buffer, 0, sizeof(buffer));
		nbytes_read = read(client_sockfd, buffer, sizeof(buffer) - 1);
		
		if (nbytes_read <= 0) {
			close(client_sockfd);
			continue;
		}

		//Extract request
		char method[16] = {0};
		char path[256] = {0};
		sscanf(buffer, "%15s %255s", method, path);

		
		char *file_html = (strcmp(path, "/") == 0) ? filename_html  : (path + 1);
		

		FILE *file = fopen(file_html, "r");
		if (file == NULL) {
			char *error_404 =  
				"HTTP/1.1 404 not Found\r\n"
				"Content-Type : text/html; charset=UTF-8\r\n"
				"Connection: close\r\n"
				"\r\n"
				"<h1>Error 404: Error File</h1>";
			write(client_sockfd, error_404, strlen(error_404));
			printf("Error File\n\n %s", error_404);
		} else {
			char *header =
				"HTTP/1.1 200 OK\r\n"
            	"Content-Type: text/html; charset=UTF-8\r\n"
            	"Connection: close\r\n"
           		"\r\n";
			write(client_sockfd, header, strlen(header));

			char file_buffer[BUFSIZ];
			size_t bytes_reads;
			while ((bytes_reads = fread(file_buffer, 1,
				 sizeof(file_buffer),
				  file)) > 0) {
					write(client_sockfd, file_buffer, bytes_reads);
			
			}
			fclose(file);

		}
		close(client_sockfd);

		
    }
    
	close(server_sockfd);
	return EXIT_SUCCESS;
	

}
