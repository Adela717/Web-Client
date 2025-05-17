#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "parson.h"
#include "buffer.h"
#include "helpers.h"
#include "requests.h"

#define HOST "63.32.125.183"
#define PORT 8081

#define ADMIN_ROUTE "/api/v1/tema/admin/login"
#define USER_ROUTE "/api/v1/tema/admin/users"
#define PAYLOAD_TYPE "application/json"

char *admin_cookie = NULL;
char *user_cookie = NULL;
char *token = NULL;
char current_user[LINELEN];

typedef struct {
    int id;
    char owner[50];
} User_info;

User_info info[100];
int info_count = 0;

void save_token(char *response) {
    char *token_start = strstr(response, "\"token\":\"");

    if(token_start) {
        token_start = token_start + 9;
        char *token_end = token_start;

        while(*token_end !='"') {
            token_end++;
        }

        int len = token_end - token_start;

        if(token != NULL) {
            free(token);
        }
        token = malloc(len + 1);

        strncpy(token, token_start, len);
        token[len] = '\0';
    }
}

void save_cookie(char *response, char **cookie_dest) {
    char *cookie_start = strstr(response, "Set-Cookie: ");
    if(cookie_start) {
        cookie_start = cookie_start + 12;
        char *cookie_end = cookie_start;

        while(*cookie_end != ';') {
            cookie_end++;
        }
        
        int len = cookie_end - cookie_start;
        if (*cookie_dest != NULL) {
            free(*cookie_dest);
        }

        *cookie_dest = (char *)malloc(len + 1);
        strncpy(*cookie_dest, cookie_start, len);
        (*cookie_dest)[len] = '\0';
    }
}

void login_admin(int sockfd) {
    if(admin_cookie != NULL) {
        printf("ERROR: reautentificare\n");
        return;
    }
    char username[50], password[50];

    printf("username=");
    fgets(username, sizeof(username), stdin);

    printf("password=");
    fgets(password, sizeof(password), stdin);

    username[strcspn(username, "\n")] = '\0';
    password[strcspn(password, "\n")] = '\0';

    strcpy(current_user, username);

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *payload = json_serialize_to_string(root_value);
    char *body_data[] = { payload };

    char *request = compute_post_request(HOST, ADMIN_ROUTE, PAYLOAD_TYPE, body_data, 1, NULL, 0, NULL);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: autentificare admin\n");

        save_cookie(response, &admin_cookie);

        if (user_cookie != NULL) {
        free(user_cookie);
        user_cookie = NULL;
}
    } else {
        printf("ERROR: autentificare admin\n");
    }
}

void add_user(int sockfd) {
    char username[50], password[50];

    printf("username=");
    fgets(username, sizeof(username), stdin);

    printf("password=");
    fgets(password, sizeof(password), stdin);

    username[strcspn(username, "\n")] = '\0';
    password[strcspn(password, "\n")] = '\0';

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *payload = json_serialize_to_string(root_value);
    char *body_data[] = { payload };

    if(admin_cookie == NULL) {
        printf("ERROR: lipsa permisiuni admin\n");
        return;
    }

    char cookie_header[LINELEN];
    snprintf(cookie_header, LINELEN, "%s", admin_cookie);
    char *cookies[] = { cookie_header };

    char *request = compute_post_request(HOST, USER_ROUTE, PAYLOAD_TYPE, body_data, 1, cookies, 1, NULL);
    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "201 CREATED")) {
        printf("SUCCESS: creare user\n");

    } else if(strstr(response, "409 CONFLICT")){
        printf("ERROR: userul exista deja\n");
    } else {
        printf("ERROR: creare user");
    }
}

