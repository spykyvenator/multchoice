#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct MC MC;
struct MC {
    uint8_t Amnt;
    uint8_t CorrectAnswer;
    char *Question;
    char **Answers;
};

static uint16_t
GetStringlen(char* in)
{
  uint16_t i;
    for (i = 0; in[i]; i++);
  return i;
}

MC*
ReadFile(char *file)
{
    uint16_t nbQuestions = 100;
    MC** Questions = malloc(sizeof(MC)*nbQuestions);
    FILE *fd;
    char *Res = NULL;
    size_t len = 0;
    ssize_t nlines;
    fd = fopen(file, "r");
    if (fd == NULL)
        return NULL;
  
  uint16_t index = 0;
  while ((nlines = getline(&Res, &len, fd) != -1)){
    uint16_t len = GetStringlen(Res);

    if (Res[0] == 45) {// start answers with -
    } else if (Res[0] == 43) {// start correct answer with +
      Questions[index]->Answers[Questions[index]->Amnt]++;
      for (unsigned char i=0; Res[i] != 0; i++){
	      Questions[index]->Answers[i] = Res[i];
	} 
    } else {
      Questions[index]->Question = malloc(sizeof(char)*len+1);
      for (unsigned char i=0; Res[i] != 0; i++){
	      Questions[index]->Question[i] = Res[i];
	}
    }
    index++;
  }
  fclose(fd);
  return Questions;
}
