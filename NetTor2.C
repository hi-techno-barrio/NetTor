/* 

//////////////////////////////////////////////////////////////////////////////////////////

                                 Christopher Coballes
                                    R&D ENgineer
                                  Hi-Techno Barrio

////////////////////////////////////////////////////////////////////////////////////////
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <googleapis/client/auth/oauth2_authorization.h>
#include <googleapis/client/auth/oauth2_service_authorization.h>
#include <googleapis/client/transport/curl_http_transport.h>
#include <googleapis/headers.h>
#include <googleapis/services/sheets/v4/sheets_service.h>

#define ESC "\033"

void print_box(const char *text, int width, int height, int fg_color, int bg_color) {
    // Print top border
    printf(ESC "[%d;%dm+", fg_color, bg_color);
    for (int i = 0; i < width - 2; i++) {
        putchar('-');
    }
    printf("+\033[0m\n");

    // Print sides with text
    for (int i = 0; i < height - 2; i++) {
        printf(ESC "[%d;%dm| ", fg_color, bg_color);
        for (int j = 0; j < width - 4; j++) {
            putchar(text[i * (width - 4) + j]);
        }
        printf(" |\033[0m\n");
    }

    // Print bottom border
    printf(ESC "[%d;%dm+", fg_color, bg_color);
    for (int i = 0; i < width - 2; i++) {
        putchar('-');
    }
    printf("+\033[0m\n");
}

void ftp_client(char *host, char *user, char *pass, char *remote_file, char *local_file) {
    // Resolve hostname
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, "ftp", &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return;
    }

    // Create control socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    // Connect to server
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        return;
    }

    // Read welcome message
    char buffer[1024];
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        return;
    }
    buffer[n] = '\0';
    printf("%s", buffer);

    // Send user command
    snprintf(buffer, sizeof(buffer), "USER %s\r\n", user);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("write");
        return;
    }

    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        return;
    }
    buffer[n] = '\0';
    printf("%s", buffer);

    // Send password command
    snprintf(buffer, sizeof(buffer), "PASS %s\r\n", pass);
   
n = write(sockfd, buffer, strlen(buffer));
if (n < 0) {
perror("write");
return;
}n = read(sockfd, buffer, sizeof(buffer) - 1);
if (n < 0) {
    perror("read");
    return;
}
buffer[n] = '\0';
printf("%s", buffer);

// Send size command
snprintf(buffer, sizeof(buffer), "SIZE %s\r\n", remote_file);
n = write(sockfd, buffer, strlen(buffer));
if (n < 0) {
    perror("write");
    return;
}

n = read(sockfd, buffer, sizeof(buffer) - 1);
if (n < 0) {
    perror("read");
    return;
}
buffer[n] = '\0';
printf("%s", buffer);

// Send PASV command
snprintf(buffer, sizeof(buffer), "PASV\r\n");
n = write(sockfd, buffer, strlen(buffer));
if (n < 0) {
    perror("write");
    return;
}

n = read(sockfd, buffer, sizeof(buffer) - 1);
if (n < 0) {
    perror("read");
    return;
}
buffer[n] = '\0';
printf("%s", buffer);

// Parse PASV response
int ip1, ip2, ip3, ip4, port1, port2;
if (sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2) != 6) {
    fprintf(stderr, "Failed to parse PASV response: %s\n", buffer);
    return;
}

// Connect to data socket
int datafd = socket(AF_INET, SOCK_STREAM, 0);
if (datafd < 0) {
    perror("socket");
    return;
}

struct sockaddr_in data_addr;
data_addr.sin_family = AF_INET;
data_addr.sin_port = htons(port1 * 256 + port2);
data_addr.sin_addr.s_addr = htonl((ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4);

if (connect(datafd, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
    perror("connect");
    return;
}

// Send RETR command
snprintf(buffer, sizeof(buffer), "RETR %s\r\n", remote_file);
n = write(sockfd, buffer, strlen(buffer));
if (n < 0) {
    perror("write");
    return;
}

n = read(sockfd, buffer, sizeof(buffer) - 1);
if (n < 0) {
    perror("read");
    return;
}
buffer[n] = '\0';
printf("%s", buffer);

// Read data from data socket and write to local file
int local_fd = open(local_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (local_fd < 0) {
    perror("open");
    return;
}

while ((n = read(datafd, buffer, sizeof(buffer))) > 0) {
    if (write(local_fd, buffer, n) < 0) {
        perror("write");
        return;
    }
}

close(local_fd);

// Send QUIT command
snprintf(buffer, sizeof(buffer), "QUIT\r\n");
n = write(sockfd, buffer, strlen(buffer));
if (n < 0) {
perror("write");
return;
}n = read(sockfd, buffer, sizeof(buffer) - 1);
if (n < 0) {
    perror("read");
    return;
}
buffer[n] = '\0';
printf("%s", buffer);

// Close sockets
close(datafd);
close(sockfd);
}

int main() {
// Print open ports
printf("Open ports:\n");
printf("-----------\n");
int i, sockfd;
struct sockaddr_in target;
for (i = 1; i <= 65535; i++) {
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) {
perror("socket");
exit(EXIT_FAILURE);
}

    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_addr.s_addr = htonl(INADDR_ANY);
    target.sin_port = htons(i);

    if (connect(sockfd, (struct sockaddr *)&target, sizeof(target)) == 0) {
        printf("Port %d open\n", i);
    }

    close(sockfd);
}

// Print highest file size
printf("\nHighest file size:\n");
printf("-----------------\n");

int max_size = -1;
char max_file[256];
struct stat st;
DIR *dir = opendir(".");
if (dir == NULL) {
    perror("opendir");
    exit(EXIT_FAILURE);
}

struct dirent *entry;
while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
        if (stat(entry->d_name, &st) == 0 && st.st_size > max_size) {
            max_size = st.st_size;
            strncpy(max_file, entry->d_name, sizeof(max_file));
        }
    }
}

printf("File '%s' has the highest size of %d bytes\n", max_file, max_size);

closedir(dir);

// Print running services
printf("\nRunning services:\n");
printf("-----------------\n");

system("ps axo comm,user | sort | uniq");

// Copy backup file via FTP
printf("\nCopying backup file via FTP:\n");
printf("-----------------------------\n");

ftp_client("ftp.example.com", "username", "password", "backup.zip", "backup.zip");

// Display colored box with message
printf("\n");
print_box("This is a colored box!", 40, 10, 33, 47);

// Send data to Google Sheets
printf("\nSending data to Google Sheets:\n");
printf("-----------------------------\n");

// Define the Google Sheets API endpoint
const char *api_url = "https://sheets.googleapis.com/v4/spreadsheets/[SPREADSHEET_ID]/values/[RANGE]:append?valueInputOption=USER_ENTERED";

// Build the request payload
char payload[1024];
snprintf(payload, sizeof(payload), "{\"values\": [[\"%s\", \"%s\", \"%d\"]]}",
    "localhost", "root", max_size);

// Build the authorization header
char auth_header[512];
snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", access_token);

// Send the request to the API endpoint
CURL *curl = curl_easy_init();
if (curl) {
    // Build the API URL with the spreadsheet ID and range
    char api_endpoint[1024];
    snprintf(api_endpoint, sizeof(api_endpoint), "%s&access_token=%s",api_url, spreadsheet_id, "Sheet1!A1:C1", access_token);    
  // Set the HTTP POST headers and payload
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, api_endpoint);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);

    // Send the request and capture the response
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Clean up
    curl_easy_cleanup(curl);
}

return 0;
}

