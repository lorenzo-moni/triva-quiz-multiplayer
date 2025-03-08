#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "utils.h"

/**
 * @brief Counts the total number of files in a directory
 *
 * @param directory pointer to the directory object whose files are to be counted
 * @return number of regular files contained in the directory
 */
int get_directory_total_files(DIR *directory)
{
    // Check that the directory is valid
    if (!directory)
    {
        printf("The selected directory is not valid\n");
        return -1;
    }
    int file_count = 0;
    struct dirent *entry;
    // Iterate through the contents of the directory
    // In each iteration, readdir() returns a different file within the directory
    while ((entry = readdir(directory)) != NULL)
    {
        // Count only regular files
        if (entry->d_type != DT_REG)
            continue;
        file_count++;
    }
    // Reset the directory stream pointer to the beginning so that the directory entries
    // can be iterated again later using readdir()
    rewinddir(directory);
    return file_count;
}

/**
 * @brief Loads the quiz from a file and creates the corresponding Quiz structure
 *
 * This function creates the Quiz structure corresponding to the quiz contained in the file specified by file_path.
 * It decodes the file according to a predefined format and allocates and fills in all the fields of the Quiz structure.
 *
 * @param file_path pointer to the path of the file that contains the quiz information
 * @return Quiz structure corresponding to the file file_path
 */
Quiz *load_quiz_from_file(const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (!file)
    {
        printf("Error opening the file\n");
        exit(EXIT_FAILURE);
    }
    // Allocate the quiz
    Quiz *quiz = (Quiz *)malloc(sizeof(Quiz));
    handle_malloc_error(quiz, "Memory allocation error for the quiz");

    // Pointer to the line used by getline
    char *lineptr = NULL;
    // Current size of the buffer lineptr
    size_t len = 0;
    // Number of characters read, including \n but excluding \0
    ssize_t read;

    int question_idx = 0;
    QuizQuestion *current_question = NULL;
    int answer_idx = 0;
    char *token, *answers_line, *answer_line_copy;

    int question_count = 0;

    quiz->questions = NULL;
    quiz->ranking_head = NULL;
    quiz->ranking_tail = NULL;
    quiz->total_clients = 0;

    // Read the first line, which contains the name of the quiz
    if ((read = getline(&lineptr, &len, file)) != -1)
    {
        // Remove the newline character and replace it with a null terminator
        lineptr[strcspn(lineptr, "\n")] = '\0';
        // Duplicate the string (including the terminator) into quiz->name
        quiz->name = strdup(lineptr);
    }

    // Count the total number of questions by counting the lines that contain the word "Question"
    while ((read = getline(&lineptr, &len, file)) != -1)
    {
        // strstr searches for the first occurrence of a substring in a string
        if (strstr(lineptr, "Question"))
            question_count++;
    }

    quiz->total_questions = question_count;

    // Allocate arrays for questions and answers
    quiz->questions = (QuizQuestion **)malloc(question_count * sizeof(QuizQuestion *));
    handle_malloc_error(quiz->questions, "Memory allocation error for quiz questions");

    // Rewind the file to read the questions and answers
    rewind(file);
    // Skip the first line that contains the quiz name
    getline(&lineptr, &len, file);

    // Loop indefinitely through the file in order to read two lines at a time
    while (true)
    {
        // Read the empty line before the question
        read = getline(&lineptr, &len, file);

        // Read the line that should contain the question
        read = getline(&lineptr, &len, file);

        if (read == -1)
            // End of file reached
            break;

        // Replace the newline character with a null terminator
        lineptr[strcspn(lineptr, "\n")] = '\0';

        // If the line does not have the correct format
        if (strncmp(lineptr, "Question: ", 10) != 0)
        {
            printf("The quiz file %s is not formatted correctly, please refer to the documentation\n", quiz->name);
            exit(EXIT_FAILURE);
        }
        // Allocate the object for the current question
        current_question = (QuizQuestion *)malloc(sizeof(QuizQuestion));
        handle_malloc_error(current_question, "Memory allocation error for a quiz question");
        current_question->question = strdup(lineptr + 10);
        current_question->total_answers = 0;

        // Read the next line, which should contain the answers
        read = getline(&lineptr, &len, file);

        lineptr[strcspn(lineptr, "\n")] = '\0';

        if (read == -1 || strncmp(lineptr, "Answers: ", 9) != 0)
        {
            // In this case, there is a question without answers
            printf("The quiz file %s is not formatted correctly, please refer to the documentation\n", quiz->name);
            exit(EXIT_FAILURE);
        }
        // Skip the string "Answers: "
        answers_line = lineptr + 9;

        // Create a copy of the answers line because strtok modifies the original string
        answer_line_copy = strdup(answers_line);

        // Count the number of possible answers
        token = strtok(answer_line_copy, ",");
        while (token)
        {
            current_question->total_answers += 1;
            token = strtok(NULL, ",");
        }
        free(answer_line_copy);

        // Allocate the array that will contain all the answers
        current_question->answers = malloc(current_question->total_answers * sizeof(char *));
        if (!current_question->answers)
        {
            printf("Memory allocation error for the answers to a question\n");
            exit(EXIT_FAILURE);
        }

        // Insert the answers into the array
        token = strtok(answers_line, ",");
        answer_idx = 0;
        while (token)
        {
            // Skip any leading spaces
            while (*token == ' ')
                token += 1;
            current_question->answers[answer_idx] = strdup(token);
            answer_idx += 1;
            token = strtok(NULL, ",");
        }
        quiz->questions[question_idx++] = current_question;
    }

    free(lineptr);
    fclose(file);

    return quiz;
}

