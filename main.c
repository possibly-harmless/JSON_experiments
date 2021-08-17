#include <stdio.h>
#include "parson.h"
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define SUCCESS 0
#define FAILURE 1
#define BOOL int


char *str_dup (const char* src);
int json_serialization_callback(void *data, int argc, char **argv, char **azColName);

JSON_Status serialize_to_json(
  JSON_Status exec(
    void * exec_data,
    int cb(void * , int , char **, char **)
  ),
  char **result_ptr
);

JSON_Status execute_mock_db(void * exec_data,int cb(void * , int , char **, char **));


int main(void) {
  char *result = NULL;

  if(serialize_to_json(execute_mock_db, &result) == JSONSuccess){
    puts(result);
    free(result);
  } else {
    puts("JSON serialization failed");
  }

  return 0;
}

  
JSON_Status serialize_to_json(
  JSON_Status exec(
    void * exec_data,
    int cb(void * , int , char **, char **)
  ),
  char **result_ptr
){
  char *serialized_string = NULL;
  JSON_Status status = JSONFailure;

  JSON_Value * root_val = json_value_init_object();
  JSON_Object * main_obj = json_value_get_object(root_val);

  status = exec((void *)main_obj, json_serialization_callback);

  if(status == JSONSuccess){
    serialized_string = json_serialize_to_string_pretty(root_val);
    if(serialized_string){
      *result_ptr = str_dup(serialized_string);
      json_free_serialized_string(serialized_string);
    } else {
      status = JSONFailure;
      *result_ptr = NULL;
    }
  }
  // This seems to also free any strings in string arrays, keys, etc.
  // TODO: double-check that!
  json_value_free(root_val); 
  return status;
}


int json_serialization_callback(void *data, int argc, char **argv, char **azColName){
  // TODO: improve sloppy resource / memory management
  JSON_Object * obj = (JSON_Object *) data;
  static BOOL first_run = TRUE;

  // TODO : temporary, until we either start throwing exceptions from  callback, or
  // switch to using the step interface
  static BOOL failed = FALSE; 

  if(failed){
    return FAILURE;
  }

  for(int i = 0; i < argc; i++){
    const char* col_name = azColName[i];
    JSON_Status status = JSONFailure;
    if(first_run){
      // Create new key and json array for values, for a column
      char *key = str_dup(col_name);
      JSON_Value * col_values = json_value_init_array();
      if (!col_values){
        failed = TRUE;
        return FAILURE;
      }
      status = json_object_set_value(obj, key, col_values);
      if(status != JSONSuccess){
        failed = TRUE;
        return FAILURE;
      }
    } 
    JSON_Array * cvals = json_object_get_array(obj, col_name);
    if(!cvals){
      // Missing column should be considered a failure
      failed = TRUE;
      return FAILURE;
    }
    // Append column value for this row
    char * val_copy = str_dup(argv[i]);
    status = json_array_append_string(cvals, val_copy);
    if(status != JSONSuccess){
      free(val_copy);
      failed = TRUE;
      return FAILURE;
    }
  }
  first_run = FALSE;
  return SUCCESS;
}


JSON_Status execute_mock_db(void * exec_data,int cb(void * , int , char **, char **)){
  char *colnames[] = {"ID", "NAME", "AGE", "ADDRESS", "SALARY"};

  char *test_array[][5] = {
    {"1", "Paul", "32", "California", "20000.00"},
    {"2", "Allen", "25", "Texas", "15000.00" },
    {"3", "Teddy", "23", "Norway", "20000.00" },
    {"4", "Mark", "25", "RichMond", "65000.00"}
  };

  size_t  
    n_rows = sizeof(test_array) / sizeof(test_array[0]), 
    n_cols = sizeof(test_array[0]) / sizeof(test_array[0][0]);

  for(int i = 0; i < n_rows; i++){
    if(cb(exec_data, n_cols, test_array[i], colnames) != SUCCESS){
      puts("Error encountered in callback. Terminating...");
      return JSONFailure;
    }
  }

  return JSONSuccess;
}


char *str_dup (const char* src){
  char * result = (char *) malloc(strlen(src) + 1), *p = result;
  if(!result){
    return NULL;
  }
  while((*p++ = *src++))
    ; 
  return result;
}
