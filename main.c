#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "./cJSON-1.7.18/cJSON.h"

#define MAX_DESCRIPTION 256
#define DATE_LENGTH 20
#define FILENAME "todo.json"

typedef struct {
    int id;
    char *description;
    char *status;  // "todo", "in-progress", "done"
    char *created_at;
    char *updated_at;
} Task;

Task *tasks = NULL;
int task_count = 0;

// Helper functions
char* get_current_time() {
    time_t now = time(NULL);
    char *time_str = malloc(DATE_LENGTH);
    strftime(time_str, DATE_LENGTH, "%Y-%m-%d %H:%M:%S", localtime(&now));
    return time_str;
}

int get_max_id() {
    int max_id = 0;
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].id > max_id) {
            max_id = tasks[i].id;
        }
    }
    return max_id;
}

// Core functions
void load_tasks() {
    FILE *file = fopen(FILENAME, "r");
    if (!file) return;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    fclose(file);
    buffer[length] = '\0';

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    
    if (!json) return;

    task_count = cJSON_GetArraySize(json);
    tasks = malloc(task_count * sizeof(Task));
    
    for (int i = 0; i < task_count; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        tasks[i].id = cJSON_GetObjectItem(item, "id")->valueint;
        tasks[i].description = strdup(cJSON_GetObjectItem(item, "description")->valuestring);
        tasks[i].status = strdup(cJSON_GetObjectItem(item, "status")->valuestring);
        tasks[i].created_at = strdup(cJSON_GetObjectItem(item, "created_at")->valuestring);
        tasks[i].updated_at = strdup(cJSON_GetObjectItem(item, "updated_at")->valuestring);
    }
    
    cJSON_Delete(json);
}

void save_tasks() {
    cJSON *json = cJSON_CreateArray();
    
    for (int i = 0; i < task_count; i++) {
        cJSON *task = cJSON_CreateObject();
        cJSON_AddNumberToObject(task, "id", tasks[i].id);
        cJSON_AddStringToObject(task, "description", tasks[i].description);
        cJSON_AddStringToObject(task, "status", tasks[i].status);
        cJSON_AddStringToObject(task, "created_at", tasks[i].created_at);
        cJSON_AddStringToObject(task, "updated_at", tasks[i].updated_at);
        cJSON_AddItemToArray(json, task);
    }
    
    char *json_str = cJSON_Print(json);
    FILE *file = fopen(FILENAME, "w");
    fputs(json_str, file);
    fclose(file);
    
    cJSON_Delete(json);
    free(json_str);
}

void add_task(const char *description) {
    tasks = realloc(tasks, (task_count + 1) * sizeof(Task));
    
    Task *new_task = &tasks[task_count];
    new_task->id = get_max_id() + 1;
    new_task->description = strdup(description);
    new_task->status = strdup("todo");
    
    char *time = get_current_time();
    new_task->created_at = strdup(time);
    new_task->updated_at = strdup(time);
    free(time);
    
    task_count++;
    printf("Added task #%d: %s\n", new_task->id, description);
}

void update_task_status(int id, const char *status) {
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].id == id) {
            free(tasks[i].status);
            tasks[i].status = strdup(status);
            
            free(tasks[i].updated_at);
            tasks[i].updated_at = get_current_time();
            
            printf("Updated task #%d to '%s'\n", id, status);
            return;
        }
    }
    printf("Task #%d not found\n", id);
}

void delete_task(int id) {
    int index = -1;
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        printf("Task #%d not found\n", id);
        return;
    }
    
    // Free task memory
    free(tasks[index].description);
    free(tasks[index].status);
    free(tasks[index].created_at);
    free(tasks[index].updated_at);
    
    // Shift array
    for (int i = index; i < task_count - 1; i++) {
        tasks[i] = tasks[i + 1];
    }
    
    task_count--;
    tasks = realloc(tasks, task_count * sizeof(Task));
    printf("Deleted task #%d\n", id);
}

void list_tasks(const char *filter) {
    printf("\nTODO List (%s):\n", filter);
    printf("ID\tStatus\t\tDescription\n");
    printf("--------------------------------\n");
    
    for (int i = 0; i < task_count; i++) {
        if (strcmp(filter, "all") == 0 || 
            strcmp(filter, tasks[i].status) == 0) {
            printf("%d\t%-12s\t%s\n", 
                  tasks[i].id, 
                  tasks[i].status, 
                  tasks[i].description);
        }
    }
    printf("\n");
}

void cleanup() {
    for (int i = 0; i < task_count; i++) {
        free(tasks[i].description);
        free(tasks[i].status);
        free(tasks[i].created_at);
        free(tasks[i].updated_at);
    }
    free(tasks);
}

void print_help() {
    printf("\nTODO List Manager\n");
    printf("Usage:\n");
    printf("  todo add <description>    Add a new task\n");
    printf("  todo update <id> <status> Update task status\n");
    printf("  todo delete <id>          Delete a task\n");
    printf("  todo list [filter]        List tasks (all|todo|in-progress|done)\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    // Initialize
    if (access(FILENAME, F_OK) {
        FILE *file = fopen(FILENAME, "w");
        fputs("[]", file);
        fclose(file);
    }
    
    load_tasks();
    
    // Process commands
    if (argc < 2) {
        print_help();
        cleanup();
        return 1;
    }
    
    const char *command = argv[1];
    
    if (strcmp(command, "add") == 0 && argc >= 3) {
        add_task(argv[2]);
    } 
    else if (strcmp(command, "update") == 0 && argc >= 4) {
        update_task_status(atoi(argv[2]), argv[3]);
    } 
    else if (strcmp(command, "delete") == 0 && argc >= 3) {
        delete_task(atoi(argv[2]));
    } 
    else if (strcmp(command, "list") == 0) {
        const char *filter = (argc >= 3) ? argv[2] : "all";
        list_tasks(filter);
    } 
    else {
        print_help();
    }
    
    save_tasks();
    cleanup();
    return 0;
}