void get_users(int sockfd) {
    if(admin_cookie == NULL) {
        printf("ERROR: lipsa permisiuni admin\n");
        return;
    }

    char* cookies[] = {admin_cookie};
    char* request = compute_get_request(HOST, USER_ROUTE, NULL, cookies, 1, NULL);

    send_to_server(sockfd, request);
    char *response = receive_from_server(sockfd);

    char *json_start = strchr(response, '{');

    JSON_Value *val = json_parse_string(json_start);
    JSON_Object *root = json_value_get_object(val);
    JSON_Array *users = json_object_get_array(root, "users");

    printf("SUCCESS: Lista utilizatorilor\n");
    for (int i = 0; i < json_array_get_count(users); i++) {
        JSON_Object *user = json_array_get_object(users, i);
        const char *username = json_object_get_string(user, "username");
        const char *password = json_object_get_string(user, "password");

        printf("#%d %s:%s\n", i + 1, username, password);
    }

}

void login(int sockfd) {
    if(admin_cookie || user_cookie) {
        printf("ERROR: deja autentificat\n");
        return;
    }
    char admin_username[50], username[50], password[50];

    printf("admin_username=");
    fgets(admin_username, sizeof(admin_username), stdin);

    printf("username=");
    fgets(username, sizeof(username), stdin);

    printf("password=");
    fgets(password, sizeof(password), stdin);

    admin_username[strcspn(admin_username, "\n")] = '\0';
    username[strcspn(username, "\n")] = '\0';
    password[strcspn(password, "\n")] = '\0';

    strcpy(current_user, username);

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "admin_username", admin_username);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *payload = json_serialize_to_string(root_value);
    char *body_data[] = { payload };

    char *request = compute_post_request(HOST, "/api/v1/tema/user/login", PAYLOAD_TYPE, body_data, 1, NULL, 0, NULL);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: autentificare user\n");

        save_cookie(response, &user_cookie);

        if (admin_cookie != NULL) {
        free(admin_cookie);
        admin_cookie = NULL;
        }
    } else {
        printf("ERROR: autentificare user\n");
    }

}

void delete_user(int sockfd) {
    if(admin_cookie == NULL) {
        printf("ERROR: lipsa permisiuni admin\n");
        return;
    }

    char delete_route[LINELEN];
    strcpy(delete_route, "/api/v1/tema/admin/users/");
    
    char deleted_user[50];
    printf("username=");
    fgets(deleted_user, sizeof(deleted_user), stdin);
    deleted_user[strcspn(deleted_user, "\n")] = '\0';

    strcat(delete_route, deleted_user);

    char* cookies[] = {admin_cookie};

    char *request = compute_delete_request(HOST, delete_route, cookies, 1, NULL);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: utilizator sters\n");
    } else if(strstr(response, "404 NOT FOUND")) {
        printf("ERROR: utilizatorul nu exista\n");
    }
}

void logout_admin(int sockfd) {
    if(admin_cookie == NULL) {
        printf("ERROR: adminul nu este autentificat\n");
        return;
    }

    char *cookies[] = {admin_cookie};
    char *request = compute_get_request(HOST, "/api/v1/tema/admin/logout", NULL, cookies, 1, NULL);

    send_to_server(sockfd, request);

    char* response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: admin delogat\n");
    }

    if (admin_cookie != NULL) {
        free(admin_cookie);
        admin_cookie = NULL;
    }
}

void logout(int sockfd) {
    if(user_cookie == NULL) {
        printf("ERROR: userul nu este autentificat\n");
        return;
    }

    char *cookies[] = {user_cookie};
    char *request = compute_get_request(HOST, "/api/v1/tema/user/logout", NULL, cookies, 1, NULL);

    send_to_server(sockfd, request);

    char* response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: user delogat\n");
    }

    if (user_cookie != NULL) {
        free(user_cookie);
        user_cookie = NULL;
    }

    if (token != NULL) {
        free(token);
        token = NULL;
    }
}

void get_access(int sockfd) {
    if(!user_cookie) {
        printf("ERROR: user neautentificat\n");
        return;
    }

    char *cookie;
    if(admin_cookie) {
        cookie = admin_cookie;
    } else {
        cookie = user_cookie;
    }

    char* cookies[] = {cookie};

    char *request = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, cookies, 1, NULL);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: acces primit\n");

        save_token(response);
    } else {
        printf("ERROR: acces refuzat\n");
    }
}

