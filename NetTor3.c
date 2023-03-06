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
