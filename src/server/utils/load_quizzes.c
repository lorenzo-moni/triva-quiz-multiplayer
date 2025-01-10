#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"

int get_directory_total_files(DIR *directory)
{
    if (!directory)
    {
        printf("Not valid directory");
        return -1;
    }
    int file_count = 0;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL)
    {
        // Andiamo a verificare che il file che stiamo contando non sia la directory
        // padre, la directory stessa e che il file sia un file regolare
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
            entry->d_type != DT_REG)
            continue;
        file_count++;
    }
    return file_count;
}

Quiz *load_quiz_from_file(const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (!file)
    {
        perror("Errore nell'apertura del file");
        return NULL;
    }

    Quiz *quiz = (Quiz *)malloc(sizeof(Quiz));
    if (!quiz)
    {
        perror("Errore di allocazione memoria per il quiz");
        fclose(file);
        return NULL;
    }
    quiz->questions = NULL;
    quiz->answers = NULL;
    quiz->ranking_head = NULL;
    quiz->ranking_tail = NULL;

    char *line = NULL; // Buffer dinamico per la lettura della linea
    size_t len = 0;    // Dimensione del buffer
    ssize_t read;

    int question_count = 0;

    // Leggi il nome del quiz (prima linea)
    if ((read = getline(&line, &len, file)) != -1)
    {
        line[strcspn(line, "\n")] = '\0'; // Rimuovi il carattere di nuova linea
        quiz->name = strdup(line);
    }

    // Conta le domande e risposte
    while ((read = getline(&line, &len, file)) != -1)
    {
        if (strchr(line, '|'))
            question_count++;
    }

    quiz->total_questions = question_count;

    // Alloca gli array per domande e risposte
    quiz->questions = (char **)malloc(question_count * sizeof(char *));
    quiz->answers = (char **)malloc(question_count * sizeof(char *));

    if (!quiz->questions || !quiz->answers)
    {
        perror("Failed to allocate questions and answer arrays");
        free(quiz);
        free(line);
        fclose(file);
        return NULL;
    }

    // Torna all'inizio del file per leggere le domande e risposte
    rewind(file);
    getline(&line, &len, file); // Salta il nome del quiz
    getline(&line, &len, file); // Salta la linea vuota successiva

    int index = 0;
    while ((read = getline(&line, &len, file)) != -1)
    {
        line[strcspn(line, "\n")] = '\0'; // Rimuovi il carattere di nuova linea
        char *delimiter = strchr(line, '|');
        if (delimiter)
        {
            *(delimiter - 1) = '\0';
            quiz->questions[index] = strdup(line);
            quiz->answers[index] = strdup(delimiter + 2);
            index++;
        }
    }

    free(line); // Libera il buffer allocato da getline
    fclose(file);

    return quiz;
}

int load_quizzes_from_directory(const char *directory_path, QuizzesInfo *quizzesInfo)
{
    struct dirent *entry;               // Puntatore per leggere le informazioni dei file
    DIR *dir = opendir(directory_path); // Apre la directory

    if (dir == NULL)
    {
        perror("Error in directory opening");
        return -1;
    }

    quizzesInfo->total_quizzes = get_directory_total_files(dir);

    // Alloco un array di puntatori a quiz nello heap che verranno popolati uno
    // alla volta leggendo i files della directory
    quizzesInfo->quizzes = (Quiz **)malloc((quizzesInfo->total_quizzes) * sizeof(Quiz *));

    if (!quizzesInfo->quizzes)
    {
        perror("Errore di allocazione memoria per l'array dei quiz");
        closedir(dir);
        return -1;
    }

    rewinddir(dir);

    char file_path[PATH_MAX];
    int i = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
            entry->d_type != DT_REG)
            continue;

        snprintf(file_path, sizeof(file_path), "%s/%s", directory_path,
                 entry->d_name);
        quizzesInfo->quizzes[i] = load_quiz_from_file(file_path);
        i++;
    }

    return 0;
}