void get_movies(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char *request = compute_get_request(HOST, "/api/v1/tema/library/movies", NULL, NULL, 0, token);

    send_to_server(sockfd, request);

    char* response = receive_from_server(sockfd);

    char *json_start = strchr(response, '[');
    JSON_Value *root_value = json_parse_string(json_start);
    JSON_Array *movies = json_value_get_array(root_value);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: Lista filmelor\n");
    }

    for (int i = 0; i < json_array_get_count(movies); i++) {
        JSON_Object *movie = json_array_get_object(movies, i);
        int id = (int)json_object_get_number(movie, "id");
        const char *title = json_object_get_string(movie, "title");
        printf("#%d %s\n", id, title);
    }
}

void add_movie(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char title[50], year[50], description[500], rating[50];

    printf("title=");
    fgets(title, sizeof(title), stdin);

    printf("year=");
    fgets(year, sizeof(year), stdin);

    printf("description=");
    fgets(description, sizeof(description), stdin);

    printf("rating=");
    fgets(rating, sizeof(rating), stdin);

    title[strcspn(title, "\n")] = '\0';
    year[strcspn(year, "\n")] = '\0';
    description[strcspn(description, "\n")] = '\0';
    rating[strcspn(rating, "\n")] = '\0';

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_number(root_object, "year", atoi(year));
    json_object_set_string(root_object, "description", description);
    json_object_set_number(root_object, "rating", atof(rating));
    char *payload = json_serialize_to_string(root_value);
    char *body_data[] = { payload };

    char *cookies[] = {user_cookie};

    char* request = compute_post_request(HOST, "/api/v1/tema/library/movies", PAYLOAD_TYPE, body_data, 1, cookies, 1, token);

    send_to_server(sockfd, request);

    char* response = receive_from_server(sockfd);

    if(strstr(response, "201 CREATED")) {
        printf("SUCCESS: film adaugat\n");
    } else {
        printf("ERROR: date invalide\n");
    }
}

void get_movie(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char id_str[50];

    printf("id=");
    fgets(id_str, sizeof(id_str), stdin);
    id_str[strcspn(id_str, "\n")] = '\0';

    if(strlen(id_str) > 7) {
        printf("ERROR: id invalid\n");
        return;
    }

    char route[LINELEN];
    strcpy(route, "/api/v1/tema/library/movies/");
    strcat(route, id_str);

    char *request = compute_get_request(HOST, route, NULL, NULL, 0, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    char *json_start = strchr(response, '{');
    JSON_Value *root_value = json_parse_string(json_start);
    JSON_Object *root_object = json_value_get_object(root_value);

    const char *title = json_object_get_string(root_object, "title");
    int year = (int) json_object_get_number(root_object, "year");
    const char *description = json_object_get_string(root_object, "description");
    const char *rating_str = json_object_get_string(root_object, "rating");
    double rating = atof(rating_str);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: film gasit\n");
        printf("title: %s\n", title);
        printf("year: %d\n", year);
        printf("description: %s\n", description);
        printf("rating: %.1f\n", rating);
    } else {
        printf("ERROR: id invalid\n");
    }

}

