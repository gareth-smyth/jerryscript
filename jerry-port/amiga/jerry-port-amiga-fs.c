#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "jerryscript-port.h"

#if defined(__amigaos__)

// Custom strnlen implementation (handles strings safely up to maxlen)
static size_t strnlen(const char *s, size_t maxlen)
{
  size_t i;
  for (i = 0; i < maxlen && s[i] != '\0'; i++);
  return i;
}

// Custom strndup implementation (duplicates up to n characters, ensures null-termination)
static char *strndup(const char *s, size_t n)
{
  size_t len = strnlen(s, n); // Ensure we don’t read beyond n
  char *new_string = (char *) malloc(len + 1); // +1 for null terminator
  if (new_string == NULL)
  {
    return NULL; // Always check allocation
  }
  strncpy(new_string, s, len);
  new_string[len] = '\0'; // Null-terminate the string
  return new_string;
}

// Helper function to clean and normalize paths
static char *normalize_path(const char *path)
{
  size_t len = strlen(path);
  char *result = (char *) malloc(len + 1); // Allocate buffer for normalized path
  if (result == NULL)
  {
    return NULL; // Handle allocation failure
  }

  const char *input = path;
  char *output = result;
  char *segments[1024]; // Store pointers to path segments
  int segment_count = 0;
  bool is_absolute = false;

  // Check if the path starts with a device name (e.g., "HD2:")
  const char *device_pos = strchr(path, ':');
  if (device_pos != NULL)
  {
    size_t device_len = (size_t)(device_pos - path) + 1;
    strncpy(output, path, device_len);
    output[device_len] = '\0'; // Null-terminate after the device name
    output += device_len;
    input = device_pos + 1; // Skip the device name part
    is_absolute = true;
  }

  while (*input != '\0')
  {
    // Skip multiple slashes
    if (*input == '/')
    {
      input++;
      continue;
    }

    // Find the next segment (delimited by '/' or end of string)
    const char *next_slash = strchr(input, '/');
    if (next_slash == NULL)
    {
      next_slash = input + strlen(input); // End of the string
    }

    size_t segment_len = (size_t)(next_slash - input);

    if (segment_len == 1 && input[0] == '.')
    {
      // Ignore "." (current directory)
    }
    else if (segment_len == 2 && input[0] == '.' && input[1] == '.')
    {
      // Handle ".." (parent directory) by removing the last valid segment
      if (segment_count > 0)
      {
        segment_count--;
      }
      else if (!is_absolute)
      {
        // Allow ".." if the path is relative
        segments[segment_count++] = strndup("..", 2);
      }
    }
    else
    {
      // Add the current segment to our list
      segments[segment_count++] = strndup(input, segment_len);
    }

    input = next_slash;
    if (*input != '\0')
    {
      input++; // Skip the '/'
    }
  }

  // Construct the normalized path
  for (int i = 0; i < segment_count; i++)
  {
    if (output > result && *(output - 1) != '/') // Add slash if needed
    {
      *output++ = '/';
    }
    strcpy(output, segments[i]);
    output += strlen(segments[i]);
    free(segments[i]); // Free temporary segment string
  }

  *output = '\0'; // Null-terminate the final path
  return result;
}

jerry_char_t *
jerry_port_path_normalize(const jerry_char_t *path_p, jerry_size_t path_size)
{
  // Ensure path is null-terminated
  char *path = (char *) malloc(path_size + 1);
  if (path == NULL)
  {
    return NULL; // Handle allocation failure
  }

  memcpy(path, path_p, path_size);
  path[path_size] = '\0';

  char *normalized_path = normalize_path(path);
  free(path); // Free the original input buffer

  return (jerry_char_t *) normalized_path; // Caller is responsible for freeing this
}

void
jerry_port_path_free(jerry_char_t *path_p)
{
  free(path_p);
}

jerry_size_t
jerry_port_path_base(const jerry_char_t *path_p)
{
  printf("Full Path: %s\n", path_p);

  // Find the last occurrence of '/' or ':'
  const jerry_char_t *last_delim_p = NULL;
  for (const jerry_char_t *current_p = path_p; *current_p != '\0'; current_p++)
  {
    if (*current_p == '/' || *current_p == ':')
    {
      last_delim_p = current_p; // Update to the latest delimiter found
    }
  }

  if (!last_delim_p) // Handle cases where no delimiter is found
  {
    printf("No base part found\n");
    return 0;
  }

  // Include the delimiter in the result by moving 1 past the last found delimiter
  jerry_size_t base_length = (jerry_size_t)((last_delim_p + 1) - path_p);

  // Extract and print the base part
  char base_part[base_length + 1];
  strncpy(base_part, (char *)path_p, base_length);
  base_part[base_length] = '\0';

  printf("Base Part: %s\n", base_part);

  // Return the length of the base path
  return base_length;
} /* jerry_port_path_base */


#endif /* defined(__amigaos__) */
