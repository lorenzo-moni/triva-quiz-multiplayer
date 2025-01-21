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
        if (strstr(line, "Domanda"))
            question_count++;
    }

    quiz->total_questions = question_count;

    // Alloca gli array per domande e risposte
    quiz->questions = (QuizQuestion **)malloc(question_count * sizeof(QuizQuestion *));

    if (!quiz->questions)
    {
        perror("Failed to allocate questions arrays");
        free(quiz);
        free(line);
        fclose(file);
        return NULL;
    }

    // Torna all'inizio del file per leggere le domande e risposte
    rewind(file);
    getline(&line, &len, file); // Salta il nome del quiz
    getline(&line, &len, file); // Salta la linea vuota successiva

    int question_idx = 0;
    QuizQuestion *current_question = NULL;
    int answer_idx = 0;
    char *token, *answers_line, *copy;

    while ((read = getline(&line, &len, file)) != -1)
    {
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "Domanda: ", 9) == 0)
        {
            current_question = (QuizQuestion *)malloc(sizeof(QuizQuestion));
            current_question->question = strdup(line + 9);
            current_question->total_answers = 0;
        }

        if (strncmp(line, "Risposte: ", 10) == 0)
        {
            if (current_question == NULL)
                continue;
            answers_line = line + 10;

            // creo una copia della linea con le risposte perché strtok modifica la riga di partenza
            copy = strdup(answers_line);

            // conto quante sono le risposte possibili
            token = strtok(copy, ",");
            while (token)
            {
                current_question->total_answers += 1;
                token = strtok(NULL, ",");
            }
            free(copy);
            // alloco il vettore che conterrà tutte le risposte
            current_question->answers = malloc(current_question->total_answers * sizeof(char *));
            // inserisco le risposte effettivamente nel array
            token = strtok(answers_line, ",");
            answer_idx = 0;
            while (token)
            {
                while (*token == ' ')
                    token += 1;
                current_question->answers[answer_idx] = strdup(token);
                answer_idx += 1;
                token = strtok(NULL, ",");
            }
            quiz->questions[question_idx++] = current_question;
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

    closedir(dir);
    return 0;
}

void deallocate_quizzes(QuizzesInfo *quizzesInfo)
{
    for (int i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        Quiz *quiz = quizzesInfo->quizzes[i];
        free(quiz->name);
        deallocate_ranking(quiz->ranking_head);
        for (int j = 0; j < quiz->total_questions; j++)
        {
            free(quiz->questions[j]->question);
            for (int k = 0; k < quiz->questions[j]->total_answers; k++)
                free(quiz->questions[j]->answers[k]);
        }
    }
}