void update_movie(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char title[50], year[50], description[500], rating[50], id_str[50];

    printf("id=");
    fgets(id_str, sizeof(id_str), stdin);

    printf("title=");
    fgets(title, sizeof(title), stdin);

    printf("year=");
    fgets(year, sizeof(year), stdin);

    printf("description=");
    fgets(description, sizeof(description), stdin);

    printf("rating=");
    fgets(rating, sizeof(rating), stdin);

    id_str[strcspn(id_str, "\n")] = '\0';
    title[strcspn(title, "\n")] = '\0';
    year[strcspn(year, "\n")] = '\0';
    description[strcspn(description, "\n")] = '\0';
    rating[strcspn(rating, "\n")] = '\0';

    for (int i = 0; year[i] != '\0'; i++) {
        if (!isdigit(year[i])) {
            printf("ERROR: an invalid\n");
            return;
        }
    }

    for (int i = 0; rating[i] != '\0'; i++) {
        if ((!isdigit(rating[i])) && (rating[i] != '.')) {
            printf("ERROR: rating invalid\n");
            return;
        }
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_number(root_object, "year", atoi(year));
    json_object_set_string(root_object, "description", description);
    json_object_set_number(root_object, "rating", atof(rating));
    char *payload = json_serialize_to_string(root_value);
    char *body_data[] = { payload };

    char *cookies[] = {user_cookie};

    char route[LINELEN];
    strcpy(route, "/api/v1/tema/library/movies/");
    strcat(route, id_str);

    char* request = compute_put_request(HOST, route, PAYLOAD_TYPE, body_data, 1, cookies, 1, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: filmul a fost actualizat\n");
    } else {
        printf("ERROR: filmul nu a fost actualizat");
    }
}

void delete_movie(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char id_str[50];
    printf("id=");
    fgets(id_str, sizeof(id_str), stdin);
    id_str[strcspn(id_str, "\n")] = '\0';

    char route[LINELEN];
    strcpy(route, "/api/v1/tema/library/movies/");
    strcat(route, id_str);

    char *cookies[] = {user_cookie};

    char *request = compute_delete_request(HOST, route, cookies, 1, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: film sters\n");
    } else if(strstr(response, "404 NOT FOUND")) {
        printf("ERROR: id invalid\n");
    }
}

void add_movie_to_collection(int sockfd, int id_movie_fct, int id_collection_fct) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    int collection_id = 0;
    int movie_id = 0;

    if(id_movie_fct == -1) {
        char movie_id_str[50], collection_id_str[10];

        printf("collection_id=");
        fgets(collection_id_str, sizeof(collection_id_str), stdin);

        printf("movie_id=");
        fgets(movie_id_str, sizeof(movie_id_str), stdin);

        collection_id_str[strcspn(collection_id_str, "\n")] = '\0';
        movie_id_str[strcspn(movie_id_str, "\n")] = '\0';

        collection_id = atoi(collection_id_str);
        movie_id = atoi(movie_id_str);
    } else {
        // pentru ajutor la functia add_collection
        collection_id = id_collection_fct;
        movie_id = id_movie_fct;
    }

    for(int i = 0; i < info_count; i++) {
        if(info[i].id == collection_id) {
            if(strcmp(info[i].owner, current_user) !=0) {
                printf("ERROR: nu esti owner\n");
                return;
            }
        }
    }

    char route[LINELEN];
    strcpy(route, "/api/v1/tema/library/collections/");

    char *str = malloc(12);
    sprintf(str, "%d", collection_id);

    strcat(route, str);
    strcat(route, "/movies");

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, "id", movie_id);
    char *payload = json_serialize_to_string(root_value);
    char *body_data[] = { payload };

    char *cookies[] = {user_cookie};

    char *request = compute_post_request(HOST, route, PAYLOAD_TYPE, body_data, 1, cookies, 1, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "201 CREATED")) {
        printf("SUCCESS: film adaugat la colectie\n");
    } else {
        printf("ERROR: filmul nu a fost adaugat la colectie\n");
    }
}

