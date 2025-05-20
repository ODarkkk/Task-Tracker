#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include ".\cJSON\cJSON.h"

// id: A unique identifier for the task
// description: A short description of the task
// status: The status of the task (todo, in-progress, done)
// createdAt: The date and time when the task was created
// updatedAt: The date and time when the task was last updated

#define MAX_DESCRIPTION 256
#define DATE_LENGTH 20
#define FILENAME "list.json"

void FileVerifier();
void AddTask(char *description);
char *GetCurrentDateTime();
int GetMaxId();

// void DeleteTask();
// void UpdateTask();
// void ListTasks();
// void SaveTasks();
// void LoadTasks();

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
  // Open the file in read mode
  fptr = fopen(FILENAME, "r");
  // Check
  if (fptr == NULL)
  {
    printf("File doesn't exist\n");
    Sleep(1000);
    printf("Creating file\n");
    Sleep(1000);
    // Create a file
    fptr = fopen(FILENAME, "w");
    if (fptr != NULL)
    {
      fputs("[]", fptr);
      printf("File created and initialized\n");
      // Close the file
      fclose(fptr);
    }
    else
    {
      printf("Error creating file\n");
    }
  }
  else
  {
    // Check if file is empty
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

  // Add to the array
  cJSON_AddItemToArray(array, json);
  // Write updated array back to file
  char *out = cJSON_Print(array);
  FILE *file = fopen(FILENAME, "w");
  fputs(out, file);
  fclose(file);
  // Cleanup
  cJSON_free(out);
  cJSON_Delete(array);
  free(newTask.createdAt);
}

void DeleteTask(int taskId)
{
  cJSON *array = LoadTasksFromFile();
  cJSON *task = cJSON_GetObjectItemCaseSensitive(array, "id");
  if (task->valueint == taskId)
  {
    cJSON_Delete(task);
    cJSON_Delete(array);
    printf("Task deleted successfully.\n");
    return;
  }
}

void ListTasks(const char *status)
{
  cJSON *array = LoadTasksFromFile();
}

// void UpdateTask(int taskId, char *newDescription)
// {

cJSON *LoadTasksFromFile()
{
  FILE *file = fopen(FILENAME, "r");
  if (!file)
  {
    printf("Failed to open %s for reading\n", FILENAME);
    return cJSON_CreateArray(); // fallback
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  rewind(file);

  char *data = malloc(length + 1);
  if (!data)
  {
    fclose(file);
    printf("Memory allocation failed\n");
    return cJSON_CreateArray(); // fallback
  }

  fread(data, 1, length, file);
  data[length] = '\0';
  fclose(file);

  cJSON *array = cJSON_Parse(data);
  if (!array || !cJSON_IsArray(array))
  {
    printf("Error reading existing JSON array\n");
    cJSON_Delete(array);         // in case it's a broken object
    array = cJSON_CreateArray(); // fallback
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

// AddToJson(task)
// {

// }
int GetMaxId()
{
  FILE *file = fopen(FILENAME, "rb");

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  rewind(file);

  char *data = (char *)malloc(length + 1);
  if (!data)
  {
    fclose(file);
    printf("Memory allocation failed\n");
    exit(1);
  }

  fread(data, 1, length, file);
  data[length] = '\0';
  fclose(file);

  cJSON *json = cJSON_Parse(data);
  if (!json)
  {
    printf("Error before: [%s]\n", cJSON_GetErrorPtr());
    free(data);
    exit(1);
  }

  int max_id = 0;
  int size = cJSON_GetArraySize(json);
  for (int i = 0; i < size; i++)
  {
    cJSON *item = cJSON_GetArrayItem(json, i);
    cJSON *id = cJSON_GetObjectItem(item, "id");

    // if (cJSON_IsNumber(id))
    // {
    if (id->valueint > max_id)
    {
      max_id = id->valueint;
    }
    //   }
    // }

    cJSON_Delete(json);
    free(data);
    return max_id;
  }
}

int main(int argc, char *argv[])
{
  if (argc < 3 || strlen(argv[2]) > MAX_DESCRIPTION)
    return 1;
  FileVerifier();
  if (strcmp(argv[1], "add") == 0)
    AddTask(argv[2]);

  return 0;
}
