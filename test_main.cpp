#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

extern "C" int evaluate(FILE* istream);

bool evaluate_and_check(FILE* istream, const int expected) {
  int result = evaluate(istream);
  if (result == expected) {
    printf("Pass\n");
  } else {
    printf("Wrong answer\n");
    printf("Got\t%d\n", result);
    printf("Expected\t%d\n", expected);
  }
  return result == expected;
}

int main() {
  const char* tests_path = "test_samples";
  const char* ans_path = "answers";
  
  int dirfd = open(tests_path, O_RDONLY | O_DIRECTORY);
  int ans_dirfd = open(ans_path, O_RDONLY | O_DIRECTORY);
  DIR* dir = fdopendir(dirfd);
  struct dirent* entry;

  while (entry = readdir(dir)) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      continue;
    }

    int answer = 0;
    int test_fd = openat(dirfd, entry->d_name, O_RDWR);
    int answer_fd = openat(ans_dirfd, entry->d_name, O_RDWR);
    if (test_fd == -1) {
      perror("test");
    } else {
      FILE *fp = fdopen(test_fd, "rw");
      FILE *answer_file = fdopen(answer_fd, "r");

      fscanf(answer_file, "%d", &answer);
      printf("Test: %s\n", entry->d_name);
      evaluate_and_check(fp, answer);

      fclose(fp);
      fclose(answer_file);
      close(test_fd);
      close(answer_fd);
    }
  }
  closedir(dir);
  return 0;
}