void add_collection(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char title[50], num_movies_str[10], movie_id_str[10];

    printf("title=");
    fgets(title, sizeof(title), stdin);

    printf("num_movies=");
    fgets(num_movies_str, sizeof(num_movies_str), stdin);

    title[strcspn(title, "\n")] = '\0';
    num_movies_str[strcspn(num_movies_str, "\n")] = '\0';

    int num_movies = atoi(num_movies_str);
    int movie_id[num_movies];

    for(int i = 0; i < num_movies; i++) {
        printf("movie_id[%d]=", i);
        fgets(movie_id_str, sizeof(movie_id_str), stdin);
        movie_id_str[strcspn(movie_id_str, "\n")] = '\0';
        movie_id[i] = atoi(movie_id_str);
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    char *payload = json_serialize_to_string(root_value);
    char *body_data[] = { payload };

    char *cookies[] = {user_cookie};

    char *request = compute_post_request(HOST, "/api/v1/tema/library/collections", PAYLOAD_TYPE, body_data, 1, cookies, 1, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    json_free_serialized_string(payload);
    json_value_free(root_value);

    char *json_start = strchr(response, '{');
    root_value = json_parse_string(json_start);
    root_object = json_value_get_object(root_value);

    int id = (int) json_object_get_number(root_object, "id");

    if(strstr(response, "201 CREATED")) {
        printf("SUCCESS: colectie adaugata\n");

        info[info_count].id = id;
        strcpy(info[info_count].owner, current_user);
        info_count++;

        for(int i = 0; i < num_movies; i++) {
            add_movie_to_collection(sockfd, movie_id[i], id);
        }

    } else {
        printf("ERROR: nu a fost adaugata colectia\n");
    }
}

void get_collections(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char *cookies[] = {user_cookie};

    char *request = compute_get_request(HOST, "/api/v1/tema/library/collections", NULL, cookies, 1, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        char *json_start = strchr(response, '[');
        JSON_Value *root_value = json_parse_string(json_start);
        JSON_Array *collections = json_value_get_array(root_value);
        int count = json_array_get_count(collections);

        printf("SUCCESS: lista colectiilor\n");

        for (int i = 0; i < count; i++) {
            JSON_Object *collection = json_array_get_object(collections, i);
            const char *col_title = json_object_get_string(collection, "title");
            int col_id = (int)json_object_get_number(collection, "id");

            printf("#%d: %s\n", col_id, col_title);
        }
    } else {
        printf("ERROR: nu se poate accesa lista colectiilor\n");
    }
}

void get_collection(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char route[LINELEN];
    strcpy(route, "/api/v1/tema/library/collections/");

    char id_str[50];
    printf("id=");
    fgets(id_str, sizeof(id_str), stdin);
    id_str[strcspn(id_str, "\n")] = '\0';

    strcat(route, id_str);

    char *cookies[] = {user_cookie};

    char *request = compute_get_request(HOST, route, NULL, cookies, 1, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: detalii colectie\n");

        char *json_start = strchr(response, '{');
        JSON_Value *root_value = json_parse_string(json_start);
        JSON_Object *root_object = json_value_get_object(root_value);

        const char *title = json_object_get_string(root_object, "title");
        const char *owner = json_object_get_string(root_object, "owner");

        printf("title: %s\n", title);
        printf("owner: %s\n", owner);

        JSON_Array *movies = json_object_get_array(root_object, "movies");
        int movie_count = json_array_get_count(movies);

        for (int i = 0; i < movie_count; i++) {
            JSON_Object *movie = json_array_get_object(movies, i);
            int movie_id = (int)json_object_get_number(movie, "id");
            const char *movie_title = json_object_get_string(movie, "title");

            printf("#%d: %s\n", movie_id, movie_title);
        }
    } else if(strstr(response, "404 NOT FOUND")) {
        printf("ERROR: id invalid\n");
    }

}

void delete_collection(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char route[LINELEN];
    strcpy(route, "/api/v1/tema/library/collections/");

    char id_str[50];
    printf("id=");
    fgets(id_str, sizeof(id_str), stdin);
    id_str[strcspn(id_str, "\n")] = '\0';

    int collection_id = atoi(id_str);

    for(int i = 0; i < info_count; i++) {
        if(info[i].id == collection_id) {
            if(strcmp(info[i].owner, current_user) !=0) {
                printf("ERROR: nu esti owner\n");
                return;
            }
        }
    }

    strcat(route, id_str);

    char *cookies[] = {user_cookie};

    char *request = compute_delete_request(HOST, route, cookies, 1, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: colectie stearsa\n");
    } else if(strstr(response, "404 NOT FOUND")) {
        printf("ERROR: id invalid\n");
    }
}

void delete_movie_from_collection(int sockfd) {
    if(token == NULL) {
        printf("ERROR: fara acces la librarie\n");
        return;
    }

    char col_id_str[50];
    printf("collection_id=");
    fgets(col_id_str, sizeof(col_id_str), stdin);
    col_id_str[strcspn(col_id_str, "\n")] = '\0';

    int collection_id = atoi(col_id_str);

    char mov_id_str[50];
    printf("movie_id=");
    fgets(mov_id_str, sizeof(mov_id_str), stdin);
    mov_id_str[strcspn(mov_id_str, "\n")] = '\0';

    for(int i = 0; i < info_count; i++) {
        if(info[i].id == collection_id) {
            if(strcmp(info[i].owner, current_user) !=0) {
                printf("ERROR: nu esti owner\n");
                return;
            }
        }
    }

    char route[LINELEN];
    strcpy(route, "/api/v1/tema/library/collections/");
    strcat(route, col_id_str);
    strcat(route, "/movies/");
    strcat(route, mov_id_str);

    char *cookies[] = {user_cookie};

    char *request = compute_delete_request(HOST, route, cookies, 1, token);

    send_to_server(sockfd, request);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "200 OK")) {
        printf("SUCCESS: film sters din colectie\n");
    } else if(strstr(response, "404 NOT FOUND")) {
        printf("ERROR: id invalid pentru film de sters din colectie\n");
    }
}

