#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "the2.h"

/*
 *  returns the order of the letter in the alphabet
 *  if given char is not an ascii letter, returns 0
 */
int letter_order(char c) {
    if (c < 91 && c > 64) {
        return c - 64;
    } else if (c < 123 && c > 96) {
        return c - 96;
    }
    return 0;
}
/*
 * Prints the connections of given user.
 */
void print_connections(User *user) {
    Connection *connection;
    int i;
    for (i = 0; i < BUCKET_SIZE; i++) {
        printf("Bucket %d: ", i);
        if (user->connections[i] != NULL) {
            connection = user->connections[i];
            do {
                printf("[user1: %s, user2: %s, connection_time: %ld] -> ", connection->user1->name,
                       connection->user2->name, connection->connection_time);

            } while ((connection = connection->next_connection) != NULL);
        }
        printf("NULL\n");


    }
}

/*
 * Returns the hash code of the user.
 */
unsigned long hash_code(User *user) {
    unsigned long hash = 0;
    int i = 0;

    while ((*user).name[i] != 0) {
        hash += letter_order((*user).name[i]) * pow(HASH_KEY, i);
        i++;
    }

    return hash;
}

/*
 * Returns the User with given id from environment.
 */
User *get_user(Environment environment, int id) {
    /* TODO: Implement this function. */
    int i = 0;
    while(environment.users[i] != NULL){
      if((environment.users[i])->id != id) i++;
      else if((environment.users[i])->id == id) return environment.users[i];
    }
    return NULL;
}

void connecthelper(User *user1,User *user2,long connection_time){
  Connection *connection_node = malloc(sizeof(Connection));
  Connection *cur_node;
  int hash2;
  /*HASH2 IS WRITTEN TO THE CONNECTION OF USER1 */
  hash2 = hash_code(user2) % 10;

  connection_node->user1 = user1;
  connection_node->user2 = user2;
  connection_node->prev_connection = NULL;
  connection_node->next_connection = NULL;
  connection_node->connection_time = connection_time;
  cur_node = user1->connections[hash2];

  if((user1->connections)[hash2] == NULL){
        user1->connections[hash2] = connection_node;
        return;
  }
  if(cur_node->next_connection == NULL && cur_node->prev_connection == NULL){
    if(cur_node->connection_time > connection_time){
      connection_node->next_connection = cur_node;
      cur_node->prev_connection = connection_node;
      user1->connections[hash2] = connection_node;
      return;
    }
    else{
      cur_node->next_connection = connection_node;
      connection_node->prev_connection = cur_node;
      return;
    }
  }
  while(cur_node->next_connection != NULL){
      if(cur_node->connection_time > connection_time) break;
      cur_node = cur_node->next_connection;
  }
  if(cur_node->prev_connection == NULL){
      connection_node->next_connection = cur_node;
      cur_node->prev_connection = connection_node;
      user1->connections[hash2] = connection_node;
      return;
  }
  if(cur_node->next_connection == NULL){
      cur_node->prev_connection->next_connection = connection_node;
      cur_node->prev_connection = connection_node;
      connection_node->next_connection = cur_node;
      return;
  }
  connection_node->next_connection = cur_node;
  connection_node->prev_connection = cur_node->prev_connection;
  cur_node->prev_connection->next_connection = connection_node;
  cur_node->prev_connection = connection_node;
  return;
}

/*
 * Connects two user and registers the related connection objects to both users' connection hash tables.
 */
void connect_users(Environment environment, int id1, int id2, long connection_time) {
    /* TODO: Implement this function. */
    User *user1,*user2;
    /* HASH1 IS WRITTEN TO THE CONECTION OF USER2, HASH2 IS WRITTEN TO THE CONNECTION OIF USER1 */
    if(get_connection(environment, id1, id2) != NULL || get_connection(environment, id2, id1) != NULL) return;
    user1 = get_user(environment, id1);
    user2 = get_user(environment, id2);
    connecthelper(user1, user2, connection_time);
    connecthelper(user2, user1, connection_time);
}


/*
 * Rstrip the given string.
 */

char *rstrip(char *string) {
  char *end;

  while(isspace((unsigned char)*string)){
    string++;
  }

  if(*string == 0) {
    return string;
  }

  end = string + strlen(string) - 1;
  while(end > string && isspace((unsigned char)*end)){
    end--;
  }

  end[1] = '\0';

  return string;
}


/*
 * Creates a new environment with the information in the given file that contains users
 * with connections and returns it.
 */
