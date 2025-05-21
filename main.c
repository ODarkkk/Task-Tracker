#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include ".\cJSON\cJSON.h"

#define MAX_DESCRIPTION 256
#define DATE_LENGTH 20
#define FILENAME "list.json"

// Function declarations
void FileVerifier();
void AddTask(char *description);
char *GetCurrentDateTime();
int GetMaxId();
void DeleteTask(int taskId);
void UpdateTaskDescription(int id, char *description);
void UpdateTaskStatus(char *status, int id);
void ListTasks(const char *status);
void WriteTasks(cJSON *task);
cJSON *LoadTasksFromFile();

typedef struct
{
  int id;
  char *description;
  char *status;
  char *createdAt;
  char *updatedAt;
} task;

void FileVerifier()
{
  FILE *fptr;
  fptr = fopen(FILENAME, "r");
  if (fptr == NULL)
  {
    printf("File doesn't exist\n");
    Sleep(1000);
    printf("Creating file\n");
    Sleep(1000);
    fptr = fopen(FILENAME, "w");
    if (fptr != NULL)
    {
      fputs("[]", fptr);
      printf("File created and initialized\n");
      fclose(fptr);
    }
    else
    {
      printf("Error creating file\n");
    }
  }
  else
  {
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    fclose(fptr);

    if (size == 0)
    {
      fptr = fopen(FILENAME, "w");
      fputs("[]", fptr);
      fclose(fptr);
      printf("File was empty - initialized as empty JSON array.\n");
    }
  }
}

cJSON *LoadTasksFromFile()
{
  FILE *file = fopen(FILENAME, "r");
  if (!file)
  {
    printf("Failed to open %s for reading\n", FILENAME);
    return cJSON_CreateArray();
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  rewind(file);

  char *data = malloc(length + 1);
  if (!data)
  {
    fclose(file);
    printf("Memory allocation failed\n");
    return cJSON_CreateArray();
  }

  fread(data, 1, length, file);
  data[length] = '\0';
  fclose(file);

  cJSON *array = cJSON_Parse(data);
  if (!array || !cJSON_IsArray(array))
  {
    printf("Error reading existing JSON array\n");
    cJSON_Delete(array);
    array = cJSON_CreateArray();
  }

  free(data);
  return array;
}

char *GetCurrentDateTime()
{
  time_t now = time(NULL);
  char *time_str = malloc(DATE_LENGTH);
  strftime(time_str, DATE_LENGTH, "%Y-%m-%d %H:%M:%S", localtime(&now));
  return time_str;
}

int GetMaxId()
{
  cJSON *json = LoadTasksFromFile();
  int max_id = 0;
  int size = cJSON_GetArraySize(json);
  for (int i = 0; i < size; i++)
  {
    cJSON *item = cJSON_GetArrayItem(json, i);
    cJSON *id = cJSON_GetObjectItem(item, "id");
    if (id && cJSON_IsNumber(id) && id->valueint > max_id)
    {
      max_id = id->valueint;
    }
  }
  cJSON_Delete(json);
  return max_id;
}

void WriteTasks(cJSON *task)
{
  cJSON *id = cJSON_GetObjectItem(task, "id");
  cJSON *description = cJSON_GetObjectItem(task, "description");
  cJSON *status = cJSON_GetObjectItem(task, "status");
  cJSON *createdAt = cJSON_GetObjectItem(task, "createdAt");
  cJSON *updatedAt = cJSON_GetObjectItem(task, "updatedAt");

  printf("ID: %d, Description: %s, Status: %s, Created At: %s, Updated At: %s\n",
         id->valueint,
         description->valuestring,
         status->valuestring,
         createdAt->valuestring,
         updatedAt ? updatedAt->valuestring : "N/A");
}

void AddTask(char *description)
{
  task newTask;
  newTask.id = GetMaxId() + 1;
  newTask.description = description;
  newTask.status = "todo";
  newTask.createdAt = GetCurrentDateTime();
  newTask.updatedAt = "";

  cJSON *array = LoadTasksFromFile();
  cJSON *json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "id", newTask.id);
  cJSON_AddStringToObject(json, "description", newTask.description);
  cJSON_AddStringToObject(json, "status", newTask.status);
  cJSON_AddStringToObject(json, "createdAt", newTask.createdAt);
  cJSON_AddStringToObject(json, "updatedAt", newTask.updatedAt);

  cJSON_AddItemToArray(array, json);

  char *out = cJSON_Print(array);
  FILE *file = fopen(FILENAME, "w");
  fputs(out, file);
  fclose(file);

  cJSON_free(out);
  cJSON_Delete(array);
  free(newTask.createdAt);
}

void DeleteTask(int taskId)
{
  cJSON *array = LoadTasksFromFile();
  cJSON *newArray = cJSON_CreateArray();
  int found = 0;

  int size = cJSON_GetArraySize(array);
  for (int i = 0; i < size; i++)
  {
    cJSON *currentTask = cJSON_GetArrayItem(array, i);
    cJSON *id = cJSON_GetObjectItem(currentTask, "id");
    if (id && cJSON_IsNumber(id) && id->valueint == taskId)
    {
      found = 1;
      continue;
    }
    cJSON_AddItemToArray(newArray, cJSON_Duplicate(currentTask, 1));
  }

  if (found)
  {
    char *out = cJSON_Print(newArray);
    FILE *file = fopen(FILENAME, "w");
    fputs(out, file);
    fclose(file);
    cJSON_free(out);
    printf("Task deleted successfully.\n");
  }
  else
  {
    printf("Task with ID %d not found.\n", taskId);
  }

  cJSON_Delete(array);
  cJSON_Delete(newArray);
}

