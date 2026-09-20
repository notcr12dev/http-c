#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};


static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("No hay memoria suficiente (realloc)\n");
        return 0;
    }

    mem->memory = ptr;
    &(mem->memory[mem->size]);
    for (size_t i = 0; i < realsize; i++) {
        mem->memory[mem->size + i] = ((char *)contents)[i];
    }
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int main(void) {
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1); 
    chunk.size = 0;           

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if(curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, "http://localhost:300/api/data");

        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);


        res = curl_easy_perform(curl_handle);

        if(res != CURLE_OK) {
            fprintf(stderr, "Error en curl_easy_perform(): %s\n", curl_easy_strerror(res));
        } else {
            printf("Respuesta de la API:\n%s\n", chunk.memory);
        }


        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
    }

    curl_global_cleanup();
    return 0;
}
