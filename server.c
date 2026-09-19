#define _XOPEN_SOURCE 700
#define MAX_ITEMS 10
#define VALUE_SIZE 256
#define KEY_SIZE 64

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

struct KeyValue {
	char key[KEY_SIZE];
	char value[VALUE_SIZE];
	bool ocuppied;
};

struct KeyValue dicc[MAX_ITEMS] = {0};

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

	//Cache
	// char cache[512] = "{\"msg\": \"No data\"}";
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

		printf("[REQ] %s %s\n", method, path);

		// PATH MANAGER

		// GET
		if (strcmp(method, "GET") == 0) //(strcmp) Sirve para comparar
		{

			//Endpoint: GET /
			// if(strcmp(path, "/") == 0){
			// 	FILE *file = fopen(filename_html, "r");
			// 	if (file == NULL) {
			// 		char *error_404 =  
			// 			"HTTP/1.1 404 not Found\r\n"
			// 			"Content-Type : text/html; charset=UTF-8\r\n"
			// 			"Connection: close\r\n"
			// 			"\r\n"
			// 			"<h1>Error 404: Error File</h1>";
			// 		write(client_sockfd, error_404, strlen(error_404));
			// 		printf("Error File\n\n %s", error_404);
			// 	} else {
			// 		char *header =  
			// 			"HTTP/1.1 200 OK\r\n"
			// 			"Content-Type: text/html; charset=UTF-8\r\n"
			// 			"Connection: close\r\n"
			// 			"\r\n";
			// 		write(client_sockfd, header, strlen(header));

			// 		char file_buffer[BUFSIZ];
			// 		size_t bytes_reads;
			// 		while ((bytes_reads = fread(file_buffer, 1,
			// 			sizeof(file_buffer),
			// 			file)) > 0) {
			// 				write(client_sockfd, file_buffer, bytes_reads);
					
			// 		}
			// 		fclose(file);
			// 	}
			//Endpoint: GET /api/status
			if (strcmp(path, "/api/status") == 0) {
					char *json_res =
						"HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html; charset=UTF-8\r\n"
						"Connection: close\r\n"
						"\r\n"
						"{\"status\": \"running\", \"msg\": \"Servidor C activo\"}";
					write(client_sockfd, json_res, strlen(json_res));
			} 
			else if (strcmp(path, "/api/data") == 0) {
					char json_body[4096] = "[";
					bool first_item = true;

					for (int i = 0; i < MAX_ITEMS; i++){
						if (dicc[i].ocuppied) {
							char item_str[512];
							snprintf(item_str, sizeof(item_str), "%s{\"key\": \"%s\", \"value\": \"%s\"}",
							first_item ? "" : ",", dicc[i].key, dicc[i].value);
							strcat(json_body, item_str);
							first_item = false;
						}
					}
					strcat(json_body, "]");

					char header[4600];
					snprintf(header, sizeof(header),
						"HTTP/1.1 200 OK\r\n"
						"Content-Type: application/json; charset=UTF-8\r\n"
						"Connection: close\r\n"
						"\r\n"
						"%s", json_body);
					
					write(client_sockfd, header, strlen(header));
			} 
			else {
				char *file_to_open = (strcmp(path, "/") == 0) ? filename_html : (path + 1);

				FILE *file = fopen(file_to_open, "r");
				if (file == NULL) {
					printf("[DEBUG] No se pudo abrir el archivo: '%s'\n", file_to_open);

					char *error_404 =  
						"HTTP/1.1 404 Not Found\r\n"
						"Content-Type: text/html; charset=UTF-8\r\n"
						"Connection: close\r\n"
						"\r\n"
						"<h1>Error 404: Archivo No Encontrado</h1>";
					write(client_sockfd, error_404, strlen(error_404));
				} else {
					char *header =
						"HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html; charset=UTF-8\r\n"
						"Connection: close\r\n"
						"\r\n";
					write(client_sockfd, header, strlen(header));

					char file_buffer[BUFSIZ];
					size_t bytes_reads;
					while ((bytes_reads = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
						write(client_sockfd, file_buffer, bytes_reads);
					}
					fclose(file);
				}
			}
		}

		// POST
		else if (strcmp(method, "POST") == 0) {
		
			// Search body
			char *body = strstr(buffer, "\r\n\r\n");
			if (body != NULL) {
				body += 4;
			} else {
				body = "";
			}

			if (strcmp(path, "/api/data") == 0) {
			printf("[POST BODY] Recibido: %s\n", body);

			char req_key[KEY_SIZE] = {0};
			char req_value[VALUE_SIZE] = {0};
			
			// Parseamos el body con formato "clave=valor"
			if (sscanf(body, "%63[^=]=%255s", req_key, req_value) == 2) {
				int found_idx = -1;
				int first_idx = -1;

				// Buscar si la clave ya existe o encontrar un hueco libre
				for (int i = 0; i < MAX_ITEMS; i++) {
					if (dicc[i].ocuppied && strcmp(dicc[i].key, req_key) == 0) {
						found_idx = i;
						break;
					}
					if (!dicc[i].ocuppied && first_idx == -1) {
						first_idx = i;
					}
				}

				int target_idx = (found_idx != -1) ? found_idx : first_idx;

				if (target_idx != -1) {
					// Guardar o actualizar los datos
					strncpy(dicc[target_idx].key, req_key, KEY_SIZE - 1);
					strncpy(dicc[target_idx].value, req_value, VALUE_SIZE - 1);
					dicc[target_idx].ocuppied = true;

					char response[512];
					snprintf(response, sizeof(response),
						"HTTP/1.1 200 OK\r\n"
						"Content-Type: application/json\r\n"
						"Connection: close\r\n"
						"\r\n"
						"{\"success\": true, \"msg\": \"Guardado correctamente\", \"key\": \"%s\", \"value\": \"%s\"}", 
						req_key, req_value);
					write(client_sockfd, response, strlen(response));
				} else {
					char *err = "HTTP/1.1 507 Insufficient Storage\r\n\r\n{\"error\": \"Cache llena\"}";
					write(client_sockfd, err, strlen(err));
				}
			} else {
				char *err = "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\": \"Formato invalido. Usa clave=valor\"}";
				write(client_sockfd, err, strlen(err));
			}
		}

			
		}
		
		// PUT
		else if (strcmp(method, "PUT") == 0){
			
			// Endpoint: PUT /api/update
			if (strcmp(path, "/api/update") == 0) {
				char *json_res = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"msg\": \"Recurso actualizado con PUT\"}";
				write(client_sockfd, json_res, strlen(json_res));
			} else {
				char *error_404 = "HTTP/1.1 404 Not Found\r\n\r\n";
				write(client_sockfd, error_404, strlen(error_404));
			}
		}

		// DELETE
		else if (strcmp(method, "DELETE") == 0) {

			// Endopint DELETE /api/delete
			if (strcmp(path, "/api/delete") == 0) {
				char *json_res = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: application/json\r\n"
					"Connection: close\r\n"
					"\r\n"
					"{\"success\": true, \"msg\": \"Recurso eliminado correctamente con DELETE\"}";
				write(client_sockfd, json_res, strlen(json_res));
			
			} else {
				char *error_404 = "HTTP/1.1 404 Not Found\r\n\r\n";
				write(client_sockfd, error_404, strlen(error_404));
			}
		}

		else {
			char *error_405 = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\n\r\nMethod not allowed.";
			write(client_sockfd, error_405, strlen(error_405));
		}

		

		
		
		close(client_sockfd);

		
    }
    
	close(server_sockfd);
	return EXIT_SUCCESS;
	

}
