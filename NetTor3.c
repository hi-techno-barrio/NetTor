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
#include <curl/curl.h>
#include <jansson.h>

#define ESC "\033"

// Function to print a colored box with text
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

// Function to send data to Google Sheets API
void send_to_google_sheets(const char *access_token, const char *spreadsheet_id, int max_file_size) {
    // Build the request payload
    json_t *data = json_array();
    json_t *row = json_pack("[s,i]", "Network Monitoring Data", max_file_size);
    json_array_append_new(data, row);

    json_t *request_body = json_pack("{s:o}", "values", data);
    char *payload = json_dumps(request_body, 0);

    // Set the API endpoint and authorization header
    const char *api_url = "https://sheets.googleapis.com/v4/spreadsheets";
    char api_endpoint[256];
    snprintf(api_endpoint, sizeof(api_endpoint), "%s/%s/values/%s:append?valueInputOption=USER_ENTERED&access_token=%s",
            api_url, spreadsheet_id, "Sheet1!A1:B1", access_token);
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", access_token);

    // Set up the cURL request
    CURL *curl = curl_easy_init();
    if (curl) {
        // Set the URL and request type
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

        // Set the API endpoint and authorization header
        char api_endpoint[256];
        snprintf(api_endpoint, sizeof(api_endpoint), "%s/%s/values/%s:append?valueInputOption=USER_ENTERED&access_token=%s",
            api_url, spreadsheet_id, "Sheet1!A1:B1", access_token);

        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", access_token);

        // Set the HTTP POST headers and payload
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth_header);

        curl_easy_setopt(curl, CURLOPT_URL, api_endpoint);
           curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);

    // Clean up
    curl_slist_free_all(headers);
    json_decref(request_body);
    curl_easy_cleanup(curl);

    // Check for errors
    if (res != CURLE_OK) {
        fprintf(stderr, "Error sending data to Google Sheets: %s\n", curl_easy_strerror(res));
    }
} else {
    fprintf(stderr, "Failed to initialize cURL\n");
}

// Free the payload
free(payload);
}
// Function to retrieve an access token from Google OAuth 2.0 API
char* get_access_token(const char *client_id, const char *client_secret, const char *refresh_token) {
// Set the API endpoint and request parameters
const char *api_url = "https://oauth2.googleapis.com/token";
char post_data[256];
snprintf(post_data, sizeof(post_data), "client_id=%s&client_secret=%s&refresh_token=%s&grant_type=refresh_token",
client_id, client_secret, refresh_token);// Set up the cURL request
CURL *curl = curl_easy_init();
if (curl) {
    // Set the URL and request type
    curl_easy_setopt(curl, CURLOPT_URL, api_url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // Set the HTTP POST headers and payload
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

    // Store the response in a buffer
    char buffer[1024];
    buffer[0] = '\0';
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);

    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Check for errors
    if (res == CURLE_OK) {
        // Parse the response and extract the access token
        json_t *root = NULL;
        json_error_t error;

        root = json_loads(buffer, 0, &error);
        if (!root) {
            fprintf(stderr, "Error parsing JSON response: %s\n", error.text);
            return NULL;
        }

        json_t *access_token_obj = json_object_get(root, "access_token");
        if (!access_token_obj || !json_is_string(access_token_obj)) {
            fprintf(stderr, "Error getting access token from JSON response\n");
            json_decref(root);
            return NULL;
        }

        const char *access_token = json_string_value(access_token_obj);
        char *result = strdup(access_token);

        json_decref(root);
        return result;
    } else {
        fprintf(stderr, "Error retrieving access token from Google OAuth 2.0 API: %s\n", curl_easy_strerror(res));
        return NULL;
    }
} else {
    fprintf(stderr, "Failed to initialize cURL\n");
    return NULL;
}
}

// Main function
int main() {
// Print open ports
printf("Open ports:\n");
printf("-----------\n");
int i, sockfd;
struct sockaddr_in target;
for (i = 1; i <= 65535; i++) {
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if    sockfd < 0) {
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

// Retrieve Google Sheets API credentials from environment variables
char *client_id = getenv("GOOGLE_CLIENT_ID");
char *client_secret = getenv("GOOGLE_CLIENT_SECRET");
char *refresh_token = getenv("GOOGLE_REFRESH_TOKEN");
char *spreadsheet_id = getenv("GOOGLE_SPREADSHEET_ID");

if (!client_id || !client_secret || !refresh_token || !spreadsheet_id) {
    fprintf(stderr, "Error: missing Google Sheets API credentials or spreadsheet ID\n");
    exit(EXIT_FAILURE);
}

// Get access token
char *access_token = get_access_token(client_id, client_secret, refresh_token);
if (!access_token) {
    fprintf(stderr, "Failed to retrieve access token from Google OAuth 2.0 API\n");
    exit(EXIT_FAILURE);
}

// Send network monitoring data to Google Sheets
printf("\nSending network monitoring data to Google Sheets:\n");
printf("------------------------------------------------\n");

// Prepare request body as JSON object
json_t *request_body = json_object();
json_object_set_new(request_body, "range", json_string("Sheet1!A1:E1"));
json_object_set_new(request_body, "majorDimension", json_string("ROWS"));
json_t *values = json_array();
json_array_append_new(values, json_string("Date"));
json_array_append_new(values, json_string("Highest file size"));
json_array_append_new(values, json_string("Open ports"));
json_array_append_new(values, json_string("Running services"));
json_array_append_new(values, json_string("IP address"));
json_object_set_new(request_body, "values", values);

// Send the data to Google Sheets
send_data_to_google_sheets(access_token, spreadsheet_id, request_body);

// Free access token
free(access_token);

// Display colored box with message
printf("\n");
print_box("This is a colored box!", 40, 10, 33, 47);

return 0;
}
