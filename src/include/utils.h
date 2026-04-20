void log_request(char *method, char *path, int status_code);
char* get_content_type(char *filename);
void build_json_response(char* response, int status_code, const char* message, const char* conn_header);
void build_json_response_with_data(char* response, int status_code, const char* data, const char* conn_header);