void exit_f() {
    if (admin_cookie != NULL) {
        free(admin_cookie);
        admin_cookie = NULL;
    }

    if (user_cookie != NULL) {
        free(user_cookie);
        user_cookie = NULL;
    }
}

int main() {
    
    while(1) {
        int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

        char buff[LINELEN];
        memset(buff, 0, LINELEN);
        fgets(buff, LINELEN, stdin);

        if(strncmp(buff, "login_admin", 10) == 0) {
            login_admin(sockfd);
        } else if(strncmp(buff, "add_user", 8) == 0) {
            add_user(sockfd);
        } else if(strncmp(buff, "get_users", 9) == 0) {
            get_users(sockfd);
        } else if(strncmp(buff, "login", 5) == 0) {
            login(sockfd);
        } else if(strncmp(buff, "delete_user", 11) == 0) {
            delete_user(sockfd);
        } else if(strncmp(buff, "logout_admin", 12) == 0) {
            logout_admin(sockfd);
        } else if(strncmp(buff, "logout", 6) == 0) {
            logout(sockfd);
        } else if(strncmp(buff, "get_access", 10) == 0) {
            get_access(sockfd);
        } else if(strncmp(buff, "get_movies", 10) == 0) {
            get_movies(sockfd);
        } else if(strncmp(buff, "add_movie_to_collection", 23) == 0) {
            add_movie_to_collection(sockfd, -1, -1);
        } else if(strncmp(buff, "add_movie", 9) == 0) {
            add_movie(sockfd);
        } else if(strncmp(buff, "get_movie", 9) == 0) {
            get_movie(sockfd);
        } else if(strncmp(buff, "update_movie", 12) == 0) {
            update_movie(sockfd);
        } else if(strncmp(buff, "delete_movie_from_collection", 28) == 0) {
            delete_movie_from_collection(sockfd);
        } else if(strncmp(buff, "delete_movie", 12) == 0) {
            delete_movie(sockfd);
        } else if(strncmp(buff, "add_collection", 14) == 0) {
            add_collection(sockfd);
        } else if(strncmp(buff, "get_collections", 15) == 0) {
            get_collections(sockfd);
        } else if(strncmp(buff, "get_collection", 14) == 0) {
            get_collection(sockfd);
        } else if(strncmp(buff, "delete_collection", 17) == 0) {
            delete_collection(sockfd);
        } else if(strncmp(buff, "exit", 4) == 0) {
            exit_f();
            break;
        }
    }
    return 0;
}