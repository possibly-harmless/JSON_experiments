#include <stdio.h>
#include "parson.h"
#include <stdlib.h>
#include <string.h>

char *str_dup (const char* src){
  char * result = (char *) malloc(strlen(src) + 1), *p = result;
  if(!result){
    return NULL;
  }
  while((*p++ = *src++))
    ; 
  return result;
}

// char ** copy_string_array(char **arr, size_t len){
//   char** result;
//   result = (char **) malloc(len * sizeof(char *))
// }

int main(void) {

  
  char *test_array[] = {"first", "second", "third"};
  char *serialized_string = NULL;
  JSON_Status status = JSONFailure;
  size_t  len;
  char **str_vals;
  
  JSON_Value * root_val = json_value_init_array();
  JSON_Array * main_array =  json_value_get_array(root_val);

  len = sizeof(test_array) / sizeof(char *);

  str_vals = (char**) malloc(len * sizeof(char *));
  if(!str_vals){
    return EXIT_FAILURE;
  }

  for(int i = 0; i < len; i++){
    char *current = str_dup(test_array[i]);
    if(!current){
      free(str_vals);
      return EXIT_FAILURE;
    }
    str_vals[i] = current;
    puts(test_array[i]);
  }  

  for(int i = 0; i < len; i++){
    status = json_array_append_string(main_array, str_vals[i]);
    if(status != JSONSuccess){
      break;
    }
  }

  if(status == JSONSuccess){
    serialized_string = json_serialize_to_string_pretty(root_val);
    puts(serialized_string);
    json_free_serialized_string(serialized_string);
  } else {
    puts("JSON array appending error");
  }

  len = json_array_get_count(main_array);

  for(int i = 0; i < len; i++){
    const char* elem = json_array_get_string(main_array, i);
    if(!elem){
      // Handle internal error, we should not have NULL pointers
      continue; // For now
    }
    // NOTE: don't need to call <free> explicitly, since json_value_free()
    // call takes care of this. This is b.t.w. not a very consistent design
    // on parson's side.
    //
    // free((void *) elem);
  }   

  json_value_free(root_val);

  return 0;
}