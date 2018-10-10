#include <stdio.h>
#include <stdlib.h>

void ShowError(char *err_message);
void ServeClient(int client_socket);
ssize_t Readline(int sockd, void *vptr, size_t maxlen);
ssize_t Writeline(int sockd, const void *vptr, size_t n);
