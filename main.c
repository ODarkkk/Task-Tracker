#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "./cJSON-1.7.18/cJSON.h"

typedef char *string;

typedef struct
{
    int id;
    string description; // Dynamically allocated
    string status;      // "todo", "in-progress", or "done"
    string created_at;
    string updated_at;
} Task;

Task *tasks = NULL;


// Function declarations
void VerifyFile();
void CreateFile();
void AddTask(string description);
void SaveTasks(const string filename);
void CleanUp();

int main(int argc, string argv[])
{
    if (argc < 2)
    {
        printf("Insufficient arguments\n");
        return 1;
    }
    VerifyFile();

    const string command = argv[1];
    if (strcmp(command, "add") == 0 && argc >= 3)
    {
        AddTask(argv[2]);
    }

    SaveTasks("list.json");
    CleanUp();
}

void VerifyFile()
{
    if (access("list.json", F_OK) != 0)
    {
        printf("File doesn't exist, creating it...\n");
        CreateFile();
    }
}

void CreateFile()
{
    printf("Creating JSON file called list.json\n");
    FILE *fptr = fopen("list.json", "w");
    if (fptr)
    {
        fprintf(fptr, "[]");
        fclose(fptr);
    }
    else
    {
        perror("Error creating file");
    }
}

void AddTask(string description)
{
    int task_count = GetIds();
    tasks = realloc(tasks, (task_count + 1) * sizeof(Task));
    if (!tasks)
    {
        perror("Memory allocation failed");
        exit(1);
    }

    Task *new_task = &tasks[task_count];
    new_task->id = task_count + 1;

    new_task->description = malloc(strlen(description) + 1);
    strcpy(new_task->description, description);

    new_task->status = malloc(strlen("todo") + 1);
    strcpy(new_task->status, "todo");

    time_t now = time(NULL);

    new_task->created_at = malloc(20);
    strftime(new_task->created_at, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));

    new_task->updated_at = malloc(20);
    strftime(new_task->updated_at, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));

    task_count++;
    printf("Added task: ID=%d, %s\n", new_task->id, new_task->description);
}

int GetIds() {
    FILE *fp = fopen("list.json", "r");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *buffer = malloc(length + 1);
    if (!buffer) {
        fclose(fp);
        return 0;
    }
    
    fread(buffer, 1, length, fp);
    fclose(fp);
    buffer[length] = '\0';

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    
    if (!json) return 0;

    int count = cJSON_GetArraySize(json);
    cJSON_Delete(json);
    
    return count;
}
void SaveTasks(const string filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Error opening file");
        return;
    }
    int task_count = GetIds() + 1;

    fprintf(file, "[\n");
    for (int i = 0; i < task_count; i++)
    {
        fprintf(file, "  {\n");
        fprintf(file, "    \"id\": %d,\n", tasks[i].id);
        fprintf(file, "    \"description\": \"%s\",\n", tasks[i].description);
        fprintf(file, "    \"status\": \"%s\",\n", tasks[i].status);
        fprintf(file, "    \"created_at\": \"%s\",\n", tasks[i].created_at);
        fprintf(file, "    \"updated_at\": \"%s\"\n", tasks[i].updated_at);
        fprintf(file, "  }%s\n", (i < task_count - 1) ? "," : "");
    }
    fprintf(file, "]\n");

    fclose(file);
    printf("Saved %d tasks to %s\n", task_count, filename);
}

void CleanUp()
{
    int task_count = GetIds();

    for (int i = 0; i < task_count; i++)
    {
        free(tasks[i].description);
        free(tasks[i].status);
        free(tasks[i].created_at);
        free(tasks[i].updated_at);
    }
    free(tasks);
}