/**
 * @brief Loads the quizzes present in a directory
 *
 * This function loads all the quizzes from a specific directory by using
 * the load_quiz_from_file function on every file in the directory.
 * The quizzes are stored in the quizzesInfo object so that the entire application can use them.
 *
 * @param directory_path path of the directory from which to load the quizzes
 * @param quizzesInfo pointer to the structure in which to store the loaded quizzes
 * @return number of quizzes loaded
 */
int load_quizzes_from_directory(const char *directory_path, QuizzesInfo *quizzesInfo)
{
    // Create the directory structure and open the specified path
    struct dirent *entry;
    DIR *directory = opendir(directory_path);
    char file_path[PATH_MAX];
    int current_quiz = 0;

    if (directory == NULL)
    {
        printf("Error opening the specified directory\n");
        exit(EXIT_FAILURE);
    }

    // Count the total number of quizzes present in the directory
    quizzesInfo->total_quizzes = get_directory_total_files(directory);

    // Allocate an array of quiz pointers that will be populated by reading the files in the directory
    quizzesInfo->quizzes = (Quiz **)malloc((quizzesInfo->total_quizzes) * sizeof(Quiz *));
    if (!quizzesInfo->quizzes)
    {
        printf("Memory allocation error for the quiz array\n");
        exit(EXIT_FAILURE);
    }

    // Iterate through the files in the directory
    while ((entry = readdir(directory)) != NULL)
    {
        // Consider only regular files
        if (entry->d_type != DT_REG)
            continue;

        // Construct the complete file path from which to load the quiz
        snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);

        // Load the quiz from the file and insert it into the array
        quizzesInfo->quizzes[current_quiz] = load_quiz_from_file(file_path);
        current_quiz++;
    }

    // Close the directory
    closedir(directory);
    return quizzesInfo->total_quizzes;
}

/**
 * @brief Deallocates all the information related to the quizzes
 *
 * @param quizzesInfo pointer to the structure that contains all the quizzes
 */
void deallocate_quizzes(QuizzesInfo *quizzesInfo)
{
    for (uint16_t i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        Quiz *quiz = quizzesInfo->quizzes[i];
        if (!quiz)
            continue;
        free(quiz->name);

        // Deallocate the doubly linked list associated with the quiz ranking
        deallocate_rankings(quiz);

        for (uint16_t j = 0; j < quiz->total_questions; j++)
        {
            QuizQuestion *question = quiz->questions[j];
            if (!question)
                continue;

            free(question->question);

            for (int k = 0; k < question->total_answers; k++)
                free(question->answers[k]);

            free(question->answers);
            free(question);
        }
        free(quiz->questions);
        free(quiz);
    }
    free(quizzesInfo->quizzes);
}
