#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>
#include "cJSON.h"

// 读取文件内容的帮助函数
char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("无法打开文件: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    const long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(length + 1);
    if (data) {
        fread(data, 1, length, file);
        data[length] = '\0';  // 确保以空字符结尾
    }

    fclose(file);
    return data;
}

// 显示服务器列表
void display_servers(const cJSON *json) {
    int index = 1;
    cJSON *server;
    cJSON_ArrayForEach(server, json) {
        printf("%d. %s\n", index, server->string);
        index++;
    }
}

// 显示某个服务器的用户名列表
void display_usernames(const cJSON *server) {
    const cJSON *usernames = cJSON_GetObjectItemCaseSensitive(server, "Username");
    if (cJSON_IsArray(usernames)) {
        int index = 1;
        cJSON *username;
        cJSON_ArrayForEach(username, usernames) {
            if (cJSON_IsString(username)) {
                printf("%d. %s\n", index, username->valuestring);
                index++;
            }
        }
    }
}

// 检测目录是否存在
int check_directory_exists(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return 1;  // 路径存在且是目录
    }
    return 0;  // 路径不存在或不是目录
}

// 读取可执行文件的路径
char *read_exe_path() {
    // 获取当前可执行文件的完整路径
    char path[MAX_PATH];
    const DWORD length = GetModuleFileName(NULL, path, sizeof(path));

    if (length == 0 || length == sizeof(path)) {
        fprintf(stderr, "获取路径失败\n");
        return NULL;  // 返回 NULL 表示获取路径失败
    }

    // 找到最后一个斜杠的位置
    char *last_slash = strrchr(path, '\\');
    if (last_slash != NULL) {
        *last_slash = '\0';  // 替换斜杠为结束符，以获得目录路径
    }

    // 动态分配内存以返回目录字符串
    char *directory = malloc(strlen(path) + 1);
    if (directory == NULL) {
        fprintf(stderr, "内存分配失败\n");
        return NULL;  // 返回 NULL 表示内存分配失败
    }

    strcpy(directory, path);  // 复制目录路径到动态分配的内存中
    return directory;  // 返回目录字符串的指针
}

int main() {
    const char full_config_path[MAX_PATH];
    const char *exe_path = read_exe_path();
    const char *filename = "\\.y5config\\ssh_config.json";
    sprintf(full_config_path, "%s%s", exe_path, filename);
    char *json_str = read_file(full_config_path);
    if (!json_str) {
        return 1;
    }

    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        printf("解析配置文件失败！\n");
        free(json_str);
        return 1;
    }

    // 1. 输出服务器列表
    printf("可用服务器列表：\n");
    display_servers(json);

    // 2. 接收服务器编号输入
    int server_choice;
    printf("请输入服务器编号：");
    scanf_s("%d", &server_choice);

    // 检查服务器编号是否有效
    int server_count = cJSON_GetArraySize(json);
    if (server_choice < 1 || server_choice > server_count) {
        printf("无效的服务器编号！\n");
        cJSON_Delete(json);
        free(json_str);
        return 1;
    }

    // 3. 根据编号选择服务器
    cJSON *selected_server = cJSON_GetArrayItem(json, server_choice - 1);
    const char *selected_server_name = selected_server->string;
    const cJSON *host = cJSON_GetObjectItemCaseSensitive(selected_server, "Host");

    // 输出选定的服务器信息
    printf("你选择的服务器是：%s (Host: %s)\n", selected_server_name, host->valuestring);

    // 4. 输出该服务器的用户名列表
    printf("可用用户名列表：\n");
    display_usernames(selected_server);

    // 5. 接收用户名编号输入
    int username_choice;
    printf("请输入用户名编号：");
    scanf_s("%d", &username_choice);

    const cJSON *usernames = cJSON_GetObjectItemCaseSensitive(selected_server, "Username");
    const int username_count = cJSON_GetArraySize(usernames);
    if (username_choice < 1 || username_choice > username_count) {
        printf("无效的用户名编号！\n");
        cJSON_Delete(json);
        free(json_str);
        return 1;
    }

    // 6. 根据编号选择用户名
    const cJSON *selected_username = cJSON_GetArrayItem(usernames, username_choice - 1);
    const char *username = selected_username->valuestring;

    // 7. 执行ssh命令
    char ssh_command[256];
    snprintf(ssh_command, sizeof(ssh_command), "ssh %s@%s", username, host->valuestring);
    printf("执行命令: %s\n", ssh_command);

    // 调用system函数执行ssh命令
    system(ssh_command);

    // 清理内存
    cJSON_Delete(json);
    free(json_str);

    return 0;
}
