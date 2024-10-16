#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>
#include "cJSON.h"

// ��ȡ�ļ����ݵİ�������
char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("�޷����ļ�: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    const long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(length + 1);
    if (data) {
        fread(data, 1, length, file);
        data[length] = '\0';  // ȷ���Կ��ַ���β
    }

    fclose(file);
    return data;
}

// ��ʾ�������б�
void display_servers(const cJSON *json) {
    int index = 1;
    cJSON *server;
    cJSON_ArrayForEach(server, json) {
        printf("%d. %s\n", index, server->string);
        index++;
    }
}

// ��ʾĳ�����������û����б�
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

// ���Ŀ¼�Ƿ����
int check_directory_exists(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return 1;  // ·����������Ŀ¼
    }
    return 0;  // ·�������ڻ���Ŀ¼
}

// ��ȡ��ִ���ļ���·��
char *read_exe_path() {
    // ��ȡ��ǰ��ִ���ļ�������·��
    char path[MAX_PATH];
    const DWORD length = GetModuleFileName(NULL, path, sizeof(path));

    if (length == 0 || length == sizeof(path)) {
        fprintf(stderr, "��ȡ·��ʧ��\n");
        return NULL;  // ���� NULL ��ʾ��ȡ·��ʧ��
    }

    // �ҵ����һ��б�ܵ�λ��
    char *last_slash = strrchr(path, '\\');
    if (last_slash != NULL) {
        *last_slash = '\0';  // �滻б��Ϊ���������Ի��Ŀ¼·��
    }

    // ��̬�����ڴ��Է���Ŀ¼�ַ���
    char *directory = malloc(strlen(path) + 1);
    if (directory == NULL) {
        fprintf(stderr, "�ڴ����ʧ��\n");
        return NULL;  // ���� NULL ��ʾ�ڴ����ʧ��
    }

    strcpy(directory, path);  // ����Ŀ¼·������̬������ڴ���
    return directory;  // ����Ŀ¼�ַ�����ָ��
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
        printf("���������ļ�ʧ�ܣ�\n");
        free(json_str);
        return 1;
    }

    // 1. ����������б�
    printf("���÷������б�\n");
    display_servers(json);

    // 2. ���շ������������
    int server_choice;
    printf("�������������ţ�");
    scanf_s("%d", &server_choice);

    // ������������Ƿ���Ч
    int server_count = cJSON_GetArraySize(json);
    if (server_choice < 1 || server_choice > server_count) {
        printf("��Ч�ķ�������ţ�\n");
        cJSON_Delete(json);
        free(json_str);
        return 1;
    }

    // 3. ���ݱ��ѡ�������
    cJSON *selected_server = cJSON_GetArrayItem(json, server_choice - 1);
    const char *selected_server_name = selected_server->string;
    const cJSON *host = cJSON_GetObjectItemCaseSensitive(selected_server, "Host");

    // ���ѡ���ķ�������Ϣ
    printf("��ѡ��ķ������ǣ�%s (Host: %s)\n", selected_server_name, host->valuestring);

    // 4. ����÷��������û����б�
    printf("�����û����б�\n");
    display_usernames(selected_server);

    // 5. �����û����������
    int username_choice;
    printf("�������û�����ţ�");
    scanf_s("%d", &username_choice);

    const cJSON *usernames = cJSON_GetObjectItemCaseSensitive(selected_server, "Username");
    const int username_count = cJSON_GetArraySize(usernames);
    if (username_choice < 1 || username_choice > username_count) {
        printf("��Ч���û�����ţ�\n");
        cJSON_Delete(json);
        free(json_str);
        return 1;
    }

    // 6. ���ݱ��ѡ���û���
    const cJSON *selected_username = cJSON_GetArrayItem(usernames, username_choice - 1);
    const char *username = selected_username->valuestring;

    // 7. ִ��ssh����
    char ssh_command[256];
    snprintf(ssh_command, sizeof(ssh_command), "ssh %s@%s", username, host->valuestring);
    printf("ִ������: %s\n", ssh_command);

    // ����system����ִ��ssh����
    system(ssh_command);

    // �����ڴ�
    cJSON_Delete(json);
    free(json_str);

    return 0;
}
