#include <stdio.h>
#include "parson.h"
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define SUCCESS 0
#define FAILURE 1

char *str_dup (const char* src);
char **str_array_copy(char **src, size_t size);

static int callback(void *data, int argc, char **argv, char **azColName);

int main(void) {

  char *colnames[] = {"ID", "NAME", "AGE", "ADDRESS", "SALARY"};

  char *test_array[][5] = {
    {"1", "Paul", "32", "California", "20000.00"},
    {"2", "Allen", "25", "Texas", "15000.00" },
    {"3", "Teddy", "23", "Norway", "20000.00" },
    {"4", "Mark", "25", "Rich-Mond", "65000.00"}
  };

  char *serialized_string = NULL;
  JSON_Status status = JSONFailure;
  size_t  len;
  char **str_vals;
  
  JSON_Value * root_val = json_value_init_object();
  JSON_Object * main_obj = json_value_get_object(root_val);




  // JSON_Value * root_val = json_value_init_array();
  // JSON_Array * main_array =  json_value_get_array(root_val);

  // len = sizeof(test_array) / sizeof(char *);

  // if(!(str_vals = str_array_copy(test_array, len))){
  //   return EXIT_FAILURE;
  // }

  // for(int i = 0; i < len; i++){
  //   status = json_array_append_string(main_array, str_vals[i]);
  //   if(status != JSONSuccess){
  //     break;
  //   }
  // }

  // // Serialization
  // if(status == JSONSuccess){
  //   serialized_string = json_serialize_to_string_pretty(root_val);
  //   puts(serialized_string);
  //   json_free_serialized_string(serialized_string);
  // } else {
  //   puts("JSON array appending error");
  // }

  // len = json_array_get_count(main_array);

  // for(int i = 0; i < len; i++){
  //   const char* elem = json_array_get_string(main_array, i);
  //   if(!elem){
  //     // Handle internal error, we should not have NULL pointers
  //     continue; // For now
  //   }
  //   // NOTE: don't need to call <free> explicitly, since json_value_free()
  //   // call takes care of this. This is b.t.w. not a very consistent design
  //   // on parson's side.
  //   //
  //   // free((void *) elem);
  // }   

  json_value_free(root_val); 

  return 0;
}

static int callback(void *data, int argc, char **argv, char **azColName){

  // TODO: improve sloppy resource / memory management

  JSON_Object * obj = (JSON_Object *) data;
  static int first_run = TRUE;

  for(int i = 0; i < argc; i++){
    const char* col_name = azColName[i];
    JSON_Status status = JSONFailure;
    if(first_run){
      // Create new key and json array for values, for a column
      char *key = str_dup(col_name);
      JSON_Value * col_values = json_value_init_array();
      if (!col_values){
        return FAILURE;
      }
      status = json_object_set_value(obj, key, col_values);
      if(status != JSONSuccess){
        return FAILURE;
      }
    } 
    JSON_Array * cvals = json_object_get_array(obj, col_name);
    if(!cvals){
      // Missing column should be considered a failure
      return FAILURE;
    }
    // Append column value for this row
    char * val_copy = str_dup(argv[i]);
    status = json_array_append_string(cvals, val_copy);
    if(!status){
      free(val_copy);
      return FAILURE;
    }
  }
  first_run = FALSE;
  return SUCCESS;
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

char **str_array_copy(char **src, size_t size){
  char **copy = NULL;
  copy = (char**) malloc(size * sizeof(char *));
  if(!copy){
    return NULL;
  }
  for(int i = 0; i < size; i++){
    char *current = str_dup(src[i]);
    if(!current){
      free(copy);
      return NULL;
    }
    copy[i] = current;
  }  
  return copy;
}