void UpdateTaskDescription(int id, char *description)
{
  cJSON *array = LoadTasksFromFile();
  cJSON *taskToUpdate = NULL;

  int size = cJSON_GetArraySize(array);
  for (int i = 0; i < size; i++)
  {
    cJSON *task = cJSON_GetArrayItem(array, i);
    cJSON *taskId = cJSON_GetObjectItem(task, "id");
    if (taskId && cJSON_IsNumber(taskId) && taskId->valueint == id)
    {
      taskToUpdate = task;
      break;
    }
  }

  if (!taskToUpdate)
  {
    printf("Task with ID %d not found\n", id);
    cJSON_Delete(array);
    return;
  }

  cJSON *old_description = cJSON_GetObjectItem(taskToUpdate, "description");
  cJSON_SetValuestring(old_description, description);

  char *newTimestamp = GetCurrentDateTime();
  cJSON *updatedAt = cJSON_GetObjectItem(taskToUpdate, "updatedAt");
  cJSON_SetValuestring(updatedAt, newTimestamp);

  char *out = cJSON_Print(array);
  FILE *file = fopen(FILENAME, "w");
  if (file)
  {
    fputs(out, file);
    fclose(file);
    printf("Task %d updated successfully\n", id);
  }
  else
  {
    printf("Error saving updated tasks\n");
  }

  free(newTimestamp);
  cJSON_free(out);
  cJSON_Delete(array);
}

void UpdateTaskStatus(char *status, int id)
{
  cJSON *array = LoadTasksFromFile();
  cJSON *taskToUpdate = NULL;

  int size = cJSON_GetArraySize(array);
  for (int i = 0; i < size; i++)
  {
    cJSON *task = cJSON_GetArrayItem(array, i);
    cJSON *taskId = cJSON_GetObjectItem(task, "id");
    if (taskId && cJSON_IsNumber(taskId) && taskId->valueint == id)
    {
      taskToUpdate = task;
      break;
    }
  }

  if (!taskToUpdate)
  {
    printf("Task with ID %d not found\n", id);
    cJSON_Delete(array);
    return;
  }

  cJSON *old_status = cJSON_GetObjectItem(taskToUpdate, "status");
  cJSON_SetValuestring(old_status, status);

  char *newTimestamp = GetCurrentDateTime();
  cJSON *updatedAt = cJSON_GetObjectItem(taskToUpdate, "updatedAt");
  cJSON_SetValuestring(updatedAt, newTimestamp);

  char *out = cJSON_Print(array);
  FILE *file = fopen(FILENAME, "w");
  if (file)
  {
    fputs(out, file);
    fclose(file);
    printf("Task %d updated successfully\n", id);
  }
  else
  {
    printf("Error saving updated tasks\n");
  }

  free(newTimestamp);
  cJSON_free(out);
  cJSON_Delete(array);
}

void ListTasks(const char *status)
{
  cJSON *json = LoadTasksFromFile();
  int size = cJSON_GetArraySize(json);

  for (int i = 0; i < size; i++)
  {
    cJSON *task = cJSON_GetArrayItem(json, i);
    cJSON *taskStatus = cJSON_GetObjectItem(task, "status");
    printf(status);
    if (strcmp(status, "all") == 0 || strcmp(status, taskStatus->valuestring) == 0)
    {
      WriteTasks(task);
    }
  }

  cJSON_Delete(json);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s <command> [arguments]\n", argv[0]);
    return 1;
  }
  if (argc > 2 && strlen(argv[2]) > MAX_DESCRIPTION)
  {
    printf("Description too long\n");
    return 1;
  }
  FileVerifier();
  printf(argv[1]);
  if (strcmp(argv[1], "add") == 0 && argc >= 3)
  {
    AddTask(argv[2]);
  }
  else if (strcmp(argv[1], "list") == 0)
  {
    if (argc >= 3)
      ListTasks(argv[2]);
    else
      ListTasks("all");
  }
  else if (strcmp(argv[1], "delete") == 0 && argc >= 3)
  {
    DeleteTask(atoi(argv[2]));
  }
  else if (strcmp(argv[1], "update") == 0 && argc >= 4)
  {
    UpdateTaskDescription(atoi(argv[2]), argv[3]);
  }
  else if (strcmp(argv[1], "mark-in-progress") == 0 && argc >= 3)
  {
    UpdateTaskStatus("in-progress", atoi(argv[2]));
  }
  else if (strcmp(argv[1], "mark-done") == 0 && argc >= 3)
  {
    UpdateTaskStatus("done", atoi(argv[2]));
  }
  else
  {
    printf("Invalid command or arguments\n");
    return 1;
  }

  return 0;
}