Environment *init_environment(char *user_file_name) {
    Environment *environment;
    User *user;
    FILE *fp;
    char *line = NULL;
    char *iter;
    char *sep = " ";
    size_t len = 0;
    size_t read;
    int id, id1, id2;
    long timestamp;
    char *name;
    User* user1, *user2;

    int u_count = 0;

    environment = malloc(sizeof(Environment));
    environment->users = malloc(sizeof(User*));
    environment->users[0] = NULL;

    fp = fopen(user_file_name, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        line = rstrip(line);
        if (strcmp(line, "#Users#") == 0) {
            continue;
        } else if (strcmp(line, "#Connections#") == 0) {
            break;
        }
        user = malloc(sizeof(User));
        iter = strtok(line, sep);
        id = atoi(iter);
        iter = strtok(NULL, sep);
        name = malloc(sizeof(char)*strlen(iter));
        strcpy(name, iter);
        user->id=id;
        user->name=name;
        environment->users = realloc(environment->users, sizeof(User*)*(u_count+2));
        environment->users[u_count] = user;
        environment->users[u_count+1] = NULL;
        u_count++;
    }

    /* Uncomment after connect_users function implemented. */
    while ((read = getline(&line, &len, fp)) != -1) {
        line = rstrip(line);
        iter = strtok(line, sep);
        id1 = atoi(iter);
        iter = strtok(NULL, sep);
        id2 = atoi(iter);
        iter = strtok(NULL, sep);
        timestamp = atol(iter);
        connect_users(*environment, id1, id2, timestamp);

    }

    return environment;

}

/*
 * Returns the connection between users with id1 and id2. The connection
 * is the one stored in user1's hash table. If there is no such a connection returns
 * NULL.
 */
Connection *get_connection(Environment environment, int id1, int id2) {
    /* TODO: Implement this function. */
    Connection *cur_node;
    User *user1, *user2;
    int hash2;
    user1 = get_user(environment, id1);
    user2 = get_user(environment, id2);
    if(user1 == NULL || user2 == NULL) return NULL;
    hash2 = hash_code(user2) % 10;
    cur_node = user1->connections[hash2];
    if(cur_node == NULL) return NULL;
    while(cur_node != NULL){
      if(cur_node->user1 == user1 && cur_node->user2 == user2) return cur_node;
      cur_node = cur_node->next_connection;
    }
    return NULL;
}

/*
 * Remove connection from the given user. Additionally, finds the other user and removes related connection
 * from her/his table also.
 */
void removehelper(Environment environment ,int id1, int id2){
    User *user1, *user2;
    Connection *cur_node;
    int hash2;
    user1 = get_user(environment, id1);
    user2 = get_user(environment, id2);
    hash2 = hash_code(user2) % 10;
    cur_node = get_connection(environment, id1, id2);
    if(cur_node->prev_connection == NULL && cur_node->next_connection == NULL){
        free(cur_node);
        user1->connections[hash2] = NULL;
    }
    else if(cur_node->prev_connection == NULL){
        cur_node->next_connection->prev_connection = NULL;
        user1->connections[hash2] = cur_node->next_connection;
        free(cur_node);
    }
    else if(cur_node->next_connection == NULL){
        cur_node->prev_connection->next_connection = NULL;
        free(cur_node);
    }
    else{
        cur_node->prev_connection->next_connection = cur_node->next_connection;
        cur_node->next_connection->prev_connection = cur_node->prev_connection;
        free(cur_node);
    }
}

void remove_connection(Environment environment, int id1, int id2) {
    /* TODO: Implement this function. */
    Connection *cur_node1, *cur_node2;
    cur_node1 = get_connection(environment, id1, id2);
    cur_node2 = get_connection(environment, id2, id1);
    if(cur_node1 == NULL || cur_node2 == NULL) return;
    else{
      removehelper(environment, id1, id2);
      removehelper(environment, id2, id1);
    }
}

/*
 * Returns dynamic User array which contains common connections. The last element
 * of the array is NULL, which is mandatory for detecting the end.
 */


User **get_common_connections(Environment environment, User *user1, User *user2) {
    /* TODO: Implement this function. */
    Connection *cur_node1, *cur_node2;
    int common_hash, i = 0;
    User **common_connections = malloc(sizeof(User *));
    for (common_hash = 0; common_hash < 10; common_hash++){
      if(user1->connections[common_hash] == NULL || user2->connections[common_hash] == NULL) continue;
      else{
        cur_node1 = user1->connections[common_hash];
        cur_node2 = user2->connections[common_hash];
        while(cur_node1 != NULL){
          while(cur_node2 != NULL){
            if(cur_node1->user2 == cur_node2->user2){
              common_connections[i] = cur_node1->user2;
              i++;
              common_connections = (User **) realloc(common_connections, sizeof(User *) * (i+1));
            }
            cur_node2 = cur_node2->next_connection;
          }
          cur_node1 = cur_node1->next_connection;
        }
      }
    }

    common_connections[i] = NULL;
    return common_connections;
}
