#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
using namespace std;

char* page;
const int PORT = 8080;
const int num_threads = 100;
const char* web_page = "rng.html";
queue<int> connections;
pthread_mutex_t con_lock;


// Get the value input by the user
int get_range(char* buffer, int buf_len){
	if (buf_len == 0)
		return -1;
	vector<string> input;
	int ends = 0;
	for (int i = 0; i < buf_len; i++){
		string temp;
		while (i < buf_len && buffer[i] != '\n' && buffer[i] != EOF){
			temp += buffer[i];
			i++;
		}
		if (temp.size() == 0 || temp[0] =='\n'){
			if (ends == 0){
				ends = input.size();
			}
		}
		input.push_back(temp);
		break;
	}
	vector<string> parsed;
	int j = 0;
	parsed.push_back("");
	for (int i = 0; i < input[0].size(); i++){
		if (input[0][i] == ' '){
			j++;
			parsed.push_back("");
		}
		else
			parsed[j].push_back(input[0][i]);
		if (j == 2)
			break;
	}
	string header = parsed[1];
	if (header.find("range") == string::npos || header.substr(header.find("range")+6) == "")
		return 1;
	return stoi(header.substr(header.find("range")+6));
}

// Connection for each thread
void *connection (void * argv){
	while (true){
		int con_value;
		pthread_mutex_lock(&con_lock);
		if (connections.empty()){
			pthread_mutex_unlock(&con_lock);
			continue;
		}
		else{
			con_value = connections.front();
			connections.pop();
			pthread_mutex_unlock(&con_lock);
		}
		char buffer[1000] = {0};
		bzero(buffer,1000);
		int buf_len = recv(con_value,buffer,1000,0);
		if (buf_len < 0){
			perror("Could not receive");
			return NULL;
		}
		int x = get_range(buffer,buf_len);
		string str = "HTTP/1.1 200 Okay\r\nContent-Type: text/html; charset=ISO-8859-4 \r\n\r\n" + string(page);
		str.replace(str.find("<!num>"),6,to_string(rand()%x));
		char* message = new char[str.length() + 1];	
		strcpy(message,str.c_str());
		int send_value = send(con_value,message,strlen(message),0);
		if (send_value < 0){
			perror("Could not send");
			return NULL;
		}
		close(con_value);
	}
}


//Add the page to the buffer
void add_page(){
	FILE *file = fopen(web_page,"r");
	char *buffer = (char *) malloc(1000*sizeof(char));
	int i = 0;
	int x;
	while ((x = fgetc(file)) != EOF)
		buffer[i++] = (char) x;
	buffer[i] = '\0';
	page = buffer;
}

int main(){
	add_page();
	int socket_descriptor = socket(AF_INET,SOCK_STREAM,0);
	if (socket_descriptor <= 0){
		perror("Could not open socket");
		return -1;
	}
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);
	int addrlen = sizeof(address);
	memset(address.sin_zero,'\0',sizeof(address.sin_zero));
	int bind_value = bind(socket_descriptor,(struct sockaddr *)&address,sizeof(address));
	if (bind_value < 0){
		perror("Bind failed");
		return -1;
	}
	const int queue_size = 10000;
	int listen_value = listen(socket_descriptor,queue_size);
	if (listen_value < 0){
		perror("Listen failed");
		return -1;
	}
	
	pthread_t threads[num_threads];
	for (int i = 0; i < num_threads; i++){
		int thread_value = pthread_create(&threads[i],NULL,connection,(void *) NULL);
		if (thread_value < 0){
			perror("Could not create thread");
			return -1;
		}
	}

	while (true){
		printf("\nWaiting for new connection\n\n");
		int con_value = accept(socket_descriptor,(struct sockaddr *) &address, (socklen_t *) &addrlen);
		if (con_value < 0){
			perror("Connection failed");
			return -1;
		}
		pthread_mutex_lock(&con_lock);
		connections.push(con_value);
		pthread_mutex_unlock(&con_lock);
	}
	shutdown(socket_descriptor,2);
}